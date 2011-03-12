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
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/crc32.h>
#include <linux/kfifo.h>
#include <linux/poll.h>

#include <pcmcia/cistpl.h>
#include <pcmcia/ciscode.h>
#include <pcmcia/ds.h>
#include <pcmcia/cisreg.h>

#include <asm/uaccess.h>


#include "com_on_air.h"
#include "com_on_air_user.h" /* ioctls ... */
#include "sc14421.h"
#include "sc14421_sniffer.h"


MODULE_AUTHOR("Matthias Wenzel comonair<a>mazzoo.de;"
              "Andreas Schuler dect<a>badterrorist.com");
MODULE_DESCRIPTION("Dosch&Amand COM-ON-AIR PCMCIA driver");
MODULE_LICENSE("GPL");

#define COA_DEVICE_NAME "com_on_air_cs"
#define COA_FIFO_SIZE 65536

struct coa_info *dev;



static int coa_open(struct inode *inode, struct file *filp)
{
	if (dev->open)
		return -EBUSY;
	dev->open = 1;

	return nonseekable_open(inode, filp);
}

/* we use some "hidden" ioctls */
#define COA_IOCTL_TEST0 0xF000
#define COA_IOCTL_TEST1 0xF001
#define COA_IOCTL_TEST2 0xF002
#define COA_IOCTL_TEST3 0xF003
#define COA_IOCTL_TEST4 0xF004
#define COA_IOCTL_TEST5 0xF005
#define COA_IOCTL_TEST6 0xF006
#define COA_IOCTL_TEST7 0xF007

