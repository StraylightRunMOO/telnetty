package telnetty

import (
	"bufio"
	"errors"
	"io"
	"net"
	"sync"
	"sync/atomic"
	"time"
)

var (
	ErrConnClosed    = errors.New("telnetty: connection closed")
	ErrBufferFull    = errors.New("telnetty: output buffer full")
	ErrLineTooLong   = errors.New("telnetty: line too long")
)

// EventHandler is called when protocol events occur.
type EventHandler interface {
	OnData(c *Conn, data []byte)
	OnCommand(c *Conn, cmd, option uint8)
	OnSubnegotiation(c *Conn, option uint8, data []byte)
	OnError(c *Conn, err error)
}

// EventHandlerFunc is an adapter for using functions as EventHandler.
type EventHandlerFunc struct {
	DataFunc func(c *Conn, data []byte)
	CmdFunc  func(c *Conn, cmd, option uint8)
	SNFunc   func(c *Conn, option uint8, data []byte)
	ErrFunc  func(c *Conn, err error)
}

func (f EventHandlerFunc) OnData(c *Conn, data []byte)       { if f.DataFunc != nil { f.DataFunc(c, data) } }
func (f EventHandlerFunc) OnCommand(c *Conn, cmd, option uint8) { if f.CmdFunc != nil { f.CmdFunc(c, cmd, option) } }
func (f EventHandlerFunc) OnSubnegotiation(c *Conn, option uint8, data []byte) { if f.SNFunc != nil { f.SNFunc(c, option, data) } }
func (f EventHandlerFunc) OnError(c *Conn, err error)         { if f.ErrFunc != nil { f.ErrFunc(c, err) } }

// Config controls Conn behavior.
type Config struct {
	EnableBinary   bool
	EnableEcho     bool
	EnableSGA      bool
	EnableNAWS     bool
	EnableTType    bool
	EnableTermSpeed bool
	EnableNewEnviron bool
	EnableCharset  bool
	EnableMCCP2    bool
	EnableMCCP3    bool
	EnableMSDP     bool
	EnableGMCP     bool
	EnableMSSP     bool
	EnableMCP      bool

	TerminalType   string
	TerminalWidth  uint16
	TerminalHeight uint16

	ReadBufferSize  int
	WriteBufferSize int

	EventHandler EventHandler
	LineMode bool
}

// Stats holds connection metrics.
type Stats struct {
	BytesRead       uint64
	BytesWritten    uint64
	CommandsRecv    uint64
	CommandsSent    uint64
	OptionsNegotiated uint64
	Subnegotiations uint64
	Errors          uint64
}

// Conn wraps a net.Conn with TELNET protocol processing.
type Conn struct {
	netConn net.Conn
	config  Config

	parser     *Parser
	parseResult ParseResult
	negotiator *Negotiator

	readBuf     *Ring
	readMu      sync.Mutex
	readScratch []byte

	writeBuf   StaticBuffer
	writeMu    sync.Mutex
	writeScratch [512]byte

	events     chan event
	eventDone  chan struct{}
	eventWg    sync.WaitGroup

	closed     uint32
	lineMode   uint32
	readLineCR bool // true if previous ReadLine ended with \r

	termType   atomic.Value
	termWidth  uint32
	termHeight uint32

	stats Stats
}

type event struct {
	typ    uint8
	cmd    uint8
	option uint8
	data   []byte
}

const (
	eventData = iota
	eventCommand
	eventSubnegotiation
	eventError
)

// NewConn wraps a net.Conn with TELNET protocol handling.
func NewConn(nc net.Conn, cfg Config) *Conn {
	if cfg.ReadBufferSize <= 0 {
		cfg.ReadBufferSize = 64 * 1024
	}
	if cfg.WriteBufferSize <= 0 {
		cfg.WriteBufferSize = 64 * 1024
	}

	c := &Conn{
		netConn:     nc,
		config:      cfg,
		parser:      NewParser(),
		negotiator:  NewNegotiator(),
		readBuf:     NewRing(cfg.ReadBufferSize),
		readScratch: getBuf(cfg.ReadBufferSize),
	}
	c.writeBuf.buf = getBuf(cfg.WriteBufferSize)
	c.writeBuf.buf = c.writeBuf.buf[:0]

	if cfg.EventHandler != nil {
		c.events = make(chan event, 256)
		c.eventDone = make(chan struct{})
		c.eventWg.Add(1)
		go c.eventDispatcher()
	}

	c.negotiateInitial()
	return c
}

