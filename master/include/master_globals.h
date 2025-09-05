#ifndef MASTER_GLOBALS_H
#define MASTER_GLOBALS_H
#include <commons/log.h>

extern t_log *logger_master;
extern char *PUERTO_ESCUCHA;
extern char *ALGORITMO_PLANIFICACION;
extern int TIEMPO_AGING;

typedef struct {
    int fd;
    int prioridad;
    char* paht;
} info_query;

typedef struct {
    int fd;
    char* identificador;
} info_worker;


#endif