long coa_ioctl(
		struct file *filp,
		unsigned int cmd,
		unsigned long arg)
{

	unsigned long __user * argp = (unsigned long __user *) arg;

        if (!dev->p_dev->devname)
		return -EIO;

	switch (cmd)
	{
	case COA_IOCTL_MODE:
	{
		uint16_t mode;
		if (copy_from_user(&mode, argp, sizeof(mode)))
		{
			printk(COA_DEVICE_NAME
				": invalid argument in ioctl()\n");
			return -EINVAL;
		}
		switch (mode & COA_MODEMASK)
		{
		case COA_MODE_IDLE:
			printk("com_on_air_cs: stopping DIP\n");
			SC14421_stop_dip(dev->sc14421_base);
			dev->operation_mode = mode;

                        kfifo_reset(&dev->rx_fifo);
                        kfifo_reset(&dev->tx_fifo);

			break;
		case COA_MODE_FP:
			printk("FIXME: implement COA_MODE_FP ;)\n");
			break;
		case COA_MODE_PP:
			printk("FIXME: implement COA_MODE_PP ;)\n");
			break;
		case COA_MODE_SNIFF:

                        kfifo_reset(&dev->rx_fifo);
                        kfifo_reset(&dev->tx_fifo);

			/* activiate sniffer */

			if (!dev->sniffer_config)
				dev->sniffer_config =
					kzalloc(sizeof(*dev->sniffer_config),
						GFP_KERNEL);
			if (!(dev->sniffer_config))
				return -ENOMEM;

			dev->sniffer_config->snifftype = mode & COA_SUBMODEMASK;
			dev->sniffer_config->channel = 0;
			dev->operation_mode = mode;

			sniffer_init(dev);
			printk(COA_DEVICE_NAME": sniffer initialized\n");
			break;
		case COA_MODE_EEPROM:
		{
                        printk("FIXME: pcmcia_read_cis_mem is not exported since 2.6.34, so we can't use it\n");
#if 0
			/* copy EEPROM to fifo */
#ifndef pcmcia_read_cis_mem /* not in any of my kernel headers :( */
			int pcmcia_read_cis_mem(struct pcmcia_socket *s, int attr, u_int addr,
					u_int len, void *ptr);
#endif

			uint8_t id = get_card_id();
			uint8_t * eeprom = kmalloc(EEPROM_SIZE, GFP_KERNEL);
			if (!eeprom) return -ENOMEM;
                        kfifo_in_locked(&dev->rx_fifo, &id, 1, &dev->rx_fifo_lock);
			pcmcia_read_cis_mem(
					dev->p_dev->socket,
					1,
					0,
					EEPROM_SIZE,
					eeprom);
                        kfifo_in_locked(&dev->rx_fifo, eeprom, EEPROM_SIZE, &dev->rx_fifo_lock);
			kfree(eeprom);
#endif
			break;
		}
		case COA_MODE_JAM:
			printk("FIXME: implement COA_MODE_JAM\n");
			break;
		default:
			printk("coa: invalid ioctl() value "
					"for COA_IOCTL_MODE\n");
			return -EINVAL;
		}
		break;
	}
	case COA_IOCTL_RADIO:
		printk("FIXME: implement COA_IOCTL_RADIO\n");
		break;
	case COA_IOCTL_RX:
		printk("FIXME: implement COA_IOCTL_RX\n");
		break;
	case COA_IOCTL_TX:
		printk("FIXME: implement COA_IOCTL_TX\n");
		break;
	case COA_IOCTL_CHAN:
	{
                uint32_t channel;
		if (copy_from_user(&channel, argp, sizeof(channel)))
		{
			printk(COA_DEVICE_NAME": invalid argument "
						"in ioctl()\n");
			return -EINVAL;
		}

		if (! ( ((channel >=  0) && (channel <=  9)) ||
			((channel >= 23) && (channel <= 27)) ) )

			return -EINVAL;

		switch (dev->operation_mode & COA_MODEMASK)
		{
		case COA_MODE_IDLE:
			break;
		case COA_MODE_FP:
			/* FIXME: implement COA_MODE_FP ;) */
			break;
		case COA_MODE_PP:
			/* FIXME: implement COA_MODE_PP ;) */
			break;
		case COA_MODE_SNIFF:
			dev->sniffer_config->channel = channel;
			break;
		case COA_MODE_JAM:
			printk("FIXME: implement COA_MODE_JAM\n");
			break;
		default:
			printk("coa: invalid ioctl() value "
					"for COA_IOCTL_MODE\n");
			return -EINVAL;
		}

		sniffer_init(dev);
		break;
	}
	case COA_IOCTL_SLOT:
		printk("FIXME: implement COA_IOCTL_SLOT\n");
		break;
	case COA_IOCTL_RSSI:
		printk("FIXME: implement COA_IOCTL_RSSI\n");
		break;
	case COA_IOCTL_FIRMWARE:
		printk("FIXME: implement COA_IOCTL_FIRMWARE\n");
		break;
	case COA_IOCTL_SETRFPI:
	{
		uint8_t RFPI[5];
                if (copy_from_user(&RFPI, argp, sizeof(RFPI)))
		{
			printk(COA_DEVICE_NAME": invalid argument in ioctl()\n");
			return -EINVAL;
		}

		switch (dev->operation_mode & COA_MODEMASK)
		{
		case COA_MODE_IDLE:
			break;
		case COA_MODE_FP:
			/* FIXME: implement COA_MODE_FP ;) */
			break;
		case COA_MODE_PP:
			/* FIXME: implement COA_MODE_PP ;) */
			break;
		case COA_MODE_SNIFF:
			memcpy(dev->sniffer_config->RFPI, RFPI, sizeof(dev->sniffer_config->RFPI));
			break;
		case COA_MODE_JAM:
			printk("FIXME: implement COA_MODE_JAM\n");
			break;
		default:
			printk("coa: invalid ioctl() value for COA_IOCTL_MODE\n");
			return -EINVAL;
		}

		break;
	}
	case COA_IOCTL_TEST7:
	case COA_IOCTL_TEST6:
	case COA_IOCTL_TEST5:
	case COA_IOCTL_TEST4:
	case COA_IOCTL_TEST3:
	case COA_IOCTL_TEST2:
	{
		printk("\n\ncom_on_air_cs: saw %d interrupts since loading\n\n\n",
				dev->irq_count);
		break;
	}
	case COA_IOCTL_TEST1:
	{
		static char teststring[] = "awake. "
				"shake dreams from your head my pretty child, "
				"my swet one. choose the day "
				"and choose the sign of your day. "
				"the day's divinity.\n";
		static char * ps = teststring;
		int ret;
                ret = kfifo_in_locked(&dev->rx_fifo, ps, 1, &dev->rx_fifo_lock);
		if (ret <= 0)
			printk("com_on_air_cs: rx fifo full? "
                                        "kfifo_in_locked() = %d\n", ret);
		ps++;
		if (!*ps)
			ps = teststring;
		break;
	}
	case COA_IOCTL_TEST0:
	{
		unsigned char bank;
		int a, b, i;
		printk("dumping complete DIP-RAM\n");
		disable_irq(dev->irq);
		for (i = 0; i < 8; i++)
		{
			bank = (4 * i);
			SC14421_switch_to_bank(dev->sc14421_base, bank);

			printk("Setting Banking-Register to %.2x\n\n", bank);
			for (a = 0; a < 16; a++)
			{
				volatile uint16_t *sc14421_base = dev->sc14421_base;
				for (b = 0; b < 16; b++)
					printk("%.2x ",
						(unsigned char)
						SC14421_READ( a*16 + b) );
#if 0
				if (SC14421_READ(511) & 0x0f)
					printk("%.2x ",
						(unsigned char)
						(SC14421_READ(511)));
				printk("\n");
#endif
			}
		}
		enable_irq(dev->irq);
		break;
	}
	default:
		printk("coa: invalid ioctl()\n");
		return -EINVAL;
	}
	return 0;
}

