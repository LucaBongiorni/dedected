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

#include "dect.h"

#define DECT_A_TA		0xe0
#define DECT_A_Q1		0x10
#define DECT_A_BA		0x0e
#define DECT_A_Q2		0x01

#define DECT_Q_TYPE		0x80
#define DECT_N_TYPE		0x60
#define DECT_P_TYPE		0xe0

#define DECT_Q_HEAD		0xf0

#define DECT_Q_HEAD_SSINFO	0x00 /* Static System info */
#define DECT_Q_HEAD_EXTRF1	0x20 /* Extended RF Carriers Part 1 */
#define DECT_Q_HEAD_FPC		0x30 /* Fixed Part Capabilities */
#define DECT_Q_HEAD_MULTIFN	0x60 /* Multi Frame Number */

#define DECT_Q_SSINFO_SLOT	0x0f


#define DECT_P_EXTFLAG		0x80
#define DECT_P_HEAD		0x70

#define DECT_P_HEAD_ZEROLP	0x00 /* Zero Length Page */
#define DECT_P_HEAD_SHORTLP	0x10 /* Short Length Page */
#define DECT_P_HEAD_FULLLP	0x20 /* Full Length Page */
#define DECT_P_HEAD_MACRES	0x30 /* MAC Resume Page */
#define DECT_P_HEAD_NOTLAST36	0x40 /* Not the Last 36 Bits of Long Page */
#define DECT_P_HEAD_FIRST36	0x50 /* First 36 Bit of Long Page */
#define DECT_P_HEAD_LAST26	0x60 /* Last 36 Bit of Long Page */
#define DECT_P_HEAD_ALLOFLP	0x70 /* All of Long Page (first and last) */

#define DECT_P_INFOTYPE		0xf0

#define DECT_P_IT_FILLBITS	0x00 /* Fill Bits */
#define DECT_P_IT_BFCIRCUIT	0x10 /* Blind Full Slot Information for \
					Circuit Mode Service */
#define DECT_P_IT_OTHERBEAR	0x20 /* Other Bearer */
#define DECT_P_IT_RECOTHERBEAR	0x30 /* Recommended Other Bearer */
#define DECT_P_IT_GOODRFPBEAR	0x40 /* Good RFP Bearer */
#define DECT_P_IT_DUMMYCLPOS	0x50 /* Dummy or C/L Bearer Position */
#define DECT_P_IT_EXTMODULTYPE	0x60 /* Extended Modulation Type */
#define DECT_P_IT_ESCAPE	0x70 /* Escape */
#define DECT_P_IT_DUMMYCLBEARM	0x80 /* Dummy or C/L Bearer Marker */
#define DECT_P_IT_REPLACEINFO	0x90 /* Bearer Handover/Replacement \
					Information */

#define DECT_P_IT_BEARERPOS_SN	0x0f /* Slot Number */
#define DECT_P_IT_BEARERPOS_SP	0xc0 /* Start Position */
#define DECT_P_IT_BEARERPOS_CN	0x3f /* Channel */

int dect_is_RFPI_Packet(unsigned char *packet)
{
	if ((packet[5] & DECT_A_TA) == DECT_N_TYPE)
		return 1;

	return 0;
}

int dect_compare_RFPI(unsigned char *packet, unsigned char* RFPI)
{
	if ((packet[5] & DECT_A_TA) == DECT_N_TYPE)
	{
		if (!memcmp(&packet[6], RFPI, 5))
			return 1;
	}

	return 0;
}

int dect_get_slot(unsigned char *packet)
{
	int slot = -1;

	if ((packet[5] & DECT_A_TA) == DECT_Q_TYPE)
	{
		if ((packet[6] & DECT_Q_HEAD) == DECT_Q_HEAD_SSINFO)
		{
			slot = packet[6] & DECT_Q_SSINFO_SLOT;
		}
	}

	return slot;
}

int dect_is_multiframe_number(unsigned char *packet)
{
	if ((packet[5] & DECT_A_TA) == DECT_Q_TYPE)
	{
		/*printk("bl"); */
		if ((packet[6] & DECT_Q_HEAD) == DECT_Q_HEAD_MULTIFN)
		{
			/*printk("aa\n"); */
			return 1;
		}
	}

	return 0;
}

int dect_is_fp_packet(unsigned char *packet)
{
	if ( (packet[3] == 0xe9) && (packet[4] == 0x8a) )
		return 1;

	return 0;
}
		
int dect_is_pp_packet(unsigned char *packet)
{
        if ( (packet[3] == 0x16) && (packet[4] == 0x75) )
                return 1;

        return 0;
}


