#include "process_handler.h"

void *process_handler(void *arg)
{
    if (strcmp(ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0)
    {
        pthread_t actualizador_de_prioridad;
        pthread_create(&actualizador_de_prioridad, NULL, &actualizador, NULL);
        pthread_detach(actualizador_de_prioridad);
    }

    if (strcmp(ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0)
    {
        pthread_t hilo_desalojador;
        pthread_create(&hilo_desalojador, NULL, &desalojador, NULL);
    }

    while (1)
    {
        sem_wait(&sem_ready);
        sem_wait(&sem_workers);

        query_t *query = obtener_query();
        worker_t *worker = buscar_worker_libre();

        hacer_par_query_worker(query, worker);
        solicitar_ejecucion_query(query, worker->fd);

        query_t **query_ptr = malloc(sizeof(query_ptr));
        *query_ptr = query;
        pthread_t response_thread;
        pthread_create(&response_thread, NULL, &esperar_respuesta, query_ptr);
        pthread_detach(response_thread);
    }
    return 0;
}

void *esperar_respuesta(void *arg)
{
    query_t *query = *(query_t **)arg;
    free(arg);
    int socket = query->worker->fd;

    t_list *list = recv_package(socket, logger_master);
    op_code opcode = get_opcode(list);

    switch (opcode)
    {
    case QUERY_FINALIZADA:
    {
        finalizar_query(query);
        liberar_worker(query->worker);
        break;
    }
    case NOTIF_READ:
    {
        char *contenido = list_get(list, 0);
        char *file = list_get(list, 1);
        char *tag = list_get(list, 2);
        notificar_read(query, file, tag, contenido);
        list_destroy_and_destroy_elements(list, &element_destroyer);
        esperar_respuesta(&query);
        break;
    }
    case QUERY_DESALOJADA:
    {
        int pc = atoi(list_get(list, 0));
        worker_t *worker = query->worker;
        query_t *query_desalojada = query;

        liberar_worker(query->worker);
        liberar_query(query, pc);

        query_t *query = obtener_query();
        hacer_par_query_worker(query, worker);

        log_info(logger_master, "## Se desaloja la Query <%d> (<%d>) y comienza a ejecutar la Query <%d> (<%d>) en el Worker <%s>", query_desalojada->id, query_desalojada->prioridad, query->id, query->prioridad, worker->id);
        solicitar_ejecucion_query(query, query->worker->fd);
        break;
    }
    default:;
    }

    list_destroy_and_destroy_elements(list, &element_destroyer);
    return 0;
}

void *actualizador()
{
    usleep(TIEMPO_AGING * 1000);
    pthread_mutex_lock(&mutex_ready);

    t_list_iterator *iterator = list_iterator_create(querys_ready);

    while (list_iterator_has_next(iterator))
    {
        query_t *query = list_iterator_next(iterator);
        if (query->prioridad > 0)
        {
            query->prioridad--;
            log_info(logger_master, "##<%d> Cambio de prioridad: <%d> - <%d>", query->id, query->prioridad++, query->prioridad);
        }
    }

    pthread_mutex_unlock(&mutex_ready);
    list_iterator_destroy(iterator);
    return 0;
}

void *desalojador()
{
    while (1)
    {
        pthread_mutex_lock(&mutex_exec);

        while (!((list_size(querys_exec) != 0) && list_size(querys_exec) == list_size(workers)))
        {
            pthread_cond_wait(&all_workers_busy, &mutex_exec);
        }

        query_t *victima = buscar_victima();
        solicitar_desalojo(victima);

        pthread_mutex_unlock(&mutex_exec);
    }
}
