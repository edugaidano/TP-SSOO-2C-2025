#ifndef MASTER_NETWORK_HANDLER
#define MASTER_NETWORK_HANDLER

#include <utils/networking.h>
#include <master_globals.h>
#include <pthread.h>
#include <master_util.h>

void *master_network_handler(void *arg);
void *administrar_query(void *info_query);
void *administrar_worker(void *fd);
void *keepalive(void *arg);

#endif