/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * pex-status: Print some information about a PEX87xx device
 *
 * Copyright (C) 2025 by Ben Collins <bcollins@kernel.org>
 */

#include <stdio.h>
#include <stdlib.h>

#include "pex87xx.h"

static void check_status(struct pex87xx_device *pex, uint8_t port)
{
	uint32_t status;

	pex87xx_read(pex, 0, port, 0, 0x3a4, &status);

	printf("PEX Status[%d]: %08x\n", port, status);
}

static void print_port_color(uint8_t port, uint32_t status)
{
	/*
	 * I have no idea what reg 0xf70 is nor what these values even mean, but
	 * it seems to correlate with enabling the port with the controller. I
	 * have a 2 node system with a back plane that has a PEX8724. This
	 * U-Boot code sends these commands, which enables things for one of the
	 * nodes, but not the other:
	 *
	 * "Assign Hotplug controller C to Port 2 , A to Port 3 "
	 * pex87xx_write(PLX_PEX8724_I2C_ADDR, 0, 0, 0, 0x3a4, 0x00820A83);
	 * pex87xx_write(PLX_PEX8724_I2C_ADDR, 0, 2, 0, 0xf70, 0x80000008);
	 * pex87xx_write(PLX_PEX8724_I2C_ADDR, 0, 3, 0, 0xf70, 0x80000008);
	 */
	switch (status) {
	case 0x80000008:
		printf(" \e[1;92m%d\e[0m", port);
		break;
	case 0x80000000:
		printf(" \e[34m%d\e[0m", port);
		break;
	case 0:
		printf(" \e[2;31m%d\e[0m", port);
		break;
	default:
		printf(" \e[2;33m%d\e[0m", port);
		break;
	}
}

static void print_port(struct pex87xx_device *pex, uint8_t stn, uint8_t port)
{
	uint32_t status = 0;

	pex87xx_read(pex, stn, port, 0, 0xf70, &status);

	print_port_color(port, status);
}

static void print_modes(struct pex87xx_device *pex)
{
	uint32_t status = 0;

	printf("NTV :");
	pex87xx_read(pex, 0, 0, PEX_MODE_NT_VIRT, 0xf70, &status);
	print_port_color(0, status);
	pex87xx_read(pex, 0, 1, PEX_MODE_NT_VIRT, 0xf70, &status);
	print_port_color(1, status);
	printf("\n");

	printf("NTL :");
	pex87xx_read(pex, 0, 0, PEX_MODE_NT_LINK, 0xf70, &status);
	print_port_color(0, status);
	pex87xx_read(pex, 0, 1, PEX_MODE_NT_LINK, 0xf70, &status);
	print_port_color(1, status);
	printf("\n");

	printf("DMA :");
	pex87xx_read(pex, 0, 0, PEX_MODE_DMA, 0xf70, &status);
	print_port_color(0, status);
	pex87xx_read(pex, 0, 1, PEX_MODE_DMA, 0xf70, &status);
	print_port_color(1, status);
	pex87xx_read(pex, 0, 2, PEX_MODE_DMA, 0xf70, &status);
	print_port_color(2, status);
	pex87xx_read(pex, 0, 3, PEX_MODE_DMA, 0xf70, &status);
	print_port_color(3, status);
	printf("\n");

	printf("DRAM:");
	pex87xx_read(pex, 0, 4, PEX_MODE_DMA, 0xf70, &status);
	print_port_color(0, status);
	printf("\n");
}

static void management_port(struct pex87xx_device *pex)
{
	uint32_t mngmt = 0, vls_mask = 0;
	int i, vs_num = 0;

	pex87xx_read(pex, 0, 0, 0, 0x354, &mngmt);
	printf("Management Port Config: %08x\n", mngmt);
	pex87xx_read(pex, 0, 0, 0, 0x358, &vls_mask);
	printf("VLS Mask: %08x\n", vls_mask);

	if ((mngmt == 0) && ((vls_mask & ~(1 << 0)) == 0))
		printf("Device is in VS mode, but not management port\n");

	printf("Management Port Active: %d\n", mngmt & 0x1f);
	printf("Management Port Redundant: %d\n", (mngmt >> 8) & 0x1F);

	for (i = 0; i < 8; i++) {
		uint32_t status;

		if (!(vls_mask & (1 << i)))
			continue;

		vs_num++;

		pex87xx_read(pex, 0, 0, 0, 0x360 + (i * 4), &status);

		printf("  VS[%d] Upstream Port Num: %d\n", i, status & 0x1f);

		pex87xx_read(pex, 0, 0, 0, 0x380 + (i * 4), &status);

		printf("  VS[%d] Dnstream Ports: %06x\n", i,
		       status & 0x00FFFFFF);
	}

	if (vs_num == 1)
		printf("  Device is in standard mode\n");

	printf("\n");
}

static void check_enabled_ports(struct pex87xx_device *pex)
{
	printf("Ports enabled: %04llx\n", pex->ports);
}

int main(int argc, char *argv[])
{
	struct pex87xx_device *pex;
	uint8_t bus, dev;
	int i;

	if (argc != 3) {
		fprintf(stderr, "Usage: pex-status <bus> <dev>\n");
		exit(1);
	}

	bus = strtol(argv[1], NULL, 0);
	dev = strtol(argv[2], NULL, 0);

	pex = pex87xx_open(bus, dev);
	if (pex == NULL) {
		perror("open");
		exit(1);
	}

	check_enabled_ports(pex);
	management_port(pex);

	check_status(pex, 0);
	check_status(pex, 8);

	for (i = 0; i <= 1; i++) {
		int c;

	        printf("PEX Ports[%d]:", i);

		for (c = 0; c < 16; c++) {
			if (!PEX_PORT_ENABLED(pex, c))
				continue;
			print_port(pex, i, c);
		}

		printf("\n");
	}

	print_modes(pex);

	pex87xx_close(pex);

	exit(0);
}
