
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */

#ifndef CONFIG_PARSER_H_
#define CONFIG_PARSER_H_

#include "btnx.h"

#ifdef DEBUG
#define CONFIG_NAME				"btnx_config_debug"
#else
#define CONFIG_NAME				"btnx_config"
#endif
#define CONFIG_PATH				"/etc/btnx"
#define EVENTS_NAME				"events"
#define DEFAULTS_CONFIG_PATH	"/etc/btnx/defaults"
#define DEFAULT_CONFIG_NAME		"default_config_"
#define CONFIG_MANAGER_FILE		CONFIG_PATH "/btnx_manager"

#define CONFIG_PARSE_BUFFER_SIZE			512
#define CONFIG_PARSE_OPTION_SIZE			64
#define CONFIG_PARSE_VALUE_SIZE				512
#define CONFIG_NAME_MAX_SIZE				64

#define IS_ENCLOSING(c) ((c) == '\'' || (c) == '"' || (c) == '`')


char *config_get_next(void);
char *config_get_prev(void);

/* Parses the configuration file */
btnx_event **config_parse(char *config_name);

char *config_add_value(btnx_event *e, int type, char *option, char *value);

int config_get_keycode(const char *value);

void config_add_mod(btnx_event *e, int mod);

char *config_set_command(btnx_event *e, char *value);

void config_set_switch_type(btnx_event *e, char *value);
void config_set_switch_name(btnx_event *e, char *value);

#endif /*CONFIG_PARSER_H_*/
