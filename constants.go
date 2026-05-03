package telnetty

// TELNET command codes (RFC 854)
const (
	CmdIAC  = 255 // Interpret As Command
	CmdDont = 254
	CmdDo   = 253
	CmdWont = 252
	CmdWill = 251
	CmdSB   = 250 // Subnegotiation begin
	CmdGA   = 249 // Go ahead
	CmdEL   = 248 // Erase line
	CmdEC   = 247 // Erase character
	CmdAYT  = 246 // Are you there?
	CmdAO   = 245 // Abort output
	CmdIP   = 244 // Interrupt process
	CmdBreak = 243
	CmdDM    = 242 // Data mark
	CmdNOP   = 241 // No operation
	CmdSE    = 240 // Subnegotiation end
	CmdEOR   = 239 // End of record
	CmdAbort = 238
	CmdSusp  = 237
	CmdEOF   = 236
)

// Standard TELNET options
const (
	OptBinary           = 0  // RFC 856
	OptEcho             = 1  // RFC 857
	OptRCP              = 2
	OptSGA              = 3  // RFC 858
	OptNAMS             = 4
	OptStatus           = 5  // RFC 859
	OptTimingMark       = 6  // RFC 860
	OptRCTE             = 7
	OptNAOL             = 8
	OptNAOP             = 9
	OptNAOCRD           = 10
	OptNAOHTS           = 11
	OptNAOHTD           = 12
	OptNAOFFD           = 13
	OptNAOVTS           = 14
	OptNAOVTD           = 15
	OptNAOLFD           = 16
	OptXASCII           = 17
	OptLogout           = 18
	OptBM               = 19
	OptDET              = 20
	OptSUPDUP           = 21
	OptSUPDUPOutput     = 22
	OptSendLoc          = 23
	OptTerminalType     = 24 // RFC 1091
	OptEOR              = 25 // RFC 885
	OptTUID             = 26
	OptOutmrk           = 27
	OptTTYLoc           = 28
	Opt3270Regime       = 29
	OptX3PAD            = 30
	OptNAWS             = 31 // RFC 1073
	OptTermSpeed        = 32 // RFC 1079
	OptRemoteFlow       = 33 // RFC 1372
	OptLinemode         = 34 // RFC 1184
	OptXDispLoc         = 35
	OptOldEnviron       = 36
	OptAuth             = 37
	OptEncrypt          = 38
	OptNewEnviron       = 39 // RFC 1572
	OptTN3270E          = 40
	OptCharset          = 42 // RFC 2066
	OptEOL              = 45
	OptMCCP2            = 86 // MUD Client Compression v2
	OptMCCP3            = 87 // MUD Client Compression v3
	OptMSSP             = 70 // MUD Server Status Protocol
	OptMSDP             = 69 // MUD Server Data Protocol
	OptGMCP             = 201 // Generic MUD Communication Protocol
	OptMCP              = 200 // MUD Client Protocol 2.1
)

// Q-Method negotiation states (RFC 1143)
const (
	QNo       = 0 // Option disabled
	QYes      = 1 // Option enabled
	QWantNo   = 2 // Want to disable
	QWantYes  = 3 // Want to enable
	QOpposite = 4 // Opposite of current state
)

// Negotiation commands
const (
	NegotiationWill = CmdWill
	NegotiationWont = CmdWont
	NegotiationDo   = CmdDo
	NegotiationDont = CmdDont
)

// MUD protocol constants
const (
	MSDPVar   = 1
	MSDPVal   = 2
	MSDPTableOpen  = 3
	MSDPTableClose = 4
	MSDPArrayOpen  = 5
	MSDPArrayClose = 6

	GMCPPrefix = "Core."
	MCPVersion = "2.1"
)

// ANSI color constants
const (
	ColorBlack   = 0
	ColorRed     = 1
	ColorGreen   = 2
	ColorYellow  = 3
	ColorBlue    = 4
	ColorMagenta = 5
	ColorCyan    = 6
	ColorWhite   = 7
	ColorDefault = 9

	AttrBold       = 1
	AttrDim        = 2
	AttrItalic     = 3
	AttrUnderline  = 4
	AttrBlink      = 5
	AttrReverse    = 7
	AttrHidden     = 8
	AttrStrike     = 9

	ResetAll       = 0
	ResetBold      = 22
	ResetItalic    = 23
	ResetUnderline = 24
	ResetBlink     = 25
	ResetReverse   = 27
	ResetHidden    = 28
	ResetStrike    = 29
)
