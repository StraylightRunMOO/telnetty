package telnetty

import (
	"strings"
	"unicode"
	"unicode/utf8"
)

// StripANSI removes ANSI escape sequences from text.
func StripANSI(s string) string {
	var b strings.Builder
	b.Grow(len(s))
	for i := 0; i < len(s); i++ {
		if s[i] == '' && i+1 < len(s) {
			i++
			if s[i] == '[' {
				i++ // skip '['
				// CSI sequence: consume until final byte (0x40-0x7E)
				for i < len(s) && !(s[i] >= 0x40 && s[i] <= 0x7E) {
					i++
				}
			}
			// For non-CSI sequences, the second byte is the command
			continue
		}
		b.WriteByte(s[i])
	}
	return b.String()
}

// WordWrap wraps text to maxWidth runes per line, preserving words.
func WordWrap(text string, maxWidth int) []string {
	if maxWidth <= 0 {
		maxWidth = 80
	}
	var lines []string
	var line strings.Builder
	var lineWidth int

	words := strings.Fields(text)
	for _, word := range words {
		wordWidth := StringWidth(word)
		if lineWidth > 0 && lineWidth+1+wordWidth > maxWidth {
			lines = append(lines, line.String())
			line.Reset()
			lineWidth = 0
		}
		if lineWidth > 0 {
			line.WriteByte(' ')
			lineWidth++
		}
		line.WriteString(word)
		lineWidth += wordWidth
	}
	if line.Len() > 0 {
		lines = append(lines, line.String())
	}
	return lines
}

// StringWidth returns the display width of a string (accounts for wide chars).
func StringWidth(s string) int {
	width := 0
	for _, r := range s {
		if unicode.Is(unicode.Mn, r) {
			continue // zero-width combining marks
		}
		w := utf8.RuneLen(r)
		if w > 1 {
			width += 2 // wide chars
		} else {
			width += 1
		}
	}
	return width
}

// PadRight pads a string to width with spaces on the right.
func PadRight(s string, width int) string {
	w := StringWidth(s)
	if w >= width {
		return s
	}
	return s + strings.Repeat(" ", width-w)
}

// PadLeft pads a string to width with spaces on the left.
func PadLeft(s string, width int) string {
	w := StringWidth(s)
	if w >= width {
		return s
	}
	return strings.Repeat(" ", width-w) + s
}

// Center centers a string within width.
func Center(s string, width int) string {
	w := StringWidth(s)
	if w >= width {
		return s
	}
	left := (width - w) / 2
	right := width - w - left
	return strings.Repeat(" ", left) + s + strings.Repeat(" ", right)
}

// Truncate truncates a string to maxWidth runes, adding ellipsis if truncated.
func Truncate(s string, maxWidth int, ellipsis string) string {
	w := StringWidth(s)
	if w <= maxWidth {
		return s
	}
	ew := StringWidth(ellipsis)
	if ew >= maxWidth {
		return ellipsis
	}
	var b strings.Builder
	b.Grow(maxWidth)
	width := 0
	for _, r := range s {
		rw := 1
		if utf8.RuneLen(r) > 1 {
			rw = 2
		}
		if width+rw > maxWidth-ew {
			b.WriteString(ellipsis)
			break
		}
		b.WriteRune(r)
		width += rw
	}
	return b.String()
}

// IsPrintable returns true if all runes in s are printable.
func IsPrintable(s string) bool {
	for _, r := range s {
		if !unicode.IsPrint(r) && !unicode.IsSpace(r) {
			return false
		}
	}
	return true
}

// Sanitize removes non-printable characters except standard whitespace.
func Sanitize(s string) string {
	var b strings.Builder
	b.Grow(len(s))
	for _, r := range s {
		if unicode.IsPrint(r) || r == '\n' || r == '\r' || r == '\t' {
			b.WriteRune(r)
		}
	}
	return b.String()
}

// Repeat repeats a string n times.
func Repeat(s string, n int) string {
	return strings.Repeat(s, n)
}

// Box draws a text box with borders.
func Box(lines []string, width int) string {
	if width <= 0 {
		for _, line := range lines {
			if w := StringWidth(line); w > width {
				width = w
			}
		}
		width += 4
	}
	var b strings.Builder
	top := "+" + strings.Repeat("-", width-2) + "+"
	b.WriteString(top)
	b.WriteByte('\n')
	for _, line := range lines {
		b.WriteString("| ")
		b.WriteString(PadRight(line, width-4))
		b.WriteString(" |\n")
	}
	b.WriteString(top)
	return b.String()
}

// ProgressBar returns an ASCII progress bar.
func ProgressBar(current, total, width int) string {
	if total <= 0 {
		return "[" + strings.Repeat("-", width) + "]"
	}
	if current > total {
		current = total
	}
	filled := current * width / total
	return "[" + strings.Repeat("=", filled) + strings.Repeat("-", width-filled) + "]"
}

// SplitLines splits on \r\n, \n, or \r.
func SplitLines(data []byte, atEOF bool) (advance int, token []byte, err error) {
	if atEOF && len(data) == 0 {
		return 0, nil, nil
	}
	for i := 0; i < len(data); i++ {
		if data[i] == '\n' {
			return i + 1, data[:i], nil
		}
		if data[i] == '\r' {
			end := i
			if i+1 < len(data) && data[i+1] == '\n' {
				return i + 2, data[:end], nil
			}
			return i + 1, data[:end], nil
		}
	}
	if atEOF {
		return len(data), data, nil
	}
	return 0, nil, nil
}

// ScanLines is a bufio.SplitFunc that handles TELNET line endings.
func ScanLines(data []byte, atEOF bool) (advance int, token []byte, err error) {
	return SplitLines(data, atEOF)
}
