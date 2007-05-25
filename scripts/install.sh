#!/bin/bash

#################################################
# btnx install script				#
# ----------------------------------------------#
# Author: Olli Salonen <mail@ollisalonen.com>	#
#################################################

INIT_DIR=/etc/init.d
SCRIPT_DIR=scripts
SCRIPT_INIT=btnx_init.sh
DATA_DIR=data
NAME=btnx
BIN_DIR=/usr/sbin
PROGRAM=$BIN_DIR/$NAME
CONFIG_DIR=/etc/btnx
CONFIG=btnx_config
EVENTS=events

# Check for config dir. Create if nonexistent.
if [ -d $CONFIG_DIR  ]; then
	echo "Config dir already exists"
else
	echo "Making config dir... "
	mkdir $CONFIG_DIR
	if [ $? -ne 0 ]; then
		echo "Error: could not make directory: $CONFIG_DIR. Did you forget sudo?"
		exit 1
	fi
	echo "Done."
fi

# Copy the events file to the config directory.
echo "Installing events... "
cp $DATA_DIR/$EVENTS $CONFIG_DIR
if [ $? -ne 0 ]; then
	echo "Error: could not copy $EVENTS to $CONFIG_DIR."
	exit 1
fi
echo "Done."

# Copy the config file to the config directory.
echo "Installing configuration... "
cp $DATA_DIR/$CONFIG $CONFIG_DIR
if [ $? -ne 0 ]; then
	echo "Error: could not copy $CONFIG to $CONFIG_DIR."
	exit 1
fi
echo "Done."

# Copy the binary file to the binary directory.
echo "Installing binary files... "
cp $NAME $BIN_DIR
if [ $? -ne 0 ]; then
	echo "Error: could not copy $NAME to $BIN_DIR."
	exit 1
fi
echo "Done."

# Make sure only root has execution privs for program
chown root $PROGRAM
chmod 0744 $PROGRAM

# Copy the program init script to the init script directory
# Change init script name to same as the program for easy
# /etc/init.d/$NAME start|stop|restart usage
echo "Installing init scripts... "
cp $SCRIPT_DIR/$SCRIPT_INIT $INIT_DIR/$NAME
if [ $? -ne 0 ]; then
	echo "Error: could not copy $SCRIPT_INIT to $INIT_DIR."
	exit 1
fi
echo "Done."

# Allow only root access to the init script
# (read allowed for all)
chown root $INIT_DIR/$NAME
chmod 0744 $INIT_DIR/$NAME

update-rc.d $NAME start 49 5 . stop 49 0 1 2 3 4 6 .


echo "$NAME is installed. Starting $NAME."

$INIT_DIR/$NAME start


