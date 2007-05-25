

#ifndef CONFIG_PARSER_H_
#define CONFIG_PARSER_H_

#include "btnx.h"

#ifdef DEBUG
#define CONFIG_NAME			".btnx_config_debug"
#else
#define CONFIG_NAME			".btnx_config"
#endif
#define CONFIG_PATH			"/etc/btnx"

#define CONFIG_PARSE_BUFFER_SIZE			512
#define CONFIG_PARSE_OPTION_SIZE			64
#define CONFIG_PARSE_VALUE_SIZE				512


/* Parses the configuration file */
btnx_event **config_parse(void);

/* Checks which option value is being parsed and calls fillConfigValue for that option. */
char *config_add_value(btnx_event *e, char *option, char *value);

int config_get_keycode(const char *value);

void config_add_mod(btnx_event *e, int mod);


#endif /*CONFIG_PARSER_H_*/
