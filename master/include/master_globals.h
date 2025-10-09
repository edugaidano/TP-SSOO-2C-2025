#ifndef MASTER_GLOBALS_H
#define MASTER_GLOBALS_H
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>

extern t_log *logger_master;
extern t_config *config_master;
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
extern pthread_mutex_t mutex_workers;
extern pthread_cond_t all_workers_busy;
typedef struct worker worker_t;
typedef struct query query_t;

typedef enum
{
    READY,
    EXEC,
    FINISHED
} state_t;

struct worker
{
    int socket;
    char *id;
    bool interrumpir;
    sem_t sem_interrupt;
    pthread_mutex_t mutex;
    state_t state;
    query_t *query;
};

struct query
{
    int id;
    int pc;
    int socket;
    char *file;
    int prioridad;
    worker_t *worker;
    pthread_mutex_t mutex;
    state_t state;
};

#endif