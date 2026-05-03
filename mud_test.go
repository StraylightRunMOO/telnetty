package telnetty

import (
	"bytes"
	"encoding/json"
	"testing"
)

func TestMarshalMSDP(t *testing.T) {
	data := map[string]*MSDPValue{
		"name":  MSDPString("test"),
		"level": MSDPString("5"),
	}
	result := MarshalMSDP(data)

	if len(result) == 0 {
		t.Fatal("expected non-empty result")
	}
	if result[0] != MSDPVar {
		t.Fatalf("expected MSDP_VAR, got %d", result[0])
	}
}

func TestUnmarshalMSDP(t *testing.T) {
	var buf bytes.Buffer
	buf.WriteByte(MSDPVar)
	buf.WriteString("name")
	buf.WriteByte(MSDPVal)
	buf.WriteString("test")
	buf.WriteByte(MSDPVar)
	buf.WriteString("level")
	buf.WriteByte(MSDPVal)
	buf.WriteString("5")

	result, err := UnmarshalMSDP(buf.Bytes())
	if err != nil {
		t.Fatal(err)
	}
	if result["name"].String != "test" {
		t.Fatalf("expected name=test, got %q", result["name"].String)
	}
	if result["level"].String != "5" {
		t.Fatalf("expected level=5, got %q", result["level"].String)
	}
}

func TestMSDPTable(t *testing.T) {
	data := map[string]*MSDPValue{
		"player": MSDPTable(map[string]*MSDPValue{
			"name":  MSDPString("Alice"),
			"level": MSDPString("10"),
		}),
	}
	result := MarshalMSDP(data)

	parsed, err := UnmarshalMSDP(result)
	if err != nil {
		t.Fatal(err)
	}
	if !parsed["player"].IsTable {
		t.Fatal("expected table")
	}
	if parsed["player"].Table["name"].String != "Alice" {
		t.Fatalf("expected Alice, got %q", parsed["player"].Table["name"].String)
	}
}

func TestMSDPArray(t *testing.T) {
	data := map[string]*MSDPValue{
		"items": MSDPArray([]*MSDPValue{
			MSDPString("sword"),
			MSDPString("shield"),
		}),
	}
	result := MarshalMSDP(data)

	parsed, err := UnmarshalMSDP(result)
	if err != nil {
		t.Fatal(err)
	}
	if !parsed["items"].IsArray {
		t.Fatal("expected array")
	}
	if len(parsed["items"].Array) != 2 {
		t.Fatalf("expected 2 items, got %d", len(parsed["items"].Array))
	}
}

func TestMarshalGMCP(t *testing.T) {
	data := map[string]interface{}{"test": "value"}
	result, err := MarshalGMCP("Core.Test", data)
	if err != nil {
		t.Fatal(err)
	}
	expected := `Core.Test {"test":"value"}`
	if string(result) != expected {
		t.Fatalf("expected %q, got %q", expected, string(result))
	}
}

func TestUnmarshalGMCP(t *testing.T) {
	data := []byte(`Core.Test {"test":"value"}`)
	msg, err := UnmarshalGMCP(data)
	if err != nil {
		t.Fatal(err)
	}
	if msg.Package != "Core.Test" {
		t.Fatalf("expected Core.Test, got %q", msg.Package)
	}
	var parsed map[string]string
	json.Unmarshal(msg.Data, &parsed)
	if parsed["test"] != "value" {
		t.Fatalf("expected value, got %q", parsed["test"])
	}
}

func TestParseMTTS(t *testing.T) {
	name, flags, err := ParseMTTS("MTTS 271")
	if err != nil {
		t.Fatal(err)
	}
	if name != "MTTS" {
		t.Fatalf("expected MTTS, got %q", name)
	}
	if flags != MTTSFlag(271) {
		t.Fatalf("expected 271, got %d", flags)
	}
	if !flags.Has(MTTSAnsi) {
		t.Fatal("expected ANSI flag")
	}
	if !flags.Has(MTTSUtf8) {
		t.Fatal("expected UTF8 flag")
	}
	if !flags.Has(MTTS256Color) {
		t.Fatal("expected 256COLOR flag")
	}
}

func TestParseMCP(t *testing.T) {
	line := "#$#mcp-negotiate-can 123 tag: mcp-negotiate min-version: 2.1 max-version: 2.1"
	msg, err := ParseMCP(line)
	if err != nil {
		t.Fatal(err)
	}
	if msg.Package != "mcp-negotiate-can" {
		t.Fatalf("expected mcp-negotiate-can, got %q", msg.Package)
	}
	if msg.AuthKey != "123" {
		t.Fatalf("expected auth 123, got %q", msg.AuthKey)
	}
	if msg.Tag != "tag:" {
		t.Fatalf("unexpected tag: %q", msg.Tag)
	}
	if msg.Params["min-version"] != "2.1" {
		t.Fatalf("unexpected min-version: %q", msg.Params["min-version"])
	}
}

func TestMCPString(t *testing.T) {
	msg := &MCPMessage{
		Package: "test",
		AuthKey: "key",
		Params:  map[string]string{"foo": "bar"},
	}
	result := msg.String()
	if !bytes.Contains([]byte(result), []byte("#$#test")) {
		t.Fatalf("expected #$#test prefix, got %q", result)
	}
}

func TestMarshalMSSP(t *testing.T) {
	vars := []MSSPVar{
		{Name: "NAME", Value: "TestMUD"},
		{Name: "PLAYERS", Value: "42"},
	}
	result := MarshalMSSP(vars)
	if len(result) == 0 {
		t.Fatal("expected non-empty result")
	}
}

func BenchmarkMarshalMSDP(b *testing.B) {
	data := map[string]*MSDPValue{
		"name": MSDPString("benchmark"),
		"level": MSDPTable(map[string]*MSDPValue{
			"hp":  MSDPString("100"),
			"mp":  MSDPString("50"),
		}),
	}

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		MarshalMSDP(data)
	}
}

func BenchmarkUnmarshalMSDP(b *testing.B) {
	data := MarshalMSDP(map[string]*MSDPValue{
		"name":  MSDPString("benchmark"),
		"level": MSDPString("99"),
	})

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		UnmarshalMSDP(data)
	}
}
