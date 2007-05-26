
 /* 
  * Copyright (C) 2007  Olli Salonen (www.ollisalonen.com)
  * see btnx.c for detailed license information
  */

#include "config_parser.h"
#include "device.h"

#define _GNU_SOURCE				// Needed for strcasestr()

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#define CONFIG_BLOCK_BEGIN		"button"
#define CONFIG_BLOCK_END		"endbutton"
#define MAX_BEVS	10

btnx_event **config_parse(void)
{
	FILE *fp;
	char buffer[CONFIG_PARSE_BUFFER_SIZE];
	char option[CONFIG_PARSE_OPTION_SIZE];
	char value[CONFIG_PARSE_VALUE_SIZE];
	char *loc_eq, *loc_com, *loc_beg, *loc_end;
	int block_begin = 0, block_end = 1;
	btnx_event **bevs;
	int i=-1, ret;
	
	sprintf(buffer,"%s/%s", CONFIG_PATH, CONFIG_NAME);
	
	if (!(fp = fopen(buffer,"r")))
	{
		if (errno == ENOENT)
		{
			sprintf(buffer, "cp %s/%s%s %s/%s 2> error.log",
					DEFAULTS_CONFIG_PATH, 
					DEFAULT_CONFIG_NAME, 
					device_get_name(-1),
					CONFIG_PATH,
					CONFIG_NAME);
			ret = system(buffer);
			if (ret != 0)
			{
				fprintf(stderr, "Error: No config defined and default configuration missing for %s.\n",
					device_get_name(-1));
				exit(1);
			}
			sprintf(buffer,"%s/%s", CONFIG_PATH, CONFIG_NAME);
			if (!(fp = fopen(buffer,"r")))
			{
				perror("Could not read the config file");
				return NULL;
			}
		}
		else
		{
			perror("Could not read the config file");
			return NULL;
		}
	}
	
	bevs = (btnx_event **) calloc(MAX_BEVS+1, sizeof(btnx_event*));
	
	while (fgets(buffer, 511, fp) != NULL)
	{
		if (buffer[0] != '#')
		{
			loc_eq = strchr(buffer, '=');
			loc_com = strchr(buffer,'#');
			loc_beg = buffer;
			while (isspace(*loc_beg)) loc_beg++;
			
			if (loc_eq && (loc_com == NULL || loc_com > loc_eq) && i>=0)
			{
				if (loc_com != NULL && loc_com != loc_eq + 1)
					*loc_com = '\0';
				
				loc_beg = buffer;
				while (isspace(*loc_beg)) loc_beg++;
				loc_end = loc_eq;
				while (isspace(*(loc_end-1))) loc_end--;
				*loc_end = '\0';
				strcpy(option, loc_beg);
				
				loc_beg = loc_eq;
				while (isspace(*(loc_beg+1))) loc_beg++;
				loc_beg++;
				loc_end = loc_beg + strlen(loc_beg);
				while (isspace(*(loc_end-1))) loc_end--;
				*loc_end = '\0';		
				strcpy(value, loc_beg);
				
				if (!config_add_value(bevs[i], option, value))
					perror("Warning: parse error");
				
				memset(value, '\0', CONFIG_PARSE_VALUE_SIZE * sizeof(char));
				memset(option, '\0', CONFIG_PARSE_OPTION_SIZE * sizeof(char));
				memset(buffer, '\0', CONFIG_PARSE_BUFFER_SIZE * sizeof(char));
			}
			else if (*loc_beg == '\0')
				continue;
			else
			{
				loc_end = loc_beg;
				
				if (loc_com == NULL)
					loc_com = &buffer[strlen(buffer)-1];
				
				while (!isspace(*loc_end) && loc_end < loc_com && *loc_end != '\0') loc_end++;
				*loc_end = '\0';
				
				if (strcasecmp(loc_beg, CONFIG_BLOCK_BEGIN) == 0)
				{
					if (block_end == 0)
					{
						fprintf(stderr, "Warning: config file parse error\n");
						continue;
					}
					block_begin = 1;
					i++;
					if (i >= MAX_BEVS)
						bevs = (btnx_event **) realloc(bevs, (i+2)*sizeof(bevs));
					bevs[i] = (btnx_event *) calloc(1, sizeof(btnx_event));
					bevs[i+1] = NULL;
				}
				else if (strcasecmp(loc_beg, CONFIG_BLOCK_END) == 0)
				{
					if (block_begin == 0)
					{
						fprintf(stderr, "Warning: config file parse error\n");
						continue;
					}
					block_end = 1;
				}
			}
		}
	}
	
	fclose(fp);
	
	return bevs;
}

char *config_add_value(btnx_event *e, char *option, char *value)
{
#ifdef DEBUG	
	printf("Loaded config value: %s\n",option);
#endif
	
	if (!strcasecmp(option, "rawcode"))
	{
		e->rawcode = strtol(value, NULL, 16);
		return option;
	}
	if (!strcasecmp(option, "type"))
	{
		e->type = strtol(value, NULL, 10);
		return option;
	}
	if (!strcasecmp(option, "keycode"))
	{
		e->keycode = config_get_keycode(value);
		return option;
	}
	if (!strcasecmp(option, "mod1"))
	{
		config_add_mod(e, config_get_keycode(value));
		return option;
	}
	if (!strcasecmp(option, "mod2"))
	{
		config_add_mod(e, config_get_keycode(value));
		return option;
	}
	if (!strcasecmp(option, "mod3"))
	{
		config_add_mod(e, config_get_keycode(value));
		return option;
	}
	
	return NULL;
}

void config_add_mod(btnx_event *e, int mod)
{
	int i;
	
	for (i=0; i<MAX_MODS; i++)
	{
		if (e->mod[i] == 0)
		{
			e->mod[i] = mod;
			return;
		}
	}
	
	fprintf(stderr, "Warning: attempting to add more mods than allowed by MAX_MODS\n");
}

int config_get_keycode(const char *value)
{
	FILE *fp;
	char buffer[128];
	char *loc_beg;
	int len;
	
	if (strlen(value) > 20)
	{
		fprintf(stderr, "Warning: possibly malformed keycode or modifier value. Ignoring.\n");
		return 0;
	}
	
	if (!strcasecmp(value, "none"))
		return 0;
	
	sprintf(buffer, "cat /home/daou/events | /bin/grep %s", value);
	fp = popen(buffer, "r");
	while (fgets(buffer, 127, fp) != NULL)
	{
		len = strlen(buffer);
		if (len > 0 && len > strlen(value))
		{
			loc_beg = strcasestr(buffer, value) + strlen(value);
			if (!isspace(*loc_beg))
				continue;
			pclose(fp);
			return strtol(loc_beg, NULL, 0);
		}
	}
	pclose(fp);
	
	return 0;
}


