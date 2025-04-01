#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "pex87xx.h"

#define PXPRT(__p)      ((1 << __p) & enabled_ports)

static uint16_t enabled_ports = 0x70f;

#define PEX_PLX_VENDOR		0x10B5

#define PEX_8713_ID		0x8713
#define PEX_8724_ID		0x8724

struct pex87xx_device {
	uint16_t	ven_id;
	uint16_t	dev_id;
	uint8_t		rev;
	const char	*name;
	uint8_t		i2c_bus;
	uint8_t		i2c_dev;
	int		fd;
};

static struct pex87xx_device known_devices[] = {
	{
		.ven_id	= PEX_PLX_VENDOR,
		.dev_id	= PEX_8713_ID,
		.name	= "PEX8713",
		.fd	= -1,
	},
	{
		.ven_id	= PEX_PLX_VENDOR,
		.dev_id	= PEX_8724_ID,
		.name	= "PEX8724",
		.fd	= -1,
	},
	{0},
};


int pex87xx_read(int file, uint8_t dev, uint8_t stn, uint8_t mode,
		 uint8_t port, uint32_t reg, uint32_t *val)
{
	uint32_t send;

	if (!PXPRT(port))
		return -EINVAL;

	send = PEX87XX_I2C_CMD(PEX87XX_CMD_RD, port, mode,
			       stn, reg, MASK_BYTE_ALL);

	struct i2c_msg msgs[] = {
		{
			.addr = dev,
			.len = 4,
			.flags = 0,
			.buf = (uint8_t *)&send,
		},
		{
			.addr = dev,
			.len = 4,
			.flags = I2C_M_RD,
			.buf = (uint8_t *)val,
		}
        };
	struct i2c_rdwr_ioctl_data data = {
		.msgs = msgs,
		.nmsgs = 2,
        };

	return ioctl(file, I2C_RDWR, &data);
}

int pex87xx_write(int file, uint8_t dev, uint8_t stn, uint8_t port,
		  uint8_t mode, uint32_t reg, uint32_t val)
{
	uint32_t cmd[2];
	uint8_t *send = (uint8_t *)cmd;

	if (!PXPRT(port))
		return -EINVAL;

	cmd[0] = PEX87XX_I2C_CMD(PEX87XX_CMD_WR, port, mode,
				 stn, reg, MASK_BYTE_ALL);
	cmd[1] = val;

	struct i2c_msg msgs[] = {
		{
			.addr = dev,
			.len = 8,
			.flags = 0,
			.buf = send,
		}
	};
	struct i2c_rdwr_ioctl_data data = {
		.msgs = msgs,
		.nmsgs = 1,
	};

	return ioctl(file, I2C_RDWR, &data);
}

static int probe_one(int fd, uint8_t bus, uint8_t id)
{
	uint32_t status;
	uint16_t ven, dev;
	uint8_t rev;
	int i;

	if (pex87xx_read(fd, id, 0, 0, 0, PCI_VENDOR_ID, &status) < 0)
		return 0;

	ven = status & 0xffff;
	dev = status >> 16;

	if (pex87xx_read(fd, id, 0, 0, 0, 0x08, &status) < 0)
		return 0;
	rev = status & 0xff;

	for (i = 0; known_devices[i].name; i++) {
		if (known_devices[i].ven_id != ven ||
		    known_devices[i].dev_id != dev)
			continue;

		printf("Found %s-%02X at %x-%04x\n",
		       known_devices[i].name, rev, bus, id);
		return 1;
	}

	return 0;
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

			count += probe_one(fd, bus, id);
			break;
		}
	}

	close(fd);

	return count;
}
