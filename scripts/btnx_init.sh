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
# description: Captures events from the mouse and \
#              reroutes them through uinput as other user-defined events.
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

# Exit if the package is not installed
[ -x "$DAEMON" ] || exit 5

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.0-6) to ensure that this file is present.
. /lib/lsb/init-functions

#
# Function that starts the daemon/service
#
do_start()
{
	# Return
	#   0 if daemon has been started
	#   1 if daemon was already running
	#   2 if daemon could not be started
	
	start_daemon $NAME -b || return 2
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
	I=100
	while [ -e $PIDFILE ]
	do
		killproc -p $PIDFILE $NAME SIGKILL
		RETVAL="$?"
		sed -i 's/[0-9]* //' $PIDFILE
		[ -s $PIDFILE ] || rm -f $PIDFILE
		[ $RETVAL = 2 ] && return 2
		I=`expr $I - 1`
		[ $I -lt 1 ] && return 2
	done

	# Many daemons don't delete their pidfiles when they exit.
	rm -f $PIDFILE

	return 0
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
	#if [ -e $PIDFILE ]; then
	#	log_success_msg "btnx already running"
	#	exit 0	
	#fi
	do_stop
	do_start
	case "$?" in
		0|1) 
			log_success_msg "btnx successfully started" 
			exit 0
			;;
		2) 
			log_failure_msg "btnx failed to start" 
			exit 1
			;;
	esac
	;;
  stop)
	echo "Stopping $NAME :" "$DESC" >&2
	do_stop
	case "$?" in
		0|1) 
			log_success_msg "btnx successfully stopped" 
			exit 0
			;;
		2) 
			log_failure_msg "btnx failed to stop" 
			exit 1
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
	echo "Restarting $NAME :" "$DESC" &>2
	do_stop
	case "$?" in
	  0|1)
	  	log_success_msg "btnx successfully stopped"
		do_start
		case "$?" in
			0|1) 
				log_success_msg "btnx successfully started" 
				exit 0 ;;
			2) 
				log_failure_msg "btnx failed to start during restart" 
				exit 1 ;;
		esac
		;;
	  *)
	  	# Failed to stop
		log_failure_msg "btnx failed to stop during restart"
		exit 1 ;;
	esac
	;;
  *)
	echo "Usage: $SCRIPTNAME {start|stop|restart|force-reload}" >&2
	exit 3
	;;
esac

:
