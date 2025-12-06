#ifndef STORAGE_GLOBALS_H
#define STORAGE_GLOBALS_H

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <pthread.h>
#include <semaphore.h>

extern t_log *logger_storage;

extern t_config *config_storage;
extern t_config *config_super_block;
extern t_config *config_hash_index;
extern char *LOG_LEVEL;
extern char *PUERTO_ESCUCHA;
extern char *PUNTO_MONTAJE;
extern char *FRESH_START;
extern int RETARDO_OPERACION;
extern int RETARDO_ACCESO_BLOQUE;
extern int FS_SIZE;
extern int BLOCK_SIZE;
extern int CANT_WORKERS;

extern pthread_mutex_t worker_count_mutex;    // para CANT_WORKERS
extern pthread_mutex_t bhi_mutex;             // mutex para controlar el block_hash_index

// Sincro de File:Tag
extern t_dictionary* locks_index;
extern sem_t sem_locks_index;

extern t_list* blk_mutex_list;                // array de mutexs para cada bloque fisico

typedef struct t_lock
{
    sem_t sem_file_tag;
    int count_waiting;
    bool delete;
} t_lock;

#endif 