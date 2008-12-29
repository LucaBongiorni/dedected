/*
 * com_on_air_cs - basic driver for the Dosch and Amand "com on air" cards
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * authors:
 * (C) 2008  Andreas Schuler <krater at badterrorist dot com>
 * (C) 2008  Matthias Wenzel <dect at mazzoo dot de>
 *
 */

#ifndef DIP_OPCODE_H
#define DIP_OPCODE_H


#define	BR		0x01
#define	JMP		0x02
#define	JMP1		0x03
#define	RTN		0x04
#define	BK_A1		0x05
#define	WNTM1		0x06
#define	WNTP1		0x07
#define	WNT		0x08
#define	WT		0x09
#define	RFDIS		0x0a
#define	RFEN		0x0b
#define	LD_PTR		0x0c
#define	SLOTZERO	0x0d
#define	BK_A		0x0e
#define	BK_C		0x0f


#define	B_RST		0x20
#define	B_ST2		0x21
#define	B_XT		0x24
#define	B_BT2		0x25
#define	B_BTFU		0x25
#define	B_XOFF		0x26
#define	B_ON		0x27
#define	B_XON		0x27
#define	UNLCK		0x28
#define	B_SR		0x29
#define	B_XR		0x2b
#define	EN_SL_ADJ	0x2c
#define	B_BR2		0x2d
#define	B_BRFU		0x2d
#define	B_RINV		0x2e
#define	B_RON		0x2f


#define	B_ST		0x31
#define	B_TX		0x31
#define	B_AT		0x32
#define	B_RC		0x33
#define	B_BT		0x34
#define	B_BTFP		0x35
#define	B_BTP		0x35
#define	B_AT2		0x37
#define	B_WRS		0x39
#define	B_AR		0x3a
#define	B_BR		0x3c
#define	B_BRP		0x3d
#define	B_BRFP		0x3d
#define	B_AR2		0x3f


#define	D_RST		0x40
#define	D_ON		0x42
#define	D_OFF		0x43
#define	D_PREP		0x44
#define	WSC		0x48


#define	D_LDK		0x50
#define	D_LDS		0x57
#define	D_WRS		0x5f


#define	U_PSC		0x60
#define	U_INT0		0x61
#define	RCK_INT		0x62
#define	RCK_EXT		0x63
#define	B_WB_OFF	0x64
#define	B_WB_ON		0x65
#define	CLK1		0x66
#define	CLK3		0x67
#define	U_CK8		0x68
#define	U_CK4		0x69
#define	U_CK2		0x6a
#define	U_INT1		0x6b
#define	U_CK1		0x6c
#define	U_INT2		0x6d
#define	U_INT3		0x6f


#define	A_RCV0		0x80
#define	A_RCV36		0x82
#define	A_RCV30		0x83
#define	A_RCV24		0x84
#define	A_RCV18		0x85
#define	A_RCV12		0x86
#define	A_RCV6		0x87
#define	A_RCV33		0x8a
#define	A_RCV27		0x8b
#define	A_RCV21		0x8c
#define	A_RCV15		0x8d
#define	A_RCV9		0x8e
#define	A_RCV3		0x8f


#define	MEN3N		0xa2
#define	MEN3		0xa3
#define	MEN1N		0xa4
#define	MEN1		0xa5
#define	MEN2N		0xa6
#define	MEN2		0xa7
#define	M_RD		0xa8
#define	M_RST		0xa9


#define	M_WRS		0xb8
#define	M_WR		0xb9


#define	A_RST		0xc0
#define	A_MUTE		0xc1
#define	A_STOFF		0xc2
#define	A_ALAW		0xc3
#define	A_DT		0xc4
#define	A_NORM		0xc5
#define	A_LDR		0xc6
#define	A_LDW		0xc7
#define	A_LIN		0xc8
#define	A_MTOFF		0xc9
#define	A_MUTE1		0xca
#define	A_MTOFF1	0xcb
#define	A_STON		0xcc
#define	A_DT1		0xcd
#define	A_LDR1		0xce
#define	A_LDW1		0xcf


#define	A_STRN		0xe0
#define	P_LD		0xe8
#define	P_EN		0xe9
#define	P_SC		0xea
#define	A_RST1		0xeb
#define	P_LDL		0xec
#define	P_LDH		0xed
#define	C_ON		0xee
#define	C_OFF		0xef


#define	C_LD		0xfa



#endif

