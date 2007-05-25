
/*----------------------------------------------------------------------*
 * btnx (Button extension): A program for MX Revolution to reroute		*
 * events from the mouse as keyboard and other mouse events (or both).	*
 * Copyright (C) 2007  Olli Salonen (www.ollisalonen.com)				*
 *																		*
 * This program is free software; you can redistribute it and/or		*
 * modify it under the terms of the GNU General Public License			*
 * as published by the Free Software Foundation; either version 2		*
 * of the License, or (at your option) any later version.				*
 *																		*
 * This program is distributed in the hope that it will be useful,		*
 * but WITHOUT ANY WARRANTY; without even the implied warranty of		*
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		*
 * GNU General Public License for more details.							*
 *																		*
 * You should have received a copy of the GNU General Public License	*
 * along with this program; if not, write to the Free Software			*
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 					*
 * Boston, MA  02110-1301, USA.											*
 *----------------------------------------------------------------------*/

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

#define INPUT_BUFFER_SIZE	512
#define CHAR2INT(c, x) (((int)(c)) << ((x) * 8))

#define BTNX1	0x00180100	// Thumb wheel forw
#define BTNX2	0x001A0100	// Thumb wheel back
#define BTNX3	0x001C0100	// Thumb wheel press
#define BTNX4	0x00130100	// Thumb button, arrow towards user
#define BTNX5	0x00140100	// Thumb button, arrow away from user
#define BTNX6	0x00880000	// Search key
#define BTNX7	0x00060001	// Wheel right
#define BTNX8	0x000600FF	// Wheel left

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
	/*
	switch (raw_code)
	{
	case BTNX1:
		bev->keycode = KEY_LEFT;
		bev->mod[0] = KEY_LEFTCTRL;
		bev->mod[1] = KEY_LEFTALT;
		bev->mod[2] = 0;
		bev->pressed = pressed;
		return 0;
	case BTNX2:
		bev->keycode = KEY_RIGHT;
		bev->mod[0] = KEY_LEFTCTRL;
		bev->mod[1] = KEY_LEFTALT;
		bev->mod[2] = 0;
		bev->pressed = pressed;
		return 0;
	case BTNX3:
		bev->keycode = BTN_LEFT;
		bev->mod[0] = KEY_LEFTALT;
		bev->mod[1] = 0;
		bev->mod[2] = 0;
		bev->pressed = pressed;
		return 0;
	case BTNX4:
		bev->keycode = KEY_PAGEDOWN;
		bev->mod[0] = KEY_LEFTCTRL;
		bev->mod[1] = 0;
		bev->mod[2] = 0;
		bev->pressed = pressed;
		return 0;
	case BTNX5:
		bev->keycode = KEY_PAGEUP;
		bev->mod[0] = KEY_LEFTCTRL;
		bev->mod[1] = 0;
		bev->mod[2] = 0;
		bev->pressed = pressed;
		return 0;
	case BTNX6:
		bev->keycode = BTN_MIDDLE;
		bev->mod[0] = 0;
		bev->mod[1] = 0;
		bev->mod[2] = 0;
		bev->pressed = pressed;
		return 0;
	case BTNX7:
		bev->keycode = KEY_RIGHT;
		bev->mod[0] = KEY_LEFTALT;
		bev->mod[1] = 0;
		bev->mod[2] = 0;
		bev->pressed = BUTTON_IMMEDIATE;
		return 0;
	case BTNX8:
		bev->keycode = KEY_LEFT;
		bev->mod[0] = KEY_LEFTALT;
		bev->mod[1] = 0;
		bev->mod[2] = 0;
		bev->pressed = BUTTON_IMMEDIATE;
		return 0;
	default:
		return -1;
	}*/
}

int btnx_event_read(int fd, int *pressed)
{
	int code;
	unsigned char buffer[INPUT_BUFFER_SIZE];
	
	memset(buffer, '\0', INPUT_BUFFER_SIZE);
	if (read(fd, buffer, INPUT_BUFFER_SIZE) < 1)
		return 0;
	
	*pressed = buffer[12];
	
	if (buffer[8] == 0x02)
		code = CHAR2INT(buffer[9], 3) | CHAR2INT(buffer[10], 2) | CHAR2INT(buffer[11], 1) | CHAR2INT(buffer[12], 0);
	else
		code = CHAR2INT(buffer[9], 3) | CHAR2INT(buffer[10], 2) | CHAR2INT(buffer[11], 1) | CHAR2INT(0, 0);

	return code;
}

int main(void)
{
	int fd_ev_btn, fd_ev_key;
	fd_set fds;
	int raw_code;
	//struct btnx_event bev;
	int max_fd, ready;
	int pressed=0;
	btnx_event **bevs = config_parse();
	int bev_index;
	
	int c=7;
	
	printf("btnx_event:\nrawcode: %08x\tkeycode: %d\tmod1: %d\tmod2: %d\tmod3: %d\n",
		bevs[c]->rawcode, bevs[c]->keycode, bevs[c]->mod[0], bevs[c]->mod[1], bevs[c]->mod[2]);
	
	fd_ev_btn = open("/dev/input/event4", O_RDONLY);
	if (fd_ev_btn < 0)
	{
		perror("Error opening button event file descriptor");
		exit(EXIT_FAILURE);
	}
	fd_ev_key = open("/dev/input/event5", O_RDONLY);
	if (fd_ev_key < 0)
	{
		perror("Error opening key event file descriptor");
		exit(EXIT_FAILURE);
	}
	
	uinput_init("btnx");
	
	if (fd_ev_btn > fd_ev_key)
		max_fd = fd_ev_btn;
	else
		max_fd = fd_ev_key;
	
	for (;;)
	{
		FD_ZERO(&fds);
		FD_SET(fd_ev_btn, &fds);
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
			else if (FD_ISSET(fd_ev_key, &fds))
				raw_code = btnx_event_read(fd_ev_key, &pressed);
			else
				continue;
			
			if ((bev_index = btnx_event_get(bevs, raw_code, pressed)) != -1)
			{
				if (bevs[bev_index]->type == BUTTON_IMMEDIATE)
				{
					printf("immediate\n");
					bevs[bev_index]->pressed = 1;
					uinput_key_press(bevs[bev_index]);
					//usleep(100);
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
