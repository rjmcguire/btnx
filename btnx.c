
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
#define PROGRAM_VERSION	"0.3.3"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <errno.h>

#include "uinput.h"
#include "btnx.h"
#include "config_parser.h"
#include "devices_parser.h"
#include "device.h"
#include "revoco.h"

#define INPUT_BUFFER_SIZE	512
#define NUM_EVENT_HANDLERS	20
#define CHAR2INT(c, x) (((int)(c)) << ((x) * 8))

#define NUM_HANDLER_LOCATIONS	3

#define TYPE_MOUSE		0
#define TYPE_KBD		1

#define PID_FILE		"/var/run/btnx.pid"

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

const char handler_locations[][15] =
{
	{"/dev"},
	{"/dev/input"},
	{"/dev/misc"}
};

const char *get_handler_location(int index)
{
	if (index < 0 || index > NUM_HANDLER_LOCATIONS - 1)
		return NULL;
	
	return handler_locations[index];
}

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
			printf("Opened handler: %s\n", loc_buffer);
			return fd;
		}
	}
	
	return -1;
}

int find_handler(int flags, int vendor, int product, int type)
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
		ioctl(fd, EVIOCGID, id);
		if (vendor == id[ID_VENDOR] && product == id[ID_PRODUCT])
		{
			ioctl(fd, EVIOCGBIT(0, EV_MAX), bit);
			if (((test_bit(EV_KEY, bit) && test_bit(EV_ABS, bit)) && type == TYPE_KBD))
			{
				return fd;
			}
			else if ((test_bit(EV_REL, bit)) && type == TYPE_MOUSE)
				return fd;
		}
		close(fd);
	}
	return -1;
}

int btnx_event_get(btnx_event **bevs, int rawcode, int pressed)
{
	int i=0;
	
	while (bevs[i] != 0)
	{
		if (bevs[i]->rawcode == rawcode)
		{
			if (bevs[i]->enabled == 0)
				return -1;
			bevs[i]->pressed = pressed;
			return i;
		}
		i++;
	}
	
	return -1;
}

hexdump *btnx_event_read(int fd)
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


void command_execute(btnx_event *bev)
{
	int pid;
	
	if (!(pid = fork()))
	{
		setuid(bev->uid);
		execv(bev->args[0], bev->args);
	}
	else if (pid < 0)
	{
		fprintf(stderr, "Error: could not fork: %s\n", strerror(errno));
		return;
	}
	return;
}

void send_extra_event(btnx_event **bevs, int index)
{
	int tmp_kc = bevs[index]->keycode;
	
	if (tmp_kc == COMMAND_EXECUTE)
	{
		command_execute(bevs[index]);
		return;
	}
	
	bevs[index]->pressed = 1;
	uinput_key_press(bevs[index]);
	bevs[index]->pressed = 0;
	bevs[index]->keycode = KEY_UNKNOWN;
	uinput_key_press(bevs[index]);
	bevs[index]->keycode = tmp_kc;
}

static int check_delay(btnx_event *bev)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	
	if (bev->last.tv_sec == 0 && bev->last.tv_usec == 0)
		return 0;
		
	if (((int)(((unsigned int)now.tv_sec - (unsigned int)bev->last.tv_sec) * 1000) +
		(int)(((int)now.tv_usec - (int)bev->last.tv_usec) / 1000))
		> (int)bev->delay)
		return 0;
	return -1;
}

static void create_pid_file(void)
{
	int fd;
	char tmp[8];
	
	if ((fd = open(PID_FILE, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
	{
		fprintf(stderr, "Warning: failed to create pid file %s: %s\n", 
				PID_FILE, strerror(errno));
		return;	
	}
	sprintf(tmp, "%d", getpid());
	if ((write(fd, tmp, strlen(tmp))) < strlen(tmp))
	{
		fprintf(stderr, "Warning: write error to pid file %s: %s\n",
				PID_FILE, strerror(errno));
	}
	close(fd);
}

static void main_args(int argc, char *argv[], int *bg)
{
	if (argc > 1)
	{
		if (!strncmp(argv[1], "-b", 2))
			*bg=1;
		else if (!strncmp(argv[1], "-v", 2))
		{
			printf(	PROGRAM_NAME " v." PROGRAM_VERSION "\n"
					"Author: Olli Salonen <oasalonen@gmail.com>\n"
					"Compatible with btnx-config >= v.0.2.0\n");
			exit(0);	
		}
		else
		{
			printf(	PROGRAM_NAME " usage:\n"
					"Argument:\tDescription:\n"
					"-v\t\tPrint version number\n"
					"-b\t\tRun process as a background daemon\n"
					"-h\t\tPrint this text\n");
			exit(0);
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
	int bg=0;
	
	main_args(argc, argv, &bg);
	
	if (bg) daemon(0,0);
	create_pid_file();
	
	if (system("modprobe uinput") != 0)
	{
		fprintf(stderr, "Warning: modprobe uinput failed. Make sure the uinput \
module is loaded before running btnx. If it's already running, no problem.\n");
	}
	else
		printf("uinput modprobed successfully.\n");
	
	bevs = config_parse();
	
	if (bevs == NULL)
	{
		fprintf(stderr, "Error: configuration file error.\n");
		exit(1);
	}
	
	fd_ev_btn = find_handler(O_RDONLY, device_get_vendor_id(), device_get_product_id(), TYPE_MOUSE);
	if (fd_ev_btn < 0)
	{
		fprintf(stderr, "Error opening button event file descriptor: %s\n", 
				strerror(errno));
		exit(EXIT_FAILURE);
	}
	fd_ev_key = find_handler(O_RDONLY, device_get_vendor_id(), device_get_product_id(), TYPE_KBD);
	
	uinput_init("btnx");
	
	if (fd_ev_btn > fd_ev_key)
		max_fd = fd_ev_btn;
	else
		max_fd = fd_ev_key;
	
	revoco_launch();
	
	fprintf(stderr, "No startup errors\n");
	
	for (;;)
	{
		FD_ZERO(&fds);
		FD_SET(fd_ev_btn, &fds);
		if (fd_ev_key != -1)
			FD_SET(fd_ev_key, &fds);
	
		ready = select(max_fd+1, &fds, NULL, NULL, NULL);	
		
		if (ready == -1)
			perror("select() error");
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
