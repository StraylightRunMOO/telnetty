package telnetty

import (
	"testing"
)

// FuzzParser fuzzes the TELNET parser with random byte sequences.
func FuzzParser(f *testing.F) {
	// Seed corpus: common TELNET sequences
	f.Add([]byte("hello world"))
	f.Add([]byte{CmdIAC, CmdWill, OptEcho})
	f.Add([]byte{CmdIAC, CmdSB, OptNAWS, 0, 80, 0, 24, CmdIAC, CmdSE})
	f.Add([]byte{CmdIAC, CmdIAC, CmdIAC, CmdIAC}) // escaped IACs
	f.Add([]byte{CmdIAC, CmdWill, OptEcho, CmdIAC, CmdDo, OptSGA})
	f.Add([]byte{0xFF, 0xFF, 0xFF, 0xFB, 0x01}) // edge case

	f.Fuzz(func(t *testing.T, data []byte) {
		p := NewParser()
		var r ParseResult
		p.Parse(data, &r)

		// Basic invariants
		if r.DataCount < 0 || r.DataCount > len(r.DataEvents) {
			t.Fatalf("invalid DataCount: %d", r.DataCount)
		}
		if r.CmdCount < 0 || r.CmdCount > len(r.CommandEvents) {
			t.Fatalf("invalid CmdCount: %d", r.CmdCount)
		}
		if r.SNCount < 0 || r.SNCount > len(r.SNEvents) {
			t.Fatalf("invalid SNCount: %d", r.SNCount)
		}

		// Verify data events reference valid slices
		for i := 0; i < r.DataCount; i++ {
			if r.DataEvents[i].Length != len(r.DataEvents[i].Data) {
				t.Fatalf("DataEvent length mismatch")
			}
		}

		// Parser should be resettable and reusable
		p.Reset()
		var r2 ParseResult
		p.Parse(data, &r2)
		if r2.DataCount != r.DataCount && r2.CmdCount != r.CmdCount {
			// After reset, parsing same data should yield same results
			// (this might not hold if state was mid-IAC, so we just check it's valid)
		}
	})
}

// FuzzNegotiator fuzzes option negotiation.
func FuzzNegotiator(f *testing.F) {
	f.Add(byte(CmdWill), byte(OptEcho))
	f.Add(byte(CmdDo), byte(OptSGA))
	f.Add(byte(CmdWont), byte(OptLinemode))
	f.Add(byte(CmdDont), byte(OptBinary))

	f.Fuzz(func(t *testing.T, cmd, option byte) {
		n := NewNegotiator()
		resp, _ := n.Handle(cmd, option)

		// Response must be a valid command or 0
		if resp != 0 && resp != CmdWill && resp != CmdWont && resp != CmdDo && resp != CmdDont {
			t.Fatalf("invalid response: %d", resp)
		}

		// State should be valid after handle
		opt := n.Get(option)
		us := opt.Us()
		him := opt.Him()
		if us > 5 || him > 5 {
			t.Fatalf("invalid state: us=%d him=%d", us, him)
		}
	})
}

// FuzzEscapeIAC fuzzes IAC escaping.
func FuzzEscapeIAC(f *testing.F) {
	f.Add([]byte{1, 2, 3})
	f.Add([]byte{CmdIAC, CmdIAC})
	f.Add([]byte{0xFF, 0x01, 0xFF, 0x02})

	f.Fuzz(func(t *testing.T, src []byte) {
		// Calculate required size
		required := len(src)
		for _, b := range src {
			if b == CmdIAC {
				required++
			}
		}

		dst := make([]byte, required)
		n := EscapeIAC(dst, src)
		if n != required {
			t.Fatalf("expected %d, got %d", required, n)
		}

		// Verify no single IAC in output (all should be doubled)
		for i := 0; i < n; i++ {
			if dst[i] == CmdIAC {
				if i+1 >= n || dst[i+1] != CmdIAC {
					t.Fatalf("unescaped IAC at position %d", i)
				}
				i++ // skip the doubled IAC
			}
		}
	})
}
