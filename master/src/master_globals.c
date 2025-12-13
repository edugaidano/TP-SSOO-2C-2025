#include "master_globals.h"

t_log *logger_master;

t_config *config_master;
char *ALGORITMO_PLANIFICACION;
char *LOG_LEVEL;
char *PUERTO_ESCUCHA;
int TIEMPO_AGING;

t_list *querys_ready;
t_list *querys_exec;
t_list *workers;

sem_t sem_ready;
sem_t sem_workers;
sem_t sem_int;
sem_t sem_check_prior;
sem_t sem_fin_check_prior;

pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_exec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_workers = PTHREAD_MUTEX_INITIALIZER;
