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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kfifo.h>

#include "sc14421.h"
#include "dip_opcodes.h"
#include "com_on_air.h"
#include "sc14421_sniffer.h"
#include "sc14421_firmware.h"
#include "dect.h"


/* dip config register control */
unsigned char scan_init1[] = {0x27,0x00,0xff,0x00,0x5f,0x04,0x00};
unsigned char scan_init2[] = {0xc2,0x05,0x00,0x03,0x00,0x00};

/* hf register type II card */
unsigned char radio_II_chan[] = {0x54,0x80,0x09/* patch */,0xa0,0x00,0x00};
/* hf register type III card */
unsigned char scan_III_init3[] = {0x54,0x80,0x09/* patch */,0xa0,0x00,0x00};

/* hf register */
unsigned char scan_init4[] = {0x15,0xa0,0xff,0x00/* &0x3f */,0x5f,0x04,0x00};

/* dip control */
unsigned char scan_init5[] = {0x27,0x00,0xff,0x00,0x5f,0x05,0x00};

/* dip control */
unsigned char sync_init1[] = {0x27,0x00,0xff,0x00,0x5f,0x04,0x00};
unsigned char sync_init2[] = {0xc2,0x05,0x00,0x03,0x00,0x00};

/* hf register type III card */
unsigned char sync_III_init3[] = {0x54,0x80,0x09/* patch */,0xa0,0x00,0x00};
/* hf register */
unsigned char sync_init4[] = {0x15,0xa0,0xff,0x00/* &0x3f */,0x5f,0x04,0x00};
/* dip control */
unsigned char sync_init5[] = {0x27,0x00,0xff,0x00,0x5f,0x05,0x00};


unsigned char fppacket[53] = {0xAA,0xAA,0xAA,0xE9,0x8A};
unsigned char pppacket[53] = {0x55,0x55,0x55,0x16,0x75};


/* FIXME:auto-generate all this stuff */

int sync_jumptable[] = {
	 JP0, 0,
	 JP2, 0,
	 JP4, 0,
	 JP6, 0,
	 JP8, 0,
	JP10, 0,
	JP12, 0,
	JP14, 0,
	JP16, 0,
	JP18, 0,
	JP20, 0,
	JP22, 0
};
int sync_patchtable[] = {
	 PP0, 0,
	 PP2, 0,
	 PP4, 0,
	 PP6, 0,
	 PP8, 0,
	PP10, 0,
	PP12, 0,
	PP14, 0,
	PP16, 0,
	PP18, 0,
	PP20, 0,
	PP22, 0
};
/* FIXME:end */

int sync_banktable[] = {
	SC14421_RAMBANK1,
	SC14421_RAMBANK1,
	SC14421_RAMBANK1,
	SC14421_RAMBANK1,
	SC14421_RAMBANK2,
	SC14421_RAMBANK2,
	SC14421_RAMBANK2,
	SC14421_RAMBANK2,
	SC14421_RAMBANK3,
	SC14421_RAMBANK3,
	SC14421_RAMBANK3,
	SC14421_RAMBANK3,
	SC14421_RAMBANK4,
	SC14421_RAMBANK4,
	SC14421_RAMBANK4,
	SC14421_RAMBANK4,
	SC14421_RAMBANK5,
	SC14421_RAMBANK5,
	SC14421_RAMBANK5,
	SC14421_RAMBANK5,
	SC14421_RAMBANK6,
	SC14421_RAMBANK6,
	SC14421_RAMBANK6,
	SC14421_RAMBANK6
};


void sniffer_init(struct coa_info *dev)
{
	int ret;

	SC14421_switch_to_bank(
		dev->sc14421_base,
		SC14421_DIPSTOPPED | SC14421_CODEBANK
		);

	ret = SC14421_check_RAM(dev->sc14421_base);
	if (ret)
		printk("Found %u memory r/w errors \n\n", ret);

	switch(dev->sniffer_config->snifftype)
	{
		case SNIFF_SCANFP:
		case SNIFF_SCANPP:
			sniffer_init_sniff_scan(dev);
			break;
		case SNIFF_SYNC:
			sniffer_init_sniff_sync(dev);
			break;
	}
}

