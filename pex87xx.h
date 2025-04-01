#ifndef __PEX87XX_H__
#define __PEX87XX_H__

#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/pci_regs.h>

#define MODE_TRANSPARENT	0x00
#define MODE_NT_LINK		0x01
#define MODE_NT_VIRT		0x02
#define MODE_DMA		0x03

#define MASK_BYTE0              0x01
#define MASK_BYTE1              0x02
#define MASK_BYTE2              0x04
#define MASK_BYTE3              0x08
#define MASK_BYTE_ALL           (MASK_BYTE0 | MASK_BYTE1 |\
                                 MASK_BYTE2 | MASK_BYTE3)

#define PEX87XX_CMD(val)            (((val) & 7) << 24)
#define PEX87XX_CMD_WR              0x03
#define PEX87XX_CMD_RD              0x04

#define PEX87XX_BYTE_ENA(val)       (((val) & 0xF) << 10)
#define PEX87XX_REG(val)            (((val) >> 2) & 0x3FF)

/* PEX87xx Device specific register defines */
#define PEX87XX_MODE(val)           (((val) & 3) << 20)
#define PEX87XX_STN(val)            (((val) & 3) << 18)
#define PEX87XX_PORT(val)           (((val) & 7) << 15)

#define PEX87XX_I2C_CMD(cmd, port, mode, stn, reg, byte_mask)   \
        (PEX87XX_CMD(cmd) |                                     \
         PEX87XX_MODE(mode) |                                   \
         PEX87XX_STN(stn) |                                     \
         PEX87XX_PORT(port) |                                   \
         PEX87XX_BYTE_ENA(byte_mask) |                          \
         PEX87XX_REG(reg))

int pex87xx_read(int file, uint8_t dev, uint8_t stn, uint8_t mode, uint8_t port,
		 uint32_t reg, uint32_t *val);

int pex87xx_write(int file, uint8_t dev, uint8_t stn, uint8_t port,
		  uint8_t mode, uint32_t reg, uint32_t val);

int pex87xx_probe_bus(uint8_t bus);

#endif
