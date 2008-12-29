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

Start:		B_RST	
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
;-------------------------------------------------------------

		BK_C	0x20
		JMP	RFInit
TryAgain:	JMP	label_B1
		WT	250
		P_SC	0x20
		P_LDH	PB_RX_ON|PB_DCTHRESHOLD
		UNLCK	
		WT	64
		B_XOFF	
		B_SR	
		WNT	20
		JMP1	SFieldFound
		BR	TryAgain
;-------------------------------------------------------------

SFieldFound:	WNT	23
		P_SC	0x00
		JMP	ReceiveSlot
		U_INT0	
		BR	TryAgain
;-------------------------------------------------------------

ReceiveSlot:	JMP	label_B1
		JMP	RecvPP
		WT	1
		B_BRFU	0x0e
		WT	255
		WT	73
		P_LDH	PB_RSSI
		P_LDL	PB_RX_ON
		B_WRS	0x00
		WT	6
		B_RST	
		P_LDL	PB_RX_ON|PB_TX_ON
		WT	5
		RTN	
;-------------------------------------------------------------

RecvPP:		P_LDH	PB_RX_ON
		P_LDL	PB_RSSI
		WT	34
		WNT	1
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

RFInit:		RFEN	
		MEN1N	
		WT	2
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

label_B1:	B_RST	
		B_RC	0x58
		WT	8
		MEN2	
		WT	182
		MEN2N	
		WT	16
		RTN	
;-------------------------------------------------------------

