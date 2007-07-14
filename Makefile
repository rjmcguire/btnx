
CC = gcc
CFLAGS = -Wall
LFLAGS =
#-lusb

DEPS = uinput.h btnx.h config_parser.h devices_parser.h device.h usb_handler.h
OBJ = btnx.o uinput.o config_parser.o devices_parser.o device.o usb_handler.o
BIN = btnx
SCRIPTS_DIR = ./scripts
INSTALL = install.sh
INSTALL_GENERIC = install-generic.sh
UNINSTALL = uninstall.sh
UNINSTALL_GENERIC = uninstall-generic.sh

%.o: %.c $(DEPS)
	$(CC) $< $(CFLAGS) -c -o $@

all: $(OBJ)
	$(CC) -o $(BIN) $^ $(CFLAGS) $(LFLAGS)

clean:
	rm -f $(OBJ) $(BIN)

install:
	chmod a+rx $(SCRIPTS_DIR)/$(INSTALL)
	$(SCRIPTS_DIR)/$(INSTALL)

uninstall:
	chmod a+rx $(SCRIPTS_DIR)/$(UNINSTALL)
	$(SCRIPTS_DIR)/$(UNINSTALL)
	
install-generic:
	chmod a+rx $(SCRIPTS_DIR)/$(INSTALL_GENERIC)
	$(SCRIPTS_DIR)/$(INSTALL_GENERIC)

uninstall-generic:
	chmod a+rx $(SCRIPTS_DIR)/$(UNINSTALL_GENERIC)
	$(SCRIPTS_DIR)/$(UNINSTALL_GENERIC)
