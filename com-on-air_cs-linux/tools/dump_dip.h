/*
 * dump_dip - simple disassembler for sc144xx DIP codes
 * 
 * usage: dump_dip [options] <files>
 * options:
 *        -b       byteswap
 *
 * example:
 *        dump_dip ../WinCD/install/M* > win_cd_dip.asm
 *
 * authors:
 *         (c) 2009  Matthias Wenzel - dect /at/ mazzoo /dot/ de
 *         (c) 1999  Alfred Arnold
 *
 * license:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */
#ifndef DUMP_DIP_H
#define DUMP_DIP_H

#define M_14400 (1 << 0)
#define M_14401 (1 << 1)
#define M_14402 (1 << 2)
#define M_14404 (1 << 3)
#define M_14405 (1 << 4)
#define M_14420 (1 << 5)
#define M_14421 (1 << 6)
#define M_14422 (1 << 7)
#define M_14424 (1 << 8)

#define PARAM_NONE	0
#define PARAM_HEX	1
#define PARAM_DEC	2
#define PARAM_LABEL	3

char * cpu_name[] =
{
	"sc14400",
	"sc14401",
	"sc14402",
	"sc14404",
	"sc14405",
	"sc14420",
	"sc14421",
	"sc14422",
	"sc14424",
};

struct op_code {
	char *   mnemonic;
	uint8_t  code;
	uint32_t cpu;
	uint32_t param_type;
};

