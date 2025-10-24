#ifndef MEMORIA_H
#define MEMORIA_H

#include <worker_globals.h>
#include "algoritmos_reemplazo.h"
#include <math.h>
#include <unistd.h>

void init_memoria();
int marco_libre();
file_tag* file_tag_en_memoria(char*identificador);
nodo_pagina* pagina_en_Tabla(char* identificador, double nro_pagina);
file_tag* agregar_file_tag_en_memoria(char* identificador, int fd_storage);
void free_file_tag(void* arg);
nodo_pagina* solicitar_pagina(char* identificador, int nro_pagina, int fd_storage, char* query_id);
void escribir_pagina(nodo_pagina* pagina, char* identificador, int direccion_base, char* datos, int fd_storage, char* query_id);
void leer_pagina(nodo_pagina* pagina, char*identificador, int direccion, int size, int fd_storage, int fd_master, char* query_id);

#endif