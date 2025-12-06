#include "process_handler.h"

void *process_handler(void *arg)
{

    if (TIEMPO_AGING != 0 && string_equals_ignore_case(ALGORITMO_PLANIFICACION, "PRIORIDADES"))
    {
        pthread_t actualizador_de_prioridad;
        pthread_create(&actualizador_de_prioridad, NULL, &actualizador, NULL);
        pthread_detach(actualizador_de_prioridad);
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

void *actualizador()
{
    while (true)
    {
        sleep(TIEMPO_AGING / 1000); // Milisegundos -> Segundos
        pthread_mutex_lock(&mutex_ready);

        t_list_iterator *iterator = list_iterator_create(querys_ready);

        while (list_iterator_has_next(iterator))
        {
            query_t *query = list_iterator_next(iterator);
            if (query->prioridad > 0)
            {
                query->prioridad--;
                log_info(logger_master, "## %d Cambio de prioridad: %d - %d", query->id, (query->prioridad + 1), query->prioridad);
            }
            else
            {

                worker_t *worker_libre = buscar_worker_libre();
                if (worker_libre == NULL && !list_is_empty(workers))
                {
                    query_t *query_victima = buscar_victima();
                    if (query_victima->prioridad > query->prioridad)
                    {
                        query_victima->worker->interrumpir = true; // Se desalijara cuando verifique la interrupcion
                    }
                }
            }
        }

        pthread_mutex_unlock(&mutex_ready);
        list_iterator_destroy(iterator);
    }
    return 0;
}