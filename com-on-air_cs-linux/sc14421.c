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
#include <linux/delay.h>
#include <asm/io.h>

#include "sc14421.h"
#include "dip_opcodes.h"

static u_int deviceConfigBase = 0;

void set_device_configbase(u_int configBase)
{
	deviceConfigBase = configBase;
}

void wait_4_IO_cycles()
{
	if (!deviceConfigBase) {
		printk("error: config base not set!\n");
		return;
	}
	inb_p(deviceConfigBase);
	inb_p(deviceConfigBase);
	inb_p(deviceConfigBase);
	inb_p(deviceConfigBase);
}

void to_dip(volatile uint16_t *dst, unsigned char *src, int length)
{
	int i;
	for (i=0; i<length; i++)
#if defined(__LITTLE_ENDIAN)
		*(dst++) = *(src++);
#elif defined(__BIG_ENDIAN)
		*(dst++) =s wab16(*(src++));
#else
#error "endianness not defined"
#endif
}

void from_dip(unsigned char *dst, volatile uint16_t *src, int length)
{
	int i;
	for (i=0; i<length; i++)
#if defined(__LITTLE_ENDIAN)
		*(dst++) = *(src++);
#elif defined(__BIG_ENDIAN)
		*(dst++) = swab16(*(src++));
#else
#error "endianness not defined"
#endif
}

void SC14421_switch_to_bank(volatile uint16_t *sc14421_base, unsigned char bank)
{
	SC14421_WRITE(511, bank);
	wait_4_IO_cycles();
}

void SC14421_stop_dip(volatile uint16_t *sc14421_base)
{
	SC14421_switch_to_bank(sc14421_base, SC14421_DIPSTOPPED);
}

void SC14421_write_cmd(volatile uint16_t *sc14421_base, int label, unsigned char opcode, unsigned char operand)
{
	SC14421_WRITE(label*2 + 0, opcode);
	SC14421_WRITE(label*2 + 1, operand);
}

uint8_t SC14421_clear_interrupt(volatile uint16_t *sc14421_base)
{
        unsigned char int1, int2, cnt = 0;

        int1 = SC14421_READ(511);

	/* is the card still plugged */
        if (int1 == 0xff)
                return 0;

        int2 = int1 & 0x0f;

        while (int1)
        {
                cnt++;
                if (cnt>254)
                        return 0;

                int1 = SC14421_READ(511) & 0x0f;
                int2 |= int1;
        }

        return (int2 & 0x0f);
}



int SC14421_check_RAM(volatile uint16_t *sc14421_base)
{
	unsigned char bank;
	int ErrCnt;
	int i;
	unsigned char x;

	ErrCnt = 0;

	for (i=0; i<8; i++)
	{
		bank = (4*i) | 0x80;
		SC14421_switch_to_bank(sc14421_base, bank);

		for (x=0; x<254; x++)
			SC14421_WRITE(x, (x+i) & 0xff);
	}

	for (i=0; i<8; i++)
	{
		bank = (4*i)|0x80;
		SC14421_switch_to_bank(sc14421_base, bank);
		
		for (x=0; x<254; x++)
		{
			if ( (SC14421_READ(x) & 0xff) != ((x+i) & 0xff) )
			{
				printk("error bank:%.2x %.2x - %.2x != %.2x \n", bank, x, SC14421_READ(x), (unsigned char)((x+i) & 0xff));
				ErrCnt++;
			}

			SC14421_WRITE(x, 0);
		}
	}

	return ErrCnt;
}
