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

#ifndef SC14421_H
#define SC14421_H

//#include <linux/swab.h>

#define SC14421_DIPSTOPPED      0x80
#define SC14421_RAMBANK0        0x00
#define SC14421_RAMBANK1        0x04
#define SC14421_RAMBANK2        0x08
#define SC14421_RAMBANK3        0x0c
#define SC14421_RAMBANK4        0x10
#define SC14421_RAMBANK5        0x14
#define SC14421_RAMBANK6        0x18
#define SC14421_RAMBANK7        0x1c
#define SC14421_CODEBANK        0x20

#if defined(__LITTLE_ENDIAN)
# define SC14421_READ(offset)		sc14421_base[(offset)]
# define SC14421_WRITE(offset, value)	{ sc14421_base[(offset)] = (value); }
#elif defined(__BIG_ENDIAN)
# define SC14421_READ(offset)		swab16(sc14421_base[(offset)])
# define SC14421_WRITE(offset, value)	{ sc14421_base[(offset)] = swab16(value); }
#else
# error "could not determine endianness"
#endif

void set_device_configbase(u_int);
void wait_4_IO_cycles(void);
void to_dip(volatile unsigned short *dst, unsigned char *src, int length);
void from_dip(unsigned char *dst, volatile uint16_t *src, int length);
void SC14421_switch_to_bank(volatile uint16_t *sc14421_base, unsigned char bank);
void SC14421_stop_dip(volatile uint16_t *sc14421_base);
void SC14421_write_cmd(volatile uint16_t *sc14421_base, int label, unsigned char opcode, unsigned char operand);
unsigned char SC14421_clear_interrupt(volatile uint16_t *sc14421_base);
int SC14421_check_RAM(volatile uint16_t *sc14421_base);

#endif
