/* SPDX-License-Identifier: GPL-2.0+ WITH Linux-syscall-note */
/*
 * pex87xx.c: Implementation for the PEX87xx I2C slave interface
 *
 * Rajat Jain <rajatjain@juniper.net>
 * Copyright 2014 Juniper Networks
 *
 * Copyright (C) 2025 by Ben Collins <bcollins@kernel.org>
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/pci_regs.h>
#include <sys/ioctl.h>
#include <asm/byteorder.h>

#include "pex87xx.h"

#define MASK_BYTE0			0x01
#define MASK_BYTE1			0x02
#define MASK_BYTE2			0x04
#define MASK_BYTE3			0x08
#define MASK_BYTE_ALL			(MASK_BYTE0 | MASK_BYTE1 |\
					 MASK_BYTE2 | MASK_BYTE3)

#define PEX87XX_CMD(val)		(((val) & 7) << 24)
#define PEX87XX_CMD_WR			0x03
#define PEX87XX_CMD_RD			0x04

#define PEX87XX_BYTE_ENA(val)		(((val) & 0xF) << 10)
#define PEX87XX_REG(val)		(((val) >> 2) & 0x3FF)

/* PEX87xx Device specific register defines */
#define PEX87XX_MODE(val)		(((val) & 3) << 20)
#define PEX87XX_STN(val)		(((val) & 3) << 18)
#define PEX87XX_PORT(val)		(((val) & 7) << 15)

static uint32_t pex87xx_create_cmd(uint8_t cmd, uint8_t port, uint8_t mode,
				   uint8_t stn, uint16_t reg, uint8_t mask)
{
	uint32_t cmd_send;

	cmd_send  = PEX87XX_CMD(cmd);
	cmd_send |= PEX87XX_MODE(mode);
	cmd_send |= PEX87XX_STN(stn);
	cmd_send |= PEX87XX_PORT(port);
	cmd_send |= PEX87XX_BYTE_ENA(mask);
	cmd_send |= PEX87XX_REG(reg);

	return __cpu_to_be32(cmd_send);
}

#define PEX_VENDOR_PLX			0x10b5
#define PEX_VENDOR_BROADCOM		0x14E4
#define PEX_VENDOR_LSI			0x1000

static struct pex87xx_device known_devices[] = {
	{
		.dev_id		= 0x8700,
		.name		= "PEX8700",
		.ports		= 0x0000000f,
		.stns		= 1,
		.stn_mask	= 0x1,
		.ports_per_stn	= 4,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8712,
		.name		= "PEX8712",
		.ports		= 0x0000000f,
		.stns		= 1,
		.stn_mask	= 0x1,
		.ports_per_stn	= 4,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8713,
		.name		= "PEX8713",
		.ports		= 0x00003f3f,
		.stns		= 2,
		.stn_mask	= 0x3,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8714,
		.name		= "PEX8714",
		.ports		= 0x0000001f,
		.stns		= 1,
		.stn_mask	= 0x1,
		.ports_per_stn	= 5,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8716,
		.name		= "PEX8716",
		.ports		= 0x0000000f,
		.stns		= 1,
		.stn_mask	= 0x1,
		.ports_per_stn	= 4,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8717,
		.name		= "PEX8717",
		.ports		= 0x00003f3f,
		.stns		= 2,
		.stn_mask	= 0x3,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8718,
		.name		= "PEX8718",
		.ports		= 0x0000000f,
		.stns		= 1,
		.stn_mask	= 0x1,
		.ports_per_stn	= 4,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8723,
		.name		= "PEX8723",
		.ports		= 0x0000070f,
		.stns		= 2,
		.stn_mask	= 0x3,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8724,
		.name		= "PEX8724",
		.ports		= 0x0000070f,
		.stns		= 2,
		.stn_mask	= 0x3,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8725,
		.name		= "PEX8725",
		.ports		= 0x00003f3f,
		.stns		= 2,
		.stn_mask	= 0x3,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8732,
		.name		= "PEX8732",
		.ports		= 0x00000f0f,
		.stns		= 2,
		.stn_mask	= 0x3,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8733,
		.name		= "PEX8733",
		.ports		= 0x003f3f3f,
		.stns		= 3,
		.stn_mask	= 0x7,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8734,
		.name		= "PEX8734",
		.ports		= 0x000000ff,
		.stns		= 2,
		.stn_mask	= 0x3,
		.ports_per_stn	= 4,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8747,
		.name		= "PEX8747",
		.ports		= 0x00030301,
		.stns		= 3,
		.stn_mask	= 0x7,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8748,
		.name		= "PEX8748",
		.ports		= 0x000f0f0f,
		.stns		= 3,
		.stn_mask	= 0x7,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8749,
		.name		= "PEX8749",
		.ports		= 0x003f3f3f,
		.stns		= 3,
		.stn_mask	= 0x7,
		.ports_per_stn	= 8,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8750,
		.name		= "PEX8750",
		.ports		= 0x00000fff,
		.stns		= 3,
		.stn_mask	= 0x7,
		.ports_per_stn	= 4,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8764,
		.name		= "PEX8764",
		.ports		= 0x0000ffff,
		.stns		= 4,
		.stn_mask	= 0xf,
		.ports_per_stn	= 4,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
		.dev_id		= 0x8780,
		.name		= "PEX8780",
		.ports		= 0x0000ffff,
		.stns		= 5,
		.stn_mask	= 0x1f,
		.ports_per_stn	= 4,
		.create_cmd	= pex87xx_create_cmd,
	},
	{
                .dev_id		= 0x8796,
                .name		= "PEX8796",
                .ports		= 0x00ffffff,
                .stns		= 6,
                .stn_mask	= 0x3f,
                .ports_per_stn	= 4,
		.create_cmd	= pex87xx_create_cmd,
        },
	{0},
};

