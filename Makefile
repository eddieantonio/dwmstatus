BIN = status

CFLAGS = -g -Wall -pedantic -std=c11
LDFLAGS = -lX11

all: $(BIN)

test: $(BIN)
	./$<

.PHONY: all test
