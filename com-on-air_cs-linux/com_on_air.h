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

#ifndef COM_ON_AIR_H
#define COM_ON_AIR_H

#include <linux/kernel.h>
#include <linux/module.h>

struct coa_info
{
	struct pcmcia_device    *p_dev;
	int                     open;

	int                     irq;
	int                     irq_count;
	struct timespec         irq_timestamp;

	struct pcmcia_device    *links[2];

	caddr_t                 membase[2];
	u_int                   memsize[2];


        /* hardware configs */
        unsigned short          *sc14421_base;
        unsigned int            card_id; /* index into com_on_air_ids[] */
        unsigned int            radio_type;

	unsigned int            operation_mode;

        /* struct fp_cfg        *fp_config; */
        /* struct pp_cfg        *pp_config; */
        struct sniffer_cfg     *sniffer_config;

        struct kfifo           rx_fifo;
	spinlock_t             rx_fifo_lock;
        struct kfifo           tx_fifo;
	spinlock_t             tx_fifo_lock;
};


int get_card_id(void);

/* radio types */
#define COA_RADIO_TYPE_II	0
#define COA_RADIO_TYPE_III	1
#define COA_RADIO_FREEPAD	2

#endif