int pex87xx_read(struct pex87xx_device *pex, uint8_t stn, uint8_t port,
		 uint8_t mode, uint32_t reg, uint32_t *val)
{
	uint32_t send, recv;
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = pex->i2c_dev,
			.len = 4,
			.flags = 0,
			.buf = (uint8_t *)&send,
		},
		{
			.addr = pex->i2c_dev,
			.len = 4,
			.flags = I2C_M_RD,
			.buf = (uint8_t *)&recv,
		}
        };
	struct i2c_rdwr_ioctl_data data = {
		.msgs = msgs,
		.nmsgs = 2,
        };

	if (!PEX_PORT_ENABLED(pex, port))
		return -EINVAL;

	send = pex->create_cmd(PEX87XX_CMD_RD, port, mode, stn, reg,
			       MASK_BYTE_ALL);

	ret = ioctl(pex->fd, I2C_RDWR, &data);

	*val = __be32_to_cpu(recv);

	return ret;
}

int pex87xx_write(struct pex87xx_device *pex, uint8_t stn, uint8_t port,
		  uint8_t mode, uint32_t reg, uint32_t val)
{
	uint32_t cmd[2];
	uint8_t *send = (uint8_t *)cmd;
	struct i2c_msg msgs[] = {
		{
			.addr = pex->i2c_dev,
			.len = 8,
			.flags = 0,
			.buf = send,
		}
	};
	struct i2c_rdwr_ioctl_data data = {
		.msgs = msgs,
		.nmsgs = 1,
	};

	if (!PEX_PORT_ENABLED(pex, port))
		return -EINVAL;

	cmd[0] = pex->create_cmd(PEX87XX_CMD_WR, port, mode, stn, reg,
				 MASK_BYTE_ALL);
	cmd[1] = __cpu_to_be32(val);

	return ioctl(pex->fd, I2C_RDWR, &data);
}

static void check_enabled_ports(struct pex87xx_device *pex)
{
	uint32_t status;

	pex87xx_read(pex, 0, 0, 0, 0x314, &status);

	pex->ports &= status;
}

static int probe_one(int fd, struct pex87xx_device **pex, uint8_t bus,
		     uint8_t id)
{
	struct pex87xx_device *new = NULL;
	struct pex87xx_device tmp;
	uint32_t status;
	uint16_t ven, dev;
	uint8_t rev;
	int i;

	/* Setup a tmp to probe */
	tmp.fd = fd;
	tmp.i2c_bus = bus;
	tmp.i2c_dev = id;
	tmp.ports = 1;

	if (pex87xx_read(&tmp, 0, 0, 0, PCI_VENDOR_ID, &status) < 0)
		return -1;

	ven = status & 0xffff;
	dev = status >> 16;

	if (pex87xx_read(&tmp, 0, 0, 0, 0x08, &status) < 0)
		return -1;

	rev = status & 0xff;

	switch (ven) {
	case PEX_VENDOR_LSI:
	case PEX_VENDOR_BROADCOM:
	case PEX_VENDOR_PLX:
		/* All good */
		break;
	default:
		return -1;
	}

	for (i = 0; known_devices[i].name; i++) {
		if (known_devices[i].dev_id != dev)
			continue;

		fprintf(stderr, "Found %s-%02X at %x-%04x\n",
			known_devices[i].name, rev, bus, id);

		if (pex == NULL)
			return i;

		new = malloc(sizeof(*new));
		if (new == NULL)
			return -1;

		memcpy(new, &known_devices[i], sizeof(*new));
		new->fd = fd;
		new->i2c_bus = bus;
		new->i2c_dev = id;
		new->rev = rev;
		new->ven_id = ven;

		check_enabled_ports(new);

		*pex = new;

		return i;
	}

	return -1;
}

struct pex87xx_device *pex87xx_open(uint8_t bus, uint8_t id)
{
	struct pex87xx_device *new = NULL;
	char path[256];
	int fd;

	sprintf(path, "/dev/i2c-%d", bus);
	if ((fd = open(path, O_RDWR)) < 0)
		return NULL;

	if (ioctl(fd, I2C_SLAVE, id) < 0)
		goto open_fail;

	probe_one(fd, &new, bus, id);

open_fail:
	if (new == NULL)
		close(fd);

	return new;
}

void pex87xx_close(struct pex87xx_device *pex)
{
	if (pex == NULL)
		return;

	close(pex->fd);
	free(pex);
}

int pex87xx_probe_bus(uint8_t bus)
{
	char path[256];
	uint8_t id;
	int fd, count = 0;

	sprintf(path, "/dev/i2c-%d", bus);

	if ((fd = open(path, O_RDWR)) < 0)
		return -1;

	for (id = 8; id < 0x77; id++) {
		switch (id) {
		case 0x38 ... 0x3f:
		case 0x58 ... 0x5f:
		case 0x68 ... 0x6f:
		case 0x70 ... 0x77:
		case 0x18 ... 0x1f:

			if (ioctl(fd, I2C_SLAVE, id) < 0)
				continue;

			probe_one(fd, NULL, bus, id);
			break;
		default:
			break;
		}
	}

	close(fd);

	return count;
}


