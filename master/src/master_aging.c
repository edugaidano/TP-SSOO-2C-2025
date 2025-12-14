#include "master_aging.h"

void *actualizador(void* query_ptr) {
    query_t* query = (query_t*)query_ptr;

    while (true) {
        if (query->prioridad == 0) return NULL;

        usleep(TIEMPO_AGING * 1000); // Milisegundos -> Mircrosegundos

        pthread_mutex_lock(&mutex_ready);

        if (!list_remove_element(querys_ready, query)) {
            pthread_mutex_unlock(&mutex_ready);
            return NULL;
        }

        if (query->prioridad > 0) {
            query->prioridad--;

            list_add_sorted(querys_ready, query, priority_comparator);
            log_info(logger_master, "## %d Cambio de prioridad: %d - %d", query->id, (query->prioridad + 1), query->prioridad);
            pthread_mutex_unlock(&mutex_ready);

            check_prior();
            continue;
        }
        
        pthread_mutex_unlock(&mutex_ready);
    }
}

void check_prior() {
    sem_wait(&sem_check_prior);

    pthread_mutex_lock(&mutex_ready);
    query_t* siguiente_query = list_get(querys_ready, 0);
    
    worker_t *worker_libre = buscar_worker_libre();
    if (worker_libre == NULL && !list_is_empty(workers))
    {
        query_t *query_victima = buscar_victima();
        if (query_victima->prioridad > siguiente_query->prioridad && !query_victima->worker->interrumpir)
        {
            query_victima->worker->interrumpir = true; // Se desalojara cuando verifique la interrupcion
            sem_wait(&sem_int);
        }
    }

    pthread_mutex_unlock(&mutex_ready);

    sem_post(&sem_check_prior);
}


