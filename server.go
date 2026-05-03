package telnetty

import (
	"context"
	"errors"
	"net"
	"runtime"
	"sync"
	"sync/atomic"
	"time"
)

var (
	ErrServerClosed = errors.New("telnetty: server closed")
	ErrNilHandler   = errors.New("telnetty: nil connection handler")
)

// ConnHandler is called for each new TELNET connection.
type ConnHandler func(*Conn)

// Server is a high-performance TELNET server with configurable options.
type Server struct {
	addr     string
	config   Config
	handler  ConnHandler
	listener atomic.Value // stores net.Listener

	// State
	closed    uint32 // atomic
	closing   chan struct{}
	closeDone chan struct{}

	// Connection management
	conns     map[*Conn]struct{}
	connMu    sync.RWMutex

	// Stats
	connsAccepted uint64
	connsClosed   uint64
	connsActive   uint32 // atomic

	// Accept tuning
	acceptLimit chan struct{} // semaphore for concurrent accepts
}

// ServerOption configures a Server.
type ServerOption func(*Server)

// WithConfig sets the TELNET configuration for all connections.
func WithConfig(cfg Config) ServerOption {
	return func(s *Server) { s.config = cfg }
}

// WithHandler sets the connection handler.
func WithHandler(h ConnHandler) ServerOption {
	return func(s *Server) { s.handler = h }
}

// WithMaxConns sets the maximum concurrent connections.
func WithMaxConns(n int) ServerOption {
	return func(s *Server) {
		s.acceptLimit = make(chan struct{}, n)
	}
}

// NewServer creates a TELNET server.
func NewServer(addr string, opts ...ServerOption) *Server {
	s := &Server{
		addr:      addr,
		closing:   make(chan struct{}),
		closeDone: make(chan struct{}),
		conns:     make(map[*Conn]struct{}),
	}
	for _, opt := range opts {
		opt(s)
	}
	if s.acceptLimit == nil {
		s.acceptLimit = make(chan struct{}, 10000)
	}
	return s
}

// ListenAndServe starts the server and blocks until closed.
func (s *Server) ListenAndServe() error {
	if s.handler == nil {
		return ErrNilHandler
	}

	ln, err := net.Listen("tcp", s.addr)
	if err != nil {
		return err
	}
	s.listener.Store(ln)

	// Start accept loop
	for {
		select {
		case <-s.closing:
			close(s.closeDone)
			return ErrServerClosed
		default:
		}

		// Rate limit accepts
		select {
		case s.acceptLimit <- struct{}{}:
		case <-s.closing:
			close(s.closeDone)
			return ErrServerClosed
		}

		conn, err := ln.Accept()
		if err != nil {
			<-s.acceptLimit
			if atomic.LoadUint32(&s.closed) != 0 {
				close(s.closeDone)
				return ErrServerClosed
			}
			continue
		}

		atomic.AddUint64(&s.connsAccepted, 1)
		atomic.AddUint32(&s.connsActive, 1)

		go s.handleConn(conn)
	}
}

// Serve accepts connections from an existing listener.
func (s *Server) Serve(ln net.Listener) error {
	s.listener.Store(ln)
	for {
		select {
		case <-s.closing:
			close(s.closeDone)
			return ErrServerClosed
		default:
		}

		select {
		case s.acceptLimit <- struct{}{}:
		case <-s.closing:
			close(s.closeDone)
			return ErrServerClosed
		}

		conn, err := ln.Accept()
		if err != nil {
			<-s.acceptLimit
			if atomic.LoadUint32(&s.closed) != 0 {
				close(s.closeDone)
				return ErrServerClosed
			}
			continue
		}

		atomic.AddUint64(&s.connsAccepted, 1)
		atomic.AddUint32(&s.connsActive, 1)

		go s.handleConn(conn)
	}
}

func (s *Server) handleConn(nc net.Conn) {
	defer func() {
		<-s.acceptLimit
		atomic.AddUint32(&s.connsActive, ^uint32(0))
		atomic.AddUint64(&s.connsClosed, 1)
	}()

	// Set TCP options for low latency
	if tcp, ok := nc.(*net.TCPConn); ok {
		tcp.SetNoDelay(true)
		tcp.SetKeepAlive(true)
		tcp.SetKeepAlivePeriod(3 * time.Minute)
	}

	c := NewConn(nc, s.config)

	s.connMu.Lock()
	s.conns[c] = struct{}{}
	s.connMu.Unlock()

	defer func() {
		s.connMu.Lock()
		delete(s.conns, c)
		s.connMu.Unlock()
		c.Close()
	}()

	s.handler(c)
}

// Close shuts down the server and all active connections.
func (s *Server) Close() error {
	if !atomic.CompareAndSwapUint32(&s.closed, 0, 1) {
		return ErrServerClosed
	}
	close(s.closing)
	if l := s.listener.Load(); l != nil {
		l.(net.Listener).Close()
	}

	// Close all active connections
	s.connMu.RLock()
	conns := make([]*Conn, 0, len(s.conns))
	for c := range s.conns {
		conns = append(conns, c)
	}
	s.connMu.RUnlock()
	for _, c := range conns {
		c.Close()
	}

	// Wait for close to complete
	<-s.closeDone
	return nil
}

// Shutdown gracefully shuts down the server with a timeout.
func (s *Server) Shutdown(ctx context.Context) error {
	if !atomic.CompareAndSwapUint32(&s.closed, 0, 1) {
		return ErrServerClosed
	}
	close(s.closing)
	if l := s.listener.Load(); l != nil {
		l.(net.Listener).Close()
	}

	// Close all active connections
	s.connMu.RLock()
	conns := make([]*Conn, 0, len(s.conns))
	for c := range s.conns {
		conns = append(conns, c)
	}
	s.connMu.RUnlock()
	for _, c := range conns {
		c.Close()
	}

	// Wait for context or close done
	select {
	case <-ctx.Done():
		return ctx.Err()
	case <-s.closeDone:
		return nil
	}
}

// Stats returns server statistics.
func (s *Server) Stats() (accepted, closed uint64, active uint32) {
	return atomic.LoadUint64(&s.connsAccepted),
		atomic.LoadUint64(&s.connsClosed),
		atomic.LoadUint32(&s.connsActive)
}

// Addr returns the server's listen address.
func (s *Server) Addr() net.Addr {
	if l := s.listener.Load(); l != nil {
		return l.(net.Listener).Addr()
	}
	return nil
}

// NumCPU returns the number of logical CPUs, useful for tuning.
func NumCPU() int {
	return runtime.NumCPU()
}
