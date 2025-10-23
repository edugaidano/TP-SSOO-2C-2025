#include <worker_globals.h>

t_log *logger_worker;
t_config *config_worker;
char *IP_MASTER;
char *PUERTO_MASTER;
char *IP_STORAGE;
char *PUERTO_STORAGE;
char *PATH_SCRIPTS;
int TAM_MEMORIA;
int RETARDO_MEMORIA;
char *ALGORITMO_REEMPLAZO;
void *memoria;
int tam_pagina;
int cantidad_marcos;
t_list* marco;
t_list* file_tag_pages;
bool* bit_map_marco;
t_temporal* cronometro;
nodo_marco* victima_clock;
int (*algoritmo_reemplazo)(int, char*);