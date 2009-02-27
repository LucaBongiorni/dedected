		CPU	SC14421
		ORG	0

		BR	Start


PB_LED		EQU	0x80
PB_RX_ON	EQU	0x40
PB_TX_ON	EQU	0x10
PB_RADIOPOWER	EQU	0x04
PB_DCTHRESHOLD	EQU	0x02
PB_RSSI		EQU	0x01

;-------------------------------------------------------------

Start:
		B_RST
		BK_C	0x00
		C_LD	0x10
		WT	10
		B_RC	0x00
		WT	8
		B_RST
		C_ON
		WT	10
		P_EN
		P_LD	0x00
		RCK_INT
		RFEN
		MEN1N
;-------------------------------------------------------------	

		BK_C	0x20
TryAgain:
		JMP	RFInit1
;		JMP	RFInit2
;		JMP	RFInit3
		JMP	RFDKnow
		MEN2N
		WT	250
		P_SC	0x60
		P_LDH	PB_RX_ON|PB_DCTHRESHOLD
		UNLCK
		B_XOFF
		B_SR
		WNT	2
		JMP1	SFieldFound
		B_RST
		BR	TryAgain
;-------------------------------------------------------------

SFieldFound:
		WNT	23
		P_SC	0x00
		JMP	RecvSlot
		U_INT0
		BR	TryAgain
;-------------------------------------------------------------

RecvSlot:
		JMP	RFDKnow
		JMP	RecvPP
		WT	1
		B_BRFU	0x0E
		WT	255
		WT	73
		P_LDH	PB_RSSI
		P_LDL	0x20|PB_DCTHRESHOLD;|PB_RADIOPOWER
		B_WRS	0x00
		WT	7
		B_RST
		RTN
;-------------------------------------------------------------

RecvPP:
		MEN2
		P_LDL		PB_RSSI
		WT		1
		RFDIS
		WNT		1
		WT		5
		B_XON
		WT		14
		B_SR
		EN_SL_ADJ
		WT		13
		P_LDH		PB_DCTHRESHOLD
		WT		32
		B_AR2		0x06
		WT		61
		RTN
;-------------------------------------------------------------

RFInit1:
		RFEN
		M_WR		0x4A
		WT		9
		M_RST
		MEN1
;		RTN
;RFInit2:
		MEN1N
		M_WR		0x4B
		WT		17
		M_RST
		MEN1
;		RTN
;RFInit3:
		MEN1N
		M_WR		0x4D
		WT		25
		M_RST
		RTN
;-------------------------------------------------------------

RFDKnow:
		B_RST
		B_RC		0x58
		WT		8
		MEN2N
		P_LDH		PB_RADIOPOWER
		WT		208
		RTN
;-------------------------------------------------------------