void set_channel(struct coa_info *dev, int ch)
{
	int channel;
	if (ch<10)
		channel = 10 - ch;
	else
		channel = ch;

	switch(dev->radio_type)
	{
	case COA_RADIO_TYPE_II:
		radio_II_chan[0] =
			(radio_II_chan[0] & 0xC1) |
			(channel << 1);
		break;
	default:
		printk("ERROR: this radio type is currently not "
			"supported. please update the driver\n");
	}
}

void sniffer_init_sniff_scan(struct coa_info *dev)
{
	volatile uint16_t *sc14421_base = dev->sc14421_base;

/*	printk("loading sniff_scan firmware\n"); */

	SC14421_switch_to_bank(
		sc14421_base,
		SC14421_DIPSTOPPED | SC14421_CODEBANK
		);
	to_dip(
		sc14421_base,
		sc14421_II_sniff_scan_fw,
		ARRAY_SIZE(sc14421_II_sniff_scan_fw));

	SC14421_clear_interrupt(sc14421_base);

	set_channel(dev, dev->sniffer_config->channel);

	if (dev->sniffer_config->snifftype == SNIFF_SCANPP)
	{
		scan_init1[0] &= 0xFE;
		scan_init5[0] &= 0xFE;
	}
	else
	{
		scan_init1[0] |= 0x01;
		scan_init5[0] |= 0x01;
	}

	SC14421_switch_to_bank(
		sc14421_base,
		SC14421_DIPSTOPPED | SC14421_RAMBANK0
		);
	to_dip(
		sc14421_base + 0x00,
		scan_init1,
		ARRAY_SIZE(scan_init1));
	to_dip(
		sc14421_base + 0x10,
		scan_init2,
		ARRAY_SIZE(scan_init2)
		);
	SC14421_switch_to_bank(
		sc14421_base,
		SC14421_DIPSTOPPED | SC14421_RAMBANK1
		);
	to_dip(
		sc14421_base + 0x4A,
		radio_II_chan,
		ARRAY_SIZE(radio_II_chan)
		);
	to_dip(
		sc14421_base + 0x58,
		scan_init5,
		ARRAY_SIZE(scan_init5)
		);
	to_dip(
		sc14421_base + 0x50,
		scan_init4,
		ARRAY_SIZE(scan_init4));

/*	printk("starting dip\n"); */
	SC14421_switch_to_bank(sc14421_base, SC14421_RAMBANK0);

}


void sniffer_init_sniff_sync(struct coa_info *dev)
{
	volatile uint16_t *sc14421_base = dev->sc14421_base;

	printk("loading sniff_sync firmware\n");

	SC14421_switch_to_bank(
		sc14421_base,
		SC14421_DIPSTOPPED | SC14421_CODEBANK
		);
	to_dip(
		sc14421_base,
		sc14421_II_sniff_sync_fw,
		ARRAY_SIZE(sc14421_II_sniff_sync_fw));

	printk("clear interrupt\n");
	SC14421_clear_interrupt(sc14421_base);

	SC14421_switch_to_bank(
		sc14421_base,
		SC14421_DIPSTOPPED | SC14421_RAMBANK0
		);
	to_dip(
		sc14421_base + 0x00,
		sync_init1,
		ARRAY_SIZE(sync_init1)
		);
	to_dip(
		sc14421_base + 0x10,
		sync_init2,
		ARRAY_SIZE(sync_init2));

	set_channel(dev, dev->sniffer_config->channel);

	sync_init5[0] |= 0x01;


	SC14421_switch_to_bank(
		sc14421_base,
		SC14421_DIPSTOPPED | SC14421_RAMBANK1
		);
	to_dip(
		sc14421_base + 0x4A,
		radio_II_chan,
		ARRAY_SIZE(radio_II_chan)
		);
	to_dip(
		sc14421_base + 0x58,
		sync_init5,
		ARRAY_SIZE(sync_init5)
		);
	to_dip(
		sc14421_base + 0x50,
		sync_init4,
		ARRAY_SIZE(sync_init4)
		);

	dev->sniffer_config->status = 0;
	sniffer_clear_slottable(dev->sniffer_config->slottable);


	printk("starting dip\n");
	SC14421_switch_to_bank(sc14421_base, SC14421_RAMBANK0);

}

