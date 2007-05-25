
 /* 
  * Copyright (C) 2007  Olli Salonen (www.ollisalonen.com)
  * see btnx.c for detailed license information
  */

#ifndef DEVICES_PARSER_H_
#define DEVICES_PARSER_H_

#define VENDOR_ID			"046d"
#define MX_PRODUCT_ID		"c51a"
#define VX_PRODUCT_ID		"c518"

#define MOUSE_PATTERN		"mouse"
#define KEYBOARD_PATTERN	"kbd"
#define EVENT_PATTERN		"event"
#define HANDLERS_PATTERN	"Handlers"

#define DP_BUFFER_SIZE		512
#define SEARCH_STRING_SIZE	64
#define DEVICES_FILE		"/proc/bus/input/devices"
#define EVENT_LOCATION		"/dev/input/"


void devices_parser(char **mouse_event, char **kbd_event);

char *get_event(char *loc_beg, char *event);

#endif /*DEVICES_PARSER_H_*/
