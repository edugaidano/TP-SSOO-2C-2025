#ifndef MASTER_NETWORK_HANDLER
#define MASTER_NETWORK_HANDLER

#include "master_aging.h"
#include "master_util.h"
#include "utils/networking.h"
#include <pthread.h>

void master_network_handler(int socket_server);

#endif