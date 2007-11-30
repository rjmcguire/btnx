
CC = gcc
CFLAGS = -Wall -g
LFLAGS =
#-lusb

DEPS = uinput.h btnx.h config_parser.h device.h revoco.h
OBJ = btnx.o uinput.o config_parser.o device.o revoco.o
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
	@rm -f $(OBJ) $(BIN)

install:
	@chmod a+rx $(SCRIPTS_DIR)/$(INSTALL)
	@$(SCRIPTS_DIR)/$(INSTALL)

uninstall:
	@chmod a+rx $(SCRIPTS_DIR)/$(UNINSTALL)
	@$(SCRIPTS_DIR)/$(UNINSTALL)
	
install-generic:
	@echo "Error: install-generic is no longer necessary. Use install instead."

uninstall-generic:
	@echo "Error: uninstall-generic is no longer necessary. Use uninstall instead."
