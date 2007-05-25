
 /* 
  * Copyright (C) 2007  Olli Salonen (www.ollisalonen.com)
  * see btnx.c for detailed license information
  */

#ifndef BTNX_H_
#define BTNX_H_

#define MAX_MODS	3

enum
{
	BUTTON_NORMAL=0,
	BUTTON_IMMEDIATE
};

typedef struct btnx_event
{
	int rawcode;
	int type;
	int keycode;
	int mod[MAX_MODS];
	int pressed;
} btnx_event;

#endif /*BTNX_H_*/