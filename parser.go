package telnetty

import (
	"sync/atomic"
	"unsafe"
)

// ParserState represents the TELNET protocol parser state machine.
type ParserState uint8

const (
	StateData ParserState = iota
	StateIAC
	StateNegotiation
	StateSubnegotiation
	StateSubnegotiationIAC
	StateSubnegotiationOption
)

// ParseResult holds the outcome of parsing a chunk of data.
// All fields are pre-allocated to avoid heap escapes.
type ParseResult struct {
	DataEvents    [64]DataEvent
	DataCount     int
	CommandEvents [16]CommandEvent
	CmdCount      int
	SNEvents      [8]SubnegotiationEvent
	SNCount       int
}

// DataEvent represents a chunk of plain data.
type DataEvent struct {
	Data   []byte
	Length int
}

// CommandEvent represents a TELNET command (WILL/WONT/DO/DONT + option).
type CommandEvent struct {
	Command uint8
	Option  uint8
}

// SubnegotiationEvent represents completed subnegotiation data.
type SubnegotiationEvent struct {
	Option uint8
	Data   []byte
	Length int
}

// Parser is a reusable, zero-allocation TELNET protocol parser.
// It maintains internal state for partial IAC sequences across calls.
type Parser struct {
	state         ParserState
	cmd           uint8       // pending command (WILL/WONT/DO/DONT)
	snOption      uint8       // current subnegotiation option
	snBuf         StaticBuffer // subnegotiation accumulator
	snEnabled     bool
	binaryMode    uint32 // atomic: 1 if binary transmission enabled
}

// NewParser creates a parser with a pre-allocated subnegotiation buffer.
func NewParser() *Parser {
	p := &Parser{}
	p.snBuf.buf = make([]byte, 0, 512)
	return p
}

// Reset clears parser state for reuse (e.g., connection recycling).
func (p *Parser) Reset() {
	p.state = StateData
	p.cmd = 0
	p.snOption = 0
	p.snBuf.Reset()
	p.snEnabled = false
	atomic.StoreUint32(&p.binaryMode, 0)
}

// IsBinary returns true if binary transmission mode is active.
func (p *Parser) IsBinary() bool {
	return atomic.LoadUint32(&p.binaryMode) != 0
}

// Parse processes input data and fills result with events.
// This is the hot path - it must not allocate.
func (p *Parser) Parse(data []byte, result *ParseResult) {
	result.DataCount = 0
	result.CmdCount = 0
	result.SNCount = 0

	var dataStart int
	for i := 0; i < len(data); i++ {
		b := data[i]

		switch p.state {
		case StateData:
			if b == CmdIAC {
				// Flush any pending data
				if i > dataStart {
					p.appendData(result, data[dataStart:i])
				}
				p.state = StateIAC
			}

		case StateIAC:
			switch b {
			case CmdIAC:
				// Escaped IAC byte - treat as data
				p.appendData(result, data[i:i+1])
				dataStart = i + 1
				p.state = StateData
			case CmdWill, CmdWont, CmdDo, CmdDont:
				p.cmd = b
				p.state = StateNegotiation
			case CmdSB:
				p.snBuf.Reset()
				p.state = StateSubnegotiationOption
			case CmdGA, CmdEL, CmdEC, CmdAYT, CmdAO, CmdIP, CmdBreak, CmdDM, CmdNOP, CmdEOR, CmdAbort, CmdSusp, CmdEOF:
				// Single-byte commands - no data to append
				dataStart = i + 1
				p.state = StateData
			default:
				// Unknown command - skip
				dataStart = i + 1
				p.state = StateData
			}

		case StateNegotiation:
			p.appendCommand(result, p.cmd, b)
			dataStart = i + 1
			p.state = StateData

		case StateSubnegotiationOption:
			p.snOption = b
			p.state = StateSubnegotiation

		case StateSubnegotiation:
			if b == CmdIAC {
				p.state = StateSubnegotiationIAC
			} else {
				p.snBuf.WriteByte(b)
			}

		case StateSubnegotiationIAC:
			if b == CmdSE {
				// End of subnegotiation
				p.appendSubnegotiation(result, p.snOption, p.snBuf.Bytes())
				dataStart = i + 1
				p.state = StateData
			} else {
				// Escaped IAC in subnegotiation
				p.snBuf.WriteByte(CmdIAC)
				p.snBuf.WriteByte(b)
				p.state = StateSubnegotiation
			}
		}
	}

	// Flush remaining data
	if p.state == StateData && dataStart < len(data) {
		p.appendData(result, data[dataStart:])
	}
}

// appendData adds a data event, coalescing with the previous one if contiguous.
func (p *Parser) appendData(r *ParseResult, data []byte) {
	if len(data) == 0 {
		return
	}
	if r.DataCount > 0 {
		last := &r.DataEvents[r.DataCount-1]
		// Check if contiguous (same underlying slice)
		if len(last.Data) > 0 && unsafe.Pointer(uintptr(unsafe.Pointer(&last.Data[len(last.Data)-1]))+1) == unsafe.Pointer(&data[0]) {
			last.Data = last.Data[:len(last.Data)+len(data)]
			last.Length += len(data)
			return
		}
	}
	if r.DataCount < len(r.DataEvents) {
		r.DataEvents[r.DataCount] = DataEvent{Data: data, Length: len(data)}
		r.DataCount++
	}
}

func (p *Parser) appendCommand(r *ParseResult, cmd, option uint8) {
	if r.CmdCount < len(r.CommandEvents) {
		r.CommandEvents[r.CmdCount] = CommandEvent{Command: cmd, Option: option}
		r.CmdCount++
	}
}

func (p *Parser) appendSubnegotiation(r *ParseResult, option uint8, data []byte) {
	if r.SNCount < len(r.SNEvents) {
		r.SNEvents[r.SNCount] = SubnegotiationEvent{Option: option, Data: data, Length: len(data)}
		r.SNCount++
	}
}

// EscapeIAC escapes IAC bytes in data by doubling them.
// Writes to dst, returning the number of bytes written.
// If dst is too small, returns the required size as a negative number.
func EscapeIAC(dst, src []byte) int {
	required := len(src)
	for _, b := range src {
		if b == CmdIAC {
			required++
		}
	}
	if len(dst) < required {
		return -required
	}
	n := 0
	for _, b := range src {
		dst[n] = b
		n++
		if b == CmdIAC {
			dst[n] = CmdIAC
			n++
		}
	}
	return n
}

// BuildCommand writes a TELNET command sequence into dst.
// Returns bytes written, or negative required size if dst too small.
func BuildCommand(dst []byte, cmd, option uint8) int {
	if len(dst) < 3 {
		return -3
	}
	dst[0] = CmdIAC
	dst[1] = cmd
	dst[2] = option
	return 3
}

// BuildSubnegotiation writes a complete subnegotiation into dst.
// Data is IAC-escaped automatically. Returns bytes written or negative required.
func BuildSubnegotiation(dst []byte, option uint8, data []byte) int {
	// Calculate required size
	required := 3 // IAC SB option
	for _, b := range data {
		required++
		if b == CmdIAC {
			required++
		}
	}
	required += 2 // IAC SE

	if len(dst) < required {
		return -required
	}

	n := 0
	dst[n] = CmdIAC
	n++
	dst[n] = CmdSB
	n++
	dst[n] = option
	n++
	for _, b := range data {
		dst[n] = b
		n++
		if b == CmdIAC {
			dst[n] = CmdIAC
			n++
		}
	}
	dst[n] = CmdIAC
	n++
	dst[n] = CmdSE
	n++
	return n
}
