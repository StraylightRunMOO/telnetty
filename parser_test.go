package telnetty

import (
	"bytes"
	"testing"
)

func TestParserPlainData(t *testing.T) {
	p := NewParser()
	defer p.Reset()

	var r ParseResult
	data := []byte("hello world")
	p.Parse(data, &r)

	if r.DataCount != 1 {
		t.Fatalf("expected 1 data event, got %d", r.DataCount)
	}
	if !bytes.Equal(r.DataEvents[0].Data, data) {
		t.Fatalf("expected %q, got %q", data, r.DataEvents[0].Data)
	}
	if r.CmdCount != 0 {
		t.Fatalf("expected 0 commands, got %d", r.CmdCount)
	}
}

func TestParserIACEscape(t *testing.T) {
	p := NewParser()
	var r ParseResult

	// IAC IAC should produce a single 255 byte
	data := []byte{CmdIAC, CmdIAC}
	p.Parse(data, &r)

	if r.DataCount != 1 {
		t.Fatalf("expected 1 data event, got %d", r.DataCount)
	}
	if r.DataEvents[0].Data[0] != CmdIAC {
		t.Fatalf("expected escaped IAC, got %d", r.DataEvents[0].Data[0])
	}
}

func TestParserNegotiation(t *testing.T) {
	p := NewParser()
	var r ParseResult

	// IAC WILL ECHO
	data := []byte{CmdIAC, CmdWill, OptEcho}
	p.Parse(data, &r)

	if r.CmdCount != 1 {
		t.Fatalf("expected 1 command, got %d", r.CmdCount)
	}
	if r.CommandEvents[0].Command != CmdWill {
		t.Fatalf("expected WILL, got %d", r.CommandEvents[0].Command)
	}
	if r.CommandEvents[0].Option != OptEcho {
		t.Fatalf("expected ECHO, got %d", r.CommandEvents[0].Option)
	}
}

func TestParserSubnegotiation(t *testing.T) {
	p := NewParser()
	var r ParseResult

	// IAC SB NAWS 0 80 0 24 IAC SE
	data := []byte{CmdIAC, CmdSB, OptNAWS, 0, 80, 0, 24, CmdIAC, CmdSE}
	p.Parse(data, &r)

	if r.SNCount != 1 {
		t.Fatalf("expected 1 subnegotiation, got %d", r.SNCount)
	}
	if r.SNEvents[0].Option != OptNAWS {
		t.Fatalf("expected NAWS, got %d", r.SNEvents[0].Option)
	}
	if !bytes.Equal(r.SNEvents[0].Data, []byte{0, 80, 0, 24}) {
		t.Fatalf("unexpected subnegotiation data: %v", r.SNEvents[0].Data)
	}
}

func TestParserMixedData(t *testing.T) {
	p := NewParser()
	var r ParseResult

	// "hello" + IAC WILL ECHO + "world"
	data := append([]byte("hello"), CmdIAC, CmdWill, OptEcho)
	data = append(data, []byte("world")...)
	p.Parse(data, &r)

	if r.DataCount != 2 {
		t.Fatalf("expected 2 data events, got %d", r.DataCount)
	}
	if !bytes.Equal(r.DataEvents[0].Data, []byte("hello")) {
		t.Fatalf("expected 'hello', got %q", r.DataEvents[0].Data)
	}
	if !bytes.Equal(r.DataEvents[1].Data, []byte("world")) {
		t.Fatalf("expected 'world', got %q", r.DataEvents[1].Data)
	}
	if r.CmdCount != 1 {
		t.Fatalf("expected 1 command, got %d", r.CmdCount)
	}
}

func TestParserMultipleCommands(t *testing.T) {
	p := NewParser()
	var r ParseResult

	// IAC WILL ECHO IAC DO SGA IAC WONT LINEMODE
	data := []byte{
		CmdIAC, CmdWill, OptEcho,
		CmdIAC, CmdDo, OptSGA,
		CmdIAC, CmdWont, OptLinemode,
	}
	p.Parse(data, &r)

	if r.CmdCount != 3 {
		t.Fatalf("expected 3 commands, got %d", r.CmdCount)
	}
	if r.CommandEvents[0].Option != OptEcho {
		t.Fatalf("expected first command ECHO, got %d", r.CommandEvents[0].Option)
	}
	if r.CommandEvents[1].Option != OptSGA {
		t.Fatalf("expected second command SGA, got %d", r.CommandEvents[1].Option)
	}
	if r.CommandEvents[2].Option != OptLinemode {
		t.Fatalf("expected third command LINEMODE, got %d", r.CommandEvents[2].Option)
	}
}

