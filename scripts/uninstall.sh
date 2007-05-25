#!/bin/bash

#################################################
# btnx uninstall script				#
# ----------------------------------------------#
# Author: Olli Salonen <mail@ollisalonen.com>	#
#################################################

SCRIPT_DIR=/etc/init.d
SCRIPT_INIT=btnx_init.sh
DATA_DIR=data
NAME=btnx
BIN_DIR=/usr/sbin
PROGRAM=$BIN_DIR/$NAME
CONFIG_DIR=/etc/btnx
CONFIG=btnx_config
EVENTS=events

# Stop btnx daemon
$SCRIPT_DIR/$NAME stop 2> /dev/null

# Remove config dir
if [ -d $CONFIG_DIR ]; then
	rm -rf $CONFIG_DIR
	if [ $? -ne 0 ]; then
		echo "Error: could not remove $CONFIG_DIR. Did you forget sudo?"
		exit 1
	fi
fi

# Remove the binary file
if [ -f $BIN_DIR/$NAME ]; then
	rm $BIN_DIR/$NAME 
	if [ $? -ne 0 ]; then
		echo "Error: could not remove $BIN_DIR/$NAME."
		exit 1
	fi
fi

# Remove init script
if [ -f $SCRIPT_DIR/$NAME ]; then
	rm $SCRIPT_DIR/$NAME
	if [ $? -ne 0 ]; then
		echo "Error: could not remove $SCRIPT_DIR/$NAME."
		exit 1
	fi
fi

# Unregister the daemon
update-rc.d -f $NAME remove


echo "Done. $NAME successfully uninstalled."


