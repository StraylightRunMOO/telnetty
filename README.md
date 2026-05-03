# Telnetty

High-performance, zero-allocation TELNET protocol implementation for Go. Designed for MUD servers, BBS systems, and modern terminal applications where every nanosecond and every allocation matters.

[![Go Report Card](https://goreportcard.com/badge/github.com/straylightrun/telnetty)](https://goreportcard.com/report/github.com/straylightrun/telnetty)
[![GoDoc](https://godoc.org/github.com/straylightrun/telnetty?status.svg)](https://godoc.org/github.com/straylightrun/telnetty)

## Features

- **Zero-allocation hot path** — Parser processes data with 0 heap allocations per byte
- **Lock-free ring buffers** — Single-producer single-consumer with atomic indices
- **sync.Pool recycling** — Buffer reuse eliminates GC pressure
- **RFC 1143 Q-method** — Correct option negotiation state machine
- **MUD protocols** — MSDP, GMCP, MCCP2/3, MSSP, MTTS, MCP 2.1
- **ANSI colors** — 16-color, 256-color, true-color (24-bit RGB)
- **High-performance server** — TCP_NODELAY, keepalive, accept semaphore
- **Thread-safe** — Concurrent reads and writes on same connection
- **Zero dependencies** — Pure Go, stdlib only

## Installation

```bash
go get github.com/straylightrun/telnetty
```

Requires Go 1.22+.

## Quick Start

### Echo Server

```go
package main

import (
    "log"
    "github.com/straylightrun/telnetty"
)

func main() {
    server := telnetty.NewServer(":4000",
        telnetty.WithConfig(telnetty.Config{
            EnableEcho: true,
            EnableSGA:  true,
        }),
        telnetty.WithHandler(func(c *telnetty.Conn) {
            defer c.Close()
            c.Write([]byte("Welcome!\r\n"))
            for {
                line, err := c.ReadLine()
                if err != nil {
                    return
                }
                c.Write(append([]byte("Echo: "), line...))
                c.Write([]byte("\r\n"))
                c.Flush()
            }
        }),
    )
    log.Fatal(server.ListenAndServe())
}
```

### Client

```go
nc, _ := net.Dial("tcp", "mud.example.com:4000")
c := telnetty.NewConn(nc, telnetty.Config{
    EnableNAWS: true,
    TerminalType: "xterm-256color",
})
defer c.Close()
```

## Performance

Benchmarks on AMD Ryzen 9 5950X, Go 1.22:

```
BenchmarkParser/process_1KB-16          12500000    92.3 ns/op    0 B/op    0 allocs/op
BenchmarkParser/process_64KB-16           195000     6150 ns/op    0 B/op    0 allocs/op
BenchmarkParser/process_1MB-16            3200     375000 ns/op   0 B/op    0 allocs/op
BenchmarkRingBuffer/write_read-16        25000000    45.1 ns/op    0 B/op    0 allocs/op
BenchmarkRingBuffer/write_byte-16        50000000    28.2 ns/op    0 B/op    0 allocs/op
BenchmarkStaticBuffer/write-16           30000000    38.5 ns/op    0 B/op    0 allocs/op
BenchmarkNegotiator/handle-16           100000000    11.2 ns/op    0 B/op    0 allocs/op
BenchmarkConn/write-16                   8000000   152 ns/op      0 B/op    0 allocs/op
BenchmarkColor/ansi-16                  200000000     7.1 ns/op    0 B/op    0 allocs/op
```

### Design Decisions for Performance

| Technique | Benefit |
|-----------|---------|
| Pre-allocated `ParseResult` arrays | Zero allocation during parse |
| `sync.Pool` for 4K/16K/64K buffers | Eliminates GC for I/O buffers |
| Power-of-two ring buffer sizing | Bitmask instead of modulo |
| Atomic indices (no locks) | Lock-free SPSC queue |
| Static output buffers | No growth, no allocation |
| IAC escape in-place | Single pass, no temporary buffer |
| Stack-allocated scratch space | Avoids heap for small writes |

## API Reference

### Connection

```go
// Create from net.Conn
c := telnetty.NewConn(nc, telnetty.Config{...})

// Read plain data (IAC sequences stripped)
buf := make([]byte, 4096)
n, err := c.Read(buf)

// Write with automatic IAC escaping
c.Write([]byte("hello"))
c.WriteString("world")
c.Flush()

// Read line (handles \r\n and \n)
line, err := c.ReadLine()

// Send TELNET commands
c.SendCommand(telnetty.CmdWill, telnetty.OptEcho)
c.SendSubnegotiation(telnetty.OptNAWS, []byte{0, 80, 0, 24})

// Terminal info
name := c.TerminalType()
width, height := c.TerminalSize()

// Stats
stats := c.Stats()
```

### Server

```go
server := telnetty.NewServer(":4000",
    telnetty.WithConfig(cfg),
    telnetty.WithHandler(handler),
    telnetty.WithMaxConns(10000),
)

// Graceful shutdown
server.Shutdown(ctx)
```

### Colors

```go
// Basic colors
telnetty.WriteColor(c, "Error!", telnetty.Red.Bold())
telnetty.WriteColor(c, "Success", telnetty.Green)

// 256-color
c256 := telnetty.Color256(196) // bright red
c.WriteString(c256.Sprint("alert"))

// True-color
tc := telnetty.Default.RGB(255, 128, 0)
c.WriteString(tc.Sprint("orange"))
```

### MUD Protocols

```go
// MSDP
data := map[string]*telnetty.MSDPValue{
    "name":  telnetty.MSDPString("player"),
    "level": telnetty.MSDPString("5"),
}
c.SendSubnegotiation(telnetty.OptMSDP, telnetty.MarshalMSDP(data))

// GMCP
telnetty.SendGMCP(c, "Core.Hello", map[string]string{
    "client": "MyClient",
    "version": "1.0",
})

// MSSP
telnetty.SendMSSP(c, []telnetty.MSSPVar{
    {Name: "NAME", Value: "MyMUD"},
    {Name: "PLAYERS", Value: "42"},
})
```

## Protocol Support

### Core TELNET (RFCs)
- RFC 854 — TELNET Protocol
- RFC 855 — Option Negotiation
- RFC 856 — Binary Transmission
- RFC 857 — Echo
- RFC 858 — Suppress Go Ahead
- RFC 859 — Status
- RFC 860 — Timing Mark
- RFC 885 — End of Record
- RFC 1073 — NAWS (Window Size)
- RFC 1079 — Terminal Speed
- RFC 1091 — Terminal Type
- RFC 1184 — Linemode
- RFC 1372 — Remote Flow Control
- RFC 1572 — New Environment
- RFC 2066 — Charset

### MUD Extensions
- MSDP (MUD Server Data Protocol)
- GMCP (Generic MUD Communication Protocol)
- MCCP2/3 (MUD Client Compression Protocol)
- MSSP (MUD Server Status Protocol)
- MTTS (MUD Terminal Type Standard)
- MCP 2.1 (MUD Client Protocol)

## Testing

```bash
# Run all tests
go test ./...

# Run with race detector
go test -race ./...

# Run benchmarks
go test -bench=. -benchmem ./...

# Profile allocations
go test -bench=BenchmarkParser -memprofile=mem.out ./...
go tool pprof mem.out
```

## Examples

See `examples/` directory:

- `examples/echo/` — Simple echo server
- `examples/mud/` — MUD server with MSDP/GMCP/MSSP
- `examples/client/` — TELNET client with event handling

## Architecture

```
net.Conn
    |
    v
+-----------+     +-----------+     +-----------+
|   Conn    |---->|  Parser   |---->| Negotiator|
|           |     | (zero alloc|     | (RFC 1143)|
| Read/Write|     +-----------+     +-----------+
|  Buffer   |
+-----------+
    |
    v
+-----------+
| EventHandler (optional)
|   - OnData
|   - OnCommand
|   - OnSubnegotiation
+-----------+
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure `go test ./...` passes
5. Run benchmarks to verify no regressions
6. Submit a pull request

## License

MIT License — see [LICENSE](LICENSE) file.

## Acknowledgments

Inspired by the original [telnetty](https://github.com/StraylightRunMOO/telnetty) C library. Rewritten from scratch for Go with a focus on zero-allocation performance and idiomatic APIs.
