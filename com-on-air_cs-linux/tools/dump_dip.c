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
 *
 * license:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "dump_dip.h"


uint32_t current_CPUs;
int files_done   = 0;

int opt_byteswap = 0;

#define OP_CODE_SIZE   2 /* 16 bit DIP opcodes */
#define THRESHOLD     50 /* min number of sequential opcodes before dumping */
#define MAX_LABELS  4096


struct op_code * get_op_code(uint8_t c)
{
	struct op_code * ret = NULL;
	struct op_code * p = dip_op_code;

	while (p->mnemonic)
	{
		if (p->code == c)
			return p;
		p++;
	}

	return ret;
}

void dump_fw(uint8_t * fw, int fwsz)
{
	int i, j;
	struct op_code * op;

	int nlabel=0; /* total number of labels in a FW */
	uint8_t label[MAX_LABELS];

	/* first run: collect labels */
	for (i=0; i<fwsz; i+=2)
	{
		op = get_op_code(fw[i]);
		if (!op)
		{
			printf("; !!! internal ERROR :(\n");
			exit(1);
		}
		if (op->param_type == PARAM_LABEL)
			label[nlabel++] = fw[i+1];
		if (nlabel == MAX_LABELS)
		{
			printf("; !!! MAX_LABELS reached.\n");
			printf("; !!! aborting current firmware\n");
			return;
		}
	}

	/* second run: emit .asm code with labels */
	for (i=0; i<fwsz; i+=2)
	{
		op = get_op_code(fw[i]);
		if (!op)
		{
			printf("; !!! internal ERROR :(\n");
			exit(1);
		}

		/* see if we need to label the address */
		for (j=0; j<nlabel; j++){
			if (i/2 == label[j])
			{
				printf("label_%2.2x:\n", label[j]);
				goto label_done;
			}
		}
label_done:

		printf("\t%s", op->mnemonic);
		switch (op->param_type)
		{
			case PARAM_NONE:
				break;
			case PARAM_LABEL:
				printf("\tlabel_%2.2x", fw[i+1]);
				break;
			case PARAM_HEX:
				printf("\t0x%2.2x", fw[i+1]);
				break;
			case PARAM_DEC:
				printf("\t %3.1d", fw[i+1]);
				break;
			default:
				printf("; !!! internal ERROR :(\n");
				exit(1);
		}
		printf("\n");
		current_CPUs &= op->cpu;
	}
}

void handle_mmap(uint8_t * map, int size, char * fname, int fcount)
{
	uint8_t  * p = map;
	int op_count = 0;
	int     rest = size;

	struct op_code * op;
	uint8_t * pstart;
	int FW_count = 0;

	while (rest>0)
	{
		pstart = p;
		while( (op = get_op_code(*p)) )
		{
			p += OP_CODE_SIZE;
			op_count++;
		}

		if (op_count > THRESHOLD)
		{
			current_CPUs = 
				M_14400 |
				M_14401 |
				M_14402 |
				M_14404 |
				M_14405 |
				M_14420 |
				M_14421 |
				M_14422 |
				M_14424;
			printf("; ----------------------------------------\n");
			printf("; firmware_%d_%d offset %d size %d bytes\n; file %s\n",
				fcount,
				FW_count,
				pstart - map,
				op_count * OP_CODE_SIZE,
				fname);
			printf("; ----------------------------------------\n");
			printf("\n");

			dump_fw(pstart, op_count * OP_CODE_SIZE);

			printf("\n");
			printf("; the above firmware is supported by the following CPUs:\n");
			int i=0;
			while (current_CPUs)
			{
				printf(";   %s\n", cpu_name[i++]);
				current_CPUs>>=1;
			}
			printf("\n");
			printf("\n");

			FW_count++;
		}

		pstart += op_count * OP_CODE_SIZE;

		rest -= op_count * OP_CODE_SIZE;
		rest --;
		p++;

		op_count = 0;
	}

}

void handle_file(char * fn)
{
	int ret;
	ret = open(fn, O_RDONLY);
	if (ret < 0)
	{
		printf("; !!! couldn't open(\"%s\"): %s\n",
			fn,
			strerror(errno));
		exit(1);
	}

	int f = ret;

	ret = lseek(f, 0, SEEK_END);
	if (ret < 0)
	{
		printf("; !!! couldn't lseek(SEEK_END): %s\n",
			strerror(errno));
		exit(1);
	}

	int fsz = ret;

	uint8_t * pf = NULL;
	pf = mmap(NULL, fsz, PROT_READ, MAP_PRIVATE, f, 0);
	if (pf == (void *) -1)
	{
#if 0
		/* most commonly directories */
		printf("!!! couldn't mmap(%s): %s\n",
			fn,
			strerror(errno));
#endif
	}else{
		uint8_t * map;
		if (opt_byteswap){
			map = malloc(fsz);
			if (!map)
			{
				printf("; !!! ERROR: couln't malloc()\n");
				exit(1);
			}
			int i;
			for (i=0; i<fsz; i+=2)
			{
				map[i+1] = pf[i+0];
				map[i+0] = pf[i+1];
			}
		}else
			map = pf;

		handle_mmap(map, fsz, fn, files_done);

		if (opt_byteswap)
			free(map);

		files_done++;
		ret = munmap(pf, fsz);
		if (ret < 0)
		{
			printf("; !!! couldn't munmap(\"%s\"): %s\n",
					fn,
					strerror(errno));
			exit(1);
		}
	}

	ret = close(f);
	if (ret < 0)
	{
		printf("; !!! couldn't close(\"%s\"): %s\n",
			fn,
			strerror(errno));
		exit(1);
	}
}

void usage(void)
{
	printf("dump_dip - simple disassembler for sc144xx DIP codes\n");
	printf("\n");
	printf("usage: dump_dip [options] <files>\n");
	printf("options:\n");
	printf("       -b       byteswap\n");
	printf("\n");
	printf("example:\n");
	printf("       dump_dip ../WinCD/install/M* > win_cd_dip.asm\n");
	printf("\n");
}

int main(int argc, char ** argv){
	int n=1;
	
	if (argc == 1)
	{
		usage();
		exit(1);
	}

	while (argv[n][0] == '-')
	{
		switch (argv[n][1]){
			case 'b':
				opt_byteswap = 1;
				break;
			default:
				printf("; !!! ERROR: unknown option %c\n", argv[n][1]);
				usage();
				exit(1);
		}
		n++;
		if (!argv[n])
		{
			usage();
			exit(1);
		}
	}

	while (argv[n])
		handle_file(argv[n++]);

	printf("; +++ processed %d input files\n", files_done);
	return 0;
}
