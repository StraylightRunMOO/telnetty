package telnetty

import (
	"fmt"
	"strconv"
	"strings"
)

// Color represents an ANSI color attribute.
type Color struct {
	Fg       int
	Bg       int
	Attrs    []int
	TrueColor bool
	R, G, B  uint8
}

// ANSI returns the ANSI escape sequence for this color.
func (c Color) ANSI() string {
	parts := make([]string, 0, len(c.Attrs)+3)
	for _, a := range c.Attrs {
		parts = append(parts, strconv.Itoa(a))
	}
	if c.TrueColor {
		parts = append(parts, fmt.Sprintf("38;2;%d;%d;%d", c.R, c.G, c.B))
		if c.Bg >= 0 {
			parts = append(parts, fmt.Sprintf("48;2;%d;%d;%d", c.Bg>>16, (c.Bg>>8)&0xFF, c.Bg&0xFF))
		}
	} else {
		if c.Fg >= 0 {
			parts = append(parts, strconv.Itoa(c.Fg+30))
		}
		if c.Bg >= 0 {
			parts = append(parts, strconv.Itoa(c.Bg+40))
		}
	}
	if len(parts) == 0 {
		return "\x1b[0m"
	}
	return "\x1b[" + strings.Join(parts, ";") + "m"
}

// Color helpers
var (
	Black   = Color{Fg: ColorBlack, Bg: -1}
	Red     = Color{Fg: ColorRed, Bg: -1}
	Green   = Color{Fg: ColorGreen, Bg: -1}
	Yellow  = Color{Fg: ColorYellow, Bg: -1}
	Blue    = Color{Fg: ColorBlue, Bg: -1}
	Magenta = Color{Fg: ColorMagenta, Bg: -1}
	Cyan    = Color{Fg: ColorCyan, Bg: -1}
	White   = Color{Fg: ColorWhite, Bg: -1}
	Default = Color{Fg: ColorDefault, Bg: -1}
)

// Bold returns a bold variant of the color.
func (c Color) Bold() Color {
	c.Attrs = append(c.Attrs, AttrBold)
	return c
}

// Underline returns an underlined variant.
func (c Color) Underline() Color {
	c.Attrs = append(c.Attrs, AttrUnderline)
	return c
}

// Blink returns a blinking variant.
func (c Color) Blink() Color {
	c.Attrs = append(c.Attrs, AttrBlink)
	return c
}

// Reverse returns a reversed variant.
func (c Color) Reverse() Color {
	c.Attrs = append(c.Attrs, AttrReverse)
	return c
}

// On sets the background color.
func (c Color) On(bg int) Color {
	c.Bg = bg
	return c
}

// RGB sets a true-color foreground.
func (c Color) RGB(r, g, b uint8) Color {
	c.TrueColor = true
	c.R, c.G, c.B = r, g, b
	return c
}

// Reset returns the ANSI reset sequence.
func Reset() string {
	return "\x1b[0m"
}

// Sprint returns the string with ANSI color applied.
func (c Color) Sprint(s string) string {
	return c.ANSI() + s + Reset()
}

// Sprintf formats and colors the string.
func (c Color) Sprintf(format string, a ...interface{}) string {
	return c.Sprint(fmt.Sprintf(format, a...))
}

// WriteColor writes colored text to a Conn.
func WriteColor(c *Conn, text string, color Color) (int, error) {
	n1, _ := c.WriteString(color.ANSI())
	n2, err := c.WriteString(text)
	n3, _ := c.WriteString(Reset())
	return n1 + n2 + n3, err
}

// Color256 returns an xterm-256 color.
func Color256(code uint8) Color {
	return Color{Fg: -1, Bg: -1, Attrs: []int{38, 5, int(code)}}
}

// Bg256 returns an xterm-256 background color.
func Bg256(code uint8) Color {
	return Color{Fg: -1, Bg: -1, Attrs: []int{48, 5, int(code)}}
}

// ParseColor parses an ANSI color string (e.g., "red", "bold", "#ff0000").
func ParseColor(s string) Color {
	s = strings.ToLower(strings.TrimSpace(s))
	switch s {
	case "black":
		return Black
	case "red":
		return Red
	case "green":
		return Green
	case "yellow":
		return Yellow
	case "blue":
		return Blue
	case "magenta":
		return Magenta
	case "cyan":
		return Cyan
	case "white":
		return White
	case "bold":
		return Default.Bold()
	case "underline":
		return Default.Underline()
	}
	// Try hex color
	if strings.HasPrefix(s, "#") && len(s) == 7 {
		var r, g, b uint8
		fmt.Sscanf(s, "#%02x%02x%02x", &r, &g, &b)
		return Default.RGB(r, g, b)
	}
	return Default
}
