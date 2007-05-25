
 /* 
  * Copyright (C) 2007  Olli Salonen (www.ollisalonen.com)
  * see btnx.c for detailed license information
  */

#ifndef UINPUT_H_
#define UINPUT_H_

#define UMOUSE_NAME		"btnx mouse"
#define UKBD_NAME		"btnx keyboard"

#define UINPUT_LOCATION	"/dev/input/uinput"

#include "btnx.h"

int uinput_init(const char *dev_name);

void uinput_key_press(struct btnx_event *bev);

#endif /*UINPUT_H_*/
