package telnetty

import (
	"bytes"
	"net"
	"sync"
	"testing"
	"time"
)

// mockConn implements net.Conn for testing.
type mockConn struct {
	readBuf  *bytes.Buffer
	writeBuf *bytes.Buffer
	closed   bool
	mu       sync.Mutex
}

func newMockConn() *mockConn {
	return &mockConn{
		readBuf:  new(bytes.Buffer),
		writeBuf: new(bytes.Buffer),
	}
}

func (m *mockConn) Read(p []byte) (n int, err error) {
	m.mu.Lock()
	defer m.mu.Unlock()
	return m.readBuf.Read(p)
}

func (m *mockConn) Write(p []byte) (n int, err error) {
	m.mu.Lock()
	defer m.mu.Unlock()
	return m.writeBuf.Write(p)
}

func (m *mockConn) Close() error {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.closed = true
	return nil
}

func (m *mockConn) LocalAddr() net.Addr  { return nil }
func (m *mockConn) RemoteAddr() net.Addr { return nil }
func (m *mockConn) SetDeadline(t time.Time) error      { return nil }
func (m *mockConn) SetReadDeadline(t time.Time) error    { return nil }
func (m *mockConn) SetWriteDeadline(t time.Time) error   { return nil }

func (m *mockConn) inject(data []byte) {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.readBuf.Write(data)
}

func (m *mockConn) written() []byte {
	m.mu.Lock()
	defer m.mu.Unlock()
	return m.writeBuf.Bytes()
}

func TestConnBasicReadWrite(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()

	// Inject plain data
	mock.inject([]byte("hello world"))

	buf := make([]byte, 100)
	n, err := c.Read(buf)
	if err != nil {
		t.Fatal(err)
	}
	if string(buf[:n]) != "hello world" {
		t.Fatalf("expected 'hello world', got %q", buf[:n])
	}
}

func TestConnIACEscaping(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()

	// Write data containing IAC
	c.Write([]byte{0xFF, 0x01, 0x02})
	c.Flush()

	written := mock.written()
	// IAC should be doubled
	if len(written) != 4 {
		t.Fatalf("expected 4 bytes (with escape), got %d", len(written))
	}
	if written[0] != CmdIAC || written[1] != CmdIAC {
		t.Fatalf("expected IAC IAC escape, got %v", written[:2])
	}
}

func TestConnCommandHandling(t *testing.T) {
	mock := newMockConn()
	var commands []CommandEvent
	var cmdMu sync.Mutex
	c := NewConn(mock, Config{
		EventHandler: EventHandlerFunc{
			CmdFunc: func(c *Conn, cmd, option uint8) {
				cmdMu.Lock()
				commands = append(commands, CommandEvent{Command: cmd, Option: option})
				cmdMu.Unlock()
			},
		},
	})
	defer c.Close()

	// Inject IAC WILL ECHO
	mock.inject([]byte{CmdIAC, CmdWill, OptEcho})

	// Read to trigger processing
	buf := make([]byte, 100)
	c.Read(buf)
	c.Flush() // flush any negotiation responses

	// Give event dispatcher time
	time.Sleep(50 * time.Millisecond)

	cmdMu.Lock()
	if len(commands) != 1 {
		cmdMu.Unlock()
		t.Fatalf("expected 1 command event, got %d", len(commands))
	}
	if commands[0].Command != CmdWill || commands[0].Option != OptEcho {
		cmdMu.Unlock()
		t.Fatalf("unexpected command: %+v", commands[0])
	}
	cmdMu.Unlock()

	// We should have responded with DO ECHO
	written := mock.written()
	if len(written) < 3 {
		t.Fatal("expected negotiation response")
	}
	// Skip initial negotiations and find DO ECHO
	found := false
	for i := 0; i < len(written)-2; i++ {
		if written[i] == CmdIAC && written[i+1] == CmdDo && written[i+2] == OptEcho {
			found = true
			break
		}
	}
	if !found {
		t.Fatalf("expected DO ECHO in output, got %v", written)
	}
}

func TestConnReadLine(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()

	mock.inject([]byte("hello world\r\n"))

	line, err := c.ReadLine()
	if err != nil {
		t.Fatal(err)
	}
	if string(line) != "hello world" {
		t.Fatalf("expected 'hello world', got %q", line)
	}
}

