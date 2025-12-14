#include "process_handler.h"

void *process_handler(void *arg)
{
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