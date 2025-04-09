# Linux I2C interface to PEX87xx PCIe Switches

## The saga continues

Broadcome has locked up all their docs in the PEX Editor. A java
application that is protected by a license which is an RSA key that shows
which chips you have access to. Further, the chip data books are encrypted
until a license key is presented.

One. I'll repeat One (1). One line of code. That's all it took to bypass all
of that bullshit. The line?

	return "8724";

* Unjar the PEX Editor
* Decompile one select java class
* Add that line of code
* Recompile the java class
* Repack the jar
* Start it up

Seriously. Why even use this bullshit? You have to know you're target audience
is programmers. If this task were on a Scrum board, I'd give it a Story Point of
1.

Anyway, Broadcom sucks at sucking and that is all. I'm using the PEX Editor and
it's not too bad, but it doesn't include some of the indepth descriptions of
NT and VS modes. That I had to dig out of the 8625 docs.

## The original issue

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

On one system I see this:
```
0000:00:00.0 PCI bridge: Freescale Semiconductor Inc T4240 with security (rev 20)
0002:00:00.0 PCI bridge: Freescale Semiconductor Inc T4240 with security (rev 20)
0002:01:00.0 PCI bridge: PLX Technology, Inc. PEX 8724 24-Lane, 6-Port PCI Express Gen 3 (8 GT/s) Switch, 19 x 19mm FCBGA (rev ca)
0002:02:01.0 PCI bridge: PLX Technology, Inc. PEX 8724 24-Lane, 6-Port PCI Express Gen 3 (8 GT/s) Switch, 19 x 19mm FCBGA (rev ca)
0002:02:02.0 PCI bridge: PLX Technology, Inc. PEX 8724 24-Lane, 6-Port PCI Express Gen 3 (8 GT/s) Switch, 19 x 19mm FCBGA (rev ca)
0002:02:03.0 PCI bridge: PLX Technology, Inc. PEX 8724 24-Lane, 6-Port PCI Express Gen 3 (8 GT/s) Switch, 19 x 19mm FCBGA (rev ca)
0002:02:08.0 PCI bridge: PLX Technology, Inc. PEX 8724 24-Lane, 6-Port PCI Express Gen 3 (8 GT/s) Switch, 19 x 19mm FCBGA (rev ca)
0002:02:09.0 PCI bridge: PLX Technology, Inc. PEX 8724 24-Lane, 6-Port PCI Express Gen 3 (8 GT/s) Switch, 19 x 19mm FCBGA (rev ca)
0002:02:0a.0 PCI bridge: PLX Technology, Inc. PEX 8724 24-Lane, 6-Port PCI Express Gen 3 (8 GT/s) Switch, 19 x 19mm FCBGA (rev ca)
0002:03:00.0 Bridge: PLX Technology, Inc. PEX PCI Express Switch NT0 Port Virtual Interface (rev ca)
0002:04:00.0 Non-Volatile memory controller: Micron Technology Inc 2550 NVMe SSD (DRAM-less) (rev 01)
```
One the other, just this:
```
0000:00:00.0 PCI bridge: Freescale Semiconductor Inc T4240 with security (rev 20)
0002:00:00.0 PCI bridge: Freescale Semiconductor Inc T4240 with security (rev 20)
```
The 8724 has 24 lanes, 6 ports. It supports ports 0-3,8-10. The bus numbering above certainly looks like 0-3,8-10,
which would mean all the ports are being directed to the one node.
