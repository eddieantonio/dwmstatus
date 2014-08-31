BIN = status

all: test

test: $(BIN)
	./$<

.PHONY: all test
