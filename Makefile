.PHONY: all test bench race lint clean cover pprof

all: test

test:
	go test -v ./...

bench:
	go test -bench=. -benchmem -count=5 ./...

race:
	go test -race -v ./...

lint:
	golangci-lint run ./...

cover:
	go test -coverprofile=coverage.out ./...
	go tool cover -html=coverage.out -o coverage.html

pprof-mem:
	go test -bench=BenchmarkParser -memprofile=mem.out ./...
	go tool pprof mem.out

pprof-cpu:
	go test -bench=BenchmarkParser -cpuprofile=cpu.out ./...
	go tool pprof cpu.out

clean:
	rm -f coverage.out coverage.html mem.out cpu.out
	go clean -testcache

build-examples:
	go build -o bin/echo ./examples/echo
	go build -o bin/mud ./examples/mud
	go build -o bin/client ./examples/client

run-echo:
	go run ./examples/echo

run-mud:
	go run ./examples/mud