func TestParserSubnegotiationWithIAC(t *testing.T) {
	p := NewParser()
	var r ParseResult

	// IAC SB GMCP {"test": 255} IAC SE
	// where 255 in the JSON needs escaping as IAC IAC
	data := []byte{CmdIAC, CmdSB, OptGMCP}
	data = append(data, []byte(`{"test": `)...)
	data = append(data, CmdIAC, CmdIAC) // escaped 255
	data = append(data, []byte(`}`)...)
	data = append(data, CmdIAC, CmdSE)
	p.Parse(data, &r)

	if r.SNCount != 1 {
		t.Fatalf("expected 1 subnegotiation, got %d", r.SNCount)
	}
	// The data should contain a single 255, not IAC IAC
	if !bytes.Contains(r.SNEvents[0].Data, []byte{255}) {
		t.Fatalf("expected escaped IAC to be unescaped in data")
	}
}

func TestParserAcrossCalls(t *testing.T) {
	p := NewParser()

	// Split IAC WILL ECHO across two calls
	var r1 ParseResult
	p.Parse([]byte{CmdIAC}, &r1)
	if r1.DataCount != 0 || r1.CmdCount != 0 {
		t.Fatal("expected no events after partial IAC")
	}

	var r2 ParseResult
	p.Parse([]byte{CmdWill, OptEcho}, &r2)
	if r2.CmdCount != 1 {
		t.Fatalf("expected 1 command in second call, got %d", r2.CmdCount)
	}
}

func TestEscapeIAC(t *testing.T) {
	src := []byte{1, 2, CmdIAC, 3, 4}
	dst := make([]byte, 10)
	n := EscapeIAC(dst, src)
	if n != 6 {
		t.Fatalf("expected 6 bytes, got %d", n)
	}
	expected := []byte{1, 2, CmdIAC, CmdIAC, 3, 4}
	if !bytes.Equal(dst[:n], expected) {
		t.Fatalf("expected %v, got %v", expected, dst[:n])
	}
}

func TestEscapeIACInsufficientSpace(t *testing.T) {
	src := []byte{CmdIAC, CmdIAC}
	dst := make([]byte, 2)
	n := EscapeIAC(dst, src)
	if n != -4 {
		t.Fatalf("expected -4, got %d", n)
	}
}

func TestBuildCommand(t *testing.T) {
	dst := make([]byte, 3)
	n := BuildCommand(dst, CmdWill, OptEcho)
	if n != 3 {
		t.Fatalf("expected 3, got %d", n)
	}
	expected := []byte{CmdIAC, CmdWill, OptEcho}
	if !bytes.Equal(dst, expected) {
		t.Fatalf("expected %v, got %v", expected, dst)
	}
}

func TestBuildSubnegotiation(t *testing.T) {
	data := []byte{1, 2, 3}
	dst := make([]byte, 20)
	n := BuildSubnegotiation(dst, OptNAWS, data)
	if n != 8 {
		t.Fatalf("expected 8, got %d", n)
	}
	expected := []byte{CmdIAC, CmdSB, OptNAWS, 1, 2, 3, CmdIAC, CmdSE}
	if !bytes.Equal(dst[:n], expected) {
		t.Fatalf("expected %v, got %v", expected, dst[:n])
	}
}

func BenchmarkParserProcess1KB(b *testing.B) {
	p := NewParser()
	var r ParseResult
	data := bytes.Repeat([]byte("hello world! "), 78) // ~1KB
	data = append(data, CmdIAC, CmdWill, OptEcho)

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		p.Reset()
		p.Parse(data, &r)
	}
}

func BenchmarkParserProcess64KB(b *testing.B) {
	p := NewParser()
	var r ParseResult
	data := bytes.Repeat([]byte("hello world! "), 5000) // ~65KB

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		p.Reset()
		p.Parse(data, &r)
	}
}

func BenchmarkParserProcess1MB(b *testing.B) {
	p := NewParser()
	var r ParseResult
	data := bytes.Repeat([]byte("hello world! "), 80000) // ~1MB

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		p.Reset()
		p.Parse(data, &r)
	}
}

func BenchmarkEscapeIAC(b *testing.B) {
	src := bytes.Repeat([]byte{1, CmdIAC, 2}, 1000)
	dst := make([]byte, len(src)*2)

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		EscapeIAC(dst, src)
	}
}
