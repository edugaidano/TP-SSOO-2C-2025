#ifndef PARSER_H
#define PARSER_H

#include <commons/string.h>
#include <stdio.h>
#include <worker_globals.h>

t_list* parsear_archivo(char* path_archivo_query);
void destruir_instrucciones(void* arg);

#endif