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
		BR	InitDIP
;-------------------------------------------------------------

SlotTable:
		SLOTZERO
JP0II:		BK_C	0x20
PP0II:		WNT	2
JP2II:		BK_C	0x30
PP2II:		WNT	2
JP4II:		BK_C	0x40
PP4II:		WNT	2
JP6II:		BK_C	0x50
PP6II:		WNT	2
JP8II:		BK_C	0x60
PP8II:		WNT	2
JP10II:		BK_C	0x70
PP10II:		WNT	2
		U_INT0
JP12II:		BK_C	0x80
PP12II:		WNT	2
JP14II:		BK_C	0x90
PP14II:		WNT	2
JP16II:		BK_C	0xA0
PP16II:		WNT	2
JP18II:		BK_C	0xB0
PP18II:		WNT	2
JP20II:		BK_C	0xC0
PP20II:		WNT	2
JP22II:		BK_C	0xD0
PP22II:		WNT	2
		U_INT3
		P_LDL	0x80
		BR	SlotTable
;-------------------------------------------------------------

RecvII:
		JMP	RFInit
		JMP	RFDKnow1
		BR	label_2D
;-------------------------------------------------------------

		JMP	RFInit
		JMP	RFDKnow2
label_2D:
		JMP	RecvPP
		WT	1
		B_BRFU	0x0E
		JMP	label_6B
		BR	label_51
;-------------------------------------------------------------

		JMP	RFInit
		JMP	RFDKnow1
		BR	label_37
;-------------------------------------------------------------

		JMP	RFInit
		JMP	RFDKnow2
label_37:
		JMP	RecvPP
		BR	label_1
;-------------------------------------------------------------

		JMP	RFInit
		JMP	RFDKnow1
		BR	label_3E
;-------------------------------------------------------------

		JMP	RFInit
		JMP	RFDKnow2
label_3E:
		JMP	label_2
		WT	1
		B_BTFU	0x0E
		JMP	label_3
		BR	label_54
;-------------------------------------------------------------

		JMP	RFInit
		JMP	RFDKnow2
		JMP	label_2
		WT	1
		B_BT	0x0E
		JMP	label_4
		BR	label_53
;-------------------------------------------------------------

		JMP	RFInit
		JMP	RFDKnow1
		BR	label_4F
;-------------------------------------------------------------

		JMP	RFInit
		JMP	RFDKnow2
label_4F:
		JMP	label_2
		BR	label_5
;-------------------------------------------------------------

label_51:
		B_WRS	0x00
		WT	6
label_53:
		B_RST
label_54:
		P_LDL	PB_RX_ON|PB_TX_ON
		WT	5
		WNT	1
label_6:
		RTN
;-------------------------------------------------------------

label_7:
		B_RST
		P_LDL	PB_RX_ON|PB_TX_ON
		BR	label_6
;-------------------------------------------------------------

RecvPP:
		P_LDH	PB_RX_ON
		P_LDL	PB_RSSI
		WT	25
		WNT	1
		WT	9
		B_XON
		P_LDH	PB_DCTHRESHOLD
		WT	5
		B_SR
		EN_SL_ADJ
		WT	12
		P_LDL	PB_DCTHRESHOLD
		WT	33
		B_AR2	0x06
		WT	61
		RTN
;-------------------------------------------------------------

label_6B:
		WT	249
		WT	79
label_10:
		P_LDH	PB_RSSI
		P_LDL	PB_RX_ON
		RTN
;-------------------------------------------------------------

label_2:
		P_LDH	0x00
		WT	40
		B_RST
		B_RC	0x50
		WNT	1
		B_ST	0x00
		WT	1
		P_LDH	PB_TX_ON
		WT	37
		B_AT2	0x06
		WT	61
		RTN
;-------------------------------------------------------------

label_3:
		WT	249
		WT	84
		B_RST
label_4:
		P_LDL	PB_TX_ON
		WT	8
		P_LDL	0x00
		RTN
;-------------------------------------------------------------

label_8:
		B_XON
		WT	15
		B_XOFF
		WT	61
		RTN
;-------------------------------------------------------------

label_9:
		WT	61
		JMP	label_8
		JMP	label_8
		JMP	label_8
		JMP	label_8
		JMP	label_8
		WT	1
		B_XON
		WT	11
		RTN
;-------------------------------------------------------------

label_1:
		B_BR	0x0E
		JMP	label_9
		WT	3
		B_XR
		WT	6
		JMP	label_10
		B_WRS	0x00
		WT	6
		BR	label_7
;-------------------------------------------------------------

label_5:
		B_BT	0x0E
		WT	3
		JMP	label_9
		B_XT
		WT	13
		B_RST
		JMP	label_4
		BR	label_7
;-------------------------------------------------------------

RFInit:
		RFEN
		MEN1N
		WT	1
		WT	1
		M_WR	0x4A
		WT	25
		M_RST
		MEN1
		MEN1N
		M_WR	0x4D
		WT	10
		M_RST
		MEN1
		RTN
;-------------------------------------------------------------

RFDKnow1:
		P_LDL	0x20
		BR	RFDKnow3
;-------------------------------------------------------------

RFDKnow2:
		P_LDH	0x20
		BR	RFDKnow3
;-------------------------------------------------------------

RFDKnow3:
		B_RST
		B_RC	0x58
		WT	8
		MEN2
		WT	118
		WT	64
		MEN2N
		P_LDH	0x00
		WT	16
		RTN
;-------------------------------------------------------------

PPSync:
		BK_C	0x20
PPSearchII:
		JMP	RFInit
		JMP	RFDKnow1
		WT	250
		P_SC	0x20
		P_LDH	PB_RX_ON|PB_DCTHRESHOLD
		UNLCK
		WT	64
		B_XOFF
		B_SR
		WNT	20
		JMP1	SFieldFound
		B_RST
		U_INT1
		WNT	23
		BR	PPSearchII
;-------------------------------------------------------------

SFieldFound:
		WNT	23
		P_SC	0x00
RecvNextII:
		JMP	RecvII
		U_INT0
		WNT	22
PPFoundII:
		BR	PPSearchII
;-------------------------------------------------------------

InitDIP:
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
		P_LD	PB_RADIOPOWER
		RCK_INT
		RFEN
		BR	PPSync
;-------------------------------------------------------------

		SHARED	PP0II,PP2II,PP4II,PP6II,PP8II,PP10II,PP12II,PP14II,PP16II,PP18II,PP20II,PP22II
		SHARED	JP0II,JP2II,JP4II,JP6II,JP8II,JP10II,JP12II,JP14II,JP16II,JP18II,JP20II,JP22II
		SHARED	RecvNextII,PPFoundII,RecvII
