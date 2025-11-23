#ifndef STORAGE_GLOBALS_H
#define STORAGE_GLOBALS_H

#include <commons/log.h>
#include <commons/config.h>

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

// Códigos de resultado fijos
#define STORAGE_RESULT_OK 0
#define STORAGE_RESULT_ERROR -1
#define STORAGE_RESULT_BUSY 1

// Tipos de operación simulados
typedef enum
{
    STORAGE_OP_CREATE = 1,
    STORAGE_OP_TRUNCATE,
    STORAGE_OP_WRITE,
    STORAGE_OP_READ,
    STORAGE_OP_COMMIT,
    STORAGE_OP_DELETE,
    STORAGE_OP_TAG
} t_storage_operation;

#endif 