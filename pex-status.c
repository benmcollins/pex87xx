#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "pex87xx.h"

#define PXADR	0x38
#define PXDEV	"/dev/i2c-2"

#define PLX_PCI_VENDOR_ID_PLX	0x10B5
#define PLX_PCI_DEVICE_ID_8724	0x8724

#define PXPRT(__p)      ((1 << __p) & enabled_ports)

static uint16_t enabled_ports = 0x70f;

static void check_status(int file, uint8_t port)
{
	uint32_t status;

	pex87xx_read(file, PXADR, 0, 0, port, 0x3a4, &status);

	printf("PEX Status[%d]: %08x\n", port, status);
}

static void print_port_color(uint8_t port, uint32_t status)
{
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

static void print_port(int file, uint8_t stn, uint8_t port)
{
	uint32_t status = 0;

	pex87xx_read(file, PXADR, stn, 0, port, 0xf70, &status);

	print_port_color(port, status);
}

static void print_modes(int file)
{
	uint32_t status = 0;

	printf("NTV :");
	pex87xx_read(file, PXADR, 0, MODE_NT_VIRT, 0, 0xf70, &status);
	print_port_color(0, status);
	pex87xx_read(file, PXADR, 0, MODE_NT_VIRT, 1, 0xf70, &status);
	print_port_color(1, status);
	printf("\n");

	printf("NTL :");
	pex87xx_read(file, PXADR, 0, MODE_NT_LINK, 0, 0xf70, &status);
	print_port_color(0, status);
	pex87xx_read(file, PXADR, 0, MODE_NT_LINK, 1, 0xf70, &status);
	print_port_color(1, status);
	printf("\n");

	printf("DMA :");
	pex87xx_read(file, PXADR, 0, MODE_DMA, 0, 0xf70, &status);
	print_port_color(0, status);
	pex87xx_read(file, PXADR, 0, MODE_DMA, 1, 0xf70, &status);
	print_port_color(1, status);
	pex87xx_read(file, PXADR, 0, MODE_DMA, 2, 0xf70, &status);
	print_port_color(2, status);
	pex87xx_read(file, PXADR, 0, MODE_DMA, 3, 0xf70, &status);
	print_port_color(3, status);
	printf("\n");

	printf("DRAM:");
	pex87xx_read(file, PXADR, 0, MODE_DMA, 4, 0xf70, &status);
	print_port_color(0, status);
	printf("\n");
}

static void print_pci_ids(int file)
{
	uint32_t status = 0;
	uint16_t ven, dev;

	pex87xx_read(file, PXADR, 0, 0, 0, PCI_VENDOR_ID, &status);
	ven = status & 0xffff;
	dev = status >> 16;

	pex87xx_read(file, PXADR, 0, 0, 0, 0x08, &status);

	printf("Device: ven[%04x] dev[%04x] rev[%02X]\n", ven, dev,
	       status & 0xff);

	if (ven != PLX_PCI_VENDOR_ID_PLX ||
	    dev != PLX_PCI_DEVICE_ID_8724) {
		printf("Unknown device\n");
		exit(1);
	}
}

static void management_port(int file)
{
	uint32_t mngmt = 0, vls_mask = 0;
	int i, vs_num = 0;

	pex87xx_read(file, PXADR, 0, 0, 0, 0x354, &mngmt);
	printf("Management Port Config: %08x\n", mngmt);
	pex87xx_read(file, PXADR, 0, 0, 0, 0x358, &vls_mask);
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

		pex87xx_read(file, PXADR, 0, 0, 0, 0x360 + (i * 4), &status);

		printf("  VS[%d] Upstream Port Num: %d\n", i, status & 0x1f);

		pex87xx_read(file, PXADR, 0, 0, 0, 0x380 + (i * 4), &status);

		printf("  VS[%d] Dnstream Ports: %06x\n", i,
		       status & 0x00FFFFFF);
	}

	if (vs_num == 1)
		printf("  Device is in standard mode\n");

	printf("\n");
}

static void check_enabled_ports(int file)
{
	uint32_t status;

	pex87xx_read(file, PXADR, 0, 0, 0, 0x314, &status);

	enabled_ports &= status;

	printf("Ports enabled: %02x\n", enabled_ports);
}

int main()
{
	int file;
	int i;

	if ((file = open(PXDEV, O_RDWR)) < 0) {
		perror("open");
		exit(1);
	}

	if (ioctl(file, I2C_SLAVE, PXADR) < 0) {
		perror("i2c slave");
		exit(1);
	}

	print_pci_ids(file);
	check_enabled_ports(file);
	management_port(file);

	check_status(file, 0);
	check_status(file, 8);

	for (i = 0; i <= 1; i++) {
		int c;

	        printf("PEX Ports[%d]:", i);

		for (c = 0; c < 16; c++) {
			if (!PXPRT(c))
				continue;
			print_port(file, i, c);
		}

		printf("\n");
	}

	print_modes(file);

	exit(0);
}
