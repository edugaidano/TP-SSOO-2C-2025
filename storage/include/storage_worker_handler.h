#ifndef STORAGE_WORKER_HANDLER_H
#define STORAGE_WORKER_HANDLER_H
#define STORAGE_RESULT_OK 0
#define STORAGE_RESULT_ERROR -1

#include "storage_operaciones.h"
#include "storage_globals.h" 
#include <utils/networking.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

void *storage_worker_handler(void *arg);

#endif