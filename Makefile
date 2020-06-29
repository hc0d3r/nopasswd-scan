CFLAGS+=-Wall -Wextra -static
LDFLAGS+=-lutil

STRIP?=strip

all: nopasswd-scan

nopasswd-scan: nopasswd-scan.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
	$(STRIP) $@

.PHONY: clean
clean:
	rm -f nopasswd-scan
