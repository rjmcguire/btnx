#!/bin/bash

#################################################
# btnx uninstall script				#
# ----------------------------------------------#
# Author: Olli Salonen <oasalonen@gmail.com>	#
#################################################

SCRIPT_DIR=/etc/init.d
SCRIPT_INIT=btnx_init.sh
INIT_PROG=
INIT_DEL=
DATA_DIR=data
NAME=btnx
BIN_DIR=/usr/sbin
PROGRAM=$BIN_DIR/$NAME
CONFIG_DIR=/etc/btnx
CONFIG=btnx_config
EVENTS=events
UDEV_DIR=/etc/udev/rules.d
UDEV=99-btnx.rules

function find_file_path {
	local IFS=:
	for PATH_DIR in $1; do
		for FP in $PATH_DIR/$2; do
			[ -x "$FP" ] && return 1;
		done
	done
	return 0;
}

find_file_path $PATH update-rc.d
if [ $? == 1 ]; then
	INIT_PROG="update-rc.d"
	INIT_DEL="update-rc.d -f $NAME remove > /dev/null"
fi
if [ -z $INIT_PROG ]; then
	find_file_path $PATH chkconfig
	if [ $? == 1 ]; then
		INIT_PROG=chkconfig
		INIT_DEL="chkconfig --del $NAME"
	fi
fi

echo "Please wait a while for the btnx daemon to stop."

# Stop btnx daemon
$SCRIPT_DIR/$NAME stop 2> /dev/null

echo "btnx daemon stopped."
echo -ne "Uninstalling..."
# Remove config dir
if [ -d $CONFIG_DIR ]; then
	rm -rf $CONFIG_DIR
	if [ $? -ne 0 ]; then
		echo "Error: could not remove $CONFIG_DIR. Are you root?"
	fi
fi
echo -ne "."

# Unregister the daemon
$INIT_DEL
echo -ne "."

# Remove the binary file
if [ -f $BIN_DIR/$NAME ]; then
	rm $BIN_DIR/$NAME 
	if [ $? -ne 0 ]; then
		echo "Error: could not remove $BIN_DIR/$NAME."
	fi
fi
echo -ne "."

# Remove init script
if [ -f $SCRIPT_DIR/$NAME ]; then
	rm $SCRIPT_DIR/$NAME
	if [ $? -ne 0 ]; then
		echo "Error: could not remove $SCRIPT_DIR/$NAME."
	fi
fi
echo -ne "."

# Remove udev
if [ -f $UDEV_DIR/$UDEV ]; then
	rm $UDEV_DIR/$UDEV
	if [ $? -ne 0 ]; then
		echo "Error: could not remove $UDEV_DIR/$UDEV."
	fi
fi
echo "."


echo "Done. $NAME successfully uninstalled."


