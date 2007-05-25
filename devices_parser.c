
 /* 
  * Copyright (C) 2007  Olli Salonen (www.ollisalonen.com)
  * see btnx.c for detailed license information
  */

#define _GNU_SOURCE				// Needed for strcasestr()

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "devices_parser.h"

void devices_parser(char **mouse_event, char **kbd_event)
{
	FILE *fp;
	char buffer[DP_BUFFER_SIZE];
	char mx_ss[64], vx_ss[64];
	int found_section=0;
	char *found_mouse=NULL, *found_kbd=NULL;
	
	*mouse_event = NULL; *kbd_event = NULL;
	
	if (!(fp = fopen(DEVICES_FILE, "r")))
	{
		perror("Could not locate the devices file.");
		exit(1);
	}
	
	sprintf(mx_ss, "Vendor=%s Product=%s", VENDOR_ID, MX_PRODUCT_ID);
	sprintf(vx_ss, "Vendor=%s Product=%s", VENDOR_ID, VX_PRODUCT_ID);
	while ((fgets(buffer, DP_BUFFER_SIZE-1, fp) != NULL &&
			!(found_mouse && found_kbd)))
	{
		if (found_section == 0)
		{
			if (strcasestr(buffer, mx_ss) != NULL || strcasestr(buffer, vx_ss) != NULL)
				found_section = 1; 
		}
		else
		{
			if (strcasestr(buffer, HANDLERS_PATTERN) == NULL)
				continue;
			if (found_mouse == NULL)
			{
				if ((found_mouse = strcasestr(buffer, MOUSE_PATTERN)) != NULL)
				{
					*mouse_event = get_event(buffer, *mouse_event);
					found_section = 0;
					continue;
				}
			}
			if (found_kbd == NULL)
			{
				if ((found_kbd = strcasestr(buffer, KEYBOARD_PATTERN)) != NULL)
				{
					*kbd_event = get_event(buffer, *kbd_event);
					found_section = 0;
					continue;
				}
			}
		}
	}
	
	printf("mouse=%s searchkey=%s\n", *mouse_event, *kbd_event);
	
	if (found_mouse == NULL || found_kbd == NULL || mouse_event == NULL || kbd_event == NULL)
	{
		fprintf(stderr, "Error: could not parse mouse event handlers\n.");
		exit(1);
	}
	
	fclose(fp);
}

char *get_event(char *loc_beg, char *event)
{
	char *loc_end;
	
	if ((loc_beg = strcasestr(loc_beg, EVENT_PATTERN)) == NULL)
	{
		fprintf(stderr, "Error: did not find an event handler for the mouse.\n");
		exit(1);
	}
	loc_end = loc_beg;
	while (!isspace(*loc_end) && *loc_end != '\0') loc_end++;
	*loc_end = '\0';
	
	event = (char *) malloc((strlen(loc_beg)+strlen(EVENT_LOCATION)+1)*sizeof(char));
	strcpy(event, EVENT_LOCATION);
	strcat(event, loc_beg);
	return event;
}

