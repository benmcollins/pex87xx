/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * pex-status: Print some information about a PEX87xx device
 *
 * Copyright (C) 2025 by Ben Collins <bcollins@kernel.org>
 */

#include <stdio.h>
#include <stdlib.h>

#include "pex87xx.h"

int main(int argc, char *argv[])
{
	struct pex87xx_device *pex;
	uint8_t bus, dev, stn, port, mode;
	uint32_t reg, value;

	if (argc != 7) {
		fprintf(stderr, "Usage: pex-read <bus> <dev> <stn> <port> <mode> <reg>\n");
		exit(1);
	}

	bus = strtoul(argv[1], NULL, 0);
	dev = strtoul(argv[2], NULL, 0);
	stn = strtoul(argv[3], NULL, 0);
	port = strtoul(argv[4], NULL, 0);
	mode = strtoul(argv[5], NULL, 0);
	reg = strtoul(argv[6], NULL, 0);

	pex = pex87xx_open(bus, dev);
	if (pex == NULL) {
		perror("open");
		exit(1);
	}

	if (pex87xx_read(pex, stn, port, mode, reg, &value) < 0)
		perror("read");
	else
		printf("%d-%04X[%03x]: %08x\n", bus, dev, reg, value);

	pex87xx_close(pex);

	exit(0);
}
