#ifndef ALGORITMOS_REEMPLAZO_H
#define ALGORITMOS_REEMPLAZO_H

#include "memoria.h"
#include "utils/paquetes.h"

int algoritmo_LRU(int fd_storage, char* identificador_nueva_pagina);
int algoritmo_CLOCK_M(int fd_storage, char* identificador_nueva_pagina);

#endif