func (c *Conn) negotiateInitial() {
	if c.config.EnableBinary   { c.SendCommand(CmdWill, OptBinary) }
	if c.config.EnableSGA      { c.SendCommand(CmdWill, OptSGA) }
	if c.config.EnableEcho     { c.SendCommand(CmdWill, OptEcho) }
	if c.config.EnableNAWS     { c.SendCommand(CmdWill, OptNAWS); c.SendNAWS(c.config.TerminalWidth, c.config.TerminalHeight) }
	if c.config.EnableTType    { c.SendCommand(CmdWill, OptTerminalType) }
	if c.config.EnableTermSpeed { c.SendCommand(CmdWill, OptTermSpeed) }
	if c.config.EnableNewEnviron { c.SendCommand(CmdWill, OptNewEnviron) }
	if c.config.EnableCharset  { c.SendCommand(CmdWill, OptCharset) }
	if c.config.EnableMCCP2    { c.SendCommand(CmdWill, OptMCCP2) }
	if c.config.EnableMCCP3    { c.SendCommand(CmdWill, OptMCCP3) }
	if c.config.EnableMSDP     { c.SendCommand(CmdWill, OptMSDP) }
	if c.config.EnableGMCP     { c.SendCommand(CmdWill, OptGMCP) }
	if c.config.EnableMSSP     { c.SendCommand(CmdWill, OptMSSP) }
	if c.config.EnableMCP      { c.SendCommand(CmdWill, OptMCP) }
	c.Flush()
}

func (c *Conn) eventDispatcher() {
	defer c.eventWg.Done()
	for {
		select {
		case e := <-c.events:
			switch e.typ {
			case eventData:
				c.config.EventHandler.OnData(c, e.data)
			case eventCommand:
				c.config.EventHandler.OnCommand(c, e.cmd, e.option)
			case eventSubnegotiation:
				c.config.EventHandler.OnSubnegotiation(c, e.option, e.data)
			case eventError:
				c.config.EventHandler.OnError(c, errors.New(string(e.data)))
			}
		case <-c.eventDone:
			return
		}
	}
}

func (c *Conn) pushEvent(e event) {
	if atomic.LoadUint32(&c.closed) != 0 {
		return
	}
	if c.events == nil {
		return
	}
	select {
	case c.events <- e:
	default:
		select {
		case <-c.events:
			c.events <- e
		default:
		}
	}
}

// Read implements io.Reader.
func (c *Conn) Read(p []byte) (n int, err error) {
	if atomic.LoadUint32(&c.closed) != 0 {
		return 0, ErrConnClosed
	}

	c.readMu.Lock()
	defer c.readMu.Unlock()

	return c.readLocked(p)
}

// readLocked reads into p without taking the lock (caller must hold readMu).
func (c *Conn) readLocked(p []byte) (n int, err error) {
	if c.readBuf.Len() > 0 {
		return c.readBuf.Read(p)
	}
	for {
		nr, err := c.netConn.Read(c.readScratch)
		if nr > 0 {
			atomic.AddUint64(&c.stats.BytesRead, uint64(nr))
			c.processReadLocked(c.readScratch[:nr])
		}
		if err != nil {
			if c.readBuf.Len() > 0 {
				return c.readBuf.Read(p)
			}
			if errors.Is(err, io.EOF) {
				return 0, io.EOF
			}
			return 0, err
		}
		if nr == 0 {
			return 0, nil
		}
		if c.readBuf.Len() > 0 {
			return c.readBuf.Read(p)
		}
		return 0, nil
	}
}

func (c *Conn) isOptionSupported(option uint8) bool {
	switch option {
	case OptBinary, OptEcho, OptSGA, OptNAWS, OptTerminalType, OptTermSpeed,
		OptNewEnviron, OptCharset, OptMCCP2, OptMCCP3, OptMSDP, OptGMCP,
		OptMSSP, OptMCP:
		return true
	default:
		return false
	}
}

func (c *Conn) isOptionEnabledLocally(option uint8) bool {
	switch option {
	case OptBinary:
		return c.config.EnableBinary
	case OptEcho:
		return c.config.EnableEcho
	case OptSGA:
		return c.config.EnableSGA
	case OptNAWS:
		return c.config.EnableNAWS
	case OptTerminalType:
		return c.config.EnableTType
	case OptTermSpeed:
		return c.config.EnableTermSpeed
	case OptNewEnviron:
		return c.config.EnableNewEnviron
	case OptCharset:
		return c.config.EnableCharset
	case OptMCCP2:
		return c.config.EnableMCCP2
	case OptMCCP3:
		return c.config.EnableMCCP3
	case OptMSDP:
		return c.config.EnableMSDP
	case OptGMCP:
		return c.config.EnableGMCP
	case OptMSSP:
		return c.config.EnableMSSP
	case OptMCP:
		return c.config.EnableMCP
	default:
		return false
	}
}

