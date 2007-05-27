
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */

#define _GNU_SOURCE				// Needed for strcasestr()

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "devices_parser.h"
#include "device.h"

#define DD_BUFFER_SIZE			128
#define SEARCH_STRING_SIZE		64


int detect_device(char *search_str)
{
	FILE *fp;
	int found=0, i=0;
	char buffer[DD_BUFFER_SIZE];
	
	while (found == 0 && !(device_get_vendor(i) == 0xFFFF && device_get_product(i) == 0xFFFF))
	{
		sprintf(buffer, "cat %s | /bin/grep -i -c \"Vendor=%04x Product=%04x\"",
			DEVICES_FILE, device_get_vendor(i), device_get_product(i));
		if (!(fp = popen(buffer, "r")))
		{
			perror("detect_device() could not open /proc/bus/input/devices");
			exit(1);
		}
		if (fgets(buffer, 2, fp) == NULL)
		{
			fprintf(stderr, "detect_device() could not read grep output\n");
			exit(1);
		}
		if (buffer[0] != '0')
		{
			found = 1;
			sprintf(search_str, "Vendor=%04x Product=%04x", 
				device_get_vendor(i), device_get_product(i));
			device_set(i);
		}
		pclose(fp);
		i++;
	}
	
	return found;
}

void devices_parser(char **mouse_event, char **kbd_event)
{
	FILE *fp;
	char buffer[DP_BUFFER_SIZE];
	char search_str[SEARCH_STRING_SIZE];
	int found_section=0;
	char *found_mouse=NULL, *found_kbd=NULL;
	
	*mouse_event = NULL; *kbd_event = NULL;
	
	if (detect_device(search_str) == 0)
	{
		fprintf(stderr, "No supported mice detected. You can make a support request for your mouse\n");
		exit(1);
	}
	
	if (!(fp = fopen(DEVICES_FILE, "r")))
	{
		perror("Could not locate the devices file.");
		exit(1);
	}
	
	while ((fgets(buffer, DP_BUFFER_SIZE-1, fp) != NULL &&
			!(found_mouse && found_kbd)))
	{
		if (found_section == 0)
		{
			if (strcasestr(buffer, search_str) != NULL)
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
	
	if (found_mouse == NULL || mouse_event == NULL)
	{
		fprintf(stderr, "Error: could not parse mouse event handlers\n.");
		exit(1);
	}
	if (found_kbd == NULL || kbd_event == NULL)
		fprintf(stderr, "Did not find additional event handlers.\n");
		
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

