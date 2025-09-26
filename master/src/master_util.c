#include <master_util.h>

int asign_query_id()
{
    static int id = 0;
    return ++id;
}

void finalizar_query(query_t *query)
{
    pthread_mutex_lock(&mutex_exec);
    list_remove_element(querys_exec, query);
    pthread_mutex_unlock(&mutex_exec);

    notificar_finalizacion(query);
}

void liberar_query(query_t *query, int pc)
{
    pthread_mutex_lock(&mutex_exec);
    list_remove_element(querys_exec, query);
    pthread_mutex_unlock(&mutex_exec);

    pthread_mutex_lock(&mutex_ready);
    list_add(querys_ready, query);
    pthread_mutex_unlock(&mutex_ready);

    sem_post(&sem_ready);
}

void notificar_finalizacion(query_t *query)
{
    int mensaje = FINALIZACION;
    paquete_t *paquete = crear_paquete(NOTIF_QUERY_CONTROL);
    agregar_a_paquete(paquete, &mensaje, sizeof(int));
    enviar_paquete(query->worker->fd, paquete, logger_master, "Query Control");
}

void notificar_read(query_t *query, char *contenido)
{
    int mensaje = READ;
    paquete_t *paquete = crear_paquete(NOTIF_QUERY_CONTROL);
    agregar_a_paquete(paquete, &mensaje, sizeof(int));
    agregar_a_paquete(paquete, contenido, sizeof(contenido));
    enviar_paquete(query->worker->fd, paquete, logger_master, "Query Control");
}

void liberar_worker(worker_t *worker)
{
    pthread_mutex_lock(&mutex_workers);
    worker->is_free = true;
    pthread_mutex_unlock(&mutex_workers);

    sem_post(&sem_workers);
}

query_t *obtener_query(t_list *list)
{
    if (strcmp(ALGORITMO_PLANIFICACION, "FIFO") == 0)
    {
        pthread_mutex_lock(&mutex_ready);
        query_t *query = list_remove(list, 0);
        pthread_mutex_unlock(&mutex_ready);
        return query;
    }
    if (strcmp(ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0)
    {
        pthread_mutex_lock(&mutex_ready);
        t_list_iterator *iterator = list_iterator_create(querys_ready);
        query_t *query_prioritaria = list_iterator_next(iterator);
        while (list_iterator_has_next(iterator))
        {
            query_t *query = list_iterator_next(iterator);
            if (query->prioridad < query_prioritaria->prioridad)
            {
                query_prioritaria = query;
            }
        }
        list_remove_element(querys_ready, query_prioritaria);
        pthread_mutex_unlock(&mutex_ready);
        list_iterator_destroy(iterator);
        return query_prioritaria;
    }
    return NULL;
}

worker_t *buscar_worker_libre()
{
    pthread_mutex_lock(&mutex_workers);
    t_list_iterator *iterator = list_iterator_create(workers);
    while (list_iterator_has_next(iterator))
    {
        worker_t *worker = list_iterator_next(iterator);
        if (worker->is_free)
        {
            pthread_mutex_unlock(&mutex_workers);
            list_iterator_destroy(iterator);
            return worker;
        }
    }
    pthread_mutex_unlock(&mutex_workers);
    list_iterator_destroy(iterator);
    return NULL;
}

void hacer_par_query_worker(query_t *query, worker_t *worker)
{
    pthread_mutex_lock(&mutex_exec);
    pthread_mutex_lock(&mutex_workers);
    list_add(querys_exec, query);
    query->worker = worker;
    worker->is_free = false;
    pthread_mutex_unlock(&mutex_exec);
    pthread_mutex_unlock(&mutex_workers);

    if (list_size(querys_exec) == list_size(workers))
    {
        pthread_cond_signal(&all_workers_busy);
    }
}

void solicitar_ejecucion_query(query_t *query, int socket)
{
    paquete_t *paquete = crear_paquete(SOLICITUD_EJECUCION);
    agregar_a_paquete(paquete, &query->id, sizeof(int));
    agregar_a_paquete(paquete, &query->pc, sizeof(int));
    agregar_a_paquete(paquete, query->file, sizeof(query->file));
    enviar_paquete(socket, paquete, logger_master, "Worker");
}

query_t *buscar_victima()
{
    t_list_iterator *iterator = list_iterator_create(querys_exec);
    query_t *victima = list_iterator_next(iterator);
    while (list_iterator_has_next(iterator))
    {
        query_t *query = list_iterator_next(iterator);
        if (query->prioridad > victima->prioridad)
        {
            victima = query;
        }
    }
    list_iterator_destroy(iterator);
    return victima;
}

void solicitar_desalojo(query_t *victima)
{
    paquete_t *paquete = crear_paquete(DESALOJO_QUERY);
    enviar_paquete(victima->worker->fd, paquete, logger_master, "Worker");
}