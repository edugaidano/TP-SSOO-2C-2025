#include "process_handler.h"

void *process_handler(void *arg)
{
    if (TIEMPO_AGING != 0 && string_equals_ignore_case(ALGORITMO_PLANIFICACION, "PRIORIDADES"))
    {
        pthread_t revisar_prioridad;
        pthread_create(&revisar_prioridad, NULL, &check_prior, NULL);
        pthread_detach(revisar_prioridad);
    }

    while (1)
    {
        sem_wait(&sem_ready);
        sem_wait(&sem_workers);

        query_t *query = obtener_query();
        worker_t *worker = buscar_worker_libre();

        hacer_par_query_worker(query, worker);
        solicitar_ejecucion_query(query, worker->socket);
    }
    return 0;
}