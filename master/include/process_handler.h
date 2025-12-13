#ifndef PROCESS_HANDLER
#define PROCESS_HANDLER

#include "master_aging.h"
#include "utils/paquetes.h"
#include <pthread.h>
#include <stddef.h>
#include <semaphore.h>
#include <string.h>

void *process_handler(void *arg);

#endif