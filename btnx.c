
/*------------------------------------------------------------------------*
 * btnx (Button extension): A program for rerouting                       *
 * events from the mouse as keyboard and other mouse events (or both).    *
 * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>                 *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 2         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                     *
 * Boston, MA  02110-1301, USA.                                           *
 *------------------------------------------------------------------------*/
 
/*------------------------------------------------------------------------*
 * File revoco.c has no copyrights (it was written by an animal), and is  *
 * not bound by the GNU GPL.                                              *
 * However, it is still distributed and modified by the blessing of its   *
 * original author. See revoco.c for contact info of the author.          *
 *------------------------------------------------------------------------*/
 
#define PROGRAM_NAME	"btnx"
#define PROGRAM_VERSION	"0.4.4"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <linux/input.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

#include "uinput.h"
#include "btnx.h"
#include "config_parser.h"
#include "device.h"
#include "revoco.h"

#define CHAR2INT(c, x) (((int)(c)) << ((x) * 8))
#define INPUT_BUFFER_SIZE		512
#define NUM_EVENT_HANDLERS		20
#define NUM_HANDLER_LOCATIONS	3
#define TYPE_MOUSE				0
#define TYPE_KBD				1
#define PID_FILE				"/var/run/btnx.pid"

/*
 * The following macros are from mouseemu, to help distinguish
 * between keyboard and mouse handlers.
 */
#define BITS_PER_LONG (sizeof(long) * 8)
#ifndef NBITS
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#endif
#define OFF(x) ((x)%BITS_PER_LONG)
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)
/* End mouseemu macros */

/* Static variables */
static char *g_exec_path=NULL; 		/* Path of this executable */
static struct timeval exec_time; 	/* time when daemon was executed. */

/* Possible paths of event handlers */
const char handler_locations[][15] =
{
	{"/dev"},
	{"/dev/input"},
	{"/dev/misc"}
};

/* Static function declarations */
static const char *get_handler_location(int index);
static int find_handler(int flags, int vendor, int product, int type);
static int btnx_event_get(btnx_event **bevs, int rawcode, int pressed);
static hexdump *btnx_event_read(int fd);
static void command_execute(btnx_event *bev);
static void config_switch(btnx_event *bev);
static void send_extra_event(btnx_event **bevs, int index);
static int check_delay(btnx_event *bev);
static void kill_pids(int fd);
static void pid_file(int kill_old, int append);
static void main_args(int argc, char *argv[], int *bg, int *log, int *kill_all, char **config_file);


/* To simplify the open_handler loop. Can't think of another reason why I
 * coded this */
static const char *get_handler_location(int index)
{
	if (index < 0 || index > NUM_HANDLER_LOCATIONS - 1)
		return NULL;
	
	return handler_locations[index];
}


/* Used to find a certain named event handler in several paths */
int open_handler(char *name, int flags)
{
	const char *loc;
	int x=0, fd;
	char loc_buffer[128];
	
	while ((loc = get_handler_location(x++)) != NULL)
	{
		sprintf(loc_buffer, "%s/%s", loc, name);
		if ((fd = open(loc_buffer, flags)) >= 0)
		{
			printf(OUT_PRE "Opened handler: %s\n", loc_buffer);
			return fd;
		}
	}
	
	return -1;
}

/* Tries to find an input handler that has a certain vendor and product
 * ID associated with it. type determines whether it is a mouse or keyboard
 * input handler that is being searched. */
static int find_handler(int flags, int vendor, int product, int type)
{
	int i, fd;
	unsigned short id[6];
	unsigned long bit[NBITS(EV_MAX)];
	char name[16];
	
	for (i=0; i<NUM_EVENT_HANDLERS; i++)
	{
		sprintf(name, "event%d", i);
		if ((fd = open_handler(name, flags)) < 0)
			continue;
		ioctl(fd, EVIOCGID, id); /* Extract IDs */
		if (vendor == id[ID_VENDOR] && product == id[ID_PRODUCT])
		{
			ioctl(fd, EVIOCGBIT(0, EV_MAX), bit);
			if (((test_bit(EV_KEY, bit) && test_bit(EV_ABS, bit)) && type == TYPE_KBD))
			{
				return fd; /* A keyboard handler found with correct IDs */
			}
			else if ((test_bit(EV_REL, bit)) && type == TYPE_MOUSE)
				return fd; /* A mouse handler found with correct IDs */
		}
		close(fd);
	}
	return -1; /* No such handler found */
}

