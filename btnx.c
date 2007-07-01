
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
 
#define PROGRAM_NAME	"btnx"
#define PROGRAM_VERSION	"0.2.11"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <linux/input.h>
#include <errno.h>

#include "uinput.h"
#include "btnx.h"
#include "config_parser.h"
#include "devices_parser.h"

#define INPUT_BUFFER_SIZE	512
#define CHAR2INT(c, x) (((int)(c)) << ((x) * 8))

#define NUM_HANDLER_LOCATIONS	3

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

int btnx_event_get(btnx_event **bevs, int rawcode, int pressed)
{
	int i=0;
	
	while (bevs[i] != 0)
	{
		if (bevs[i]->rawcode == rawcode)
		{
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
		// DEBUG for AMD64 MX Revo
		/*if (codes[j].rawcode == 0x020006FF || codes[j].rawcode == 0x02000600)
		{
			printf("Interpreted a sidescroll! Here is the hexdump:\n");
			int k;
			for (k=0; k<ret; k++)
			{
				if (k != 0 && k%2 == 0)
					printf(" ");
				if (k != 0 && k%16 == 0)
					printf("\n");
				if (k%2 == 0 && k<ret-1)
					printf("%02x", buffer[k+1]);
				else
					printf("%02x", buffer[k-1]);
			}
			printf("\n");
		}*/
		j++;
		// !DEBUG
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
		//setgid(1000);
		execv(bev->args[0], bev->args); //&(bev->args[1]));
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

int main(void)
{
	int fd_ev_btn=0, fd_ev_key=-1;
	fd_set fds;
	hexdump *raw_codes;
	int max_fd, ready;
	btnx_event **bevs;
	int bev_index;
	char *mouse_event=NULL, *kbd_event=NULL;
	int i;
	int suppress_release=1;
	
	devices_parser(&mouse_event, &kbd_event);
	bevs = config_parse();
	
	if (bevs == NULL)
	{
		fprintf(stderr, "Error: configuration file error.\n");
		exit(1);
	}
	
	fd_ev_btn = open_handler(mouse_event, O_RDONLY);	//open(mouse_event, O_RDONLY);
	if (fd_ev_btn < 0)
	{
		fprintf(stderr, "Error opening button event file descriptor\"%s\": %s\n", 
				mouse_event, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (kbd_event != NULL)
	{
		fd_ev_key = open_handler(kbd_event, O_RDONLY);	//open(kbd_event, O_RDONLY);
		if (fd_ev_key < 0)
		{
			perror("Error opening key event file descriptor");
			exit(EXIT_FAILURE);
		}
	}
	
	uinput_init("btnx");
	
	if (fd_ev_btn > fd_ev_key)
		max_fd = fd_ev_btn;
	else
		max_fd = fd_ev_key;
	
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
					// DEBUG for AMD64 MX Revo
					/*
					printf("Got rawcode: 0x%08x\n", bevs[bev_index]->rawcode);
					printf("Sending event: key=0x%04x mod1==0x%04x mod2==0x%04x mod3==0x%04x\n",
							bevs[bev_index]->keycode, bevs[bev_index]->mod[0],bevs[bev_index]->mod[1],bevs[bev_index]->mod[2]);
					*/
					// !DEBUG
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
