#ifndef PARSER_H
#define PARSER_H

#include "worker_globals.h"
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>

t_list* parsear_archivo(char* path_archivo_query);
void destruir_instruccion(void* arg);

#endif