uint8_t sniffer_irq_handler(struct coa_info *dev)
{
	uint8_t irq = 0;

	if (dev->sc14421_base)
	{
		irq = SC14421_clear_interrupt(dev->sc14421_base);

		switch(dev->sniffer_config->snifftype)
		{
			case SNIFF_SCANFP:
			case SNIFF_SCANPP:
				sniffer_sniff_scan_irq(dev, irq);
				break;
			case SNIFF_SYNC:
				sniffer_sniff_sync_irq(dev, irq);
				break;
		}
	}
	return irq;
}

void sniffer_sniff_scan_irq(struct coa_info *dev, int irq)
{
	volatile uint16_t *sc14421_base = dev->sc14421_base;
	int ret;
	uint8_t station[7];

	if (dev->open)
	{
		SC14421_switch_to_bank(sc14421_base, SC14421_RAMBANK1);

		if ( (SC14421_READ(1) & 0xc0) == 0xc0) /* Checksum ok */
		{
			uint8_t rssi = SC14421_READ(0);
			from_dip(
				fppacket + 5,
				sc14421_base + 6,
				6);

			SC14421_WRITE(1, 0); /* Clear Checksum-Flag */

			if (dect_is_RFPI_Packet(fppacket))
			{

				station[0] = dev->sniffer_config->channel;
				station[1] = rssi;
				memcpy(&station[2], &fppacket[6], 5); /* RFPI */

				ret = kfifo_put(dev->rx_fifo, station, 7);
				if (ret <= 0)
				{
					printk("com_on_air_cs: rx fifo full? "
						"kfifo_put() = %d\n", ret);
				}
			}
		}
	}
}

