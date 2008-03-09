#! /bin/sh
### BEGIN INIT INFO
# Provides:          btnx
# Required-Start:    $local_fs $remote_fs
# Required-Stop:     $local_fs $remote_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Mouse button rerouter daemon.
# Description:       Captures events from the mouse and
#                    reroutes them through uinput as other user-defined events.
### END INIT INFO

## BEGIN CHKCONFIG
# chkconfig: 2345 49 49
# description: Mouse button rerouter daemon.
## END CHKCONFIG

# Author: Olli Salonen <oasalonen@gmail.com>

# Do NOT "set -e"

# PATH should only include /usr/* if it runs after the mountnfs.sh script
PATH=$PATH:/sbin:/usr/sbin:/bin:/usr/bin
DESC="Button Extension - mouse button rerouter daemon"
NAME=btnx
DAEMON=/usr/sbin/$NAME
DAEMON_ARGS=""
PIDFILE=/var/run/$NAME.pid
SCRIPTNAME=/etc/init.d/$NAME
LOG=/etc/btnx/$NAME.log
FAIL_STATUS=6
ARG_CONFIG=

# Exit if the package is not installed
[ -x "$DAEMON" ] || exit 5

# If LSB init-functions use start-stop-daemon, use full path for launching
# daemon through start_daemon. Otherwise, just give the name of the executable.
# Edited out: do_start() attempts both
# cat /lib/lsb/init-functions | grep start-stop-daemon > /dev/null
# [ $? -eq 1 ] && DAEMON=$NAME

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.0-6) to ensure that this file is present.
. /lib/lsb/init-functions

if [ -z $2 ]; then # empty string
	ARG_CONFIG=""
else
	ARG_CONFIG="-c \"$2\""
fi

#
# Function that checks event handlers. Prevents
# udev loading hang.
#
check_handlers()
{
	ls /dev/event* > /dev/null 2> /dev/null
	[ $? = 0 ] && return 0
	ls /dev/input/event* > /dev/null 2> /dev/null
	[ $? = 0 ] && return 0
	ls /dev/misc/event* > /dev/null 2> /dev/null
	[ $? = 0 ] && return 0

	echo "No input event handlers found. Start blocked."

	return 1
}

#
# Function that starts the daemon/service
#
do_start()
{
	# Return
	#   0 if daemon has been started
	#   1 if daemon was already running
	#   2 if daemon could not be started
	
	start_daemon $DAEMON -b $ARG_CONFIG
	RET=$?
	if [ $RET -eq 2 ]; then
		log_failure_msg "start_daemon failed to start btnx with full path. Trying without"
		start_daemon $NAME -b $ARG_CONFIG
		RET=$?
	fi
	return $RET
}

#
# Function that stops the daemon/service
#
do_stop()
{
	# Return
	#   0 if daemon has been stopped
	#   1 if daemon was already stopped
	#   2 if daemon could not be stopped
	#   other if a failure occurred
	#I=100
	#while [ -e $PIDFILE ]
	#do
	#	killproc -p $PIDFILE $DAEMON SIGKILL
	#	RETVAL="$?"
	#	sed -i 's/[0-9]* //' $PIDFILE
	#	[ -s $PIDFILE ] || rm -f $PIDFILE
	#	[ $RETVAL = 2 ] && return 2
	#	I=`expr $I - 1`
	#	[ $I -lt 1 ] && return 2
	#done

	start_daemon $DAEMON -k
	RET=$?
	if [ $RET -eq 2 ]; then
		log_failure_msg "start_daemon failed to stop btnx with full path. Trying without"
		start_daemon $NAME -k
		RET=$?
	fi
	return $RET

	# Many daemons don't delete their pidfiles when they exit.
	# rm -f $PIDFILE

	#return $?
}

#
# Function that sends a SIGHUP to the daemon/service
#
do_reload() {
	#
	# If the daemon can reload its configuration without
	# restarting (for example, when it is sent a SIGHUP),
	# then implement that here.
	
	return 0
}

case "$1" in
  start)
	echo "Starting $NAME :" "$DESC" >&2
	check_handlers
	[ $? -ne 0 ] && exit 1
	#do_stop
	do_start
	RET=$?
	case "$RET" in
		0) 
			echo "btnx successfully started" 
			#log_success_msg "btnx successfully started" 
			exit 0
			;;
		*) 
			echo "btnx failed to start (error code $RET)"
			#log_failure_msg "btnx failed to start (error code $RET)" 
			exit $RET
			;;
	esac
	;;
  stop)
	echo "Stopping $NAME :" "$DESC" >&2
	do_stop
	case "$?" in
		0|1) 
			echo "btnx successfully stopped" 
			#log_success_msg "btnx successfully stopped" 
			exit 0
			;;
		2) 
			echo "btnx failed to stop" 
			#log_failure_msg "btnx failed to stop" 
			exit $FAIL_STATUS
			;;
	esac
	;;
  #reload|force-reload)
	#
	# If do_reload() is not implemented then leave this commented out
	# and leave 'force-reload' as an alias for 'restart'.
	#
	#log_daemon_msg "Reloading $DESC" "$NAME"
	#do_reload
	#log_end_msg $?
	#;;
  restart|force-reload)
	#
	# If the "reload" option is implemented then remove the
	# 'force-reload' alias
	#
	echo "Restarting $NAME :" "$DESC" >&2
	do_stop
	case "$?" in
	  0|1)
		echo "btnx successfully stopped"
	  	#log_success_msg "btnx successfully stopped"
		do_start
		RET=$?
		case "$RET" in
			0) 
				echo "btnx successfully started"
				#log_success_msg "btnx successfully started" 
				exit 0 ;;
			*) 
				echo "btnx failed to start during restart" 
				#log_failure_msg "btnx failed to start during restart" 
				exit $RET ;;
		esac
		;;
	  *)
	  	# Failed to stop
		echo "btnx failed to stop during restart"
		#log_failure_msg "btnx failed to stop during restart"
		exit $FAIL_STATUS ;;
	esac
	;;
  *)
	echo "Usage: $SCRIPTNAME {start|stop|restart|force-reload}" >&2
	exit 3
	;;
esac

:
