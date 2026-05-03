# Telnetty Examples

## Echo Server

Simple echo server demonstrating basic read/write:

```bash
go run ./echo
```

Connect with: `telnet localhost 4000`

## MUD Server

Full MUD server with MSDP, GMCP, MSSP, colors:

```bash
go run ./mud
```

Commands: `look`, `score`, `quit`

## Client

TELNET client with event logging:

```bash
go run ./client localhost:4000
```
