CC=gcc
CFLAGS=-Wall -Werror -O3 -D_GNU_SOURCE

TARGETS = pex-status pex-dump pex-probe

COMMON = pex87xx.o

all: $(TARGETS)

pex-status: pex-status.o $(COMMON)

pex-dump: pex-dump.o $(COMMON)

pex-probe: pex-probe.o $(COMMON)

clean:
	$(RM) $(TARGETS) *.o

.PHONY: clean all
