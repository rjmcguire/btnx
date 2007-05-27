
/*------------------------------------------------------------------------*
 * btnx (Button extension): A program for rerouting                       *
 * events from the mouse as keyboard and other mouse events (or both).    *
 * Copyright (C) 2007  Olli Salonen (www.ollisalonen.com)                 *
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
#define PROGRAM_VERSION	"0.02"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/input.h>

#include "uinput.h"
#include "btnx.h"
#include "config_parser.h"
#include "devices_parser.h"

#define INPUT_BUFFER_SIZE	512
#define CHAR2INT(c, x) (((int)(c)) << ((x) * 8))


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

int btnx_event_read(int fd, int *pressed)
{
	int code;
	unsigned char buffer[INPUT_BUFFER_SIZE];
	
	memset(buffer, '\0', INPUT_BUFFER_SIZE);
	if (read(fd, buffer, INPUT_BUFFER_SIZE) < 1)
		return 0;

	*pressed = buffer[12];
	
	code = CHAR2INT(buffer[8], 3) | CHAR2INT(buffer[11], 2) | CHAR2INT(buffer[10], 1) | CHAR2INT(buffer[13], 0);

	return code;
}

int main(void)
{
	int fd_ev_btn=0, fd_ev_key=-1;
	fd_set fds;
	int raw_code;
	int max_fd, ready;
	int pressed=0;
	btnx_event **bevs;
	int bev_index;
	char *mouse_event=NULL, *kbd_event=NULL;
	
	devices_parser(&mouse_event, &kbd_event);
	bevs = config_parse();
	
	if (bevs == NULL)
	{
		fprintf(stderr, "Error: configuration file error.\n");
		exit(1);
	}
	
	fd_ev_btn = open(mouse_event, O_RDONLY);
	if (fd_ev_btn < 0)
	{
		perror("Error opening button event file descriptor");
		exit(EXIT_FAILURE);
	}
	if (kbd_event != NULL)
	{
		fd_ev_key = open(kbd_event, O_RDONLY);
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
				raw_code = btnx_event_read(fd_ev_btn, &pressed);
			else if (fd_ev_key != 0)
			{
				if (FD_ISSET(fd_ev_key, &fds))
					raw_code = btnx_event_read(fd_ev_key, &pressed);
				else
					continue;
			}
			else
				continue;
			
			if ((bev_index = btnx_event_get(bevs, raw_code, pressed)) != -1)
			{
				if (bevs[bev_index]->type == BUTTON_IMMEDIATE)
				{
					bevs[bev_index]->pressed = 1;
					uinput_key_press(bevs[bev_index]);
					bevs[bev_index]->pressed = 0;
					uinput_key_press(bevs[bev_index]);
				}
				else
					uinput_key_press(bevs[bev_index]);
			}
		}
	}
	
	return 0;
}
