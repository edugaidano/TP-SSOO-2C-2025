#include <master_globals.h>

t_log *logger_master;
char *PUERTO_ESCUCHA;
char *ALGORITMO_PLANIFICACION;
int TIEMPO_AGING;
t_list *querys_ready;
t_list *querys_exec;
t_list *workers;
sem_t sem_ready;
sem_t sem_workers;
pthread_mutex_t mutex_ready;
pthread_mutex_t mutex_exec;
pthread_mutex_t mutex_worker;