/* Find the btnx_event structure that is associated with a captured rawcode
 * and return its index. */
static int btnx_event_get(btnx_event **bevs, int rawcode, int pressed)
{
	int i=0;
	
	while (bevs[i] != 0)
	{
		if (bevs[i]->rawcode == rawcode)
		{
			if (bevs[i]->enabled == 0)
				return -1; /* associated rawcode found, but event is disabled */
			bevs[i]->pressed = pressed;
			return i; /* rawcode found and event is enabled */
		}
		i++;
	}
	/* no such rawcode in configuration */
	return -1; 
}

/* Extract the rawcode(s) of an input event. */
static hexdump *btnx_event_read(int fd)
{
	static hexdump codes[MAX_RAWCODES];
	int ret, i, j=0;
	unsigned char buffer[INPUT_BUFFER_SIZE];
	
	memset(buffer, '\0', INPUT_BUFFER_SIZE);
	if ((ret = read(fd, buffer, INPUT_BUFFER_SIZE)) < 1)
		return 0;
	
	for (i=0; (i < (ret / HEXDUMP_SIZE) - 1) && (i < MAX_RAWCODES - 1); i++)
	{
		if ((int)buffer[1 + i*HEXDUMP_SIZE] != 0x00)
			continue;
		codes[j].rawcode = 	CHAR2INT(buffer[0 + i*HEXDUMP_SIZE], 3) | 
							CHAR2INT(buffer[3 + i*HEXDUMP_SIZE], 2) | 
							CHAR2INT(buffer[2 + i*HEXDUMP_SIZE], 1) | 
							CHAR2INT(buffer[5 + i*HEXDUMP_SIZE], 0);
		codes[j].pressed =	buffer[4+i*HEXDUMP_SIZE];
		j++;
	}
	for (; j < MAX_RAWCODES; j++)
	{
		codes[j].rawcode = 0;
		codes[j].pressed = 0;
	}

	return codes;
}

/* Execute a shell script or binary file */
static void command_execute(btnx_event *bev)
{
	int pid;
	
	if (!(pid = fork()))
	{
		setuid(bev->uid);
		execv(bev->args[0], bev->args);
	}
	else if (pid < 0)
	{
		fprintf(stderr, OUT_PRE "Error: could not fork: %s\n", strerror(errno));
		return;
	}
	return;
}

/* Perform a configuration switch */
static void config_switch(btnx_event *bev)
{
	const char *name;
	struct timeval now;
	
	/* Block in case last config switch button is the same as a current one.
	 * This helps prevent a situation where configurations switch multiple
	 * times if the button is held down while the switch occurs. */
	gettimeofday(&now, NULL);
	if (((int)(((unsigned int)now.tv_sec - (unsigned int)exec_time.tv_sec) * 1000) +
		(int)(((int)now.tv_usec - (int)exec_time.tv_usec) / 1000))
		< (int)500)
			return;
	/* ------------------------------------------------------------------- */
	
	switch (bev->switch_type)
	{
	case CONFIG_SWITCH_NEXT:
		name = config_get_next();
		break;
	case CONFIG_SWITCH_PREV:
		name = config_get_prev();
		break;
	case CONFIG_SWITCH_TO:
		name = bev->switch_name;
	}
	
	if (name == NULL)
	{
		fprintf(stderr, OUT_PRE "Warning: config switch failed. Invalid configuration "
				"name.\n");
		return;
	}
	
	printf(OUT_PRE "switching to config: %s\n", name);
	
	execl(g_exec_path, g_exec_path, "-c", name, (char *) NULL);
}