static unsigned int coa_poll(struct file *file, poll_table * wait)
{
	unsigned int mask = 0;

        if (!dev->p_dev->devname)
		return -EIO;

        if (kfifo_len(&dev->rx_fifo))
		mask |= POLLIN  | POLLRDNORM;
        if (COA_FIFO_SIZE - kfifo_len(&dev->tx_fifo))
		mask |= POLLOUT | POLLWRNORM;
	return mask;
}

static ssize_t coa_read(
		struct file *filp,
		char __user *buf,
		size_t count_want,
		loff_t *ppos)
{
	size_t to_copy;
	size_t not_copied;
	unsigned char *data;

        if (!dev->p_dev->devname)
		return -EIO;

        to_copy = min((size_t)kfifo_len(&dev->rx_fifo), count_want);
	data = kmalloc(to_copy, GFP_KERNEL);
	if (!data)
		return -ENOMEM;

        to_copy = kfifo_out_locked(&dev->rx_fifo, data, to_copy, &dev->rx_fifo_lock);
	if (to_copy < 0) {
		kfree(data);
		return -EIO;
	}

	not_copied = copy_to_user(buf, data, to_copy);
	kfree(data);

	return to_copy - not_copied;
}


static int coa_close(struct inode *inode, struct file *filp)
{
        if (!dev->p_dev->devname)
		return -EIO;
	SC14421_stop_dip(dev->sc14421_base);

	dev->open = 0;

        kfifo_reset(&dev->rx_fifo);
        kfifo_reset(&dev->tx_fifo);

	return 0;
}


static const struct file_operations coa_fops =
{
	.owner   = THIS_MODULE,
	.open    = coa_open,
        .unlocked_ioctl = coa_ioctl,
	.poll    = coa_poll,
	.read    = coa_read,
	.release = coa_close,
};

/**********************************************************************/

static int com_on_air_suspend (struct pcmcia_device *link)
{
	return 0;
}
static int com_on_air_resume (struct pcmcia_device *link)
{
	return 0;
}

static irqreturn_t
com_on_air_irq_handler(int irq, void *dev_id)
{
	struct coa_info * dev = dev_id;

	uint8_t dip_irq = 0;

	jiffies_to_timespec(jiffies, &dev->irq_timestamp);
	dev->irq_count++;

	switch(dev->operation_mode & COA_MODEMASK)
	{
	case COA_MODE_FP:
		/*station_irq_handler(dev); */
		break;
	case COA_MODE_PP:
		/*phone_irq_handler(dev); */
		break;
	case COA_MODE_SNIFF:
		dip_irq = sniffer_irq_handler(dev);
		break;
	default:
		if (dev->sc14421_base)
			dip_irq = SC14421_clear_interrupt(dev->sc14421_base);
	}

	return dip_irq ? IRQ_HANDLED:IRQ_NONE;
}

