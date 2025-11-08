#include "process_handler.h"

void *process_handler(void *arg)
{
    
    if (TIEMPO_AGING != 0 && string_equals_ignore_case(ALGORITMO_PLANIFICACION, "PRIORIDADES"))
    {
        pthread_t actualizador_de_prioridad;
        pthread_create(&actualizador_de_prioridad, NULL, &actualizador, NULL);
        pthread_detach(actualizador_de_prioridad);
    }
    /*
    if (string_equals_ignore_case(ALGORITMO_PLANIFICACION, "PRIORIDADES"))
    {
        pthread_t hilo_desalojador;
        pthread_create(&hilo_desalojador, NULL, &desalojador, NULL);
    }
    */
    
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

void *esperar_respuesta(void *arg)
{
    query_t *query = *(query_t **)arg;
    free(arg);
    int socket = query->worker->socket;

    t_list *list = recv_package(socket, logger_master);
    op_code opcode = get_opcode(list);

    switch (opcode)
    {
    case QUERY_FINALIZADA:
    {
        finalizar_query(query, FINALIZACION_CORRECTA);
        liberar_worker(query->worker);
        list_destroy_and_destroy_elements(list, free);
        break;
    }
    case NOTIF_READ:
    {
        char *contenido = list_get(list, 0);
        char *file = list_get(list, 1);
        char *tag = list_get(list, 2);
        notificar_read(query, file, tag, contenido);
        list_destroy_and_destroy_elements(list, free);
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
        solicitar_ejecucion_query(query, query->worker->socket);
        list_destroy_and_destroy_elements(list, free);
        break;
    }
    default:;
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
                log_info(logger_master, "##<%d> Cambio de prioridad: <%d> - <%d>", query->id, (query->prioridad + 1), query->prioridad);
            }
        }

        pthread_mutex_unlock(&mutex_ready);
        list_iterator_destroy(iterator);
    }
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
