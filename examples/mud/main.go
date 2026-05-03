package main

import (
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/straylightrun/telnetty"
)

type Player struct {
	Name  string
	Level int
	HP    int
	MaxHP int
}

func main() {
	server := telnetty.NewServer(":8888",
		telnetty.WithConfig(telnetty.Config{
			EnableEcho:     false, // We handle echo
			EnableSGA:      false,
			EnableNAWS:     true,
			EnableTType:    true,
			EnableMSDP:     true,
			EnableGMCP:     true,
			EnableMSSP:     true,
			EnableMCCP2:    true,
			TerminalWidth:  80,
			TerminalHeight: 24,
		}),
		telnetty.WithHandler(handleMUDConn),
		telnetty.WithMaxConns(500),
	)

	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, syscall.SIGINT, syscall.SIGTERM)
	go func() {
		<-sigCh
		log.Println("Shutting down MUD server...")
		server.Close()
	}()

	log.Println("MUD server listening on :8888")
	if err := server.ListenAndServe(); err != nil {
		log.Printf("Server exited: %v", err)
	}
}

func handleMUDConn(c *telnetty.Conn) {
	defer c.Close()

	// Send MSSP info
	telnetty.SendMSSP(c, []telnetty.MSSPVar{
		{Name: "NAME", Value: "TelnettyMUD"},
		{Name: "PLAYERS", Value: "1"},
		{Name: "UPTIME", Value: fmt.Sprintf("%d", time.Now().Unix())},
	})
	c.Flush()

	// Send GMCP core hello
	telnetty.SendGMCP(c, "Core.Hello", map[string]string{
		"client": "TelnettyMUD",
		"version": "1.0.0",
	})
	c.Flush()

	// Colorful welcome
	telnetty.WriteColor(c, "Welcome to TelnettyMUD!\r\n", telnetty.Cyan.Bold())
	telnetty.WriteColor(c, "=========================\r\n\r\n", telnetty.Yellow)
	c.Flush()

	player := &Player{
		Name:  "Adventurer",
		Level: 1,
		HP:    100,
		MaxHP: 100,
	}

	for {
		c.Write([]byte("> "))
		c.Flush()

		line, err := c.ReadLine()
		if err != nil {
			return
		}

		cmd := string(line)
		switch cmd {
		case "look":
			telnetty.WriteColor(c, "You are in a vast digital realm.\r\n", telnetty.Green)
		case "score":
			// Send MSDP update
			msdpData := map[string]*telnetty.MSDPValue{
				"name":  telnetty.MSDPString(player.Name),
				"level": telnetty.MSDPString(fmt.Sprintf("%d", player.Level)),
				"hp":    telnetty.MSDPString(fmt.Sprintf("%d", player.HP)),
				"maxhp": telnetty.MSDPString(fmt.Sprintf("%d", player.MaxHP)),
			}
			c.SendSubnegotiation(telnetty.OptMSDP, telnetty.MarshalMSDP(msdpData))
			c.Flush()

			c.Write([]byte(fmt.Sprintf("Name: %s  Level: %d  HP: %d/%d\r\n",
				player.Name, player.Level, player.HP, player.MaxHP)))
		case "quit":
			telnetty.WriteColor(c, "Farewell, brave adventurer!\r\n", telnetty.Magenta)
			c.Flush()
			return
		default:
			c.Write([]byte("Unknown command. Try: look, score, quit\r\n"))
		}
		c.Flush()
	}
}