void sniffer_sniff_sync_irq(struct coa_info *dev, int irq)
{
	volatile uint16_t *sc14421_base = dev->sc14421_base;
	struct sniffer_cfg *config = dev->sniffer_config;
	struct sniffed_packet packet;
	int ret;
	int slot;
	int a;
	int memofs;

	SC14421_switch_to_bank(sc14421_base, SC14421_RAMBANK1);


	if (!(config->status & SNIFF_STATUS_FOUNDSTATION))
	{
		if (irq & 0x01)
		{
#if 0
			printk("N:");
			for (r=0; r<16; r++)
				printk("%.2x ", SC14421_READ(r));

			printk("\n");
#endif
			if ( (SC14421_READ(1) & 0xc0) == 0xc0) /* Checksum ok */
			{
				SC14421_WRITE(1, 0); /* clear checksum flag */

				from_dip(
					fppacket + 5,
					sc14421_base + 6,
					6);

				if (dect_compare_RFPI(fppacket, config->RFPI))
				{
					printk("found station for sync\n");
					config->status |=
						SNIFF_STATUS_FOUNDSTATION;

					SC14421_switch_to_bank(
						sc14421_base,
						SC14421_CODEBANK
						);
					SC14421_write_cmd(
						sc14421_base,
						sync_label_D4,
						BR,
						sync_label_D1
						);
				}
			}
		}
	}
	else if (!(config->status & SNIFF_STATUS_INSYNC))
	{
		if (irq & 0x01)
		{

#if 0
			printk("S:");	
			for (i=0; i<16; i++)
				printk("%.2x ", SC14421_READ(i));
			printk("\n");
#endif
			if ( (SC14421_READ(1) & 0xc0) == 0xc0) /* Checksum ok */
			{
				SC14421_WRITE(1, 0); /* clear checksum flag */
				from_dip(
					fppacket + 5,
					sc14421_base + 6,
					48);

				slot = dect_get_slot(fppacket);
				if (slot != -1)
				{
					/* printk("station in slot %u\n", slot); */
					config->status |= SNIFF_STATUS_INSYNC;
					slot %= 12;
					if (slot%2)
						printk("slot not possible "
							"with this firmware\n");

					config->slottable[slot].active = 1;
					config->slottable[slot].channel =
						config->channel;
					config->slottable[slot].type =
						DECT_SLOTTYPE_CARRIER;
					config->slottable[slot].errcnt = 0;

					sniffer_sync_patchloop(
						dev,
						config->slottable,
						SNIFF_SLOTPATCH_FP
						);
					sniffer_sync_patchloop(
						dev,
						config->slottable,
						SNIFF_SLOTPATCH_PP
						);

					SC14421_switch_to_bank(
						sc14421_base,
						SC14421_CODEBANK
						);
					printk("set jump to %u\n",
						sync_jumptable[slot]);
					SC14421_write_cmd(
						sc14421_base,
						sync_label_D4,
						BR,
						sync_jumptable[slot]
						);

					printk("we are in sync :)\n");



					packet.rssi = SC14421_READ(0x00);
					packet.channel = config->channel;
					packet.slot = slot;
					memcpy(packet.data, fppacket, 53);

					packet.timestamp =
						dev->irq_timestamp;
					ret = kfifo_put(
						dev->rx_fifo,
						(unsigned char*) &packet,
						sizeof(struct sniffed_packet));
					if (ret <= 0)
						printk("com_on_air_cs: rx fifo "
							"full? kfifo_put() "
							"= %d\n", ret);
				}
			}
		}
	}
	else
	{
		if ( (irq & 0x09) == 0x09)
			printk("interrupt too slow , lost packets!\n");

		if (irq & 0x01)
		{

			for (a=0; a<12; a++)
			{
				if (config->slottable[a].active)
				{
					SC14421_switch_to_bank(
						sc14421_base,
						sync_banktable[a]);

					if ( (a/2) % 2)
						memofs = 0x80;
					else
						memofs = 0x00;

					if ( (SC14421_READ(1+memofs) & 0xc0) ==
						0xc0) /* Checksum ok */
					{
						struct sniffed_packet packet;
/* fixing indention / coding style is useless beyond here
 * need to talk to krater about where to cut in functions
 * - mazzoo */
						packet.rssi =
							SC14421_READ(memofs);
						packet.channel =
							config->slottable[a].channel;
						packet.slot = a;
						memcpy(
							packet.data,
							fppacket,
							5);
						from_dip(
							&packet.data[5],
							sc14421_base+memofs+6,
							48);

						if (config->slottable[a].type ==
							DECT_SLOTTYPE_SCAN)
							/* we received data on a scan-slot , channel is incemented before , but we want hear the old channel */
						{
							packet.channel--;
							printk("slot in scanmode\n");
						}

						if (dect_is_multiframe_number(packet.data))			/* if there was a multiframe number , then this packet was in frame 8 (0) */
						{
							/* printk("found multiframe number\n");						 */
							config->framenumber = 1;
						}

						/* if (dev->open) */
						{
							packet.timestamp = dev->irq_timestamp;
							ret = kfifo_put(dev->rx_fifo, (unsigned char*) &packet, sizeof(struct sniffed_packet));
							if (ret <= 0)
							{
								printk("com_on_air_cs: rx fifo full? kfifo_put() = %d\n", ret);
							}
						}



#if 0 
						printk("F:");
			        		for (i=0; i<16; i++)
			        		        printk("%.2x ", SC14421_READ(i+memofs));

			        		printk("  : %.2x : %x\n", irq, a);
#endif
						SC14421_WRITE(1+memofs, 0);	/* clear checksum flag */


						if (dect_update_slottable(config->slottable, a, packet.data))
						{
							config->updateppslots = 1;
							config->updatefpslots = 1;
							/* printk("new slot , must update slots\n"); */
						}

					}
					else
					{
						if (dect_receive_error(config->slottable, a))
						{
							config->updateppslots = 1;
							config->updatefpslots = 1;
							printk("died slot , must update slots\n");
						}
					}
				}
			}

			if ( (!(irq & 0x08)) && (config->updatefpslots) )
			{
				/* printk("patching fp slots\n"); */
				sniffer_sync_patchloop(dev, config->slottable, SNIFF_SLOTPATCH_FP);
				config->updatefpslots = 0;
			}

		}

		if (irq & 0x08)
		{

			for (a=12; a<24; a++)
			{
				if (config->slottable[a].active)
				{

					SC14421_switch_to_bank(sc14421_base, sync_banktable[a]);

					if ( (a/2) % 2)
						memofs = 0x80;
					else
						memofs = 0x00;

					if ( (SC14421_READ(1+memofs) & 0xc0) == 0xc0)		/* Checksum ok */
					{
						struct sniffed_packet packet;

						packet.rssi = SC14421_READ(memofs);
						packet.channel = config->slottable[a].channel;
						packet.slot = a;
						memcpy(packet.data, pppacket, 5);
						from_dip(&packet.data[5], sc14421_base+memofs+6, 48);
						if (config->slottable[a].type == DECT_SLOTTYPE_SCAN)
						{
							packet.channel--;
							printk("slot in scanmode\n");
						}

						/* if (dev->open) */
						{
							packet.timestamp = dev->irq_timestamp;
							ret = kfifo_put(dev->rx_fifo, (unsigned char*) &packet, sizeof(struct sniffed_packet));
							if (ret <= 0)
							{
								printk("com_on_air_cs: rx fifo full? kfifo_put() = %d\n", ret);
							}
						}


#if 0
						printk("F:");
			        		for (i=0; i<16; i++)
			        		        printk("%.2x ", SC14421_READ(i+memofs));

			        		printk("  : %.2x : %x\n", irq, a);
#endif

						SC14421_WRITE(1+memofs, 0);	/* clear checksum flag */


						if (dect_update_slottable(config->slottable, a, packet.data))
						{
							config->updateppslots = 1;
							config->updatefpslots = 1;
							/* printk("new slot , must update slots\n"); */
						}

					}
					else
					{
						if (dect_receive_error(config->slottable, a))
						{
							config->updateppslots = 1;
							config->updatefpslots = 1;
							/* printk("died slot , must update slots\n"); */
						}
					}

				}

			}

			if ( (!(irq & 0x01)) && (config->updateppslots) )
			{
				/* printk("patching pp slots\n"); */
				sniffer_sync_patchloop(dev, config->slottable, SNIFF_SLOTPATCH_PP);
				config->updateppslots = 0;
			}

			if (dect_update_scanchannels(config->slottable))
			{
				config->updateppslots = 1;
				config->updatefpslots = 1;
				/* printk("new slot , must update slots\n"); */
			}

			if (config->framenumber >= 7)
				config->framenumber = 0;
			else
				config->framenumber++;
		}

	}
}


