# telnetty

A high-performance, modular, headers-only implementation of the TELNET protocol with comprehensive support for MUD-specific extensions, compression, and modern terminal features.

## Features

### Core TELNET Protocol
- **RFC 854/855 Compliant**: Full compliance with TELNET protocol standards
- **Event-Driven Architecture**: Clean separation of concerns with callback-based event handling
- **State Machine Implementation**: Robust protocol state management
- **Zero-Copy Operations**: Optimized for high-throughput scenarios
- **Thread-Safe Options**: Optional thread safety for multi-threaded applications

### TELNET Options
- **Binary Transmission** (RFC 856): 8-bit clean data transmission
- **Echo** (RFC 857): Local and remote echo control
- **Suppress Go Ahead** (RFC 858): Half-duplex communication
- **Status** (RFC 859): Option status reporting
- **Timing Mark** (RFC 860): Synchronization support
- **Terminal Type** (RFC 1091): Terminal capability detection
- **Window Size** (RFC 1073): NAWS (Negotiate About Window Size)
- **Terminal Speed** (RFC 1079): Speed negotiation
- **Remote Flow Control** (RFC 1372): Flow control management
- **Linemode** (RFC 1184): Line-at-a-time mode
- **Environment Variables** (RFC 1572): NEW-ENVIRON support
- **Charset** (RFC 2066): Character set negotiation
- **End of Record** (RFC 885): Prompt marking

### MUD-Specific Protocols
- **MSDP** (MUD Server Data Protocol): Structured data exchange
- **GMCP** (Generic MUD Communication Protocol): JSON-based messaging
- **MTTS** (MUD Terminal Type Standard): Client capability detection
- **MSSP** (MUD Server Status Protocol): Server information broadcasting
- **MCCP2/MCCP3** (MUD Client Compression Protocol): zlib compression
- **MCP 2.1** (MUD Client Protocol): Structured communication protocol

### Advanced Features
- **Color Support**: ANSI, 256-color, and true color (24-bit RGB)
- **Buffer Management**: Efficient buffer pooling and zero-copy chains
- **Event System**: Extensible event handling with priorities
- **Compression**: Automatic compression negotiation and management
- **Performance Monitoring**: Built-in statistics and metrics
- **Memory Management**: Custom allocators and memory tracking

## Architecture

### Header Structure

The library is organized as modular headers that can be included independently:

```
telnetty_core.h      - Core TELNET protocol implementation
telnetty_events.h    - Event system and queue management
telnetty_buffer.h    - High-performance buffer management
telnetty_options.h   - Standard TELNET option implementations
telnetty_mud.h       - MUD-specific protocol implementations
telnetty_compression.h - Compression protocol support
telnetty_mcp.h       - MCP 2.1 protocol implementation
telnetty_color.h     - Terminal color support
```

### Design Principles

1. **Modularity**: Each protocol is implemented as a separate, optional header
2. **Performance**: Optimized for high-throughput scenarios with zero-copy operations
3. **Extensibility**: Easy to add new protocols and features
4. **Compatibility**: Works with existing TELNET clients and servers
5. **Thread Safety**: Optional thread-safe operations for multi-threaded applications

## Quick Start

### Building the Library

```bash
# Clone the repository
git clone https://github.com/StraylightRunMOO/telnetty.git
cd telnetty

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DBUILD_EXAMPLES=ON

# Build
make -j$(nproc)

# Install (optional)
sudo make install
```

### Dependencies

- **Required**: C99 compatible compiler
- **Optional**: zlib (for compression support)
- **Optional**: libuv (for high-performance server example)
- **Optional**: CMake 3.10+ (for building)

### Basic Server Example