func TestConnReadLineLFOnly(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()

	mock.inject([]byte("hello world\n"))

	line, err := c.ReadLine()
	if err != nil {
		t.Fatal(err)
	}
	if string(line) != "hello world" {
		t.Fatalf("expected 'hello world', got %q", line)
	}
}

func TestConnSendCommand(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()

	c.SendCommand(CmdWill, OptSGA)
	c.Flush()

	written := mock.written()
	// Skip initial negotiations
	found := false
	for i := 0; i < len(written)-2; i++ {
		if written[i] == CmdIAC && written[i+1] == CmdWill && written[i+2] == OptSGA {
			found = true
			break
		}
	}
	if !found {
		t.Fatalf("expected WILL SGA in output")
	}
}

func TestConnSendSubnegotiation(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()

	c.SendSubnegotiation(OptNAWS, []byte{0, 80, 0, 24})
	c.Flush()

	written := mock.written()
	// Skip initial negotiations and find NAWS subnegotiation
	found := false
	for i := 0; i < len(written)-4; i++ {
		if written[i] == CmdIAC && written[i+1] == CmdSB && written[i+2] == OptNAWS {
			// Check for IAC SE at end
			for j := i + 3; j < len(written)-1; j++ {
				if written[j] == CmdIAC && written[j+1] == CmdSE {
					found = true
					break
				}
			}
		}
	}
	if !found {
		t.Fatalf("expected NAWS subnegotiation in output")
	}
}

func TestConnTerminalType(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{EnableTType: true})
	defer c.Close()

	// Simulate TTYPE subnegotiation response
	mock.inject([]byte{CmdIAC, CmdSB, OptTerminalType, 0})
	mock.inject([]byte("xterm-256color"))
	mock.inject([]byte{CmdIAC, CmdSE})

	// Read to process
	buf := make([]byte, 100)
	c.Read(buf)

	// Terminal type should be set
	if c.TerminalType() != "xterm-256color" {
		t.Fatalf("expected 'xterm-256color', got %q", c.TerminalType())
	}
}

func TestConnTerminalSize(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{EnableNAWS: true})
	defer c.Close()

	// Simulate NAWS subnegotiation
	mock.inject([]byte{CmdIAC, CmdSB, OptNAWS, 0, 80, 0, 24, CmdIAC, CmdSE})

	// Read to process
	buf := make([]byte, 100)
	c.Read(buf)

	w, h := c.TerminalSize()
	if w != 80 || h != 24 {
		t.Fatalf("expected 80x24, got %dx%d", w, h)
	}
}

func TestConnStats(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()

	mock.inject([]byte("test data"))
	buf := make([]byte, 100)
	c.Read(buf)
	c.Write([]byte("response"))
	c.Flush()

	stats := c.Stats()
	if stats.BytesRead != 9 {
		t.Fatalf("expected 9 bytes read, got %d", stats.BytesRead)
	}
	if stats.BytesWritten != 8 {
		t.Fatalf("expected 8 bytes written, got %d", stats.BytesWritten)
	}
}

func TestConnClose(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{})

	c.Close()
	if !mock.closed {
		t.Fatal("expected underlying connection to be closed")
	}

	// Double close should return error
	err := c.Close()
	if err != ErrConnClosed {
		t.Fatalf("expected ErrConnClosed, got %v", err)
	}
}

func TestConnConcurrentReadWrite(t *testing.T) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()

	var wg sync.WaitGroup
	wg.Add(2)

	// Writer goroutine
	go func() {
		defer wg.Done()
		for i := 0; i < 100; i++ {
			c.Write([]byte("writer data\n"))
			c.Flush()
		}
	}()

	// Reader goroutine
	go func() {
		defer wg.Done()
		for i := 0; i < 100; i++ {
			mock.inject([]byte("reader data\n"))
			buf := make([]byte, 100)
			c.Read(buf)
		}
	}()

	wg.Wait()
}

func BenchmarkConnWrite(b *testing.B) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()
	data := []byte("benchmark data for writing to connection")

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		c.Write(data)
	}
}

func BenchmarkConnRead(b *testing.B) {
	mock := newMockConn()
	c := NewConn(mock, Config{})
	defer c.Close()
	data := bytes.Repeat([]byte("benchmark data "), 100)

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		mock.readBuf.Reset()
		mock.readBuf.Write(data)
		buf := make([]byte, len(data))
		c.Read(buf)
	}
}
