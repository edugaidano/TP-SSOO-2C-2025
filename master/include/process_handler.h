#ifndef PROCESS_HANDLER
#define PROCESS_HANDLER

#include <stddef.h>
#include <semaphore.h>
#include <master_globals.h>
#include <pthread.h>
#include <string.h>
#include <utils/paquetes.h>
#include <utils/op_codes.h>
#include <master_util.h>

void *process_handler(void *arg);
void *esperar_respuesta(void *arg);
void *actualizador();
void *desalojador();

#endif