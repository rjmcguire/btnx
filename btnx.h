
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */

#ifndef BTNX_H_
#define BTNX_H_

#define MAX_MODS		3
#define MAX_RAWCODES	10
#define HEXDUMP_SIZE	8

enum
{
	BTNX_EXTRA_EVENTS=0xFFF0,
	REL_WHEELFORWARD,
	REL_WHEELBACK,
	COMMAND_EXECUTE
};

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
	int enabled;
	char *command;
	char **args;
	int uid;
} btnx_event;

typedef struct hexdump
{
	int rawcode;
	int pressed;
} hexdump;

int open_handler(char *name, int flags);

#endif /*BTNX_H_*/
