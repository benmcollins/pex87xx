# pex87xx

Userspace I2C interface for PLX PEX87xx line for PCI-e switches

If anyone has docs on the PEX87xx register space, please let me know.

Mainly I am looking to understand this:

    /*
     * I have no idea what reg 0xf70 is nor what these values even mean, but
     * it seems to correlate with enabling the port with the controller. I
     * have a 2 node system with a back plane that has a PEX8724. This
     * U-Boot code sends these commands, which enables things for one of the
     * nodes, but not the other:
     */
    /* "Assign Hotplug controller C to Port 2 , A to Port 3 " */
    pex87xx_write(PLX_PEX8724_I2C_ADDR, 0, 0, 0, 0x3a4, 0x00820A83);
    pex87xx_write(PLX_PEX8724_I2C_ADDR, 0, 2, 0, 0xf70, 0x80000008);
    pex87xx_write(PLX_PEX8724_I2C_ADDR, 0, 3, 0, 0xf70, 0x80000008);
