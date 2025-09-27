#ifndef MASTER_UTIL
#define MASTER_UTIL

#include <master_globals.h>
#include <utils/paquetes.h>

int asign_query_id();
void finalizar_query(query_t *query);
void liberar_query(query_t *query, int pc);
void notificar_finalizacion(query_t *query);
void notificar_read(query_t *query, char *file, char *tag, char *contenido);
void liberar_worker(worker_t *worker);
query_t *obtener_query();
worker_t *buscar_worker_libre();
void hacer_par_query_worker(query_t *query, worker_t *worker);
void solicitar_ejecucion_query(query_t *query, int socket);
query_t *buscar_victima();
void solicitar_desalojo(query_t *victima);

#endif