static int com_on_air_probe (struct pcmcia_device *link)
{
	int ret;

	dev->p_dev = link;
	link->priv = dev;

	link->devname = kzalloc(strlen(COA_DEVICE_NAME), GFP_KERNEL);
        if (!link->devname) 
                return -ENOMEM;

	sprintf(link->devname, COA_DEVICE_NAME);

	printk("com_on_air_cs: >>>>>>>>>>>>>>>>>>>>>>>>\n");
	printk("com_on_air_cs: card in slot        %s\n", link->devname);

	if (link->prod_id[0])
		printk("com_on_air_cs: prod_id[0]          %s\n",
				link->prod_id[0]);
	if (link->prod_id[1])
		printk("com_on_air_cs: prod_id[1]          %s\n",
				link->prod_id[1]);
	if (link->prod_id[2])
		printk("com_on_air_cs: prod_id[2]          %s\n",
				link->prod_id[2]);
	if (link->prod_id[3])
		printk("com_on_air_cs: prod_id[3]          %s\n",
				link->prod_id[3]);

        link->resource[0]->flags |= IO_DATA_PATH_WIDTH_AUTO;
        link->resource[0]->end    = 16;
        link->resource[1]->flags |= 0;  // this is a NOP, right ?!?!

        link->config_flags     = CONF_ENABLE_IRQ;
        link->config_index     = 1;
        link->config_regs      = PRESENT_OPTION;
        link->config_base      = 0x1020;


        link->resource[2]->flags = WIN_DATA_WIDTH_16 | WIN_ENABLE;
        link->resource[2]->start = 0;
        link->resource[2]->end = 0x1000;

        ret = pcmcia_request_window(link, link->resource[2], 500);
	if (ret != 0)
	{
		printk("couldn't pcmcia_request_window() = 0x%x\n", ret);
		goto probe_out_3;
	}

	dev->links[0]   = link;
        dev->memsize[0] = resource_size(link->resource[2]);
        dev->membase[0] = ioremap_nocache(link->resource[2]->start, resource_size(link->resource[2]));

	if (!dev->membase[0])
	{
		printk("com_on_air_cs: ERROR: couldn't ioremap()\n");
		ret = -EIO;
		goto probe_out_2;
	}
	printk("com_on_air_cs: ioremap()'d baseaddr %p\n", dev->membase[0]);

	link->socket->functions = 0;

	dev->irq_count = 0;

        ret = pcmcia_request_irq(link, com_on_air_irq_handler);
	if (ret != 0)
	{
		printk("\ncom_on_air_cs: unable to allocate IRQ %d, ret=%x\n",
                                link->irq, ret);
		dev->irq = -1;
		goto probe_out_1;
	} else {
		printk("com_on_air_cs: registered IRQ %d\n",
                                link->irq);
                dev->irq = link->irq;
	}
	/* FIXME: there are devces which arrive here but can only allocate a
	 *        shared interrupt!
	 * */

        ret = pcmcia_enable_device(link);
	if (ret != 0)
	{
                printk("could not enable pcmcia device\n");
		goto probe_out_0;
	}

	printk("com_on_air_cs: %svalid client.\n",
               (link->config_flags) ? "":"in");
	printk("com_on_air_cs: type          0x%x\n",
	       link->socket->state);

	printk("com_on_air_cs: function      0x%x\n",
	       link->func);


	/*
	 * I found no really easy/sensible source for those on newer kernels -
	 * and they dont seem to be that interesting anyway
	printk("com_on_air_cs: Vcc           %d\n",
	       link->conf.Vcc);

	printk("com_on_air_cs: Vpp1          %d\n",
	       link->conf.Vpp1);
	printk("com_on_air_cs: Vpp2          %d\n",
	       link->conf.Vpp2);
	*/

        printk("com_on_air_cs: config_flags  %d\n",
                        link->config_flags);
        printk("com_on_air_cs: config_base  %d\n",
                        link->config_base);
        printk("com_on_air_cs: config_regs  %d\n",
                        link->config_regs);

        printk("com_on_air_cs: IRQ           0x%x\n",
               link->irq);

	printk("com_on_air_cs: BasePort1     0x%x\n",
               link->resource[0]->start);
	printk("com_on_air_cs: NumPorts1     0x%x\n",
               link->resource[0]->end);
        printk("com_on_air_cs: Attributes1   0x%lx\n",
               link->resource[0]->flags);

	printk("com_on_air_cs: BasePort2     0x%x\n",
               link->resource[1]->start);
	printk("com_on_air_cs: NumPorts2     0x%x\n",
               link->resource[1]->end);
        printk("com_on_air_cs: Attributes2   0x%lx\n",
               link->resource[1]->flags);
	printk("com_on_air_cs: IOAddrLines   0x%x\n",
               link->io_lines);
	printk("com_on_air_cs: has%s function_config\n",
	       (link->function_config) ? "":" no");

        set_device_configbase(link->config_base);

	dev->sc14421_base = ((volatile uint16_t*)(dev->membase[0]));

	ret = get_card_id();
	printk("com_on_air_cs: get_card_id() = %d\n", ret);
	switch (ret)
	{
		case 0:
		case 3:
			dev->radio_type = COA_RADIO_TYPE_II;
			break;
		case 1:
		case 2:
			dev->radio_type = COA_RADIO_TYPE_III;
			break;
		default:
			printk("com_on_air_cs: ERROR; unknown radio type, "
					"please update driver\n");
	}
	dev->card_id = ret;

	printk("com_on_air_cs: -----------------------\n");

	return 0;

probe_out_0:
	pcmcia_disable_device(link);
	free_irq(dev->irq, dev);
probe_out_1:
	iounmap(dev->membase[0]);
probe_out_2:
        pcmcia_release_window(link, link->resource[2]);
probe_out_3:
	kfree(link->devname);
        link->devname = NULL;
	return ret;
}

