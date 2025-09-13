#ifndef WORKER_GLOBALS_H
#define WORKER_GLOBALS_H

#include <utils/networking.h>
#include <commons/log.h>
#include <commons/config.h>

extern t_log *logger_worker;
extern t_config *config_worker;
extern char *IP_MASTER;
extern char *PUERTO_MASTER;
extern char *IP_STORAGE;
extern char *PUERTO_STORAGE;
extern int TAM_MEMORIA;
extern int RETARDO_MEMORIA;
extern int ALGORITMO_REEMPLAZO;

#endif