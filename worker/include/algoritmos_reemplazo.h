#ifndef ALGORITMOS_REEMPLAZO_H
#define ALGORITMOS_REEMPLAZO_H

#include "memoria.h"
#include "utils/paquetes.h"

int algoritmo_LRU(char* id_nueva_pagina, int nro_nueva_pagina);
int algoritmo_CLOCK_M(char* id_nueva_pagina, int nro_nueva_pagina);

#endif