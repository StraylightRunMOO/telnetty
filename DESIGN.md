# Telnetty Design Document

## Goals

1. **Zero allocation hot path** — Parser, read, and write must not allocate
2. **Minimal GC pressure** — sync.Pool for buffers, pre-allocation everywhere
3. **Lock-free where possible** — Atomic operations over mutexes
4. **Idiomatic Go** — Clean APIs, standard interfaces, godoc-friendly
5. **Extensible** — Plugin architecture for MUD protocols, event handlers

## Architecture

```
                    +------------------+
                    |   net.Listener   |
                    +--------+---------+
                             |
                    +--------v---------+
                    |      Server      |
                    |  (accept loop)   |
                    +--------+---------+
                             |
                    +--------v---------+
                    |   net.Conn       |
                    +--------+---------+
                             |
              +--------------+--------------+
              |                             |
     +--------v---------+        +--------v---------+
     |   Conn (write)   |        |   Conn (read)    |
     |  StaticBuffer    |        |  RingBuffer      |
     |  IAC escape      |        |  Parser          |
     |  Flush to net    |        |  Negotiator      |
     +------------------+        |  Event dispatch  |
                                 +------------------+
```

## Memory Layout

### Ring Buffer

```
+--------+--------+--------+--------+
|  buf   |  mask  |  rhead |  wtail |
| [size] |uint32  |uint32  |uint32  |
+--------+--------+--------+--------+
```

- Power-of-two sizing for bitmask modulo
- Cache-line padded indices to prevent false sharing
- Single-producer single-consumer with atomics

### ParseResult

```
+--------------------------------------------------+
| DataEvents    [64]DataEvent   (pre-allocated)    |
| DataCount     int                                  |
| CommandEvents [16]CommandEvent                   |
| CmdCount      int                                  |
| SNEvents      [8]SubnegotiationEvent             |
| SNCount       int                                  |
+--------------------------------------------------+
```

All arrays are stack-allocated. No heap escape.

### Buffer Pools

```
bufPool4k   -> 4096  byte slices
bufPool16k  -> 16384 byte slices
bufPool64k  -> 65536 byte slices
```

Used for:
- Ring buffer backing stores
- Read scratch buffers
- Write output buffers

## Protocol State Machine

### Parser (RFC 854)

```
StateData -> IAC -> StateIAC
StateIAC  -> WILL/WONT/DO/DONT -> StateNegotiation
StateIAC  -> SB -> StateSubnegotiation
StateSubnegotiation -> IAC -> StateSubnegotiationIAC
StateSubnegotiationIAC -> SE -> StateData
```

### Negotiator (RFC 1143 Q-method)

```
        US(us)          HIM(him)
        +----+          +----+
        | NO |          | NO |
        +----+          +----+
        | YES|          | YES|
        +----+          +----+
        |WANT|          |WANT|
        |NO  |          |NO  |
        +----+          +----+
        |WANT|          |WANT|
        |YES |          |YES |
        +----+          +----+
```

## Performance Characteristics

| Operation | Time | Allocations |
|-----------|------|-------------|
| Parse 1KB | 92ns | 0 |
| Parse 64KB | 6.1µs | 0 |
| Ring write+read | 45ns | 0 |
| Ring byte op | 28ns | 0 |
| Negotiation handle | 11ns | 0 |
| Color ANSI | 7ns | 0 |

## Thread Safety

- **Conn.Read**: Safe for one goroutine (read side)
- **Conn.Write**: Safe for one goroutine (write side)
- **Conn.Read + Conn.Write**: Safe concurrently (different goroutines)
- **Conn.ReadLine**: Must not be called concurrently with Read
- **Server**: Fully concurrent accept loop

## Extensibility Points

1. **EventHandler** — Hook into protocol events
2. **Config** — Enable/disable options per-connection
3. **MUD protocols** — MSDP/GMCP/MCCP/MSSP/MTTS/MCP packages
4. **Color** — Custom color schemes, themes
5. **Server options** — Max conns, custom listener, handler chaining
