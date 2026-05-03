package main

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"os"

	"github.com/straylightrun/telnetty"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: client <host:port>")
		os.Exit(1)
	}

	addr := os.Args[1]
	nc, err := net.Dial("tcp", addr)
	if err != nil {
		log.Fatal(err)
	}

	c := telnetty.NewConn(nc, telnetty.Config{
		EnableEcho:     true,
		EnableSGA:      true,
		EnableNAWS:     true,
		EnableTType:    true,
		TerminalType:   "telnetty-client",
		TerminalWidth:  80,
		TerminalHeight: 24,
		EventHandler: telnetty.EventHandlerFunc{
			CmdFunc: func(c *telnetty.Conn, cmd, option uint8) {
				fmt.Fprintf(os.Stderr, "[CMD] %d %d\n", cmd, option)
			},
			SNFunc: func(c *telnetty.Conn, option uint8, data []byte) {
				fmt.Fprintf(os.Stderr, "[SN] option=%d len=%d\n", option, len(data))
			},
		},
	})
	defer c.Close()

	// Read from server in background
	go func() {
		buf := make([]byte, 4096)
		for {
			n, err := c.Read(buf)
			if err != nil {
				return
			}
			os.Stdout.Write(buf[:n])
		}
	}()

	// Read from stdin and send to server
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := scanner.Text()
		c.Write([]byte(line))
		c.Write([]byte("\r\n"))
		c.Flush()
	}
}
