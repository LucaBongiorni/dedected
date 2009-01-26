#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "com_on_air_user.h"

#define DEV "/dev/coa"
#define FNAME_FMT "eeprom_%.1x_%.2x_%.2x_%.2x_%.2x_%.2x.bin"

int main(int argc, char ** argv)
{
	uint8_t eeprom[EEPROM_SIZE+1];
	char fname[99];
	int fd;
	int ret;
	uint16_t val;
	int rfpi_off = 0x204;

	fd = open(DEV, O_RDWR | O_NONBLOCK);
	if (fd < 0)
	{
		printf("!!! couldn't open(\"%s\"): %s\n",
				DEV,
				strerror(errno));
		exit(1);
	}
	val = COA_MODE_EEPROM;
	if (ioctl(fd, COA_IOCTL_MODE, &val)){
		printf("couldn't ioctl(): %s\n",
			strerror(errno));
		exit(1);
	}
	ret = read(fd, eeprom, EEPROM_SIZE+1);
	if (ret != EEPROM_SIZE+1)
	{
		printf("!!! read(\"%s\") returned %d\n",
				DEV,
				ret);
		exit(1);
	}
	ret = close(fd);
	if (ret)
	{
		printf("!!! couldn't close(\"%s\"): %s\n",
				DEV,
				strerror(errno));
		exit(1);
	}

	if ( (eeprom[0] == 0) || (eeprom[0] == 3) )
		rfpi_off = 0x203; /* tpye II cards */
	sprintf(fname, FNAME_FMT,
		eeprom[0],
		eeprom[rfpi_off+0],
		eeprom[rfpi_off+1],
		eeprom[rfpi_off+2],
		eeprom[rfpi_off+3],
		eeprom[rfpi_off+4]
		);
	fd = open(fname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0)
	{
		printf("!!! couldn't open(\"%s\"): %s\n",
				fname,
				strerror(errno));
		exit(1);
	}
	ret = write(fd, eeprom, EEPROM_SIZE+1);
	if (ret != EEPROM_SIZE+1)
	{
		printf("!!! write(\"%s\") returned %d\n",
				DEV,
				ret);
		exit(1);
	}
	ret = close(fd);
	if (ret)
	{
		printf("!!! couldn't close(\"%s\"): %s\n",
				fname,
				strerror(errno));
		exit(1);
	}

	printf("successfully wrote EEPROM to %s\n", fname);

	return 0;
}
