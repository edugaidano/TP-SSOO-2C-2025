#ifndef STORAGE_GLOBALS_H
#define STORAGE_GLOBALS_H
#include <commons/log.h>

extern t_log *logger_storage;
extern char *PUERTO_ESCUCHA;
extern int FRESH_START;
extern int RETARDO_OPERACION;
extern int RETARDO_ACCESO_BLOQUE;
extern int FS_SIZE;
extern int BLOCK_SIZE;
extern int CANT_WORKERS;
#endif