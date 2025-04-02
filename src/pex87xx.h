/* SPDX-License-Identifier: GPL-2.0+ WITH Linux-syscall-note */
/*
 * pex87xx.h: Header for the PEX87xx I2C slave interface
 *
 * Rajat Jain <rajatjain@juniper.net>
 * Copyright 2014 Juniper Networks
 *
 * Copyright (C) 2025 by Ben Collins <bcollins@kernel.org>
 */

#ifndef __PEX87XX_H__
#define __PEX87XX_H__

#include <sys/types.h>
#include <stdint.h>

#define PEX_MODE_TRANSPARENT	0x00
#define PEX_MODE_NT_LINK	0x01
#define PEX_MODE_NT_VIRT	0x02
#define PEX_MODE_DMA		0x03

#define PEX_PORT_ENABLED(__pex, __p) \
	((1 << __p) & __pex->ports)

struct pex87xx_device {
	uint16_t	ven_id;
	uint16_t	dev_id;
	uint8_t		rev;
	const char	name[16];
	uint8_t		i2c_bus;
	uint8_t		i2c_dev;
	uint64_t	ports;
	uint8_t		stns;
	uint8_t		stn_mask;
	uint8_t		ports_per_stn;
	int		fd;

	uint32_t (*create_cmd)(uint8_t cmd, uint8_t port, uint8_t mode,
			       uint8_t stn, uint16_t reg, uint8_t mask);
};

struct pex87xx_device *pex87xx_open(uint8_t bus, uint8_t id);
void pex87xx_close(struct pex87xx_device *pex);

int pex87xx_read(struct pex87xx_device *pex, uint8_t stn, uint8_t port,
		 uint8_t mode, uint32_t reg, uint32_t *val);

int pex87xx_write(struct pex87xx_device *pex, uint8_t stn, uint8_t port,
		  uint8_t mode, uint32_t reg, uint32_t val);

int pex87xx_probe_bus(uint8_t bus);

#endif