static void com_on_air_remove(struct pcmcia_device *link)
{

	int j;

	printk("com_on_air_cs: COM-ON-AIR card ejected\n");
	printk("com_on_air_cs: <<<<<<<<<<<<<<<<<<<<<<<\n");

	dev->sc14421_base=0;

	if (dev->membase[0])
	{
		printk("com_on_air_cs: iounmap()ing membase[0]\n");
		iounmap(dev->membase[0]);
	}


	printk("com_on_air_cs: pcmcia_disable_device()\n");
	pcmcia_disable_device(link);
	if (link->devname)
	{
		printk("com_on_air_cs: freeing devname\n");
		kfree(link->devname);
		link->devname = 0;
	}
	if (dev->sniffer_config)
	{
		printk("com_on_air_cs: freeing sniffer_config\n");
		kfree(dev->sniffer_config);
		dev->sniffer_config = 0;
	}
}

static struct pcmcia_device_id com_on_air_ids[] =
{
	/*
	 * the crc32 hashes below are generated by the tool in
	 * Documentation/pcmcia/devicetable.txt
	 */
	PCMCIA_DEVICE_PROD_ID12  ("DECTDataDevice", "PCMCIA F22",
			           0x11fe69e9,       0x253670b2),
	PCMCIA_DEVICE_PROD_ID12  ("DECTDataDevice", "PCMCIA",
			           0x11fe69e9,       0x281f1c5d),
	PCMCIA_DEVICE_PROD_ID1234("DOSCH-AMAND",    "MMAP PCMCIA",
			          "MXM500",         "V1.00",
				   0x4bc552e7,       0x0df519bb,
				   0x09e43c7c,       0x3488c81a),
	PCMCIA_DEVICE_PROD_ID12  ("DECTVoIPDevice", "PCMCIA DA099",
				   0xeabb0be4,       0xd7b915fe),

#if 0
	There are more devices out there, I only own the above three.
	an excerpt from win32 dna.inf:

%String1%=pcmcia.install,PCMCIA\DOSCH-AMAND-MMAP_PCMCIA-C7D7
%String1%=pcmcia.install,PCMCIA\Dosch-Amand-DECT_MultiMedia-BD0D
%String1%=pcmcia.install,PCMCIA\DOSCH_&_AMAND-DECT_MULTIMEDIA-1A9F
%String1%=pcmcia.install,PCMCIA\DECTDataDevice-F13-6433
%String1%=pcmcia.install,PCMCIA\DECTDataDevice-PCMCIA-0EF8
%String4%=pci.install,PCI\VEN_11E3&DEV_0001&SUBSYS_000111E3&REV_00
%String4%=pci.install,PCI\VEN_11E3&DEV_0001&SUBSYS_00011786&REV_32
%String4%=pci.install,PCI\VEN_1786&DEV_0001&SUBSYS_000111E3&REV_00
%String5%=freekey2.install,PCMCIA\DECTDataDevice-PCMCIA-FEF2
%String6%=freekey2.install,PCMCIA\DECTDataDevice-PCMCIA_F22-4BD3
%String6%=freekey2.install,PCMCIA\DECTDataDevice-PCMCIA_F22-BBD9

#endif
	PCMCIA_DEVICE_NULL
};

