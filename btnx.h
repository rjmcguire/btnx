
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */

#ifndef BTNX_H_
#define BTNX_H_

#define MAX_MODS		3
#define MAX_RAWCODES	5
#define HEXDUMP_SIZE	16

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
