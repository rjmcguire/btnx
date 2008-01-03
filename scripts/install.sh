#!/bin/bash

#################################################
# btnx install script				#
# ----------------------------------------------#
# Author: Olli Salonen <oasalonen@gmail.com>	#
#################################################

INIT_DIR=/etc/init.d
INIT_PROG=""
INIT_DEL=
INIT_ADD=
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

function find_file_path {
	local IFS=:
	for PATH_DIR in $1; do
		for FP in $PATH_DIR/$2; do
			#echo "looking for $FP"
			[ -x "$FP" ] && return 1;
		done
	done
	return 0;
}

PATH=$PATH:/sbin:/usr/sbin:/usr/local/sbin

find_file_path $PATH update-rc.d
if [ $? == 1 ]; then
	#echo "found update-rc.d"
	INIT_PROG="update-rc.d"
	INIT_ADD="update-rc.d $NAME start 49 2 3 4 5 . stop 49 0 1 6 ."
	INIT_DEL="update-rc.d -f $NAME remove > /dev/null"
fi
if [ -z $INIT_PROG ]; then
	#echo "trying to find another prog"
	find_file_path $PATH chkconfig
	if [ $? == 1 ]; then
		#echo "found chkconfig"
		INIT_PROG=chkconfig
		INIT_ADD="chkconfig --add $NAME"
		INIT_DEL="chkconfig --del $NAME"
	fi
fi
#exit 0

echo "Installing $NAME..."

# Make sure a previous btnx daemon is not running
if [ -f $INIT_DIR/$NAME ]; then
	echo "Existing $NAME service found. Attempting to stop the service."
	/etc/init.d/btnx stop 2> /dev/null
	if [ $? -ne 0 ]; then
		echo "* Warning: error attemping to stop existing $NAME daemon. Continuing..."
	else
		echo "Unregistering previous $NAME init script."
		[ -n $INIT_PROG ] && $INIT_DEL
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
		echo -e "\n* Error: could not make directory: $CONFIG_DIR. \rAre you root?"
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

# Copy the events file to the config directory.
#echo -ne "Installing events... "
cp $DATA_DIR/$EVENTS $CONFIG_DIR
if [ $? -ne 0 ]; then
	echo -e "\n* Error: could not copy $EVENTS to $CONFIG_DIR."
	exit 1
fi
echo -ne "."

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
if [ -n $INIT_PROG ]; then
	$INIT_ADD
else
	echo "$NAME daemon init script could not be registered: No known init script installation utility was found. Looked for update-rc.d and chkconfig. If your distro uses a different script installation utility, make a request for support."
fi

echo -e "$NAME has been successfully installed!\n"
echo -e "You can type '/etc/init.d/btnx start' as root to start btnx if you have made a configuration file with btnx-config."
#$INIT_DIR/$NAME start


