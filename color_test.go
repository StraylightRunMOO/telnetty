package telnetty

import (
	"strings"
	"testing"
)

func TestColorANSI(t *testing.T) {
	c := Red
	seq := c.ANSI()
	if !strings.Contains(seq, "[") {
		t.Fatalf("expected ANSI escape, got %q", seq)
	}
	if !strings.Contains(seq, "31") {
		t.Fatalf("expected color code 31, got %q", seq)
	}
}

func TestColorBold(t *testing.T) {
	c := Red.Bold()
	seq := c.ANSI()
	if !strings.Contains(seq, "1;") {
		t.Fatalf("expected bold attribute, got %q", seq)
	}
}

func TestColorUnderline(t *testing.T) {
	c := Blue.Underline()
	seq := c.ANSI()
	if !strings.Contains(seq, "4;") {
		t.Fatalf("expected underline attribute, got %q", seq)
	}
}

func TestColorOn(t *testing.T) {
	c := White.On(ColorBlue)
	seq := c.ANSI()
	if !strings.Contains(seq, "44") {
		t.Fatalf("expected bg code 44, got %q", seq)
	}
}

func TestColorRGB(t *testing.T) {
	c := Default.RGB(255, 128, 0)
	seq := c.ANSI()
	if !strings.Contains(seq, "38;2;255;128;0") {
		t.Fatalf("expected truecolor sequence, got %q", seq)
	}
}

func TestColorSprint(t *testing.T) {
	c := Green
	result := c.Sprint("hello")
	if !strings.HasPrefix(result, "[") {
		t.Fatalf("expected ANSI prefix, got %q", result)
	}
	if !strings.HasSuffix(result, "[0m") {
		t.Fatalf("expected reset suffix, got %q", result)
	}
	if !strings.Contains(result, "hello") {
		t.Fatalf("expected content 'hello', got %q", result)
	}
}

func TestColor256(t *testing.T) {
	c := Color256(196)
	seq := c.ANSI()
	if !strings.Contains(seq, "38;5;196") {
		t.Fatalf("expected 256-color sequence, got %q", seq)
	}
}

func TestParseColor(t *testing.T) {
	tests := []struct {
		input    string
		expected int
	}{
		{"red", ColorRed},
		{"green", ColorGreen},
		{"blue", ColorBlue},
		{"bold", -1},
		{"#ff0000", -1},
	}
	for _, tt := range tests {
		c := ParseColor(tt.input)
		if tt.expected >= 0 && c.Fg != tt.expected {
			t.Fatalf("ParseColor(%q): expected fg=%d, got %d", tt.input, tt.expected, c.Fg)
		}
	}
}

func TestReset(t *testing.T) {
	r := Reset()
	if r != "[0m" {
		t.Fatalf("expected reset sequence, got %q", r)
	}
}

func BenchmarkColorANSI(b *testing.B) {
	c := Red.Bold().Underline()
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = c.ANSI()
	}
}
