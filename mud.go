package telnetty

import (
	"bytes"
	"compress/zlib"
	"encoding/json"
	"fmt"
	"strconv"
	"strings"
)

// ============================================================================
// MSDP (MUD Server Data Protocol)
// ============================================================================

// MSDPValue represents a value in an MSDP container.
type MSDPValue struct {
	String  string
	Table   map[string]*MSDPValue
	Array   []*MSDPValue
	IsTable bool
	IsArray bool
}

// MSDPString returns an MSDP string value.
func MSDPString(s string) *MSDPValue {
	return &MSDPValue{String: s}
}

// MSDPTable returns an MSDP table value.
func MSDPTable(t map[string]*MSDPValue) *MSDPValue {
	return &MSDPValue{Table: t, IsTable: true}
}

// MSDPArray returns an MSDP array value.
func MSDPArray(a []*MSDPValue) *MSDPValue {
	return &MSDPValue{Array: a, IsArray: true}
}

// MarshalMSDP serializes a map to MSDP wire format.
func MarshalMSDP(data map[string]*MSDPValue) []byte {
	var buf bytes.Buffer
	for k, v := range data {
		buf.WriteByte(MSDPVar)
		buf.WriteString(k)
		writeMSDPValue(&buf, v)
	}
	return buf.Bytes()
}

func writeMSDPValue(buf *bytes.Buffer, v *MSDPValue) {
	switch {
	case v.IsTable:
		buf.WriteByte(MSDPTableOpen)
		for k, val := range v.Table {
			buf.WriteByte(MSDPVar)
			buf.WriteString(k)
			writeMSDPValue(buf, val)
		}
		buf.WriteByte(MSDPTableClose)
	case v.IsArray:
		buf.WriteByte(MSDPArrayOpen)
		for _, val := range v.Array {
			writeMSDPValue(buf, val)
		}
		buf.WriteByte(MSDPArrayClose)
	default:
		buf.WriteByte(MSDPVal)
		buf.WriteString(v.String)
	}
}

// UnmarshalMSDP parses MSDP wire format into a map.
func UnmarshalMSDP(data []byte) (map[string]*MSDPValue, error) {
	result := make(map[string]*MSDPValue)
	i := 0
	for i < len(data) {
		if data[i] != MSDPVar {
			return nil, fmt.Errorf("expected MSDP_VAR at position %d", i)
		}
		i++
		start := i
		for i < len(data) && data[i] != MSDPVal && data[i] != MSDPTableOpen && data[i] != MSDPArrayOpen {
			i++
		}
		key := string(data[start:i])
		val, n := parseMSDPValue(data[i:])
		if n < 0 {
			return nil, fmt.Errorf("failed to parse MSDP value for key %s", key)
		}
		result[key] = val
		i += n
	}
	return result, nil
}

func parseMSDPValue(data []byte) (*MSDPValue, int) {
	if len(data) == 0 {
		return MSDPString(""), 0
	}
	switch data[0] {
	case MSDPVal:
		i := 1
		start := i
		for i < len(data) && data[i] != MSDPVar && data[i] != MSDPVal && data[i] != MSDPTableClose && data[i] != MSDPArrayClose {
			i++
		}
		return MSDPString(string(data[start:i])), i
	case MSDPTableOpen:
		table := make(map[string]*MSDPValue)
		i := 1
		for i < len(data) && data[i] != MSDPTableClose {
			if data[i] != MSDPVar {
				return nil, -1
			}
			i++
			start := i
			for i < len(data) && data[i] != MSDPVal && data[i] != MSDPTableOpen && data[i] != MSDPArrayOpen {
				i++
			}
			key := string(data[start:i])
			val, n := parseMSDPValue(data[i:])
			if n < 0 {
				return nil, -1
			}
			table[key] = val
			i += n
		}
		if i >= len(data) || data[i] != MSDPTableClose {
			return nil, -1
		}
		return MSDPTable(table), i + 1
	case MSDPArrayOpen:
		var array []*MSDPValue
		i := 1
		for i < len(data) && data[i] != MSDPArrayClose {
			val, n := parseMSDPValue(data[i:])
			if n < 0 {
				return nil, -1
			}
			array = append(array, val)
			i += n
		}
		if i >= len(data) || data[i] != MSDPArrayClose {
			return nil, -1
		}
		return MSDPArray(array), i + 1
	default:
		i := 0
		start := i
		for i < len(data) && data[i] != MSDPVar && data[i] != MSDPVal && data[i] != MSDPTableClose && data[i] != MSDPArrayClose {
			i++
		}
		return MSDPString(string(data[start:i])), i
	}
}

