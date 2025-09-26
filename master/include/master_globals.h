#ifndef MASTER_GLOBALS_H
#define MASTER_GLOBALS_H
#include <commons/log.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>

extern t_log *logger_master;
extern char *PUERTO_ESCUCHA;
extern char *ALGORITMO_PLANIFICACION;
extern int TIEMPO_AGING;
extern t_list *querys_ready;
extern t_list *querys_exec;
extern t_list *workers;
extern sem_t sem_ready;
extern sem_t sem_workers;
extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_exec;
extern pthread_mutex_t mutex_worker;

typedef struct
{
    int fd;
    char *id;
    bool is_free;
} worker;

typedef struct
{
    int id;
    int pc;
    int *controler_socket;
    char *file;
    char *prioridad;
    worker *worker;
} query;


#endif