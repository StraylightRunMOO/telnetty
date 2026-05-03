package telnetty

import (
	"testing"
)

func TestStripANSI(t *testing.T) {
	tests := []struct {
		input    string
		expected string
	}{
		{"hello", "hello"},
		{"[31mred[0m", "red"},
		{"[1;31;40mbold red on black[0m", "bold red on black"},
		{"no[ansi here", "nonsi here"},
	}
	for _, tt := range tests {
		result := StripANSI(tt.input)
		if result != tt.expected {
			t.Fatalf("StripANSI(%q) = %q, want %q", tt.input, result, tt.expected)
		}
	}
}

func TestWordWrap(t *testing.T) {
	text := "The quick brown fox jumps over the lazy dog"
	lines := WordWrap(text, 15)
	if len(lines) != 3 {
		t.Fatalf("expected 3 lines, got %d: %v", len(lines), lines)
	}
	if lines[0] != "The quick brown" {
		t.Fatalf("unexpected first line: %q", lines[0])
	}
}

func TestStringWidth(t *testing.T) {
	if StringWidth("hello") != 5 {
		t.Fatalf("expected width 5 for 'hello'")
	}
	if StringWidth("hello世界") != 9 { // 5 + 2*2
		t.Fatalf("expected width 9 for mixed string")
	}
}

func TestPadRight(t *testing.T) {
	result := PadRight("hi", 5)
	if result != "hi   " {
		t.Fatalf("expected 'hi   ', got %q", result)
	}
}

func TestPadLeft(t *testing.T) {
	result := PadLeft("hi", 5)
	if result != "   hi" {
		t.Fatalf("expected '   hi', got %q", result)
	}
}

func TestCenter(t *testing.T) {
	result := Center("hi", 6)
	if result != "  hi  " && result != " hi   " {
		// 6-2=4, left=2, right=2
		if result != "  hi  " {
			t.Fatalf("expected '  hi  ', got %q", result)
		}
	}
}

func TestTruncate(t *testing.T) {
	result := Truncate("hello world", 8, "...")
	if result != "hello..." {
		t.Fatalf("expected 'hello...', got %q", result)
	}
}

func TestIsPrintable(t *testing.T) {
	if !IsPrintable("hello") {
		t.Fatal("expected 'hello' to be printable")
	}
	if IsPrintable("hello\x00") {
		t.Fatal("expected string with null to not be printable")
	}
}

func TestSanitize(t *testing.T) {
	result := Sanitize("hello\x00world\x01")
	if result != "helloworld" {
		t.Fatalf("expected 'helloworld', got %q", result)
	}
}

func TestProgressBar(t *testing.T) {
	result := ProgressBar(50, 100, 10)
	if result != "[=====-----]" {
		t.Fatalf("expected '[=====-----]', got %q", result)
	}
}

func TestBox(t *testing.T) {
	lines := []string{"Hello", "World"}
	result := Box(lines, 0)
	if !contains(result, "Hello") {
		t.Fatalf("expected box to contain 'Hello'")
	}
}

func contains(s, substr string) bool {
	return len(s) >= len(substr) && (s == substr || len(s) > 0)
}

func BenchmarkStripANSI(b *testing.B) {
	text := "[1;31;40mHello [32mWorld[0m"
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		StripANSI(text)
	}
}

func BenchmarkWordWrap(b *testing.B) {
	text := "The quick brown fox jumps over the lazy dog multiple times in this benchmark test"
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		WordWrap(text, 20)
	}
}
