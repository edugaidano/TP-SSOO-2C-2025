#ifndef MEMORIA_H
#define MEMORIA_H

#include "algoritmos_reemplazo.h"
#include "worker_globals.h"
#include <math.h>
#include <unistd.h>

void init_memoria();

file_tag* file_tag_en_memoria(char*identificador);
void agregar_file_tag(paquete_t* paquete, char* ft);
void free_file_tag(void* arg);

nodo_pagina* pagina_en_Tabla(char* identificador, int nro_pagina);

void escribir_pagina(nodo_pagina* pagina, char* identificador, int direccion_base, char* datos);
void leer_pagina(nodo_pagina* pagina, char*identificador, int direccion, int size);

#endif