/* Special events, like wheel scrolls and command executions need to be
 * handled differently. They use this function. */
static void send_extra_event(btnx_event **bevs, int index)
{
	int tmp_kc = bevs[index]->keycode;
	
	if (tmp_kc == COMMAND_EXECUTE)
	{
		command_execute(bevs[index]);
		return;
	}
	if (tmp_kc == CONFIG_SWITCH)
	{
		config_switch(bevs[index]);
		return;
	}
	
	/* Perform a "button down" and "button up" event for relative events
	 * such as wheel scrolls. */
	bevs[index]->pressed = 1;
	uinput_key_press(bevs[index]);
	bevs[index]->pressed = 0;
	/* Don't remember why the KEY_UNKNOWN is necessary. */
	bevs[index]->keycode = KEY_UNKNOWN;
	uinput_key_press(bevs[index]);
	bevs[index]->keycode = tmp_kc;
}

/* This function checks if there has been sufficient delay between two
 * occurrances of the same event. Delay is in milliseconds, defined in the
 * configuration file. 
 * Returns 0 if delay is satisfied, -1 if there has not been enough delay. */
static int check_delay(btnx_event *bev)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	
	if (bev->last.tv_sec == 0 && bev->last.tv_usec == 0)
		return 0;
	
	/* Perform a millisecond conversion and compare */
	if (((int)(((unsigned int)now.tv_sec - (unsigned int)bev->last.tv_sec) * 1000) +
		(int)(((int)now.tv_usec - (int)bev->last.tv_usec) / 1000))
		> (int)bev->delay)
		return 0;
	return -1;
}

/* Kill any previous btnx processes found in the PID file */
static void kill_pids(int fd)
{
	char tmp[16];
	int x=0, len;
	pid_t pid;
	
	if (fd < 0)
	{
		fprintf(stderr, OUT_PRE "Warning: kill_pids was passed an invalid fd.\n");
		return;
	}
	
	lseek(fd, 0, SEEK_SET);
	memset(tmp, '\0', 15);
	while (x<15)
	{
		while (tmp[x] == '\0')
		{
			if ((len = read(fd, tmp+x, 1)) < 0)
			{
				fprintf(stderr, OUT_PRE "Error: kill_pids read error: %s\n",
						strerror(errno));
				exit(BTNX_ERROR_FATAL);
			}
			if (len == 0)
				break;
		}
		if (x==0 && (tmp[x] == '\0' || tmp[x] == EOF))
			break;
		if (!isdigit(tmp[x]))
		{
			tmp[x+1] = '\0';
			pid = (pid_t)strtol(tmp, NULL, 10);
			/* Don't kill self or any process groups */
			if (pid >= 1 && pid != getpid())
			{
				if (kill(pid, SIGKILL) < 0)
				{
					fprintf(stderr, OUT_PRE "old btnx process was not killed: %d: %s\n",
							pid, strerror(errno));
				}
			}
			else
			{
				fprintf(stderr, OUT_PRE "old btnx process was not killed: %d %d\n",
						getpid(), pid);
			}
			if (tmp[x] == '\0' || tmp[x] == EOF)
				break;
			memset(tmp, '\0', 15);
			x=0;
			continue;
		}
		x++;
	}
	if (x >= 15)
		fprintf(stderr, OUT_PRE "Warning: kill_pids pid overflow.\n");
	ftruncate(fd, 0);
	//fsync(fd);
	//close(fd);
}

