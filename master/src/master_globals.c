#include <master_globals.h>

t_log *logger_master;
t_config *config_master;
char *PUERTO_ESCUCHA;
char *ALGORITMO_PLANIFICACION;
int TIEMPO_AGING;
t_list *querys_ready;
t_list *querys_exec;
t_list *workers;
sem_t sem_ready;
sem_t sem_workers;
pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_exec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_workers = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t all_workers_busy = PTHREAD_COND_INITIALIZER;
