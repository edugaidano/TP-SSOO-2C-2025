#include "query_globals.h"

t_log *logger_query_control;

t_config *config_query_control;
char *PUERTO_MASTER;
char *IP_MASTER;
char *LOG_LEVEL;

int socket_master = -1;