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
	 * 0x3a4 is the Hotplug enable.
	 *
	 * 0x00898382
	 *            C          B          A
	 *            E    P 9   E    P 3   E    P 2
	 *            1000 1001  1000 0011  1000 0010
	 * 0000 0000  0000 0000  0000 0000  0000 0000
	 *
	 *
	 * 0x00820A83
	 *            C          B          A
	 *            E    P 2   ?    P 10  E    P 3
	 * 0000 0000  1000 0010  0000 1010  1000 0011
	 *
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

/*
 *  * Draco 2
 *              Transp  NTV0 NTV1 NTL0 NTL1 DMA0 DMA1 DMA2 DMA3 DMA_RAM ALUT
 *   Mode          0      2    2    1    1    3    3    3    3     3      3
 *   Stn_Sel      S#      0    0    0    0    0    0    0    0     0      2
 *   Port_Sel     P#      0    1    0    1    0    1    2    3     4     R#
 *
 *   case PLX_FAMILY_SCOUT:
 *   case PLX_FAMILY_DRACO_1:
 *   case PLX_FAMILY_DRACO_2:
 *        bitPosMode    = 20;
 *        bitPosStnSel  = 18;
 *        addr_NTV0Base = 0x3E000;
 *        addr_DmaBase  = 0x20000;
 *        offset_DebugCtrl = 0x350;
 *        offset_NTV0Base  = 0x3E000;
 */

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

	printf("ALUT:");
	pex87xx_read(pex, 2, 0, PEX_MODE_DMA, 0xf70, &status);
	print_port_color(0, status);
}

static void management_port(struct pex87xx_device *pex)
{
	uint32_t mngmt = 0, vls_mask = 0;
	int i, vs_num = 0;

	printf("Management Interface:\n");

	pex87xx_read(pex, 0, 0, 0, 0x354, &mngmt);
	printf("  Port Config: %08x\n", mngmt);
	pex87xx_read(pex, 0, 0, 0, 0x358, &vls_mask);
	printf("  VLS Mask   : %08x\n", vls_mask);

	printf("  Device Mode: ");
	if ((vls_mask & 0xff) != 1) {
		printf("Virtual Switch, %smanagment node\n",
		       mngmt ? "" : "non-");
	} else {
		printf("Standard\n");
	}

	if (mngmt & (1 << 5))
		printf("  Management : Active\n");
	if (mngmt & (1 << 13))
		printf("  Redundant  : Active\n");

	for (i = 0; i < 8; i++) {
		uint32_t status;

		if (!(vls_mask & (1 << i)))
			continue;

		vs_num++;

		pex87xx_read(pex, 0, 0, 0, 0x360 + (i * 4), &status);

		printf("  VS[%d] Upstream Port Num: %d\n", i, status & 0x1f);
		if (i == 0) {
			printf("    NT0: %sabled\n",
			       status & (1 << 13) ? "en" : "dis");
			printf("    NT1: %sabled\n",
			       status & (1 << 21) ? "en" : "dis");
		}
	}
}

static void check_enabled_ports(struct pex87xx_device *pex)
{
	printf("Ports enabled: %04llx\n", pex->ports);
}

static void check_eeprom_status(struct pex87xx_device *pex)
{
	uint32_t status;

	pex87xx_read(pex, 0, 0, 0, 0x260, &status);
	printf("EEPROM: ");
	if (status & (1 << 16)) {
		int width = (status >> 22) & 0x3;

		if (status & (1 << 17)) {
			printf("Capable, but bad CRC or not present\n");
		} else {
			printf("Enabled and present, %dB width\n",
			       width);
		}
	} else {
		printf("Not enabled\n");
	}
}

static void check_captured_bus(struct pex87xx_device *pex)
{
	uint32_t status;

	pex87xx_read(pex, 0, 0, 0, 0x1dc, &status);
	printf("Captured BusNo: %d\n", status & 0xff);
}

static void check_nt_port_status(struct pex87xx_device *pex)
{
	uint32_t status;

	pex87xx_read(pex, 0, 0, 0, 0xc8c, &status);

	printf("NT Port Config: %08x\n", status);
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
	check_eeprom_status(pex);
	management_port(pex);
	check_captured_bus(pex);
	check_nt_port_status(pex);

	printf("\n");

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
