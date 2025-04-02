/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * pex-probe: Scan i2c busses for possibel PEX87xx devices
 *
 * Copyright (C) 2025 by Ben Collins <bcollins@kernel.org>
 */

#include <stdlib.h>

#include "pex87xx.h"

int main()
{
	int i;

	for (i = 0; i <= 128; i++) {
		if (pex87xx_probe_bus(i) < 0)
			break;
	}

	exit(0);
}
