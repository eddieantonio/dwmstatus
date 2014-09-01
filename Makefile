BIN = dwmstatus

CFLAGS = -Wall -Werror -pedantic -std=c11 -Os
LDFLAGS = -lX11

all: $(BIN)

test: $(BIN)
	./$<

.PHONY: all test
