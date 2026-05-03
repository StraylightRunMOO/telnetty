# Contributing to Telnetty

Thank you for your interest in contributing! This project prioritizes raw
performance and zero-allocation design. Please keep these principles in mind.

## Performance First

- The hot path (parser, read/write) must remain zero-allocation
- Use `sync.Pool` for any necessary allocations
- Prefer stack allocation over heap
- Use atomic operations over locks where possible
- Benchmark before and after any change

## Code Style

- Follow standard Go conventions (gofmt, golint)
- Document all exported types and functions
- Keep functions small and focused
- Add tests for new functionality
- Maintain backward compatibility

## Testing

```bash
# Run all tests
go test -v ./...

# Run with race detector
go test -race ./...

# Run benchmarks
go test -bench=. -benchmem ./...

# Profile allocations
go test -bench=BenchmarkParser -memprofile=mem.out ./...
go tool pprof mem.out
```

## Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Benchmark Regression Policy

Any PR that introduces allocation regressions in the hot path will be
questioned. Please include benchmark comparisons showing:

- `ns/op` before/after
- `B/op` before/after  
- `allocs/op` before/after

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
