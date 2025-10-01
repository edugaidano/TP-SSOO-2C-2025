#ifndef _MASTER_KEEPALIVE_
#define _MASTER_KEEPALIVE_

#include <pthread.h>
#include <commons/collections/list.h>
#include <master_globals.h>
#include <sys/socket.h>
#include <master_util.h>
#include <unistd.h>

void *master_keepalive(void *arg);
void *worker_monitor(void *arg);

#endif