// ============================================================================
// GMCP (Generic MUD Communication Protocol)
// ============================================================================

// GMCPMessage represents a GMCP message.
type GMCPMessage struct {
	Package string
	Data    json.RawMessage
}

// MarshalGMCP serializes a GMCP message.
func MarshalGMCP(pkg string, data interface{}) ([]byte, error) {
	var buf bytes.Buffer
	buf.WriteString(pkg)
	buf.WriteByte(' ')
	if d, ok := data.([]byte); ok {
		buf.Write(d)
	} else {
		b, err := json.Marshal(data)
		if err != nil {
			return nil, err
		}
		buf.Write(b)
	}
	return buf.Bytes(), nil
}

// UnmarshalGMCP parses GMCP data.
func UnmarshalGMCP(data []byte) (*GMCPMessage, error) {
	space := bytes.IndexByte(data, ' ')
	if space < 0 {
		return &GMCPMessage{Package: string(data)}, nil
	}
	return &GMCPMessage{
		Package: string(data[:space]),
		Data:    json.RawMessage(data[space+1:]),
	}, nil
}

// SendGMCP sends a GMCP message through a Conn.
func SendGMCP(c *Conn, pkg string, data interface{}) error {
	payload, err := MarshalGMCP(pkg, data)
	if err != nil {
		return err
	}
	return c.SendSubnegotiation(OptGMCP, payload)
}

// ============================================================================
// MCCP (MUD Client Compression Protocol)
// ============================================================================

// MCCPWriter wraps a Conn with zlib compression for MCCP2/3.
type MCCPWriter struct {
	conn   *Conn
	writer *zlib.Writer
	level  int
}

// EnableMCCP2 enables MCCP2 compression on the connection.
// After this call, all writes go through zlib.
func EnableMCCP2(c *Conn, level int) (*MCCPWriter, error) {
	if level < zlib.NoCompression || level > zlib.BestCompression {
		level = zlib.DefaultCompression
	}
	if err := c.SendCommand(CmdWill, OptMCCP2); err != nil {
		return nil, err
	}
	if err := c.Flush(); err != nil {
		return nil, err
	}
	return &MCCPWriter{conn: c, level: level}, nil
}

// StartCompression begins zlib compression output.
func (mw *MCCPWriter) StartCompression() error {
	// Send IAC SB MCCP2 IAC SE to signal start
	if err := mw.conn.SendSubnegotiation(OptMCCP2, nil); err != nil {
		return err
	}
	if err := mw.conn.Flush(); err != nil {
		return err
	}

	var err error
	mw.writer, err = zlib.NewWriterLevel(mw.conn, mw.level)
	return err
}

// Write implements io.Writer.
func (mw *MCCPWriter) Write(p []byte) (int, error) {
	if mw.writer == nil {
		return mw.conn.Write(p)
	}
	return mw.writer.Write(p)
}

// Flush flushes the zlib stream.
func (mw *MCCPWriter) Flush() error {
	if mw.writer != nil {
		if err := mw.writer.Flush(); err != nil {
			return err
		}
	}
	return mw.conn.Flush()
}

// Close closes the compression stream.
func (mw *MCCPWriter) Close() error {
	if mw.writer != nil {
		if err := mw.writer.Close(); err != nil {
			return err
		}
		mw.writer = nil
	}
	return nil
}

// ============================================================================
// MSSP (MUD Server Status Protocol)
// ============================================================================

// MSSPVar represents an MSSP variable.
type MSSPVar struct {
	Name  string
	Value string
}

// MarshalMSSP serializes MSSP data.
func MarshalMSSP(vars []MSSPVar) []byte {
	var buf bytes.Buffer
	for _, v := range vars {
		buf.WriteByte(1) // MSSP_VAR
		buf.WriteString(v.Name)
		buf.WriteByte(2) // MSSP_VAL
		buf.WriteString(v.Value)
	}
	return buf.Bytes()
}