func (c *Conn) processReadLocked(data []byte) {
	c.parser.Parse(data, &c.parseResult)

	for i := 0; i < c.parseResult.DataCount; i++ {
		ev := c.parseResult.DataEvents[i]
		if ev.Length > 0 {
			c.readBuf.Write(ev.Data)
		}
	}

	for i := 0; i < c.parseResult.CmdCount; i++ {
		cmd := c.parseResult.CommandEvents[i]

		// Refuse options the client asks us to enable that we don't support/want.
		if cmd.Command == CmdDo && !c.isOptionEnabledLocally(cmd.Option) {
			c.SendCommand(CmdWont, cmd.Option)
			c.pushEvent(event{typ: eventCommand, cmd: cmd.Command, option: cmd.Option})
			continue
		}

		// Refuse options the client offers that we don't know about.
		if cmd.Command == CmdWill && !c.isOptionSupported(cmd.Option) {
			c.SendCommand(CmdDont, cmd.Option)
			c.pushEvent(event{typ: eventCommand, cmd: cmd.Command, option: cmd.Option})
			continue
		}

		resp, changed := c.negotiator.Handle(cmd.Command, cmd.Option)
		if resp != 0 {
			c.SendCommand(resp, cmd.Option)
		}
		if changed {
			atomic.AddUint64(&c.stats.OptionsNegotiated, 1)
		}
		c.pushEvent(event{typ: eventCommand, cmd: cmd.Command, option: cmd.Option})
	}

	for i := 0; i < c.parseResult.SNCount; i++ {
		sn := c.parseResult.SNEvents[i]
		atomic.AddUint64(&c.stats.Subnegotiations, 1)
		c.handleSubnegotiation(sn.Option, sn.Data)
		c.pushEvent(event{typ: eventSubnegotiation, option: sn.Option, data: sn.Data})
	}
}

func (c *Conn) handleSubnegotiation(option uint8, data []byte) {
	switch option {
	case OptTerminalType:
		if len(data) > 1 && data[0] == 0 {
			c.termType.Store(string(data[1:]))
		}
	case OptNAWS:
		if len(data) >= 4 {
			w := uint16(data[0])<<8 | uint16(data[1])
			h := uint16(data[2])<<8 | uint16(data[3])
			atomic.StoreUint32(&c.termWidth, uint32(w))
			atomic.StoreUint32(&c.termHeight, uint32(h))
		}
	}
}

// Write implements io.Writer.
func (c *Conn) Write(p []byte) (n int, err error) {
	if atomic.LoadUint32(&c.closed) != 0 {
		return 0, ErrConnClosed
	}

	c.writeMu.Lock()
	defer c.writeMu.Unlock()

	needsEscape := false
	for _, b := range p {
		if b == CmdIAC {
			needsEscape = true
			break
		}
	}

	if !needsEscape {
		if len(c.writeBuf.buf)+len(p) <= cap(c.writeBuf.buf) {
			c.writeBuf.buf = append(c.writeBuf.buf, p...)
			return len(p), nil
		}
	}

	for _, b := range p {
		if len(c.writeBuf.buf) >= cap(c.writeBuf.buf) {
			if err := c.flushLocked(); err != nil {
				return n, err
			}
		}
		c.writeBuf.buf = append(c.writeBuf.buf, b)
		if b == CmdIAC {
			c.writeBuf.buf = append(c.writeBuf.buf, CmdIAC)
		}
		n++
	}
	return n, nil
}

func (c *Conn) WriteByte(b byte) error {
	_, err := c.Write([]byte{b})
	return err
}

func (c *Conn) WriteString(s string) (int, error) {
	return c.Write([]byte(s))
}

func (c *Conn) SendCommand(cmd, option uint8) error {
	if atomic.LoadUint32(&c.closed) != 0 {
		return ErrConnClosed
	}
	c.writeMu.Lock()
	defer c.writeMu.Unlock()
	if len(c.writeBuf.buf)+3 > cap(c.writeBuf.buf) {
		if err := c.flushLocked(); err != nil {
			return err
		}
	}
	c.writeBuf.buf = append(c.writeBuf.buf, CmdIAC, cmd, option)
	atomic.AddUint64(&c.stats.CommandsSent, 1)
	return nil
}

func (c *Conn) SendSubnegotiation(option uint8, data []byte) error {
	if atomic.LoadUint32(&c.closed) != 0 {
		return ErrConnClosed
	}
	c.writeMu.Lock()
	defer c.writeMu.Unlock()

	required := 3 + len(data) + 2
	for _, b := range data {
		if b == CmdIAC {
			required++
		}
	}
	if len(c.writeBuf.buf)+required > cap(c.writeBuf.buf) {
		if err := c.flushLocked(); err != nil {
			return err
		}
	}

	c.writeBuf.buf = append(c.writeBuf.buf, CmdIAC, CmdSB, option)
	for _, b := range data {
		c.writeBuf.buf = append(c.writeBuf.buf, b)
		if b == CmdIAC {
			c.writeBuf.buf = append(c.writeBuf.buf, CmdIAC)
		}
	}
	c.writeBuf.buf = append(c.writeBuf.buf, CmdIAC, CmdSE)
	return nil
}

