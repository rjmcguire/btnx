#!/bin/bash

#################################################
# btnx install script				#
# ----------------------------------------------#
# Author: Olli Salonen <oasalonen@gmail.com>	#
#################################################

INIT_DIR=/etc/init.d
SCRIPT_DIR=scripts
SCRIPT_INIT=btnx_init.sh
DATA_DIR=data
DEFAULTS_DIR=$DATA_DIR/defaults
NAME=btnx
BIN_DIR=/usr/sbin
PROGRAM=$BIN_DIR/$NAME
CONFIG_DIR=/etc/btnx
CONFIG=btnx_config
EVENTS=events


function prompt_yn {
	while [ 1 ]; do
	  echo -n " (y or n)? : "
	  read CONFIRM
	  case $CONFIRM in
	    y|Y|YES|yes|Yes) return 1 ;;
	    n|N|no|NO|No) return 0 ;;
	    *) echo "Invalid option"
	  esac
	done
}

echo "Installing $NAME..."

# Make sure a previous btnx daemon is not running
if [ -f $INIT_DIR/$NAME ]; then
	echo "Existing btnx service found. Attempting to stop the service."
	/etc/init.d/btnx stop 2> /dev/null
	if [ $? -ne 0 ]; then
		echo "* Warning: error attemping to stop existing btnx daemon. Continuing..."
	fi
fi
echo -ne "."


# Check for config dir. Create if nonexistent.
if [ -d $CONFIG_DIR  ]; then
	echo -ne "."
else
	#echo -ne "Making config dir... "
	mkdir $CONFIG_DIR
	if [ $? -ne 0 ]; then
		echo -e "\n* Error: could not make directory: $CONFIG_DIR. \rDid you forget sudo?"
		exit 1
	fi
	echo -ne "."
fi

# Check for /usr/sbin dir. Create if nonexistent.
if [ -d $BIN_DIR  ]; then
	echo -ne "."
else
	#echo -ne "Making bin dir... "
	mkdir $BIN_DIR
	if [ $? -ne 0 ]; then
		echo -e "\n* Error: could not make directory: $BIN_DIR."
		exit 1
	fi
	echo -ne "."
fi

# Check for /etc/init.d dir. Create if nonexistent.
if [ -d $INIT_DIR  ]; then
	echo -ne "."
else
	#echo -ne "Making init dir... "
	mkdir $INIT_DIR
	if [ $? -ne 0 ]; then
		echo -e "\n* Error: could not make directory: $INIT_DIR."
		exit 1
	fi
	echo -ne "."
fi

# Copy the default configurations to the config directory.
#cp -r $DEFAULTS_DIR $CONFIG_DIR
#if [ $? -ne 0 ]; then
#	echo "Error: could not copy $DEFAULTS_DIR to $CONFIG_DIR."
#	exit 1
#fi
#echo -ne "."

# Copy the events file to the config directory.
#echo -ne "Installing events... "
cp $DATA_DIR/$EVENTS $CONFIG_DIR
if [ $? -ne 0 ]; then
	echo -e "\n* Error: could not copy $EVENTS to $CONFIG_DIR."
	exit 1
fi
echo -ne "."

# Make sure an old config file isn't installed. Won't
# copy the correct default config otherwise on first run.
# Create a backup of the old one.
#CONFIG_BAK=$CONFIG.bak
#CONFIG_INDEX=1
#if [ -f $CONFIG_DIR/$CONFIG ]; then
#	echo "Detected a previous btnx_config file."
#	echo -ne "Keep old configuration file "
#	prompt_yn
#	if [ $? -ne 1 ]; then
#		while [ -f $CONFIG_DIR/${CONFIG_BAK}${CONFIG_INDEX} ]; do
#			CONFIG_INDEX=`expr $CONFIG_INDEX + 1`
#		done
#		mv $CONFIG_DIR/$CONFIG $CONFIG_DIR/${CONFIG_BAK}${CONFIG_INDEX}
#		if [ $? -ne 0 ]; then
#			echo "Error: could not backup old config file."
#			exit 1
#		fi
#		echo ""
#		echo "Backed up old configuration file to $CONFIG_DIR/${CONFIG_BAK}${CONFIG_INDEX}"
#	fi
#f

# Copy the binary file to the binary directory.
#echo -ne "Installing binary files... "
cp $NAME $BIN_DIR
if [ $? -ne 0 ]; then
	echo -e "\n*Error: could not copy $NAME to $BIN_DIR."
	exit 1
fi
echo -ne "."

# Make sure only root has execution privs for program
chown root $PROGRAM
chmod 0744 $PROGRAM

echo -ne "."

# Copy the program init script to the init script directory
# Change init script name to same as the program for easy
# /etc/init.d/$NAME start|stop|restart usage
#echo -ne "Installing init scripts... "
cp $SCRIPT_DIR/$SCRIPT_INIT $INIT_DIR/$NAME
if [ $? -ne 0 ]; then
	echo "\n* Error: could not copy $SCRIPT_INIT to $INIT_DIR."
	exit 1
fi
echo -ne "."

# Allow only root access to the init script
# (read allowed for all)
chown root $INIT_DIR/$NAME
chmod 0744 $INIT_DIR/$NAME

echo "."

#update-rc.d $NAME start 49 2 3 4 5 . stop 49 0 1 6 . > /dev/null


echo -e "$NAME has been successfully installed!\n"
echo -e "You can type 'sudo /etc/init.d/btnx start' to start btnx if you have made a configuration file with btnx-config."
#$INIT_DIR/$NAME start