// SendMSSP sends MSSP server status.
func SendMSSP(c *Conn, vars []MSSPVar) error {
	return c.SendSubnegotiation(OptMSSP, MarshalMSSP(vars))
}

// ============================================================================
// MTTS (MUD Terminal Type Standard)
// ============================================================================

// MTTSFlag represents terminal capability flags.
type MTTSFlag uint32

const (
	MTTSAnsi       MTTSFlag = 1 << 0
	MTTSVt100      MTTSFlag = 1 << 1
	MTTSUtf8       MTTSFlag = 1 << 2
	MTTS256Color   MTTSFlag = 1 << 3
	MTTSMouse      MTTSFlag = 1 << 4
	MTTSOscColor   MTTSFlag = 1 << 5
	MTTSCscreen    MTTSFlag = 1 << 6
	MTTSProxy      MTTSFlag = 1 << 7
	MTTSTrueColor  MTTSFlag = 1 << 8
	MTTSMnes       MTTSFlag = 1 << 9
	MTTSMccp       MTTSFlag = 1 << 10
	MTTSMsp        MTTSFlag = 1 << 11
	MTTSMxp        MTTSFlag = 1 << 12
	MTTSSsl        MTTSFlag = 1 << 13
)

// Has returns true if the flag is set.
func (f MTTSFlag) Has(flag MTTSFlag) bool {
	return f&flag != 0
}

// ParseMTTS parses an MTTS terminal type string like "MTTS 271".
func ParseMTTS(s string) (name string, flags MTTSFlag, err error) {
	parts := strings.Fields(s)
	if len(parts) < 2 || !strings.EqualFold(parts[0], "MTTS") {
		return s, 0, nil
	}
	name = parts[0]
	f, err := strconv.ParseUint(parts[1], 10, 32)
	if err != nil {
		return name, 0, err
	}
	return name, MTTSFlag(f), nil
}

// ============================================================================
// MCP (MUD Client Protocol 2.1)
// ============================================================================

// MCPMessage represents an MCP message.
type MCPMessage struct {
	AuthKey string
	Package string
	Tag     string
	Params  map[string]string
}

// ParseMCP parses an MCP line.
func ParseMCP(line string) (*MCPMessage, error) {
	// Format: #$#package authkey tag param: value param: value...
	if !strings.HasPrefix(line, "#$#") {
		return nil, fmt.Errorf("not an MCP message")
	}
	line = line[3:]
	parts := strings.Fields(line)
	if len(parts) < 2 {
		return nil, fmt.Errorf("invalid MCP message")
	}

	msg := &MCPMessage{
		Package: parts[0],
		Params:  make(map[string]string),
	}

	// Check for auth key
	if len(parts) > 1 && !strings.Contains(parts[1], ":") {
		msg.AuthKey = parts[1]
		parts = parts[2:]
	} else {
		parts = parts[1:]
	}

	// Check for tag
	if len(parts) > 0 {
		msg.Tag = parts[0]
		parts = parts[1:]
	}

	// Parse parameters (handle "key: value" split by Fields)
	for i := 0; i < len(parts); i++ {
		part := parts[i]
		colon := strings.Index(part, ":")
		if colon > 0 {
			key := part[:colon]
			val := ""
			if colon+1 < len(part) {
				val = part[colon+1:]
			}
			// If value is empty and next part doesn't contain ':', use next part as value
			if val == "" && i+1 < len(parts) && !strings.Contains(parts[i+1], ":") {
				val = parts[i+1]
				i++
			}
			msg.Params[key] = val
		}
	}

	return msg, nil
}

// String returns the MCP wire format.
func (m *MCPMessage) String() string {
	var buf strings.Builder
	buf.WriteString("#$#")
	buf.WriteString(m.Package)
	if m.AuthKey != "" {
		buf.WriteByte(' ')
		buf.WriteString(m.AuthKey)
	}
	if m.Tag != "" {
		buf.WriteByte(' ')
		buf.WriteString(m.Tag)
	}
	for k, v := range m.Params {
		buf.WriteByte(' ')
		buf.WriteString(k)
		buf.WriteString(": ")
		buf.WriteString(v)
	}
	return buf.String()
}