int dect_update_slottable(struct dect_slot_info *slottable, int slot, unsigned char *packet)
{
	if (slottable[slot].type == DECT_SLOTTYPE_SCAN)
	{
		printk("received packet on scanslot %u on carrier %u\n", slot, slottable[slot].channel);
		slottable[slot].type = DECT_SLOTTYPE_CARRIER;
		if (slottable[slot].channel == 0)
			slottable[slot].channel = 9;
		else
			slottable[slot].channel--;		
		slottable[slot].errcnt = 0;
		slottable[slot].update = 1;

		if (slot > 12)
		{
			slottable[slot-12].type = DECT_SLOTTYPE_CARRIER;
			if (slottable[slot-12].channel == 0)
				slottable[slot-12].channel = 9;
			else
				slottable[slot-12].channel--;
			slottable[slot-12].errcnt = 0;
			slottable[slot].update = 1;
		}
		else
		{
			slottable[slot+12].type = DECT_SLOTTYPE_CARRIER;
			if (slottable[slot+12].channel == 0)
				slottable[slot+12].channel = 9;
			else
				slottable[slot+12].channel--;
			slottable[slot+12].errcnt = 0;
			slottable[slot].update = 1;
		}
	}
	
	if (dect_is_fp_packet(packet))
	{

		switch(packet[5] & DECT_A_TA)
		{
		case DECT_P_TYPE:
			if ( ( (packet[6] & DECT_P_HEAD) ==
					DECT_P_HEAD_ZEROLP) ||
				( (packet[6] & DECT_P_HEAD) ==
				 	DECT_P_HEAD_SHORTLP) )
			{
				switch((packet[9] & DECT_P_INFOTYPE))
				{
					case DECT_P_IT_OTHERBEAR:
					{
						int newslot = (packet[9] & DECT_P_IT_BEARERPOS_SN) % 12;
						slottable[newslot].active = 1;
						slottable[newslot].channel = packet[10] & DECT_P_IT_BEARERPOS_CN;
						slottable[newslot].type = DECT_SLOTTYPE_CARRIER;
						slottable[newslot].errcnt = 0;
						slottable[newslot].update = 1;
				
			                        slottable[newslot+12].active = 1;
						slottable[newslot+12].channel = packet[10] & DECT_P_IT_BEARERPOS_CN;
						slottable[newslot+12].type = DECT_SLOTTYPE_CARRIER;
						slottable[newslot+12].errcnt = 0;
						slottable[newslot+12].update = 1;

						/*printk("\n\nstation switching to slot %u channel %u\n\n",newslot, packet[10] & DECT_P_IT_BEARERPOS_CN); */
						return 1;
					}

					case DECT_P_IT_BFCIRCUIT:	/* set scanning slots */
					{
						int i, x, ret = 0;
						unsigned int slots = (((unsigned int)(packet[9] & ~DECT_P_INFOTYPE)) << 8) | packet[10];

						for (i=0; i<12; i++)
						{
							x = (slots & 0x800 ? 1:0);
							slots <<= 1;
							
							if (x && (!slottable[i].active))
							{
								printk("scanning on slot %u\n", i);
								slottable[i].active = 1;
								slottable[i].channel = 0;		/* channel unknown at this position */
								slottable[i].type = DECT_SLOTTYPE_SCAN;
								slottable[i].errcnt = 0;
								slottable[i].update = 1;
								ret = 1;
							}

							if (x && (!slottable[i+12].active))
							{
								printk("scanning on slot %u\n", i + 12);
								slottable[i+12].active = 1;
								slottable[i+12].channel = 0;	/* channel unknown at this position */
								slottable[i+12].type = DECT_SLOTTYPE_SCAN;
								slottable[i+12].errcnt = 0;
								slottable[i+12].update = 1;
								ret = 1;
							}
						}		

						return ret;
					}
				}
			}

			break;
		case DECT_Q_TYPE:
			if ((packet[6] & DECT_Q_HEAD) == DECT_Q_HEAD_SSINFO)
			{
				uint8_t pscan = packet[10];
				int i, ret = 0;
	
				for (i=0; i<24; i++)
				{
					if (slottable[i].active)
					{
						if (slottable[i].type == DECT_SLOTTYPE_SCAN)
						{
/*							if (pscan == 0)
								pscan = 9;
							else
								pscan--;
*/
							slottable[i].channel = pscan;
							slottable[i].update = 1;
							ret = 1;
						}
					}
				}

				return ret;			
			}

			break;
			
		}
		
	}

	return 0;
}

int dect_receive_error(struct dect_slot_info *slottable, int slot)
{
	slottable[slot].errcnt++;

	if (slottable[slot].type != DECT_SLOTTYPE_SCAN)
	{
		if (slottable[slot].errcnt > 32)
		{
			slottable[slot].active = 0;
			slottable[slot].update = 1;

			printk("slot %u on channel %u died\n", slot, slottable[slot].channel);

			return 1;
		}		
	}

	return 0;
}

int dect_update_scanchannels(struct dect_slot_info *slottable)
{
	int i, ret = 0;
				
	for (i=0; i<24; i++)					/* Increment primary scan channel */
	{
		if (slottable[i].active)
		{
			if (slottable[i].type == DECT_SLOTTYPE_SCAN)
			{
				slottable[i].channel++;
				if (slottable[i].channel > 9)
					slottable[i].channel = 0;

				slottable[i].update = 1;

				ret = 1;

				/*printk("slot %u will now scan on carrier %u\n", i, slottable[i].channel); */
			}
		}
	}

	return ret;
}