```c
#include "telnetty_core.h"
#include "telnetty_options.h"
#include "telnetty_color.h"

void handle_event(telnetty_context_t* ctx, telnetty_event_type_t event, 
                  const telnetty_event_data_union_t* data, void* user_data) {
    switch (event) {
        case TELNETTY_EVENT_DATA:
            /* Echo back with color */
            telnetty_color_send(ctx, "Echo: ", TELNETTY_COLOR_CYAN, TELNETTY_COLOR_BLACK, 0);
            telnetty_send(ctx, data->data.data, data->data.length);
            break;
            
        case TELNETTY_EVENT_CONNECT:
            /* Send welcome message */
            telnetty_color_send(ctx, "Welcome!\r\n", 
                            TELNETTY_COLOR_GREEN, TELNETTY_COLOR_BLACK, 0);
            break;
    }
}

int main() {
    telnetty_context_t* ctx = telnetty_create(handle_event, NULL);
    
    /* Enable options */
    telnetty_enable_echo(ctx);
    telnetty_enable_color(ctx, true);
    
    /* Your server loop here */
    
    telnetty_destroy(ctx);
    return 0;
}
```

## Example Programs

### Simple Server (`example_server_simple`)

A single-threaded TELNET server demonstrating basic functionality:

```bash
# Run the simple server on port 4000
./example_server_simple 4000

# Connect with a TELNET client
telnet localhost 4000
```

### Simple Client (`example_client_simple`)

A TELNET client with interactive terminal support:

```bash
# Connect to a TELNET server
./example_client_simple localhost 4000

# Connect to a MUD
./example_client_simple mud.example.com 23
```

### High-Performance Server (`example_server_libuv`)

A multi-threaded server using libuv for maximum performance:

```bash
# Run with 4 threads on port 4000
./example_server_libuv 4000 4

# Run with custom settings
./example_server_libuv 4000 8  # 8 threads
```

## Usage Guide

### Including the Library

Include only the headers you need:

```c
#include "telnetty_core.h"      /* Core functionality (required) */
#include "telnetty_options.h"   /* Standard TELNET options */
#include "telnetty_mud.h"       /* MUD protocols */
#include "telnetty_color.h"     /* Color support */
```

### Creating a Context

```c
telnetty_context_t* ctx = telnetty_create(event_handler, user_data);
if (!ctx) {
    /* Handle error */
}
```

### Processing Data

```c
int processed = telnetty_process(ctx, data, length);
if (processed < 0) {
    /* Handle error */
}
```

### Sending Data

```c
/* Send raw data */
telnetty_send(ctx, data, length);

/* Send TELNET command */
telnetty_send_command(ctx, TELNETTY_IAC);

/* Send option negotiation */
telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_ECHO);
```

### Enabling Features

```c
/* Enable standard options */
telnetty_enable_echo(ctx);
telnetty_enable_naws(ctx);
telnetty_enable_ttype(ctx);

/* Enable MUD protocols */
telnetty_enable_msdp(ctx, msdp_handler, user_data);
telnetty_enable_gmcp(ctx, gmcp_handler, user_data);

/* Enable compression */
telnetty_enable_mccp2(ctx, 6);  /* Level 6 compression */
telnetty_enable_mccp3(ctx, 6);

/* Enable color support */
telnetty_color_init(ctx, true);
```

## Protocol Support

### TELNET Options

| Option | Code | Description |
|--------|------|-------------|
| BINARY | 0 | 8-bit data transmission |
| ECHO | 1 | Local/remote echo |
| SGA | 3 | Suppress Go Ahead |
| STATUS | 5 | Option status |
| TIMING-MARK | 6 | Synchronization |
| TTYPE | 24 | Terminal type |
| NAWS | 31 | Window size |
| NEW_ENVIRON | 39 | Environment variables |
| CHARSET | 42 | Character sets |
| EOR | 25 | End of record |

### MUD Protocols

| Protocol | Code | Description |
|----------|------|-------------|
| MSDP | 69 | Structured data exchange |
| GMCP | 201 | JSON messaging |
| MSSP | 70 | Server status |
| MCCP2 | 86 | Compression v2 |
| MCCP3 | 87 | Compression v3 |
| MCP | 200 | MUD Client Protocol |

## Performance Features

### Memory Management
- **Buffer Pooling**: Pre-allocated buffers reduce malloc/free overhead
- **Zero-Copy Operations**: Direct buffer manipulation where possible
- **Custom Allocators**: User-provided memory allocation functions
- **Memory Tracing**: Optional memory usage tracking

### Optimization Features
- **Fast Path Optimization**: Optimized paths for common operations
- **Vectorized Operations**: SIMD instructions where supported
- **State Machine Optimization**: Table-driven state transitions
- **Branch Prediction**: Optimized code layout

