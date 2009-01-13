		CPU     SC14421

		ORG 0
		BR      Start


PB_LED          EQU     0x80
PB_RX_ON        EQU     0x40
PB_TX_ON        EQU     0x10
PB_RADIOPOWER   EQU     0x04
PB_DCTHRESHOLD  EQU     0x02
PB_RSSI		EQU	0x01

;-------------------------------------------------------------

Start:		BR	InitDIP
;-------------------------------------------------------------

SlotTable:	SLOTZERO	
JP0:		BK_C	0x20
PP0:		WNT	2
JP2:		BK_C	0x30
PP2:		WNT	2
JP4:		BK_C	0x40
PP4:		WNT	2
JP6:		BK_C	0x50
PP6:		WNT	2
JP8:		BK_C	0x60
PP8:		WNT	2
JP10:		BK_C	0x70
PP10:		WNT	2
		U_INT0
JP12:		BK_C	0x80
PP12:		WNT	2
JP14:		BK_C	0x90
PP14:		WNT	2
JP16:		BK_C	0xA0
PP16:		WNT	2
JP18:		BK_C	0xB0
PP18:		WNT	2
JP20:		BK_C	0xC0
PP20:		WNT	2
JP22:		BK_C	0xD0
PP22:		WNT	2
		U_INT3
		P_LDL	0x80
		BR	SlotTable
;-------------------------------------------------------------

sync_label_28:	JMP	RFInit
		JMP	label_B1
		BR	label_2D
;-------------------------------------------------------------

		JMP	RFInit
		JMP	label_B3
label_2D:	JMP	RecvPP
		WT	1
		B_BRFU	0x0E
		JMP	label_6B
		BR	label_51
;-------------------------------------------------------------

		JMP	RFInit
		JMP	label_B1
		BR	label_37
;-------------------------------------------------------------

		JMP	RFInit
		JMP	label_B3
label_37:	JMP	RecvPP
		BR	label_92
;-------------------------------------------------------------

		JMP	RFInit
		JMP	label_B1
		BR	label_3E
;-------------------------------------------------------------

		JMP	RFInit
		JMP	label_B3
label_3E:	JMP	label_70
		WT	1
		B_BTFU	0x0E
		JMP	label_7C
		BR	label_54
;-------------------------------------------------------------

		JMP	RFInit
		JMP	label_B3
		JMP	label_70
		WT	1
		B_BT	0x0E
		JMP	label_7F
		BR	label_53
;-------------------------------------------------------------

		JMP	RFInit
		JMP	label_B1
		BR	label_4F
;-------------------------------------------------------------

		JMP	RFInit
		JMP	label_B3
label_4F:	JMP	label_70
		BR	label_9B
;-------------------------------------------------------------

label_51:	B_WRS	0x00
		WT	6
label_53:	B_RST	
label_54:	P_LDL	0x50
		WT	5
		WNT	1
label_57:	RTN	
;-------------------------------------------------------------

label_58:	B_RST	
		P_LDL	0x50
		BR	label_57
;-------------------------------------------------------------

RecvPP:		P_LDH	0x40
		P_LDL	PB_RSSI	
		WT	25
		WNT	1
		WT	9
		B_XON	
		P_LDH	0x02
		WT	5
		B_SR	
		EN_SL_ADJ	
		WT	12
		P_LDL	0x02
		WT	33
		B_AR2	0x06
		WT	61
		RTN	
;-------------------------------------------------------------

label_6B:	WT	249
		WT	79
label_6D:	P_LDH	PB_RSSI
		P_LDL	0x40
		RTN	
;-------------------------------------------------------------

label_70:	P_LDH	0x00
		WT	40
		B_RST	
		B_RC	0x50
		WNT	1
		B_ST	0x00
		WT	1
		P_LDH	0x10
		WT	37
		B_AT2	0x06
		WT	61
		RTN	
;-------------------------------------------------------------

label_7C:  	WT	249
             	WT	84
             	B_RST	
label_7F:  	P_LDL	0x10
             	WT	8
             	P_LDL	0x00
             	RTN	
;-------------------------------------------------------------

label_83:  	B_XON	
             	WT	15
             	B_XOFF	
             	WT	61
             	RTN	
;-------------------------------------------------------------

label_88:  	WT	61
             	JMP	label_83
             	JMP	label_83
             	JMP	label_83
             	JMP	label_83
             	JMP	label_83
             	WT	1
             	B_XON	
             	WT	11
             	RTN	
;-------------------------------------------------------------

label_92:  	B_BR	0x0E
             	JMP	label_88
             	WT	3
             	B_XR	
             	WT	6
             	JMP	label_6D
             	B_WRS	0x00
             	WT	6
             	BR	label_58
;-------------------------------------------------------------

label_9B:  	B_BT	0x0E
             	WT	3
             	JMP	label_88
             	B_XT	
             	WT	13
             	B_RST	
             	JMP	label_7F
             	BR	label_58
;-------------------------------------------------------------

RFInit:  	RFEN	
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

label_B1:  	P_LDL	0x20
             	BR	label_B5
;-------------------------------------------------------------

label_B3:  	P_LDH	0x20
             	BR	label_B5
;-------------------------------------------------------------

label_B5:	B_RST	
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

PPSync:		BK_C	0x20
label_C0:	JMP	RFInit
		JMP	label_B1
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
		BR	label_C0
;-------------------------------------------------------------

SFieldFound:	WNT	23
		P_SC	0x00
sync_label_D1:	JMP	sync_label_28
		U_INT0	
		WNT	22
sync_label_D4:	BR	label_C0
;-------------------------------------------------------------

InitDIP:	B_RST	
		BK_C	0x00
		C_LD	0x10
		WT	10
		B_RC	0x00
		WT	8
		B_RST	
		BK_A	0x00
		A_LDR	0x8C
		A_LDW	0xB4
		BK_A1	0x00
		A_LDR1	0x0C
		A_LDW1	0x34
		C_ON	
		A_NORM	
		WT	10
		P_EN	
		P_LD	0x04
		RCK_INT	
		RFEN	
		BR	PPSync
;-------------------------------------------------------------

		SHARED		PP0,PP2,PP4,PP6,PP8,PP10,PP12,PP14,PP16,PP18,PP20,PP22
		SHARED		JP0,JP2,JP4,JP6,JP8,JP10,JP12,JP14,JP16,JP18,JP20,JP22
		SHARED		sync_label_D1,sync_label_D4,sync_label_28
