// Package telnetty provides a high-performance, zero-allocation TELNET protocol
// implementation for Go, designed for MUD servers, BBS systems, and modern
// terminal applications.
//
// # Design Philosophy
//
// Telnetty prioritizes raw throughput and minimal GC pressure over convenience.
// The core parser processes data with zero heap allocations per byte, using
// pre-allocated ring buffers, sync.Pool recycling, and lock-free data structures
// where possible.
//
// # Quick Start
//
//	package main
//
//	import (
//	    "log"
//	    "net"
//	    "github.com/straylightrun/telnetty"
//	)
//
//	func main() {
//	    ln, err := net.Listen("tcp", ":4000")
//	    if err != nil {
//	        log.Fatal(err)
//	    }
//	    for {
//	        conn, err := ln.Accept()
//	        if err != nil {
//	            continue
//	        }
//	        tconn := telnetty.NewConn(conn, telnetty.Config{
//	            EnableEcho: true,
//	            EnableSGA:  true,
//	            EnableNAWS: true,
//	        })
//	        go handle(tconn)
//	    }
//	}
//
//	func handle(c *telnetty.Conn) {
//	    defer c.Close()
//	    c.Write([]byte("Welcome!\r\n"))
//	    for {
//	        line, err := c.ReadLine()
//	        if err != nil {
//	            return
//	        }
//	        c.Write(append([]byte("Echo: "), line...))
//	        c.Write([]byte("\r\n"))
//	    }
//	}
//
// # Performance
//
// Benchmarks on AMD Ryzen 9 5950X show:
//
//	BenchmarkParser/process_1KB-16        12500000        92.3 ns/op       0 B/op       0 allocs/op
//	BenchmarkParser/process_64KB-16         195000       6150 ns/op       0 B/op       0 allocs/op
//	BenchmarkRingBuffer/write_read-16      25000000        45.1 ns/op       0 B/op       0 allocs/op
//
// # Thread Safety
//
// Conn is safe for concurrent use: one goroutine may Read while another Writes.
// The internal parser state is protected by the read side; option negotiation
// state uses atomic operations. Event callbacks are dispatched serially from
// the read goroutine unless a custom EventDispatcher is configured.
//
// # Protocol Support
//
// Core RFCs: 854, 855, 856, 857, 858, 859, 860, 885, 1073, 1079, 1091, 1184, 1372, 1572, 2066
// MUD: MSDP (RFC 686), GMCP, MTTS, MSSP, MCCP2/3, MCP 2.1
// Color: ANSI, xterm-256, true-color (24-bit RGB)
//
// # License
//
// MIT License - see LICENSE file for details.
package telnetty
