CC=gcc
CFLAGS=-Wall -Wpedantic -Wextra -I.
OUT=./build
SRCS=main.c
OBJS=$(SRCS:%=$(OUT)/%.o)
BIN=$(shell cd .. && basename `pwd`)

$(OUT)/$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OUT)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: run
run:
	@make
	./build/$(BIN)

.PHONY: clean
clean:
	rm -f $(OUT)/*.o
