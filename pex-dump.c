/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * pex-dump: Dump full register set for PEX87xx device ports
 *
 * Copyright (C) 2025 by Ben Collins <bcollins@kernel.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pex87xx.h"

int main(int argc, char *argv[])
{
	struct pex87xx_device *pex;
	size_t addr;
	uint8_t port, bus, dev, stn, mode;
	uint32_t reg;

	if (argc != 6) {
		fprintf(stderr, "Usage: pex-dump <bus> <dev> <stn> <port> <mode>\n");
		exit(1);
	}

	bus = strtol(argv[1], NULL, 0);
	dev = strtol(argv[2], NULL, 0);
	stn = strtol(argv[3], NULL, 0);
	port = strtol(argv[4], NULL, 0);
	mode = strtol(argv[5], NULL, 0);

	pex = pex87xx_open(bus, dev);
	if (pex == NULL) {
		perror("opening i2c device");
		exit(1);
	}

	if (!PEX_PORT_ENABLED(pex, port)) {
		fprintf(stderr, "%d is not a valid port\n", port);
		pex87xx_close(pex);
		exit(1);
	}

	for (addr = 0; addr < 4096; addr += 4) {
		if (pex87xx_read(pex, stn, port, mode, addr, &reg) < 0)
			break;
		write(1, &reg, 4);
	}

	pex87xx_close(pex);

	exit(0);
}
