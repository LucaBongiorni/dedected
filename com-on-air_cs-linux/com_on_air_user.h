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

#ifndef COM_ON_AIR_USER_H
#define COM_ON_AIR_USER_H

/* operation modes */
#define COA_MODEMASK			0xFF00
#define COA_SUBMODEMASK			0x00FF

#define COA_MODE_IDLE   		0x0000
#define COA_MODE_FP     		0x0100
#define COA_MODE_PP     		0x0200
#define COA_MODE_SNIFF  		0x0300
#define COA_MODE_JAM    		0x0400


#define COA_SUBMODE_SNIFF_SCANFP	0x0001
#define COA_SUBMODE_SNIFF_SCANPP	0x0002
#define COA_SUBMODE_SNIFF_SYNC		0x0003


/* ioctl */

#define COA_IOCTL_MODE			0xD000
#define COA_IOCTL_RADIO			0xD001
#define COA_IOCTL_RX			0xD002
#define COA_IOCTL_TX			0xD003
#define COA_IOCTL_CHAN			0xD004
#define COA_IOCTL_SLOT			0xD005
#define COA_IOCTL_RSSI			0xD006
#define COA_IOCTL_FIRMWARE		0xD007 /* request_firmware() */
#define COA_IOCTL_SETRFPI		0xD008

#endif