MODULE_DEVICE_TABLE(pcmcia, com_on_air_ids);

/* returns an index into com_on_air_ids[] */
int get_card_id(void)
{
	u32 hash[4] = { 0, 0, 0, 0};
	int i;
	int found = 0;
	int ret = -1;
	for (i=0; i<4; i++)
	{
		if (dev->p_dev->prod_id[i])
			hash[i] = crc32(0,
					dev->p_dev->prod_id[i],
					strlen(dev->p_dev->prod_id[i]));
	}

	i = 0;
	while(com_on_air_ids[i].match_flags) /* != PCMCIA_DEVICE_NULL */
	{
		if ( (hash[0] == com_on_air_ids[i].prod_id_hash[0]) &&
		     (hash[1] == com_on_air_ids[i].prod_id_hash[1]) &&
		     (hash[2] == com_on_air_ids[i].prod_id_hash[2]) &&
		     (hash[3] == com_on_air_ids[i].prod_id_hash[3]) )
		{
			found = 1;
			ret = i;
		}
		i++;
	}
	if (!found)
		printk("com_on_air_cs: ERROR: get_card_id() "
				"didn't find card, please update driver\n");
	return ret;
}

static struct pcmcia_driver coa_driver =
{
	.owner = THIS_MODULE,
        .name = COA_DEVICE_NAME,
	.probe    = com_on_air_probe,
	.remove   = com_on_air_remove,

	.suspend  = com_on_air_suspend,
	.resume   = com_on_air_resume,

	.id_table = com_on_air_ids,
};

static int __init init_com_on_air_cs(void)
{
	int ret = 0;
	printk(">>> loading " COA_DEVICE_NAME "\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	ret = pcmcia_register_driver(&coa_driver);
	if (ret != 0)
	{
		printk("couldn't pcmcia_register_driver()\n");
		goto init_out_3;
	}

	ret = register_chrdev(0xDEC, COA_DEVICE_NAME, &coa_fops);
	if (ret < 0)
	{
		printk("couldn't register_chrdev()\n");
		goto init_out_2;
	}

	spin_lock_init(&dev->rx_fifo_lock);
        ret = kfifo_alloc(&dev->rx_fifo, COA_FIFO_SIZE, GFP_KERNEL);
        if (ret < 0)
	{
		printk("couldn't kfifo_alloc(dev->rx_fifo)\n");
		goto init_out_1;
	}

	spin_lock_init(&dev->tx_fifo_lock);
        ret = kfifo_alloc(&dev->tx_fifo, COA_FIFO_SIZE, GFP_KERNEL);
        if (ret < 0)
	{
		printk("couldn't kfifo_alloc(dev->tx_fifo)\n");
		goto init_out_0;
	}

	return 0;

init_out_0:
        kfifo_free(&dev->rx_fifo);
init_out_1:
	unregister_chrdev(0xDEC, COA_DEVICE_NAME);
init_out_2:
	pcmcia_unregister_driver(&coa_driver);
init_out_3:
	kfree(dev);
	return ret;
}

static void __exit exit_com_on_air_cs(void)
{
	printk("<<< unloading " COA_DEVICE_NAME "\n");

	if (!dev) return;

	unregister_chrdev(0xDEC, COA_DEVICE_NAME);

	pcmcia_unregister_driver(&coa_driver);

        kfifo_free(&dev->rx_fifo);
        kfifo_free(&dev->tx_fifo);

	kfree(dev);
}

module_init(init_com_on_air_cs);
module_exit(exit_com_on_air_cs);

