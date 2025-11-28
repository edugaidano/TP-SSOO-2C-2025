#include "storage_globals.h"

t_log *logger_storage;

t_config *config_storage;
t_config *config_super_block;
t_config *config_hash_index;
char *LOG_LEVEL;
char *PUERTO_ESCUCHA;
char *PUNTO_MONTAJE;
char *FRESH_START;
int RETARDO_OPERACION;
int RETARDO_ACCESO_BLOQUE;
int FS_SIZE;
int BLOCK_SIZE;
int CANT_WORKERS;

pthread_mutex_t fs_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t worker_count_mutex = PTHREAD_MUTEX_INITIALIZER;