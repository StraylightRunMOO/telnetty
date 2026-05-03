package telnetty

import (
	"bytes"
	"context"
	"net"
	"sync"
	"testing"
	"time"
)

func TestServerBasic(t *testing.T) {
	var wg sync.WaitGroup
	wg.Add(1)

	var received []byte
	var mu sync.Mutex

	server := NewServer("127.0.0.1:0",
		WithConfig(Config{}),
		WithHandler(func(c *Conn) {
			defer wg.Done()
			buf := make([]byte, 100)
			n, _ := c.Read(buf)
			mu.Lock()
			received = append(received, buf[:n]...)
			mu.Unlock()
			c.Write([]byte("ok"))
			c.Flush()
		}),
		WithMaxConns(10),
	)

	go server.ListenAndServe()
	defer server.Close()

	// Wait for server to start
	time.Sleep(50 * time.Millisecond)

	// Connect
	addr := server.Addr()
	nc, err := net.Dial("tcp", addr.String())
	if err != nil {
		t.Fatal(err)
	}
	defer nc.Close()

	nc.Write([]byte("hello"))
	nc.Write([]byte("\r\n"))

	// Read response
	buf := make([]byte, 100)
	nc.SetReadDeadline(time.Now().Add(time.Second))
	n, _ := nc.Read(buf)

	wg.Wait()

	mu.Lock()
	if !bytes.Contains(received, []byte("hello")) {
		t.Fatalf("expected 'hello' in received, got %q", received)
	}
	mu.Unlock()

	if !bytes.Contains(buf[:n], []byte("ok")) {
		t.Fatalf("expected 'ok' in response, got %q", buf[:n])
	}
}

func TestServerClose(t *testing.T) {
	server := NewServer("127.0.0.1:0",
		WithHandler(func(c *Conn) {
			c.Read(make([]byte, 100))
		}),
	)

	go server.ListenAndServe()
	time.Sleep(50 * time.Millisecond)

	err := server.Close()
	if err != nil {
		t.Fatal(err)
	}

	// Double close should error
	err = server.Close()
	if err != ErrServerClosed {
		t.Fatalf("expected ErrServerClosed, got %v", err)
	}
}

func TestServerShutdown(t *testing.T) {
	server := NewServer("127.0.0.1:0",
		WithHandler(func(c *Conn) {
			c.Read(make([]byte, 100))
		}),
	)

	go server.ListenAndServe()
	time.Sleep(50 * time.Millisecond)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	err := server.Shutdown(ctx)
	if err != nil {
		t.Fatal(err)
	}
}

func TestServerMaxConns(t *testing.T) {
	var active int32
	var mu sync.Mutex

	server := NewServer("127.0.0.1:0",
		WithHandler(func(c *Conn) {
			mu.Lock()
			active++
			mu.Unlock()
			defer func() {
				mu.Lock()
				active--
				mu.Unlock()
			}()
			c.Read(make([]byte, 100))
		}),
		WithMaxConns(2),
	)

	go server.ListenAndServe()
	time.Sleep(50 * time.Millisecond)

	addr := server.Addr().String()

	// Connect 3 clients
	for i := 0; i < 3; i++ {
		go func() {
			nc, _ := net.Dial("tcp", addr)
			if nc != nil {
				nc.Write([]byte("test\r\n"))
				time.Sleep(200 * time.Millisecond)
				nc.Close()
			}
		}()
	}

	time.Sleep(100 * time.Millisecond)

	mu.Lock()
	if active > 2 {
		t.Fatalf("expected max 2 active, got %d", active)
	}
	mu.Unlock()

	server.Close()
}

func TestServerStats(t *testing.T) {
	var wg sync.WaitGroup
	wg.Add(1)

	server := NewServer("127.0.0.1:0",
		WithHandler(func(c *Conn) {
			defer wg.Done()
			c.Read(make([]byte, 100))
		}),
	)

	go server.ListenAndServe()
	time.Sleep(50 * time.Millisecond)

	nc, _ := net.Dial("tcp", server.Addr().String())
	nc.Write([]byte("test\r\n"))
	time.Sleep(50 * time.Millisecond)
	nc.Close()

	wg.Wait()
	server.Close()

	accepted, closed, active := server.Stats()
	if accepted != 1 {
		t.Fatalf("expected 1 accepted, got %d", accepted)
	}
	if closed != 1 {
		t.Fatalf("expected 1 closed, got %d", closed)
	}
	if active != 0 {
		t.Fatalf("expected 0 active, got %d", active)
	}
}

func BenchmarkServerAccept(b *testing.B) {
	server := NewServer("127.0.0.1:0",
		WithHandler(func(c *Conn) {
			c.Read(make([]byte, 100))
			c.Close()
		}),
		WithMaxConns(1000),
	)
	go server.ListenAndServe()
	defer server.Close()
	time.Sleep(50 * time.Millisecond)

	addr := server.Addr().String()

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		nc, err := net.Dial("tcp", addr)
		if err != nil {
			b.Fatal(err)
		}
		nc.Write([]byte("x\r\n"))
		nc.Close()
	}
}
