package telnetty

import (
	"bytes"
	"testing"
)

// BenchmarkSuite runs a comprehensive benchmark suite.
func BenchmarkSuite(b *testing.B) {
	b.Run("parser", func(b *testing.B) {
		b.Run("1KB_plain", benchmarkParser1KBPlain)
		b.Run("1KB_mixed", benchmarkParser1KBMixed)
		b.Run("64KB_plain", benchmarkParser64KBPlain)
		b.Run("64KB_heavy_IAC", benchmarkParser64KBHeavyIAC)
		b.Run("1MB_plain", benchmarkParser1MBPlain)
	})

	b.Run("ring", func(b *testing.B) {
		b.Run("write_read_64B", benchmarkRingWriteRead64B)
		b.Run("write_read_1KB", benchmarkRingWriteRead1KB)
		b.Run("write_byte", benchmarkRingWriteByte)
		b.Run("contention", benchmarkRingContention)
	})

	b.Run("buffer", func(b *testing.B) {
		b.Run("static_write_1KB", benchmarkStaticWrite1KB)
		b.Run("static_write_64B", benchmarkStaticWrite64B)
	})

	b.Run("negotiator", func(b *testing.B) {
		b.Run("handle_will", benchmarkNegotiatorHandleWill)
		b.Run("handle_do", benchmarkNegotiatorHandleDo)
		b.Run("full_cycle", benchmarkNegotiatorFullCycle)
	})

	b.Run("escape", func(b *testing.B) {
		b.Run("no_IAC_1KB", benchmarkEscapeNoIAC)
		b.Run("all_IAC_1KB", benchmarkEscapeAllIAC)
		b.Run("mixed_1KB", benchmarkEscapeMixed)
	})
}

func benchmarkParser1KBPlain(b *testing.B) {
	p := NewParser()
	var r ParseResult
	data := bytes.Repeat([]byte("hello world! "), 78)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		p.Reset()
		p.Parse(data, &r)
	}
}

func benchmarkParser1KBMixed(b *testing.B) {
	p := NewParser()
	var r ParseResult
	data := append([]byte("hello"), CmdIAC, CmdWill, OptEcho)
	data = append(data, bytes.Repeat([]byte(" world"), 160)...)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		p.Reset()
		p.Parse(data, &r)
	}
}

func benchmarkParser64KBPlain(b *testing.B) {
	p := NewParser()
	var r ParseResult
	data := bytes.Repeat([]byte("hello world! "), 5000)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		p.Reset()
		p.Parse(data, &r)
	}
}

func benchmarkParser64KBHeavyIAC(b *testing.B) {
	p := NewParser()
	var r ParseResult
	data := bytes.Repeat([]byte{CmdIAC, CmdWill, OptEcho}, 21000)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		p.Reset()
		p.Parse(data, &r)
	}
}

func benchmarkParser1MBPlain(b *testing.B) {
	p := NewParser()
	var r ParseResult
	data := bytes.Repeat([]byte("hello world! "), 80000)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		p.Reset()
		p.Parse(data, &r)
	}
}

func benchmarkRingWriteRead64B(b *testing.B) {
	r := NewRing(65536)
	defer r.Free()
	data := bytes.Repeat([]byte("x"), 64)
	out := make([]byte, 64)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		r.Write(data)
		r.Read(out)
	}
}

func benchmarkRingWriteRead1KB(b *testing.B) {
	r := NewRing(65536)
	defer r.Free()
	data := bytes.Repeat([]byte("x"), 1024)
	out := make([]byte, 1024)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		r.Write(data)
		r.Read(out)
	}
}

func benchmarkRingWriteByte(b *testing.B) {
	r := NewRing(65536)
	defer r.Free()
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		r.WriteByte(byte(i))
		r.ReadByte()
	}
}

func benchmarkRingContention(b *testing.B) {
	r := NewRing(65536)
	defer r.Free()
	data := []byte("contention test data packet")
	out := make([]byte, len(data))
	b.ReportAllocs()
	b.ResetTimer()
	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			r.Write(data)
			r.Read(out)
		}
	})
}

func benchmarkStaticWrite1KB(b *testing.B) {
	sb := NewStaticBuffer(65536)
	defer sb.Free()
	data := bytes.Repeat([]byte("x"), 1024)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		sb.Reset()
		sb.Write(data)
	}
}

func benchmarkStaticWrite64B(b *testing.B) {
	sb := NewStaticBuffer(65536)
	defer sb.Free()
	data := bytes.Repeat([]byte("x"), 64)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		sb.Reset()
		sb.Write(data)
	}
}

func benchmarkNegotiatorHandleWill(b *testing.B) {
	n := NewNegotiator()
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		n.Handle(CmdWill, OptEcho)
	}
}

func benchmarkNegotiatorHandleDo(b *testing.B) {
	n := NewNegotiator()
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		n.Handle(CmdDo, OptSGA)
	}
}

func benchmarkNegotiatorFullCycle(b *testing.B) {
	n := NewNegotiator()
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		n.Handle(CmdWill, OptEcho)
		n.Handle(CmdDo, OptEcho)
		n.Handle(CmdWont, OptEcho)
		n.Handle(CmdDont, OptEcho)
	}
}

func benchmarkEscapeNoIAC(b *testing.B) {
	src := bytes.Repeat([]byte("no IAC here at all "), 58)
	dst := make([]byte, len(src))
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		EscapeIAC(dst, src)
	}
}

func benchmarkEscapeAllIAC(b *testing.B) {
	src := bytes.Repeat([]byte{CmdIAC}, 1000)
	dst := make([]byte, len(src)*2)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		EscapeIAC(dst, src)
	}
}

func benchmarkEscapeMixed(b *testing.B) {
	src := bytes.Repeat([]byte{1, CmdIAC, 2, 3, CmdIAC, 4}, 167)
	dst := make([]byte, len(src)*2)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		EscapeIAC(dst, src)
	}
}
