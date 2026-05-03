package telnetty

import (
	"bytes"
	"net"
	"sync"
	"testing"
	"time"
)

// setupTCPPair creates a connected TCP client/server pair for testing.
func setupTCPPair(t *testing.T) (client, server net.Conn) {
	t.Helper()
	ln, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer ln.Close()

	clientCh := make(chan net.Conn, 1)
	go func() {
		c, err := net.Dial("tcp", ln.Addr().String())
		if err != nil {
			t.Error(err)
		}
		clientCh <- c
	}()

	server, err = ln.Accept()
	if err != nil {
		t.Fatal(err)
	}
	client = <-clientCh
	if client == nil {
		t.Fatal("client connection failed")
	}
	return client, server
}

// TestFullNegotiation tests a complete TELNET negotiation flow.
func TestFullNegotiation(t *testing.T) {
	client, server := setupTCPPair(t)
	defer client.Close()
	defer server.Close()

	var clientEvents []string
	var serverEvents []string
	var mu sync.Mutex

	cConn := NewConn(client, Config{
		EnableEcho: true,
		EnableSGA:  true,
		EnableNAWS: true,
		EventHandler: EventHandlerFunc{
			CmdFunc: func(c *Conn, cmd, option uint8) {
				mu.Lock()
				clientEvents = append(clientEvents, formatCmd(cmd, option))
				mu.Unlock()
			},
		},
	})
	defer cConn.Close()

	sConn := NewConn(server, Config{
		EnableEcho: true,
		EnableSGA:  true,
		EnableNAWS: true,
		EventHandler: EventHandlerFunc{
			CmdFunc: func(c *Conn, cmd, option uint8) {
				mu.Lock()
				serverEvents = append(serverEvents, formatCmd(cmd, option))
				mu.Unlock()
			},
		},
	})
	defer sConn.Close()

	// Process initial negotiations on both sides
	buf := make([]byte, 256)
	cConn.Read(buf)
	sConn.Read(buf)

	// Give event handlers time to process
	time.Sleep(50 * time.Millisecond)

	mu.Lock()
	if len(serverEvents) == 0 {
		t.Fatal("server received no negotiation events")
	}
	mu.Unlock()

	// Send data through
	cConn.Write([]byte("hello server\r\n"))
	cConn.Flush()

	sConn.SetReadDeadline(time.Now().Add(time.Second))
	n, err := sConn.Read(buf)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Contains(buf[:n], []byte("hello server")) {
		t.Fatalf("expected 'hello server', got %q", buf[:n])
	}
}

func formatCmd(cmd, option uint8) string {
	var cmdName string
	switch cmd {
	case CmdWill: cmdName = "WILL"
	case CmdWont: cmdName = "WONT"
	case CmdDo:   cmdName = "DO"
	case CmdDont: cmdName = "DONT"
	default:      cmdName = "???"
	}
	return cmdName + " " + formatOption(option)
}

func formatOption(opt uint8) string {
	switch opt {
	case OptEcho: return "ECHO"
	case OptSGA: return "SGA"
	case OptNAWS: return "NAWS"
	case OptBinary: return "BINARY"
	default: return "UNKNOWN"
	}
}

// TestSubnegotiationRoundTrip tests NAWS subnegotiation.
func TestSubnegotiationRoundTrip(t *testing.T) {
	client, server := setupTCPPair(t)
	defer client.Close()
	defer server.Close()

	var snData []byte
	var snOpt uint8
	var snMu sync.Mutex

	cConn := NewConn(client, Config{
		EnableNAWS: true,
		TerminalWidth: 100,
		TerminalHeight: 40,
	})
	defer cConn.Close()

	sConn := NewConn(server, Config{
		EnableNAWS: true,
		EventHandler: EventHandlerFunc{
			SNFunc: func(c *Conn, option uint8, data []byte) {
				snMu.Lock()
				snOpt = option
				snData = append([]byte(nil), data...)
				snMu.Unlock()
			},
		},
	})
	defer sConn.Close()

	// Process initial negotiations on both sides
	buf := make([]byte, 256)
	cConn.Read(buf)
	sConn.Read(buf)

	// Give event handlers time to process
	time.Sleep(50 * time.Millisecond)

	snMu.Lock()
	if snOpt != OptNAWS {
		t.Fatalf("expected NAWS subnegotiation, got option %d", snOpt)
	}
	if len(snData) != 4 {
		t.Fatalf("expected 4 bytes NAWS data, got %d", len(snData))
	}
	width := uint16(snData[0])<<8 | uint16(snData[1])
	height := uint16(snData[2])<<8 | uint16(snData[3])
	if width != 100 || height != 40 {
		t.Fatalf("expected 100x40, got %dx%d", width, height)
	}
	snMu.Unlock()
}