/* Append own PID to PID file, optionally kill previous processes */
static void pid_file(int kill_old, int append)
{
	int fd, flags;
	char tmp[16];
	
	if (kill_old == 1)
		flags = O_RDWR | O_CREAT;
	else
		flags = O_WRONLY | O_CREAT | O_APPEND;
	
	/* Open for write with -rw-r--r-- permissions */
	if ((fd = open(PID_FILE, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
	{
		fprintf(stderr, OUT_PRE "Warning: failed to create pid file %s: %s\n", 
				PID_FILE, strerror(errno));
		exit(BTNX_ERROR_FATAL);
		//return;	
	}
	/* Lock the file for this process, or wait for lock release */
	if (flock(fd, LOCK_EX))
	{
		fprintf(stderr, OUT_PRE "Error: pid file lock set failed.\n");
		close(fd);
		exit(BTNX_ERROR_FATAL);
	}
	/* Kill old processes */
	if (kill_old == 1)
		kill_pids(fd);
	
	/* Append pid */
	if (append)
	{
		sprintf(tmp, "%d ", getpid());
		if ((write(fd, tmp, strlen(tmp))) < strlen(tmp))
		{
			fprintf(stderr, OUT_PRE "Warning: write error to pid file %s: %s\n",
					PID_FILE, strerror(errno));
			flock(fd, LOCK_UN);
			close(fd);
			exit(BTNX_ERROR_FATAL);
		}
	}
	fsync(fd);
	/* Release the PID file lock */
	flock(fd, LOCK_UN);
	close(fd);
}

/* Parses command line arguments. */
static void main_args(int argc, char *argv[], int *bg, int *log, int *kill_all, char **config_file)
{
	g_exec_path = argv[0];
	
	if (argc > 1)
	{
		int x;
		for (x=1; x<argc; x++)
		{
			/* Background daemon */
			if (!strncmp(argv[x], "-b", 2))
				*bg=1;
			/* Print version information */
			else if (!strncmp(argv[x], "-v", 2))
			{
				printf(	PROGRAM_NAME " v." PROGRAM_VERSION "\n"
						"Author: Olli Salonen <oasalonen@gmail.com>\n"
						"Compatible with btnx-config >= v.0.4.0\n");
				exit(BTNX_EXIT_NORMAL);	
			}
			/* Start with specific configuration */
			else if (!strncmp(argv[x], "-c", 2))
			{
				if (x < argc - 1)
				{
					if (strlen(argv[x+1]) >= CONFIG_NAME_MAX_SIZE)
					{
						fprintf(stderr, OUT_PRE "Error: invalid configuration name.\n");
						goto usage;
					}
					*config_file = (char *) malloc((strlen(argv[x+1])+1) * sizeof(char));
					strcpy(*config_file, argv[x+1]);
					x++;
				}
				else
				{
					fprintf(stderr, OUT_PRE "Error: -c argument used but no "
							"configuration file name specified.\n");
					goto usage;
				}
			}
			/* Output stderr to log file */
			else if (!strncmp(argv[x], "-l", 2))
				*log = 1;
			else if (!strncmp(argv[x], "-k", 2))
				*kill_all = 1;
			else
			{
				usage:
				printf(	PROGRAM_NAME " usage:\n"
						"\tArgument:\tDescription:\n"
						"\t-v\t\tPrint version number\n"
						"\t-b\t\tRun process as a background daemon\n"
						"\t-c CONFIG\tRun with specified configuration\n"
						"\t-k\t\tKill all btnx daemons\n"
						"\t-h\t\tPrint this text\n");
				exit(BTNX_ERROR_FATAL);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int fd_ev_btn=0, fd_ev_key=-1;
	fd_set fds;
	hexdump *raw_codes;
	int max_fd, ready;
	btnx_event **bevs;
	int bev_index;
	int i;
	int suppress_release=1;
	int bg=0, log=0;
	char *config_name=NULL;
	int kill_all=0;
	
	main_args(argc, argv, &bg, &log, &kill_all, &config_name);
	
	if (kill_all)
	{
		pid_file(1, 0);
		exit(0);
	}
	else
		pid_file(1, 1);
	
	if (log)
	{
		/* Redirect stderr output to a log file */
		stderr = freopen("/etc/btnx/btnx_log", "a", stderr);
		fprintf(stderr, OUT_PRE "btnx started\n");
	}
	
	if (system("modprobe uinput") != 0)
	{
		fprintf(stderr, OUT_PRE "Warning: modprobe uinput failed. Make sure the uinput "
				"module is loaded before running btnx. If it's already running,"
				" no problem.\n");
	}
	else
		fprintf(stderr, OUT_PRE "uinput modprobed successfully.\n");
	
	bevs = config_parse(config_name);
	
	if (bevs == NULL)
	{
		fprintf(stderr, OUT_PRE "Error: configuration file error.\n");
		exit(BTNX_ERROR_NO_CONFIG);
	}
	
	fd_ev_btn = find_handler(	O_RDONLY,
								device_get_vendor_id(),
								device_get_product_id(),
								TYPE_MOUSE);
	if (fd_ev_btn < 0)
	{
		fprintf(stderr, OUT_PRE "Error opening button event file descriptor: %s\n", 
				strerror(errno));
		exit(BTNX_ERROR_OPEN_HANDLER);
	}
	fd_ev_key = find_handler(	O_RDONLY, 
								device_get_vendor_id(), 
								device_get_product_id(), 
								TYPE_KBD);
	uinput_init("btnx");
	
	if (fd_ev_btn > fd_ev_key)
		max_fd = fd_ev_btn;
	else
		max_fd = fd_ev_key;
	
	revoco_launch();
	
	fprintf(stderr, OUT_PRE "No startup errors\n");
	
	if (log)
	{
		fclose(stderr);
		stderr = fdopen(STDERR_FILENO, "w");
	}
		
	if (bg) daemon(0,0);
	pid_file(0, 1);
	
	gettimeofday(&exec_time, NULL);
	
	for (;;)
	{
		FD_ZERO(&fds);
		FD_SET(fd_ev_btn, &fds);
		if (fd_ev_key != -1)
			FD_SET(fd_ev_key, &fds);
	
		ready = select(max_fd+1, &fds, NULL, NULL, NULL);	
		
		if (ready == -1)
			perror(OUT_PRE "select() error");
		else if (ready == 0)
			continue;
		else
		{
			if (FD_ISSET(fd_ev_btn, &fds))
				raw_codes = btnx_event_read(fd_ev_btn);
			else if (fd_ev_key != 0)
			{
				if (FD_ISSET(fd_ev_key, &fds))
					raw_codes = btnx_event_read(fd_ev_key);
				else
					continue;
			}
			else
				continue;
			
			for (i=0; (i < MAX_RAWCODES); i++)
			{
				if (raw_codes[i].rawcode == 0)
					continue;
				if ((bev_index = btnx_event_get(bevs, raw_codes[i].rawcode, raw_codes[i].pressed)) != -1)
				{
					if (bevs[bev_index]->pressed == 1 || bevs[bev_index]->type == BUTTON_IMMEDIATE)
					{
						if (check_delay(bevs[bev_index]) < 0)
							continue;
						gettimeofday(&(bevs[bev_index]->last), NULL);
					}
					/* Force release, ignore button release */
					if (bevs[bev_index]->type == BUTTON_IMMEDIATE &&
						bevs[bev_index]->pressed == 0)
						continue;
					if (bevs[bev_index]->type == BUTTON_IMMEDIATE && 
						bevs[bev_index]->keycode < BTNX_EXTRA_EVENTS)
					{
						bevs[bev_index]->pressed = 1;
						uinput_key_press(bevs[bev_index]);
						bevs[bev_index]->pressed = 0;
						uinput_key_press(bevs[bev_index]);
					}
					else if (bevs[bev_index]->keycode > BTNX_EXTRA_EVENTS)
					{
						if (bevs[bev_index]->type == BUTTON_NORMAL)
						{
							if ((suppress_release = !suppress_release) != 1)
								send_extra_event(bevs, bev_index);
						}
						else if (bevs[bev_index]->type == BUTTON_IMMEDIATE)
							send_extra_event(bevs, bev_index);
					}
					else
						uinput_key_press(bevs[bev_index]);
				}
			}
		}
		
		/* Clean up the undead */
		waitpid(-1, NULL, WNOHANG);	
	}
	
	return 0;
}
