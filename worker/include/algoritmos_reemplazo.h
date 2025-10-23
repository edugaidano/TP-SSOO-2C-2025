#ifndef ALGORITMOS_REEMPLAZO_H
#define ALGORITMOS_REEMPLAZO_H

#include <worker_globals.h>
#include <utils/paquetes.h>
#include <memoria.h>

int algoritmo_LRU(int fd_storage, char* identificador_nueva_pagina);
int algoritmo_CLOCK_M(int fd_storage, char* identificador_nueva_pagina);

#endif