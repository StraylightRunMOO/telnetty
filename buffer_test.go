package telnetty

import (
	"bytes"
	"testing"
)

func TestRingBasic(t *testing.T) {
	r := NewRing(1024)
	defer r.Free()

	data := []byte("hello world")
	n, err := r.Write(data)
	if err != nil {
		t.Fatal(err)
	}
	if n != len(data) {
		t.Fatalf("expected %d, got %d", len(data), n)
	}

	if r.Len() != len(data) {
		t.Fatalf("expected len %d, got %d", len(data), r.Len())
	}

	out := make([]byte, len(data))
	n, err = r.Read(out)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(out, data) {
		t.Fatalf("expected %q, got %q", data, out)
	}
}

func TestRingWrapAround(t *testing.T) {
	r := NewRing(16) // capacity 15 usable
	defer r.Free()

	// Fill almost to capacity
	data := bytes.Repeat([]byte("a"), 14)
	r.Write(data)

	// Read half
	out := make([]byte, 7)
	r.Read(out)

	// Write more - should wrap
	r.Write([]byte("wrapped"))

	// Read all remaining
	result := r.Bytes()
	if !bytes.Equal(result[:7], data[7:]) || !bytes.Equal(result[7:], []byte("wrapped")) {
		t.Fatalf("wrap-around failed: %q", result)
	}
}

func TestRingFull(t *testing.T) {
	r := NewRing(8) // capacity 7 usable
	defer r.Free()

	data := bytes.Repeat([]byte("x"), 10)
	n, err := r.Write(data)
	if err != errBufferFull {
		t.Fatalf("expected errBufferFull, got %v", err)
	}
	if n != 7 {
		t.Fatalf("expected 7, got %d", n)
	}
}

func TestRingEmpty(t *testing.T) {
	r := NewRing(1024)
	defer r.Free()

	out := make([]byte, 10)
	_, err := r.Read(out)
	if err != errBufferEmpty {
		t.Fatalf("expected errBufferEmpty, got %v", err)
	}
}

func TestRingByteOperations(t *testing.T) {
	r := NewRing(16)
	defer r.Free()

	for i := byte(0); i < 10; i++ {
		if err := r.WriteByte(i); err != nil {
			t.Fatal(err)
		}
	}

	for i := byte(0); i < 10; i++ {
		b, err := r.ReadByte()
		if err != nil {
			t.Fatal(err)
		}
		if b != i {
			t.Fatalf("expected %d, got %d", i, b)
		}
	}
}

func TestRingPeek(t *testing.T) {
	r := NewRing(1024)
	defer r.Free()

	data := []byte("peek test")
	r.Write(data)

	p := r.Peek(4)
	if !bytes.Equal(p, []byte("peek")) {
		t.Fatalf("expected 'peek', got %q", p)
	}

	// Verify data still there after peek
	if r.Len() != len(data) {
		t.Fatalf("peek consumed data")
	}
}

func TestRingDiscard(t *testing.T) {
	r := NewRing(1024)
	defer r.Free()

	data := []byte("discard me")
	r.Write(data)
	r.Discard(5)

	if r.Len() != 5 {
		t.Fatalf("expected 5 remaining, got %d", r.Len())
	}

	out := r.Bytes()
	if !bytes.Equal(out, []byte("rd me")) {
		t.Fatalf("expected 'rd me', got %q", out)
	}
}

func TestRingReset(t *testing.T) {
	r := NewRing(1024)
	defer r.Free()

	r.Write([]byte("data"))
	r.Reset()

	if r.Len() != 0 {
		t.Fatalf("expected empty after reset")
	}
}

func TestStaticBuffer(t *testing.T) {
	sb := NewStaticBuffer(1024)
	defer sb.Free()

	data := []byte("static buffer test")
	n, err := sb.Write(data)
	if err != nil {
		t.Fatal(err)
	}
	if n != len(data) {
		t.Fatalf("expected %d, got %d", len(data), n)
	}

	if !bytes.Equal(sb.Bytes(), data) {
		t.Fatalf("expected %q, got %q", data, sb.Bytes())
	}

	sb.Reset()
	if sb.Len() != 0 {
		t.Fatalf("expected empty after reset")
	}
}

func TestStaticBufferOverflow(t *testing.T) {
	sb := NewStaticBuffer(8)
	defer sb.Free()

	data := bytes.Repeat([]byte("x"), 10)
	n, err := sb.Write(data)
	if err != nil {
		t.Fatal(err)
	}
	if n != 8 {
		t.Fatalf("expected 8, got %d", n)
	}
}

func BenchmarkRingWriteRead(b *testing.B) {
	r := NewRing(65536)
	defer r.Free()
	data := []byte("benchmark data packet")

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		r.Write(data)
		out := make([]byte, len(data))
		r.Read(out)
	}
}

func BenchmarkRingWriteByte(b *testing.B) {
	r := NewRing(65536)
	defer r.Free()

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		r.WriteByte(byte(i))
		r.ReadByte()
	}
}

func BenchmarkStaticBufferWrite(b *testing.B) {
	sb := NewStaticBuffer(65536)
	defer sb.Free()
	data := []byte("benchmark data packet")

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		sb.Reset()
		sb.Write(data)
	}
}
