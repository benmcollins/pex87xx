#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "pex87xx.h"

#define PXPRT(__p)      ((1 << __p) & enabled_ports)

static uint16_t enabled_ports = 0x70f;

#define PXADR   0x38
#define PXDEV   "/dev/i2c-2"

#define PLX_PCI_VENDOR_ID_PLX   0x10B5
#define PLX_PCI_DEVICE_ID_8724  0x8724

int main(int argc, char *argv[])
{
	int file;
	size_t addr;
	uint8_t port;
	uint32_t reg;

	if (argc != 2) {
		fprintf(stderr, "Need a port number\n");
		exit(1);
	}

	port = strtol(argv[1], NULL, 0);

	if (!PXPRT(port)) {
		fprintf(stderr, "%d is not a valid port\n", port);
		exit(1);
	}

	if ((file = open(PXDEV, O_RDWR)) < 0) {
		perror("open");
		exit(1);
	}

	if (ioctl(file, I2C_SLAVE, PXADR) < 0) {
		perror("i2c slave");
		exit(1);
	}


	for (addr = 0; addr < 4096; addr += 4) {
		if (pex87xx_read(file, PXADR, 0, 0, port, addr, &reg) < 0)
			break;
		write(1, &reg, 4);
	}

	exit(0);
}
