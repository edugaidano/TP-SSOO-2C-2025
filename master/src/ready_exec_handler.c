#include "ready_exec_handler.h"

void *ready_exec_handler(void *arg)
{
    if (strcmp(ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0)
    {
        pthread_t actualizador_de_prioridad;
        pthread_create(actualizador_de_prioridad, NULL, &actualizador, NULL);
    }

    while (1)
    {
        sem_wait(&sem_ready);
        sem_wait(&sem_workers);

        query *query = obtener_query(querys_ready);
        worker *worker = buscar_worker_libre();

        hacer_par_query_worker(query, worker);
        solicitar_ejecucion_query(query, worker->fd);

        esperar_respuesta(worker->fd);
    }
    return 0;
}

void *actualizador()
{
    usleep(TIEMPO_AGING * 1000);
    pthread_mutex_lock(&mutex_ready);

    t_list_iterator *iterator = list_iterator_create(querys_ready);

    while (list_iterator_has_next(iterator))
    {
        query *query = list_iterator_next(iterator);
        if (query->prioridad > 0)
        {
            query->prioridad--;
        }
    }

    pthread_mutex_unlock(&mutex_ready);
    list_iterator_destroy(iterator);
}

query *obtener_query(t_list *list)
{
    if (strcmp(ALGORITMO_PLANIFICACION, "FIFO") == 0)
    {
        pthread_mutex_lock(&mutex_ready);
        return list_remove(list, 0);
        pthread_mutex_unlock(&mutex_ready);
    }
    if (strcmp(ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0)
    {
        pthread_mutex_lock(&mutex_ready);
        t_list_iterator *iterator = list_iterator_create(querys_ready);
        query *query_prioritaria = list_iterator_next(iterator);
        while (list_iterator_has_next(iterator))
        {
            query *query = list_iterator_next(iterator);
            if (query->prioridad < query_prioritaria->prioridad)
            {
                query_prioritaria = query;
            }
        }
        pthread_mutex_unlock(&mutex_ready);
        list_iterator_destroy(iterator);
        return query_prioritaria;
    }
}

worker *buscar_worker_libre()
{
    pthread_lock(&mutex_worker);
    t_list_iterator *iterator = list_iterator_create(workers);
    while (list_iterator_has_next(iterator))
    {
        worker *worker = list_iterator_next(iterator);
        if (worker->is_free)
        {
            pthread_mutex_unlock(&mutex_worker);
            list_iterator_destroy(iterator);
            return worker;
        }
    }
    pthread_mutex_unlock(&mutex_worker);
    list_iterator_destroy(iterator);
    return NULL;
}

void hacer_par_query_worker(query *query, worker *worker)
{
    pthread_mutex_lock(&mutex_exec);
    list_add(querys_exec, query);
    query->worker = worker;
    pthread_mutex_unlock(&mutex_exec);
}

solicitar_ejecucion_query(query *query, int socket)
{
    paquete_t *paquete = (SOLICITUD_EJECUCION);
    agregar_a_paquete(socket, &query->id, sizeof(int));
    agregar_a_paquete(socket, &query->pc, sizeof(int));
    agregar_a_paquete(socket, query->file, sizeof(query->file));
    enviar_paquete(socket, paquete);
}

esperar_respuesta(int socket)
{
}
