
CC = gcc
CFLAGS = -Wall

DEPS = uinput.h btnx.h config_parser.h devices_parser.h device.h
OBJ = btnx.o uinput.o config_parser.o devices_parser.o device.o
BIN = btnx
SCRIPTS_DIR = ./scripts
INSTALL = install.sh
UNINSTALL = uninstall.sh

%.o: %.c $(DEPS)
	$(CC) $< $(CFLAGS) -c -o $@

all: $(OBJ)
	$(CC) -o $(BIN) $^ $(CFLAGS)

clean:
	rm -f $(OBJ) $(BIN)

install:
	$(SCRIPTS_DIR)/$(INSTALL)

uninstall:
	$(SCRIPTS_DIR)/$(UNINSTALL)
