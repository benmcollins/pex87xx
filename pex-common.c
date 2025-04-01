#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "pex87xx.h"

#if 0
#define PEX_PORT_ENABLED(__pex, __p) \
	((1 << __p) & __pex->ports)
#else
#define PEX_PORT_ENABLED(__pex, __p) (1)
#endif

#define PEX_PLX_VENDOR		0x10B5

#define PEX_8713_ID		0x8713
#define PEX_8724_ID		0x8724

static struct pex87xx_device known_devices[] = {
	{
		.ven_id		= PEX_PLX_VENDOR,
		.dev_id		= PEX_8713_ID,
		.name		= "PEX8713",
		.ports		= 0x00003F3F,
		.stns		= 2,
		.stn_mask	= 0x3,
		.ports_per_stn	= 8,
	},
	{
		.ven_id		= PEX_PLX_VENDOR,
		.dev_id		= PEX_8724_ID,
		.name		= "PEX8724",
		.ports		= 0x0000070f,
		.stns		= 2,
		.stn_mask	= 0x3,
		.ports_per_stn	= 8,
	},
	{0},
};

int pex87xx_read(struct pex87xx_device *pex, uint8_t stn, uint8_t mode,
		 uint8_t port, uint32_t reg, uint32_t *val)
{
	uint32_t send;

	if (!PEX_PORT_ENABLED(pex, port))
		return -EINVAL;

	send = PEX87XX_I2C_CMD(PEX87XX_CMD_RD, port, mode,
			       stn, reg, MASK_BYTE_ALL);

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
			.buf = (uint8_t *)val,
		}
        };
	struct i2c_rdwr_ioctl_data data = {
		.msgs = msgs,
		.nmsgs = 2,
        };

	return ioctl(pex->fd, I2C_RDWR, &data);
}

int pex87xx_write(struct pex87xx_device *pex, uint8_t stn, uint8_t port,
		  uint8_t mode, uint32_t reg, uint32_t val)
{
	uint32_t cmd[2];
	uint8_t *send = (uint8_t *)cmd;

	if (!PEX_PORT_ENABLED(pex, port))
		return -EINVAL;

	cmd[0] = PEX87XX_I2C_CMD(PEX87XX_CMD_WR, port, mode,
				 stn, reg, MASK_BYTE_ALL);
	cmd[1] = val;

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

	return ioctl(pex->fd, I2C_RDWR, &data);
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

	for (i = 0; known_devices[i].name; i++) {
		if (known_devices[i].ven_id != ven ||
		    known_devices[i].dev_id != dev)
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