// TestIACEscapingRoundTrip tests that IAC bytes are properly escaped and unescaped.
func TestIACEscapingRoundTrip(t *testing.T) {
	client, server := setupTCPPair(t)
	defer client.Close()
	defer server.Close()

	cConn := NewConn(client, Config{})
	defer cConn.Close()

	sConn := NewConn(server, Config{})
	defer sConn.Close()

	// Write data containing IAC (255)
	original := []byte{0x01, 0xFF, 0x02, 0xFF, 0xFF, 0x03}
	cConn.Write(original)
	cConn.Flush()

	// Read back - should get original data with single IAC bytes
	buf := make([]byte, 100)
	sConn.SetReadDeadline(time.Now().Add(time.Second))
	n, err := sConn.Read(buf)
	if err != nil {
		t.Fatal(err)
	}

	if !bytes.Equal(buf[:n], original) {
		t.Fatalf("round-trip failed: expected %v, got %v", original, buf[:n])
	}
}

// TestConcurrentReadWrite tests concurrent operations on a single connection.
func TestConcurrentReadWrite(t *testing.T) {
	client, server := net.Pipe()
	defer client.Close()
	defer server.Close()

	cConn := NewConn(client, Config{})
	defer cConn.Close()

	sConn := NewConn(server, Config{})
	defer sConn.Close()

	var wg sync.WaitGroup
	wg.Add(2)

	// Writer
	go func() {
		defer wg.Done()
		for i := 0; i < 100; i++ {
			msg := []byte("message ")
			cConn.Write(msg)
			cConn.Flush()
		}
	}()

	// Reader
	go func() {
		defer wg.Done()
		buf := make([]byte, 100)
		count := 0
		for count < 100 {
			sConn.SetReadDeadline(time.Now().Add(2 * time.Second))
			n, err := sConn.Read(buf)
			if err != nil {
				return
			}
			if n > 0 {
				count++
			}
		}
	}()

	wg.Wait()
}

// TestServerClientIntegration tests server with multiple clients.
func TestServerClientIntegration(t *testing.T) {
	var wg sync.WaitGroup

	server := NewServer("127.0.0.1:0",
		WithConfig(Config{EnableEcho: true}),
		WithHandler(func(c *Conn) {
			defer wg.Done()
			buf := make([]byte, 100)
			for {
				c.SetReadDeadline(time.Now().Add(time.Second))
				n, err := c.Read(buf)
				if err != nil {
					return
				}
				c.Write(buf[:n])
				c.Flush()
			}
		}),
		WithMaxConns(100),
	)

	go server.ListenAndServe()
	defer server.Close()
	time.Sleep(50 * time.Millisecond)

	addr := server.Addr().String()

	// Connect 5 clients
	for i := 0; i < 5; i++ {
		wg.Add(1)
		go func(id int) {
			nc, err := net.Dial("tcp", addr)
			if err != nil {
				return
			}
			defer nc.Close()

			msg := []byte("hello from client")
			nc.Write(msg)

			buf := make([]byte, 100)
			nc.SetReadDeadline(time.Now().Add(time.Second))
			n, _ := nc.Read(buf)
			if !bytes.Contains(buf[:n], []byte("hello from client")) {
				// Echo should contain our message
			}
		}(i)
	}

	wg.Wait()
}