### Threading Support
- **Thread-Safe Options**: Optional per-context locks
- **Atomic Operations**: Lock-free algorithms where possible
- **Multi-Threaded Examples**: High-performance server implementation

## Configuration Options

### Compilation Flags

```cmake
# Enable zlib compression support
-DENABLE_ZLIB=ON

# Enable debug output
-DENABLE_DEBUG=ON

# Build examples
-DBUILD_EXAMPLES=ON

# Build tests
-DBUILD_TESTS=ON
```

### Runtime Configuration

```c
/* Set buffer sizes */
#define TELNETTY_MAX_BUFFER_SIZE (64 * 1024)
#define TELNETTY_INITIAL_BUFFER_SIZE 4096

/* Set compression level */
#define TELNETTY_COMPRESSION_LEVEL 6

/* Set timeouts */
#define TELNETTY_OPTION_TIMEOUT 5000
```

## API Reference

### Core Functions

| Function | Description |
|----------|-------------|
| `telnetty_create()` | Create a new TELNET context |
| `telnetty_destroy()` | Destroy a TELNET context |
| `telnetty_process()` | Process incoming TELNET data |
| `telnetty_send()` | Send data through the connection |
| `telnetty_send_command()` | Send a TELNET command |
| `telnetty_send_option()` | Send option negotiation |
| `telnetty_register_option()` | Register an option handler |

### Event Functions

| Function | Description |
|----------|-------------|
| `telnetty_fire_event()` | Fire an event |
| `telnetty_fire_event_ex()` | Fire an event with priority |
| `telnetty_fire_custom_event()` | Fire a custom event |
| `telnetty_process_events()` | Process queued events |

### Color Functions

| Function | Description |
|----------|-------------|
| `telnetty_color_init()` | Initialize color support |
| `telnetty_color_set_foreground()` | Set foreground color |
| `telnetty_color_set_background()` | Set background color |
| `telnetty_color_send()` | Send colored text |
| `telnetty_color_reset()` | Reset all colors |

## Testing

### Unit Tests

```bash
# Build with tests enabled
cmake .. -DBUILD_TESTS=ON
make

# Run tests
make test
ctest --verbose
```

### Integration Tests

The example programs serve as integration tests:

```bash
# Test server and client together
./example_server_simple 4000 &
sleep 1
./example_client_simple localhost 4000
```

## Troubleshooting

### Common Issues

1. **Compilation Errors**
   - Ensure C99 compiler is available
   - Check for missing dependencies (zlib, libuv)
   - Verify CMake version (3.10+ required)

2. **Connection Problems**
   - Check firewall settings
   - Verify server is listening on correct port
   - Check for port conflicts

3. **Protocol Issues**
   - Enable debug output with `-DENABLE_DEBUG=ON`
   - Check TELNET option negotiation
   - Verify client compatibility

### Debug Output

Enable debug output for troubleshooting:

```cmake
cmake .. -DENABLE_DEBUG=ON
```

This enables:
- Memory allocation tracking
- Protocol debugging
- Event logging
- Error reporting

## Contributing

### Code Style

- Follow C99 standard
- Use consistent indentation (4 spaces)
- Document all public functions
- Include error checking
- Use meaningful variable names

### Testing

- Include unit tests for new features
- Update integration tests
- Test with multiple TELNET clients
- Verify protocol compliance

## License

This library is released under the [MIT License](LICENSE). See the LICENSE file for details.

## Acknowledgments

- **MTH Library** (https://github.com/scandum/mth) - MUD Telnet Handler
- **libtelnet** (https://github.com/seanmiddleditch/libtelnet) - TELNET protocol library
- **MCP 2.1 Specification** (https://www.moo.mud.org/mcp/mcp2.html)
- **zlib** (https://zlib.net/) - Compression library
- **libuv** (https://libuv.org/) - Cross-platform asynchronous I/O

## Support

For questions, bug reports, or feature requests:

- **GitHub Issues**: https://github.com/yourusername/unified-telnet/issues
- **Documentation**: See the `docs/` directory
- **Examples**: Check the `examples/` directory for usage patterns

---

**Note**: This is a headers-only implementation. Simply include the headers you need and define the appropriate implementation macro before including them.
