CC ?= gcc
WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
	-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
	-Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
	-Wconversion -Wstrict-prototypes
CFLAGS := -g -std=gnu99 $(WARNINGS)
SRCFILES = $(wildcard *.c)
OBJFILES = $(patsubst %.c, %.o, $(SRCFILES))

.PHONY: all clean

all: arp_ping

$(OBJFILES): $(SRCFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

arp_ping: $(OBJFILES)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	@rm -f $(OBJFILES) arp_ping
