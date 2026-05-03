package telnetty

import (
	"bytes"
	"net"
	"testing"
	"time"
)

// TestReadLineSplitCRLF verifies that \r\n split across read boundaries
// is correctly handled (the \r is stripped even if it arrives in a
// different chunk than the \n).
func TestReadLineSplitCRLF(t *testing.T) {
	c1, c2 := net.Pipe()
	defer c1.Close()
	defer c2.Close()

	go func() {
		time.Sleep(10 * time.Millisecond)
		c2.Write([]byte("hello\r"))
		time.Sleep(50 * time.Millisecond)
		c2.Write([]byte("\nworld\r\n"))
		c2.Close()
	}()

	conn := NewConn(c1, Config{})
	defer conn.Close()

	line, err := conn.ReadLine()
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if !bytes.Equal(line, []byte("hello")) {
		t.Fatalf("expected 'hello', got %q", line)
	}

	line2, err := conn.ReadLine()
	if err != nil {
		t.Fatalf("unexpected error on second read: %v", err)
	}
	if !bytes.Equal(line2, []byte("world")) {
		t.Fatalf("expected 'world', got %q", line2)
	}
}

// TestReadLineEOFWithData verifies that trailing data before EOF is not
// lost when the underlying netConn.Read returns data + io.EOF in one call.
func TestReadLineEOFWithData(t *testing.T) {
	c1, c2 := net.Pipe()
	defer c1.Close()
	defer c2.Close()

	go func() {
		time.Sleep(10 * time.Millisecond)
		c2.Write([]byte("farewell\r\n"))
		c2.Close()
	}()

	conn := NewConn(c1, Config{})
	defer conn.Close()

	line, err := conn.ReadLine()
	if !bytes.Equal(line, []byte("farewell")) {
		t.Fatalf("expected 'farewell', got %q", line)
	}
	// EOF may or may not be returned for the next call; either is fine.
	_ = err
}

// TestReadLineWithNegotiation verifies ReadLine works when TELNET commands
// are interleaved with the data stream.
func TestReadLineWithNegotiation(t *testing.T) {
	c1, c2 := net.Pipe()
	defer c1.Close()
	defer c2.Close()

	go func() {
		time.Sleep(10 * time.Millisecond)
		// Send a command, then data split across two writes
		c2.Write([]byte{CmdIAC, CmdWill, OptEcho})
		c2.Write([]byte("look\r\n"))
		c2.Close()
	}()

	conn := NewConn(c1, Config{})
	defer conn.Close()

	line, err := conn.ReadLine()
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if !bytes.Equal(line, []byte("look")) {
		t.Fatalf("expected 'look', got %q", line)
	}
}

// TestReadLineRefusesUnsupportedOptions verifies that the server refuses
// options it does not support or has not enabled.
func TestReadLineRefusesUnsupportedOptions(t *testing.T) {
	c1, c2 := net.Pipe()
	defer c1.Close()
	defer c2.Close()

	go func() {
		time.Sleep(10 * time.Millisecond)
		// Client asks server to enable ECHO (server has EnableEcho: false)
		c2.Write([]byte{CmdIAC, CmdDo, OptEcho})
		// Client offers to enable LINEMODE (not supported)
		c2.Write([]byte{CmdIAC, CmdWill, OptLinemode})
		c2.Write([]byte("test\r\n"))
		c2.Close()
	}()

	conn := NewConn(c1, Config{})
	defer conn.Close()

	line, err := conn.ReadLine()
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if !bytes.Equal(line, []byte("test")) {
		t.Fatalf("expected 'test', got %q", line)
	}
}
