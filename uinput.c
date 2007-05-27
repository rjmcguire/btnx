
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */

#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "uinput.h"
#include "btnx.h"

static int uinput_mouse_fd = -1;
static int uinput_kbd_fd = -1;

int uinput_init(const char *dev_name) 
{
  struct uinput_user_dev dev_mouse, dev_kbd;
  int i;

  uinput_mouse_fd = open(UINPUT_LOCATION, O_WRONLY | O_NDELAY);
  if (uinput_mouse_fd < 0) 
  {
    perror("Error opening the uinput device.\nMake sure you have loaded the uinput module (modprobe uinput)");
    exit(EXIT_FAILURE);
  }
  uinput_kbd_fd = open(UINPUT_LOCATION, O_WRONLY | O_NDELAY);
  if (uinput_kbd_fd < 0) 
  {
    perror("Error opening the uinput device");
    exit(EXIT_FAILURE);
  }

  memset(&dev_mouse, 0, sizeof(dev_mouse));
  strcpy(dev_mouse.name, UMOUSE_NAME);
  write(uinput_mouse_fd, &dev_mouse, sizeof(dev_mouse));
  
  memset(&dev_kbd, 0, sizeof(dev_kbd));
  strcpy(dev_kbd.name, UKBD_NAME);
  write(uinput_kbd_fd, &dev_kbd, sizeof(dev_kbd));

  ioctl(uinput_mouse_fd, UI_SET_EVBIT, EV_REL);
  ioctl(uinput_mouse_fd, UI_SET_RELBIT, REL_X);
  ioctl(uinput_mouse_fd, UI_SET_RELBIT, REL_Y);
  ioctl(uinput_mouse_fd, UI_SET_EVBIT, EV_KEY);
  
  for (i=BTN_MISC; i<KEY_OK; i++)
  {
  	ioctl(uinput_mouse_fd, UI_SET_KEYBIT, i);
  }
   
  ioctl(uinput_kbd_fd, UI_SET_EVBIT, EV_KEY);
  for (i=0; i<BTN_MISC; i++)
  {
  	ioctl(uinput_kbd_fd, UI_SET_KEYBIT, i);
  }
  for (i=KEY_OK; i<KEY_MAX; i++)
  {
  	ioctl(uinput_kbd_fd, UI_SET_KEYBIT, i);
  }
  
  ioctl(uinput_kbd_fd, UI_DEV_CREATE, 0);
  ioctl(uinput_mouse_fd, UI_DEV_CREATE, 0);
  
  return 0;
}

void uinput_key_press(struct btnx_event *bev)
{
	struct input_event event;
	int fd;
	int i;
	
	if (uinput_mouse_fd < 0 || uinput_kbd_fd < 0)
	{
		fprintf(stderr, "Warning: uinput_fd not valid\n");
		return;
	}
	
	if (bev->keycode <= KEY_UNKNOWN || bev->keycode >= KEY_OK)
		fd = uinput_kbd_fd;
	else
		fd = uinput_mouse_fd;
		
	gettimeofday(&event.time, NULL);
	
	for (i=0; i<MAX_MODS; i++)
	{
		if (bev->mod[i] == 0)
			continue;
			
		event.type = EV_KEY;
		event.code = bev->mod[i];
		event.value = bev->pressed;
		write(uinput_kbd_fd, &event, sizeof(event));
		
		event.type = EV_SYN;
  		event.code = SYN_REPORT;
  		event.value = 0;
  		write(uinput_kbd_fd, &event, sizeof(event));
  		
  		usleep(10);	// Needs a little delay for mouse + modifier combo
	}
	
	event.type = EV_KEY;
	event.code = bev->keycode;
	event.value = bev->pressed;
	write(fd, &event, sizeof(event));
	
	event.type = EV_SYN;
  	event.code = SYN_REPORT;
  	event.value = 0;
  	write(fd, &event, sizeof(event));
}

