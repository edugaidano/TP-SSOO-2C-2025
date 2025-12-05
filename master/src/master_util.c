#include "master_util.h"

int asign_query_id()
{
    static int id = 0;
    return ++id;
}

// el c_error solo es requerido para razon=ERR_STORAGE, en otro caso completar con 0
void finalizar_query(query_t *query, razon_fin razon, rta_storage c_error)
{
    pthread_mutex_lock(&mutex_exec);
    list_remove_element(querys_exec, query);
    pthread_mutex_unlock(&mutex_exec);
    query->state = FINISHED;

    switch (razon)
    {
    case FINALIZACION_CORRECTA:
        log_info(logger_master, "## Se terminó la Query %d en el Worker %s", query->id, query->worker->id);
        break;
    case ERR_DESC_WORKER:
        log_info(logger_master, "## Se desconecta el Worker %s - Se finaliza la Query %d - Cantidad total de Workers: %d", 
            query->worker->id, query->id, (list_size(workers) - 1));
        break;
    case ERR_STORAGE:
        log_error(logger_master, "Se finzaliza la query %d por un error durante la ejecucion de la misma", query->id);
        break;
    default:
        log_error(logger_master, "Se finaliza la query %d (razon no definida)", query->id);
        break;
    }

    notificar_finalizacion(query, razon, c_error);
}

bool priority_comparator(void* a, void* b)
{
    query_t *query_a = (query_t*) a;
    query_t *query_b = (query_t*) b;
    return query_a->prioridad < query_b->prioridad;
}

void liberar_query(query_t *query, int pc)
{
    pthread_mutex_lock(&mutex_exec);
    list_remove_element(querys_exec, query);
    query->worker = NULL;
    query->state = READY;
    pthread_mutex_unlock(&mutex_exec);

    pthread_mutex_lock(&mutex_ready);
    if (pc != -1)
    {
        query->pc = pc;
    }
    list_add(querys_ready, query);
    pthread_mutex_unlock(&mutex_ready);

    sem_post(&sem_ready);
}

void destruir_query(query_t *query)
{
    if (query->worker)
    {
        pthread_mutex_lock(&mutex_exec);
        list_remove_element(querys_exec, query);
        pthread_mutex_unlock(&mutex_exec);
    }
    else
    {
        pthread_mutex_lock(&mutex_ready);
        list_remove_element(querys_ready, query);
        pthread_mutex_unlock(&mutex_ready);
    }

    close(query->socket);
    free(query->file);
    free(query);
}

void notificar_finalizacion(query_t *query, razon_fin razon_enum, rta_storage c_error)
{
    int mensaje = NOTIF_FINAL;

    paquete_t *paquete = crear_paquete(NOTIF_QUERY_CONTROL);
    agregar_a_paquete(paquete, &mensaje, sizeof(int));
    agregar_a_paquete(paquete, &razon_enum, sizeof(razon_fin));
    if (razon_enum == ERR_STORAGE)
    {
        agregar_a_paquete(paquete, &c_error, sizeof(rta_storage));
    }
    
    enviar_paquete(query->socket, paquete, logger_master);
}

void notificar_read(query_t *query, char *file, char *tag, char *contenido)
{
    notif_query_control mensaje = NOTIF_READ;
    paquete_t *paquete = crear_paquete(NOTIF_QUERY_CONTROL);
    agregar_a_paquete(paquete, &mensaje, sizeof(notif_query_control));
    agregar_a_paquete(paquete, file, string_length(file) + 1);
    agregar_a_paquete(paquete, tag, string_length(tag) + 1);
    agregar_a_paquete(paquete, contenido, string_length(contenido) + 1);
    enviar_paquete(query->socket, paquete, logger_master);

    log_info(logger_master, 
        "## Se envía un mensaje de lectura de la Query %d en el Worker %s al Query Control", 
        query->id, query->worker->id);
}

void liberar_worker(worker_t *worker)
{
    pthread_mutex_lock(&mutex_workers);
    worker->state = READY;
    worker->query = NULL;
    worker->interrumpir = false;
    pthread_mutex_unlock(&mutex_workers);
}

void destruir_worker(worker_t *worker)
{
    pthread_mutex_lock(&mutex_workers);
    list_remove_element(workers, worker);
    pthread_mutex_unlock(&mutex_workers);

    sem_close(&(worker->sem_interrupt));
    close(worker->socket);
    free(worker->id);
    free(worker);
}

query_t *obtener_query()
{
    pthread_mutex_lock(&mutex_ready);
    query_t *query = list_remove(querys_ready, 0);
    pthread_mutex_unlock(&mutex_ready);
    return query;
}

worker_t *buscar_worker_libre()
{
    pthread_mutex_lock(&mutex_workers);
    if (!list_is_empty(workers))
    {
        t_list_iterator *iterator = list_iterator_create(workers);
        while (list_iterator_has_next(iterator))
        {
            worker_t *worker = list_iterator_next(iterator);
            if (worker->state == READY)
            {
                pthread_mutex_unlock(&mutex_workers);
                list_iterator_destroy(iterator);
                return worker;
            }
        }
        list_iterator_destroy(iterator);
    }
    pthread_mutex_unlock(&mutex_workers);
    return NULL;
}

void hacer_par_query_worker(query_t *query, worker_t *worker)
{
    pthread_mutex_lock(&mutex_exec);
    pthread_mutex_lock(&mutex_workers);
    list_add(querys_exec, query);
    query->worker = worker;
    query->state = EXEC;
    worker->query = query;
    worker->state = EXEC;
    pthread_mutex_unlock(&mutex_exec);
    pthread_mutex_unlock(&mutex_workers);
}

void solicitar_ejecucion_query(query_t *query, int socket)
{
    paquete_t *paquete = crear_paquete(SOLICITUD_EJECUCION);
    agregar_a_paquete(paquete, query->file, strlen(query->file) + 1);
    agregar_a_paquete(paquete, &query->id, sizeof(int));
    agregar_a_paquete(paquete, &query->pc, sizeof(int));
    enviar_paquete(socket, paquete, logger_master);

    log_info(logger_master, 
        "Se envía la Query %d (%d) al Worker %s", query->id, query->prioridad, query->worker->id);
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
    enviar_paquete(victima->worker->socket, paquete, logger_master);
}
