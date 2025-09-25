#ifndef MASTER_GLOBALS_H
#define MASTER_GLOBALS_H
#include <commons/log.h>
#include <commons/collections/list.h>

extern t_log *logger_master;
extern char *PUERTO_ESCUCHA;
extern char *ALGORITMO_PLANIFICACION;
extern int TIEMPO_AGING;
extern t_list *querys_ready;
extern t_list *querys_exec;

typedef struct
{
    int id;
    int controler_socket;
    char *file;
    char *prioridad;
} query;

typedef struct
{
    int fd;
    char *identificador;
} info_worker;

#endif