struct op_code dip_op_code[] =
{
	{"BR"       , 0x01, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_LABEL},
	{"BRK"      , 0x6e,                     M_14402 | M_14404 | M_14405                     | M_14422 | M_14424, PARAM_NONE },
	{"JMP"      , 0x02, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_LABEL},
	{"JMP1"     , 0x03, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_LABEL},
	{"RTN"      , 0x04, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"WNT"      , 0x08, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_DEC  },
	{"WT"       , 0x09, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_DEC  },
	{"WSC"      , 0x48,                               M_14404 | M_14405           | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"RFEN"     , 0x0b, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"RFDIS"    , 0x0a, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"BK_A"     , 0x0e, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"BK_A1"    , 0x05,                               M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"BK_C"     , 0x0f, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"SLOTZERO" , 0x0d, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"EN_SL_ADJ", 0x2c, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"WNTP1"    , 0x07, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"WNTM1"    , 0x06, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"LD_PTR"   , 0x0c,                     M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"UNLCK"    , 0x28, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RX"     , 0x49,                               M_14404 | M_14405                     | M_14422 | M_14424, PARAM_HEX  },
	{"A_TX"     , 0x4a,                               M_14404 | M_14405                     | M_14422 | M_14424, PARAM_HEX  },
	{"A_MUTE"   , 0xc1, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_MTOFF"  , 0xc9,                               M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_MUTE1"  , 0xca,                                                   M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_MTOFF1" , 0xcb,                                                   M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_STOFF"  , 0xc2, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_STON"   , 0xcc,                     M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV0"   , 0x80, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV36"  , 0x82, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV30"  , 0x83, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV24"  , 0x84, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV18"  , 0x85, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV12"  , 0x86, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV6"   , 0x87, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV33"  , 0x8a, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV27"  , 0x8b, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV21"  , 0x8c, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV15"  , 0x8d, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV9"   , 0x8e, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RCV3"   , 0x8f, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_NORM"   , 0xc5, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_RST"    , 0xc0, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_LDR"    , 0xc6, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"A_LDW"    , 0xc7, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"A_RST1"   , 0xeb,                                                   M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"A_LDR1"   , 0xce,                               M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"A_LDW1"   , 0xcf,                               M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"A_ST18"   , 0xe1,                     M_14402                                                            , PARAM_NONE },
	{"B_ST"     , 0x31, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_ST2"    , 0x21,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_PPT"    , 0x22,                               M_14404 | M_14405                               | M_14424, PARAM_NONE },
	{"B_ZT"     , 0x22, M_14400                                                                                , PARAM_NONE },
	{"B_ZR"     , 0x2a, M_14400                                                                                , PARAM_NONE },
	{"B_AT"     , 0x32, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_AT2"    , 0x37,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BT"     , 0x34, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BTFM"   , 0x23,                               M_14404 | M_14405                               | M_14424, PARAM_HEX  },
	{"B_BTFU"   , 0x25,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BTFP"   , 0x35,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BTDU"   , 0x71,                               M_14404 | M_14405                               | M_14424, PARAM_HEX  },
	{"B_BTDP"   , 0x72,                               M_14404 | M_14405                               | M_14424, PARAM_HEX  },
	{"B_XON"    , 0x27, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_XOFF"   , 0x26, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_SR"     , 0x29, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_AR"     , 0x3a, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_AR2"    , 0x3f,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_RON"    , 0x2f, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_RINV"   , 0x2e, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_BR"     , 0x3c, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BRFU"   , 0x2d,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BRFP"   , 0x3d,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BRFD"   , 0x2a,                               M_14404 | M_14405                               | M_14424, PARAM_HEX  },
	{"B_BRDU"   , 0x79,                               M_14404 | M_14405                               | M_14424, PARAM_HEX  },
	{"B_BRDP"   , 0x7a,                               M_14404 | M_14405                               | M_14424, PARAM_HEX  },
	{"B_XR"     , 0x2b, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_XT"     , 0x24, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_WB_ON"  , 0x65,                     M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_WB_OFF" , 0x64,                     M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_WRS"    , 0x39, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_RC"     , 0x33, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_RST"    , 0x20, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"B_DIV1"   , 0x4f,                                         M_14405                                        , PARAM_NONE },
	{"B_DIV2"   , 0x4e,                                         M_14405                                        , PARAM_NONE },
	{"B_DIV4"   , 0x4d,                                         M_14405                                        , PARAM_NONE },
	{"C_LD"     , 0xfa,           M_14401 | M_14402                     | M_14420 | M_14421 | M_14422          , PARAM_HEX  },
	{"C_ON"     , 0xee,                                                   M_14420 | M_14421 | M_14422          , PARAM_NONE },
	{"C_OFF"    , 0xef,                                                   M_14420 | M_14421 | M_14422          , PARAM_NONE },
	{"C_LD2"    , 0xba,                                                                                 M_14424, PARAM_HEX  },
	{"C_ON2"    , 0xae,                                                                                 M_14424, PARAM_NONE },
	{"C_OFF2"   , 0xaf,                                                                                 M_14424, PARAM_NONE },
	{"D_LDK"    , 0x50, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"D_PREP"   , 0x44, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"D_WRS"    , 0x5f, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"D_LDS"    , 0x57, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"D_RST"    , 0x40, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"M_WR"     , 0xb9, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"M_RST"    , 0xa9, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"M_INI0"   , 0xa0,                               M_14404 | M_14405                               | M_14424, PARAM_NONE },
	{"M_INI1"   , 0xa1,                               M_14404 | M_14405                               | M_14424, PARAM_NONE },
	{"MEN1N"    , 0xa4, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"MEN1"     , 0xa5, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"P_EN"     , 0xe9,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"P_LDH"    , 0xed,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"P_LDL"    , 0xec,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"P_LD"     , 0xe8,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"P_SC"     , 0xea,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"U_INT0"   , 0x61, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"U_INT1"   , 0x6b, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"U_INT2"   , 0x6d, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"U_INT3"   , 0x6f, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"U_PSC"    , 0x60, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{"U_VINT"   , 0x63,                               M_14404 | M_14405                     | M_14422 | M_14424, PARAM_HEX  },

	/* obsolete stuff - argument range may be incorrect */

	{"D_ON"     , 0x42, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"D_OFF"    , 0x43, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"RCK_INT"  , 0x62,                                                   M_14420 | M_14421                    , PARAM_NONE },
	{"CLK1"     , 0x66,                                                   M_14420 | M_14421                    , PARAM_NONE },
	{"CLK3"     , 0x67,                                                   M_14420 | M_14421                    , PARAM_NONE },
	{"U_CK8"    , 0x68, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"U_CK4"    , 0x69, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"U_CK2"    , 0x6a, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"U_CK1"    , 0x6c, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"MEN3N"    , 0xa2, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"MEN3"     , 0xa3, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"MEN2N"    , 0xa6, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"MEN2"     , 0xa7, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"M_RD"     , 0xa8, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_HEX  },
	{"M_WRS"    , 0xb8, M_14400                                         | M_14420 | M_14421                    , PARAM_HEX  },
	{"A_ALAW"   , 0xc3, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"A_DT"     , 0xc4, M_14400 | M_14401                               | M_14420 | M_14421                    , PARAM_NONE },
	{"A_LIN"    , 0xc8,                                                   M_14420 | M_14421                    , PARAM_NONE },
	{"A_DT1"    , 0xcd,                                                   M_14420 | M_14421                    , PARAM_NONE },
	{"A_STRN"   , 0xe0,                                                   M_14420 | M_14421                    , PARAM_NONE },
	{"RCK_EXT"  , 0x63,                                                   M_14420 | M_14421                    , PARAM_HEX  },
	{"P_SPD0"   , 0xe8, M_14400                                                                                , PARAM_HEX  },
	{"P_SPD1"   , 0xe9, M_14400                                                                                , PARAM_HEX  },
	{"P_SPD2"   , 0xea, M_14400                                                                                , PARAM_HEX  },
	{"P_SPD3"   , 0xeb, M_14400                                                                                , PARAM_HEX  },
	{"P_SPD4"   , 0xec, M_14400                                                                                , PARAM_HEX  },
	{"P_SPD5"   , 0xed, M_14400                                                                                , PARAM_HEX  },
	{"P_SPD6"   , 0xee, M_14400                                                                                , PARAM_HEX  },
	{"P_SPD7"   , 0xef, M_14400                                                                                , PARAM_HEX  },
	{"P_RPD0"   , 0xe0, M_14400                                                                                , PARAM_HEX  },
	{"P_RPD1"   , 0xe1, M_14400                                                                                , PARAM_HEX  },
	{"P_RPD2"   , 0xe2, M_14400                                                                                , PARAM_HEX  },
	{"P_RPD3"   , 0xe3, M_14400                                                                                , PARAM_HEX  },
	{"P_RPD4"   , 0xe4, M_14400                                                                                , PARAM_HEX  },
	{"P_RPD5"   , 0xe5, M_14400                                                                                , PARAM_HEX  },
	{"P_RPD6"   , 0xe6, M_14400                                                                                , PARAM_HEX  },
	{"P_RPD7"   , 0xe7, M_14400                                                                                , PARAM_HEX  },

	/* aliases */

	{"B_TX"     , 0x31, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BT2"    , 0x25,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BR2"    , 0x2d,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BTP"    , 0x35,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_BRP"    , 0x3d,           M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_HEX  },
	{"B_ON"     , 0x27, M_14400 | M_14401 | M_14402 | M_14404 | M_14405 | M_14420 | M_14421 | M_14422 | M_14424, PARAM_NONE },
	{NULL, 0, 0, 0}
};

#endif /* DUMP_DIP_H */
