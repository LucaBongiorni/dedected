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
JP0III:		BK_C	0x20
PP0III:		WNT	2
JP2III:		BK_C	0x30
PP2III:		WNT	2
JP4III:		BK_C	0x40
PP4III:		WNT	2
JP6III:		BK_C	0x50
PP6III:		WNT	2
JP8III:		BK_C	0x60
PP8III:		WNT	2
JP10III:	BK_C	0x70
PP10III:	WNT	2
		U_INT0
JP12III:	BK_C	0x80
PP12III:	WNT	2
JP14III:	BK_C	0x90
PP14III:	WNT	2
JP16III:	BK_C	0xA0
PP16III:	WNT	2
JP18III:	BK_C	0xB0
PP18III:	WNT	2
JP20III:	BK_C	0xC0
PP20III:	WNT	2
JP22III:	BK_C	0xD0
PP22III:	WNT	2
		U_INT3
		P_LDH	PB_LED
		BR	SlotTable
;-------------------------------------------------------------

RecvIII:
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
;-----------------------------------------------------

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
		WT	7
label_53:
		B_RST
label_54:
		MEN1N
		WNT	1
label_6:
		RTN
;-------------------------------------------------------------

label_7:
		B_RST
		MEN1N
		BR	label_6
;-------------------------------------------------------------

RecvPP:
		MEN2
		P_LDL	PB_RSSI
		WT	1
		RFDIS
		WNT	1
		WT	5
		B_XON
		WT	14
		B_SR
		EN_SL_ADJ
		WT	13
		P_LDH	PB_DCTHRESHOLD
		WT	32
		B_AR2	0x06
		WT	61
		RTN
;-------------------------------------------------------------

label_6B:
		WT	249
		WT	79
label_10:
		P_LDH	PB_RSSI
		P_LDL	0x20|PB_RADIOPOWER|PB_DCTHRESHOLD
		RTN
;-------------------------------------------------------------

label_2:
		MEN2
		WT	2
		RFDIS
		B_RST
		B_RC	0x50
		WT	8
		WNT	1
		WT	4
		P_LDH	PB_RX_ON
		WT	3
		P_LDH	PB_TX_ON
		WT	3
		B_ST2
		WT	31
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
		P_LDL	PB_RX_ON|PB_RADIOPOWER
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
		WT	1
		WT	1
		M_WR	0x4A
		WT	9
		M_RST
		JMP	label_C0
		M_WR	0x4B
		WT	17
		M_RST
		JMP	label_C0
		M_WR	0x4D
		WT	25
		M_RST
		RTN
;-------------------------------------------------------------

RFDKnow1:
		JMP	label_C0
		BR	RFDKnow3
;-------------------------------------------------------------

RFDKnow2:
		JMP	label_C3
RFDKnow3:
		B_RST
		B_RC	0x58
		WT	8
		MEN2N
		P_LDH	PB_RADIOPOWER
		WT	208
		RTN
;-------------------------------------------------------------

label_C0:
		MEN1
		MEN1N
		RTN
;-------------------------------------------------------------

label_C3:
		MEN1
		RTN
;-------------------------------------------------------------

PPSync:
		BK_C	0x20
PPSearchIII:
		JMP	RFInit
		JMP	RFDKnow1
		MEN2N
		WT	250
		P_SC	0x60
		P_LDH	PB_DCTHRESHOLD
		UNLCK
		WT	64
		B_XOFF
		B_SR
		WNT	20
		JMP1	SFieldFound
		B_RST
		U_INT1
		WNT	23
		BR	PPSearchIII
;-------------------------------------------------------------

SFieldFound:
		WNT		23
		P_SC	0x00
RecvNextIII:
		JMP	RecvIII
		U_INT0
		WNT	22
PPFoundIII:
		BR	PPSearchIII
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
		P_LD	0x00
		RCK_INT
		RFEN
		BR	PPSync
;-------------------------------------------------------------

		SHARED	PP0III,PP2III,PP4III,PP6III,PP8III,PP10III,PP12III,PP14III,PP16III,PP18III,PP20III,PP22III
		SHARED	JP0III,JP2III,JP4III,JP6III,JP8III,JP10III,JP12III,JP14III,JP16III,JP18III,JP20III,JP22III
		SHARED	RecvNextIII,PPFoundIII,RecvIII
