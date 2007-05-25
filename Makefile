
CC = gcc
CFLAGS = -Wall

DEPS = uinput.h btnx.h config_parser.h
OBJ = btnx.o uinput.o config_parser.o
BIN = btnx

%.o: %.c $(DEPS)
	$(CC) $< $(CFLAGS) -c -o $@

all: $(OBJ)
	$(CC) -o $(BIN) $^ $(CFLAGS)

clean:
	rm -f $(OBJ) $(BIN)