void sniffer_sync_patchloop(struct coa_info *dev, struct dect_slot_info *slottable, int type)
{
	static int fixme_count = 23;

	int slot, offset = 0;
	volatile uint16_t *sc14421_base = dev->sc14421_base;
	struct sniffer_cfg *config = dev->sniffer_config;
	int memofs;


	if (type == SNIFF_SLOTPATCH_PP)
		offset = 12;

	for (slot = offset; slot < (offset+12); slot++)
	{
		if (slottable[slot].update)
		{
			slottable[slot].update = 0;

			if (slottable[slot].active && (slot%2))
			{
				if (fixme_count)
				{
					fixme_count--;
					printk("can't use slot %u with this firmware !\n", slot);
				}
				continue;
			}


			if (slottable[slot].active)
			{

				set_channel(dev, slottable[slot].channel);

				if (slot > 11)
					sync_init5[0] &= 0xFE;
				else
					sync_init5[0] |= 0x01;

				if ( (slot/2) % 2)
					memofs = 0x80;
				else
					memofs = 0x00;

				sync_init5[6] = config->framenumber%8;

				SC14421_switch_to_bank(sc14421_base, sync_banktable[slot]);

				to_dip(sc14421_base + 0x4A + memofs, radio_II_chan, ARRAY_SIZE(radio_II_chan));
				to_dip(sc14421_base + 0x58 + memofs, sync_init5, ARRAY_SIZE(sync_init5));
				to_dip(sc14421_base + 0x50 + memofs, sync_init4, ARRAY_SIZE(sync_init4));


				/* printk("patching slot %u at addr %u\n", slot, sync_patchtable[slot]); */
				SC14421_switch_to_bank(sc14421_base, SC14421_CODEBANK);
				SC14421_write_cmd(sc14421_base, sync_patchtable[slot], JMP, sync_label_28);

			}
			else
			{
				SC14421_switch_to_bank(sc14421_base, SC14421_CODEBANK);
				SC14421_write_cmd(sc14421_base, sync_patchtable[slot], WNT, 2);
				/* printk("patching addr %u for wait\n", sync_patchtable[slot]); */
			}
		}
		else if (slottable[slot].active && (slottable[slot].type == DECT_SLOTTYPE_CARRIER))
		{
			SC14421_switch_to_bank(sc14421_base, sync_banktable[slot]);
			SC14421_WRITE(0x5e, config->framenumber%8);
		}
	}
}


void sniffer_clear_slottable(struct dect_slot_info *slottable)
{
	int i;
	for (i=0; i<24; i++)
	{
		slottable[i].active = 0;
		slottable[i].update = 1;
	}
}