func (c *Conn) SendNAWS(width, height uint16) error {
	data := []byte{byte(width >> 8), byte(width), byte(height >> 8), byte(height)}
	return c.SendSubnegotiation(OptNAWS, data)
}

func (c *Conn) SendTType(termType string) error {
	data := append([]byte{0}, termType...)
	return c.SendSubnegotiation(OptTerminalType, data)
}

func (c *Conn) Flush() error {
	c.writeMu.Lock()
	defer c.writeMu.Unlock()
	return c.flushLocked()
}

func (c *Conn) flushLocked() error {
	if len(c.writeBuf.buf) == 0 {
		return nil
	}
	n, err := c.netConn.Write(c.writeBuf.buf)
	if n > 0 {
		atomic.AddUint64(&c.stats.BytesWritten, uint64(n))
	}
	if err != nil {
		return err
	}
	c.writeBuf.buf = c.writeBuf.buf[:0]
	return nil
}

// ReadLine reads a line up to \r\n or \n. NOT safe for concurrent use with Read.
func (c *Conn) ReadLine() ([]byte, error) {
	var line []byte
	for {
		var buf [1024]byte
		n, err := c.readLocked(buf[:])
		if n > 0 {
			for i := 0; i < n; i++ {
				if buf[i] == '\n' {
					end := i
					if end > 0 && buf[end-1] == '\r' {
						end--
					} else if end == 0 && c.readLineCR {
						// \r\n split across reads: drop trailing \r from previous line segment
						if len(line) > 0 {
							line = line[:len(line)-1]
						}
					}
					line = append(line, buf[:end]...)
					if i+1 < n {
						c.readBuf.Write(buf[i+1 : n])
					}
					c.readLineCR = false
					return line, nil
				}
			}
			line = append(line, buf[:n]...)
			// Remember if the buffer ended with \r for next iteration
			c.readLineCR = n > 0 && buf[n-1] == '\r'
		} else {
			c.readLineCR = false
		}
		if err != nil {
			return line, err
		}
	}
}

func (c *Conn) BufferedReader() *bufio.Reader {
	return bufio.NewReader(c)
}

func (c *Conn) TerminalType() string {
	v := c.termType.Load()
	if v == nil {
		return ""
	}
	return v.(string)
}

func (c *Conn) TerminalSize() (width, height uint16) {
	return uint16(atomic.LoadUint32(&c.termWidth)),
		uint16(atomic.LoadUint32(&c.termHeight))
}

func (c *Conn) Stats() Stats {
	return Stats{
		BytesRead:         atomic.LoadUint64(&c.stats.BytesRead),
		BytesWritten:      atomic.LoadUint64(&c.stats.BytesWritten),
		CommandsRecv:      atomic.LoadUint64(&c.stats.CommandsRecv),
		CommandsSent:      atomic.LoadUint64(&c.stats.CommandsSent),
		OptionsNegotiated: atomic.LoadUint64(&c.stats.OptionsNegotiated),
		Subnegotiations:   atomic.LoadUint64(&c.stats.Subnegotiations),
		Errors:            atomic.LoadUint64(&c.stats.Errors),
	}
}

func (c *Conn) LocalAddr() net.Addr  { return c.netConn.LocalAddr() }
func (c *Conn) RemoteAddr() net.Addr { return c.netConn.RemoteAddr() }
func (c *Conn) SetDeadline(t time.Time) error      { return c.netConn.SetDeadline(t) }
func (c *Conn) SetReadDeadline(t time.Time) error  { return c.netConn.SetReadDeadline(t) }
func (c *Conn) SetWriteDeadline(t time.Time) error { return c.netConn.SetWriteDeadline(t) }
func (c *Conn) Underlying() net.Conn { return c.netConn }

func (c *Conn) Close() error {
	if !atomic.CompareAndSwapUint32(&c.closed, 0, 1) {
		return ErrConnClosed
	}
	c.Flush()
	if c.eventDone != nil {
		close(c.eventDone)
		c.eventWg.Wait()
	}
	// Synchronize with in-progress Read/Write before freeing buffers
	c.readMu.Lock()
	c.readBuf.Free()
	c.readMu.Unlock()
	c.writeMu.Lock()
	c.writeBuf.Free()
	c.writeMu.Unlock()
	putBuf(c.readScratch)
	return c.netConn.Close()
}
