#ifndef ALGORITMOS_REEMPLAZO_H
#define ALGORITMOS_REEMPLAZO_H

#include "memoria.h"
#include "worker_globals.h"
#include "utils/paquetes.h"

int algoritmo_LRU(char* id_nueva_pagina, int nro_nueva_pagina);
int algoritmo_CLOCK_M(char* id_nueva_pagina, int nro_nueva_pagina);

void notificar_cambios (nodo_pagina* victima, char* identificador, void* p_marco, int fd_storage);

#endif