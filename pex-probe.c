#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "pex87xx.h"

int main()
{
	int i;

	for (i = 1; i <= 0xff; i++) {
		if (pex87xx_probe_bus(i) < 0)
			break;
	}

	exit(0);
}
