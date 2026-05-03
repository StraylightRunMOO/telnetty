package main

import (
	"log"
	"os"
	"os/signal"
	"syscall"

	"github.com/straylightrun/telnetty"
)

func main() {
	server := telnetty.NewServer(":4000",
		telnetty.WithConfig(telnetty.Config{
			EnableEcho: true,
			EnableSGA:  true,
			EnableNAWS: true,
		}),
		telnetty.WithHandler(handleConn),
		telnetty.WithMaxConns(1000),
	)

	// Graceful shutdown
	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, syscall.SIGINT, syscall.SIGTERM)
	go func() {
		<-sigCh
		log.Println("Shutting down...")
		server.Close()
	}()

	log.Println("Echo server listening on :4000")
	if err := server.ListenAndServe(); err != nil {
		log.Printf("Server exited: %v", err)
	}
}

func handleConn(c *telnetty.Conn) {
	defer c.Close()

	c.Write([]byte("Welcome to the Telnetty Echo Server!\r\n"))
	c.Write([]byte("Type 'quit' to disconnect.\r\n\r\n"))
	c.Flush()

	for {
		line, err := c.ReadLine()
		if err != nil {
			return
		}

		if string(line) == "quit" {
			c.Write([]byte("Goodbye!\r\n"))
			c.Flush()
			return
		}

		c.Write([]byte("Echo: "))
		c.Write(line)
		c.Write([]byte("\r\n"))
		c.Flush()
	}
}
