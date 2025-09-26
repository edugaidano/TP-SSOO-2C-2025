#ifndef READY_EXEC_HANDLER
#define READY_EXEC_HANDLER

#include <stddef.h>
#include <semaphore.h>
#include <master_globals.h>
#include <pthread.h>
#include <string.h>
#include <utils/paquetes.h>
#include <utils/op_codes.h>

void *ready_exec_handler(void *arg);
void *actualizador();
query *obtener_query(t_list *list);
worker *buscar_worker_libre();

#endif