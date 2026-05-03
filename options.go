package telnetty

import (
	"sync/atomic"
)

// OptionState tracks the Q-method state for one side of an option.
type OptionState uint8

const (
	OptDisabled    OptionState = QNo
	OptEnabled     OptionState = QYes
	OptWantNo      OptionState = QWantNo
	OptWantYes     OptionState = QWantYes
	OptWantNoOpp   OptionState = QOpposite // WANTNO opposite
	OptWantYesOpp  OptionState = 5          // WANTYES opposite (not in RFC but useful)
)

// Option represents the state of a single TELNET option for both sides.
type Option struct {
	us  uint32 // atomic: our state
	him uint32 // atomic: peer state
}

// NewOption creates an option tracker with both sides disabled.
func NewOption() *Option {
	return &Option{}
}

// Reset clears option state.
func (o *Option) Reset() {
	atomic.StoreUint32(&o.us, uint32(OptDisabled))
	atomic.StoreUint32(&o.him, uint32(OptDisabled))
}

// Us returns our state for this option.
func (o *Option) Us() OptionState {
	return OptionState(atomic.LoadUint32(&o.us))
}

// Him returns the peer's state for this option.
func (o *Option) Him() OptionState {
	return OptionState(atomic.LoadUint32(&o.him))
}

// IsEnabled returns true if either side has the option enabled.
func (o *Option) IsEnabled() bool {
	return o.Us() == OptEnabled || o.Him() == OptEnabled
}

// IsNegotiating returns true if either side is in a pending state.
func (o *Option) IsNegotiating() bool {
	us := o.Us()
	him := o.Him()
	return us == OptWantNo || us == OptWantYes || him == OptWantNo || him == OptWantYes
}

// Negotiator implements RFC 1143 Q-method option negotiation.
// It is safe for concurrent use via atomic operations.
type Negotiator struct {
	options [256]*Option
}

// NewNegotiator creates a negotiator with all options initialized.
func NewNegotiator() *Negotiator {
	n := &Negotiator{}
	for i := range n.options {
		n.options[i] = NewOption()
	}
	return n
}

// Reset clears all option states.
func (n *Negotiator) Reset() {
	for _, opt := range n.options {
		if opt != nil {
			opt.Reset()
		}
	}
}

// Get returns the option state for a given option code.
func (n *Negotiator) Get(option uint8) *Option {
	return n.options[option]
}

// Handle processes an incoming negotiation command and returns the response
// command (if any) and whether the option state changed.
// This implements the RFC 1143 Q-method state machine.
func (n *Negotiator) Handle(command, option uint8) (response uint8, changed bool) {
	opt := n.options[option]
	if opt == nil {
		return 0, false
	}

	switch command {
	case CmdWill:
		return n.handleWill(opt, option)
	case CmdWont:
		return n.handleWont(opt, option)
	case CmdDo:
		return n.handleDo(opt, option)
	case CmdDont:
		return n.handleDont(opt, option)
	}
	return 0, false
}

// handleWill processes peer WILL option.
func (n *Negotiator) handleWill(opt *Option, option uint8) (uint8, bool) {
	him := opt.Him()
	switch him {
	case OptDisabled:
		// Peer wants to enable, we agree
		atomic.StoreUint32(&opt.him, uint32(OptEnabled))
		return CmdDo, true
	case OptEnabled:
		// Already enabled, ignore
		return 0, false
	case OptWantNo:
		// Peer agreed to disable - conflict, disable it
		atomic.StoreUint32(&opt.him, uint32(OptDisabled))
		return CmdDont, true
	case OptWantYes:
		// Peer agreed to enable
		atomic.StoreUint32(&opt.him, uint32(OptEnabled))
		return 0, true
	}
	return 0, false
}

// handleWont processes peer WONT option.
func (n *Negotiator) handleWont(opt *Option, option uint8) (uint8, bool) {
	him := opt.Him()
	switch him {
	case OptDisabled:
		// Already disabled, ignore
		return 0, false
	case OptEnabled:
		// Peer wants to disable, we agree
		atomic.StoreUint32(&opt.him, uint32(OptDisabled))
		return CmdDont, true
	case OptWantNo:
		// Peer agreed to disable
		atomic.StoreUint32(&opt.him, uint32(OptDisabled))
		return 0, true
	case OptWantYes:
		// Peer refused to enable
		atomic.StoreUint32(&opt.him, uint32(OptDisabled))
		return 0, true
	}
	return 0, false
}

// handleDo processes peer DO option.
func (n *Negotiator) handleDo(opt *Option, option uint8) (uint8, bool) {
	us := opt.Us()
	switch us {
	case OptDisabled:
		// Peer wants us to enable, we agree
		atomic.StoreUint32(&opt.us, uint32(OptEnabled))
		return CmdWill, true
	case OptEnabled:
		// Already enabled, ignore
		return 0, false
	case OptWantNo:
		// Peer agreed to disable - conflict
		atomic.StoreUint32(&opt.us, uint32(OptDisabled))
		return CmdWont, true
	case OptWantYes:
		// Peer agreed to enable
		atomic.StoreUint32(&opt.us, uint32(OptEnabled))
		return 0, true
	}
	return 0, false
}

// handleDont processes peer DONT option.
func (n *Negotiator) handleDont(opt *Option, option uint8) (uint8, bool) {
	us := opt.Us()
	switch us {
	case OptDisabled:
		// Already disabled, ignore
		return 0, false
	case OptEnabled:
		// Peer wants us to disable, we agree
		atomic.StoreUint32(&opt.us, uint32(OptDisabled))
		return CmdWont, true
	case OptWantNo:
		// Peer agreed to disable
		atomic.StoreUint32(&opt.us, uint32(OptDisabled))
		return 0, true
	case OptWantYes:
		// Peer refused to enable
		atomic.StoreUint32(&opt.us, uint32(OptDisabled))
		return 0, true
	}
	return 0, false
}

// RequestEnable initiates enabling an option on our side (send WILL).
func (n *Negotiator) RequestEnable(option uint8) uint8 {
	opt := n.options[option]
	if opt == nil {
		return 0
	}
	us := opt.Us()
	if us == OptDisabled {
		atomic.StoreUint32(&opt.us, uint32(OptWantYes))
		return CmdWill
	}
	return 0
}

// RequestDisable initiates disabling an option on our side (send WONT).
func (n *Negotiator) RequestDisable(option uint8) uint8 {
	opt := n.options[option]
	if opt == nil {
		return 0
	}
	us := opt.Us()
	if us == OptEnabled {
		atomic.StoreUint32(&opt.us, uint32(OptWantNo))
		return CmdWont
	}
	return 0
}

// RequestPeerEnable initiates asking peer to enable (send DO).
func (n *Negotiator) RequestPeerEnable(option uint8) uint8 {
	opt := n.options[option]
	if opt == nil {
		return 0
	}
	him := opt.Him()
	if him == OptDisabled {
		atomic.StoreUint32(&opt.him, uint32(OptWantYes))
		return CmdDo
	}
	return 0
}

// RequestPeerDisable initiates asking peer to disable (send DONT).
func (n *Negotiator) RequestPeerDisable(option uint8) uint8 {
	opt := n.options[option]
	if opt == nil {
		return 0
	}
	him := opt.Him()
	if him == OptEnabled {
		atomic.StoreUint32(&opt.him, uint32(OptWantNo))
		return CmdDont
	}
	return 0
}
