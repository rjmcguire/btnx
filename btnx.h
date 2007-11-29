
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */

#ifndef BTNX_H_
#define BTNX_H_

#include <sys/time.h>

#define MAX_MODS		3
#define MAX_RAWCODES	10
#define HEXDUMP_SIZE	8

#define OUT_PRE			" btnx: "

enum
{
	BTNX_EXIT_NORMAL=0,
	BTNX_ERROR_FATAL,
	BTNX_ERROR_NO_BIN_RESERVED,
	BTNX_ERROR_NO_CONFIG=150,
	BTNX_ERROR_BAD_CONFIG,
	BTNX_ERROR_OPEN_HANDLER,
	BTNX_ERROR_OPEN_UINPUT,
};

enum
{
	BTNX_EXTRA_EVENTS=0xFFF0,
	REL_WHEELFORWARD,
	REL_WHEELBACK,
	COMMAND_EXECUTE,
	CONFIG_SWITCH
};

enum
{
	CONFIG_SWITCH_NONE,
	CONFIG_SWITCH_NEXT,
	CONFIG_SWITCH_PREV,
	CONFIG_SWITCH_TO
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
	int delay;
	struct timeval last;
	int keycode;
	int mod[MAX_MODS];
	int pressed;
	int enabled;
	char *command;
	char **args;
	int uid;
	int switch_type;
	char *switch_name;
} btnx_event;

typedef struct hexdump
{
	int rawcode;
	int pressed;
} hexdump;

int open_handler(char *name, int flags);

#endif /*BTNX_H_*/
