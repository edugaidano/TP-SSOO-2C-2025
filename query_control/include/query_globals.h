#ifndef QUERY_GLOBALS_H
#define QUERY_GLOBALS_H

#include <commons/log.h>
#include <commons/config.h>

extern t_log *logger_query_control;

extern t_config *config_query_control;
extern char *PUERTO_MASTER;
extern char *IP_MASTER;

extern int socket_master;

#endif