package telnetty

import (
	"testing"
)

func TestNegotiatorHandleWill(t *testing.T) {
	n := NewNegotiator()

	// Peer sends WILL ECHO - we should respond DO
	resp, changed := n.Handle(CmdWill, OptEcho)
	if resp != CmdDo {
		t.Fatalf("expected DO, got %d", resp)
	}
	if !changed {
		t.Fatal("expected state change")
	}
	if n.Get(OptEcho).Him() != OptEnabled {
		t.Fatalf("expected HIM=enabled, got %d", n.Get(OptEcho).Him())
	}
}

func TestNegotiatorHandleWont(t *testing.T) {
	n := NewNegotiator()

	// First enable it
	n.Handle(CmdWill, OptEcho)

	// Then peer sends WONT
	resp, changed := n.Handle(CmdWont, OptEcho)
	if resp != CmdDont {
		t.Fatalf("expected DONT, got %d", resp)
	}
	if !changed {
		t.Fatal("expected state change")
	}
	if n.Get(OptEcho).Him() != OptDisabled {
		t.Fatalf("expected HIM=disabled, got %d", n.Get(OptEcho).Him())
	}
}

func TestNegotiatorHandleDo(t *testing.T) {
	n := NewNegotiator()

	// Peer sends DO SGA - we should respond WILL
	resp, changed := n.Handle(CmdDo, OptSGA)
	if resp != CmdWill {
		t.Fatalf("expected WILL, got %d", resp)
	}
	if !changed {
		t.Fatal("expected state change")
	}
	if n.Get(OptSGA).Us() != OptEnabled {
		t.Fatalf("expected US=enabled, got %d", n.Get(OptSGA).Us())
	}
}

func TestNegotiatorHandleDont(t *testing.T) {
	n := NewNegotiator()

	// First enable it
	n.Handle(CmdDo, OptSGA)

	// Then peer sends DONT
	resp, changed := n.Handle(CmdDont, OptSGA)
	if resp != CmdWont {
		t.Fatalf("expected WONT, got %d", resp)
	}
	if !changed {
		t.Fatal("expected state change")
	}
	if n.Get(OptSGA).Us() != OptDisabled {
		t.Fatalf("expected US=disabled, got %d", n.Get(OptSGA).Us())
	}
}

func TestNegotiatorDuplicateWill(t *testing.T) {
	n := NewNegotiator()

	// First WILL
	n.Handle(CmdWill, OptEcho)

	// Second WILL - should be ignored
	resp, changed := n.Handle(CmdWill, OptEcho)
	if resp != 0 {
		t.Fatalf("expected no response, got %d", resp)
	}
	if changed {
		t.Fatal("expected no state change")
	}
}

func TestNegotiatorRequestEnable(t *testing.T) {
	n := NewNegotiator()

	resp := n.RequestEnable(OptEcho)
	if resp != CmdWill {
		t.Fatalf("expected WILL, got %d", resp)
	}
	if n.Get(OptEcho).Us() != OptWantYes {
		t.Fatalf("expected US=wantyes, got %d", n.Get(OptEcho).Us())
	}
}

func TestNegotiatorRequestDisable(t *testing.T) {
	n := NewNegotiator()

	// Enable first
	n.RequestEnable(OptEcho)
	n.Handle(CmdDo, OptEcho) // peer agrees

	resp := n.RequestDisable(OptEcho)
	if resp != CmdWont {
		t.Fatalf("expected WONT, got %d", resp)
	}
	if n.Get(OptEcho).Us() != OptWantNo {
		t.Fatalf("expected US=wantno, got %d", n.Get(OptEcho).Us())
	}
}

func TestNegotiatorRequestPeerEnable(t *testing.T) {
	n := NewNegotiator()

	resp := n.RequestPeerEnable(OptEcho)
	if resp != CmdDo {
		t.Fatalf("expected DO, got %d", resp)
	}
	if n.Get(OptEcho).Him() != OptWantYes {
		t.Fatalf("expected HIM=wantyes, got %d", n.Get(OptEcho).Him())
	}
}

func TestNegotiatorRequestPeerDisable(t *testing.T) {
	n := NewNegotiator()

	// Enable first
	n.RequestPeerEnable(OptEcho)
	n.Handle(CmdWill, OptEcho) // peer agrees

	resp := n.RequestPeerDisable(OptEcho)
	if resp != CmdDont {
		t.Fatalf("expected DONT, got %d", resp)
	}
	if n.Get(OptEcho).Him() != OptWantNo {
		t.Fatalf("expected HIM=wantno, got %d", n.Get(OptEcho).Him())
	}
}

func TestNegotiatorIsEnabled(t *testing.T) {
	n := NewNegotiator()

	if n.Get(OptEcho).IsEnabled() {
		t.Fatal("expected not enabled initially")
	}

	n.Handle(CmdWill, OptEcho)
	if !n.Get(OptEcho).IsEnabled() {
		t.Fatal("expected enabled after WILL")
	}
}

func TestNegotiatorIsNegotiating(t *testing.T) {
	n := NewNegotiator()

	if n.Get(OptEcho).IsNegotiating() {
		t.Fatal("expected not negotiating initially")
	}

	n.RequestEnable(OptEcho)
	if !n.Get(OptEcho).IsNegotiating() {
		t.Fatal("expected negotiating after request")
	}
}

func TestNegotiatorReset(t *testing.T) {
	n := NewNegotiator()

	n.Handle(CmdWill, OptEcho)
	n.Handle(CmdDo, OptSGA)
	n.Reset()

	if n.Get(OptEcho).Him() != OptDisabled {
		t.Fatal("expected reset to disabled")
	}
	if n.Get(OptSGA).Us() != OptDisabled {
		t.Fatal("expected reset to disabled")
	}
}

func BenchmarkNegotiatorHandle(b *testing.B) {
	n := NewNegotiator()

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		n.Handle(CmdWill, OptEcho)
		n.Handle(CmdWont, OptEcho)
	}
}
