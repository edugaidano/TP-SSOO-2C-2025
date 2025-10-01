#include <master_network_handler.h>

void *master_network_handler(void *arg)
{
    int socket_server = *(int *)arg;

    while (1)
    {
        int connection_socket = accept_connection(socket_server, logger_master);
        t_list *list = recv_package(connection_socket, logger_master);
        op_code opcode = get_opcode(list);
        //
        switch (opcode)
        {
        case HANDSHAKE_QUERY_MASTER:
        {
            char *archivo = string_duplicate(list_get(list, 0));
            int prioridad = atoi(list_get(list, 1));

            query_t *query = malloc(sizeof(*query));
            query->id = asign_query_id();
            query->pc = 0;
            query->file = archivo;
            query->prioridad = prioridad;
            query->socket = connection_socket;
            query->state = READY;
            query->worker = NULL;

            list_add(querys_ready, query);
            send(connection_socket, "ACK", 4, 0);

            sem_post(&sem_ready);

            log_info(logger_master, "## Se conecta un Query Control para ejecutar la Query <%s> con prioridad <%d>", archivo, prioridad);
            log_info(logger_master, "## Id asignado: <%d>. Nivel multiprocesamiento <%d>", query->id, list_size(workers));

            pthread_t query_handler_thread;
            pthread_create(&query_handler_thread, NULL, &query_handler, query);
            pthread_detach(query_handler_thread);
            break;
        }
        case HANDSHAKE_WORKER_MASTER:
        {
            char *id = string_duplicate(list_get(list, 0));

            worker_t *worker = malloc(sizeof(*worker));
            worker->id = id;
            worker->socket = connection_socket;
            worker->state = READY;
            worker->query = NULL;

            list_add(workers, worker);
            send(connection_socket, "ACK", 4, 0);

            sem_post(&sem_workers);

            log_info(logger_master, "## Se conecta el Worker <%s> - Cantidad total de Workers: <%d>", id, list_size(workers));

            pthread_t worker_handler_thread;
            pthread_create(&worker_handler_thread, NULL, &worker_handler, worker);
            break;
        }
        default:;
        }
        list_destroy_and_destroy_elements(list, free);
    }
}

void *query_handler(void *arg)
{
    query_t *query = arg;
    t_list *list = recv_package(query->socket, logger_master);
    int opcode = get_opcode(list);
    if (opcode != DESCONEXION)
    {
        log_error(logger_master, "opcode no identificado en master");
    }
    log_info(logger_master, "## Se desconecta un Query Control. Se finaliza la Query <%d> con prioridad <%d>. Nivel multiprocesamiento <%d>", query->id, query->prioridad, list_size(workers));

    switch (query->state)
    {
    case READY:
        log_info(logger_master, "la query no estaba en ninguna cpu, eliminando query, id: %d", query->id);
        log_info(logger_master, "la query fue eliminada, querys en ready: %d", list_size(querys_ready) - 1);
        destruir_query(query);
        break;
    case EXEC:
        log_info(logger_master, "la query: <%d> estaba en exec, desalojando cpu: %s", query->id, query->worker->id);
        liberar_worker(query->worker);
        log_info(logger_master, "la query fue eliminada, querys en exec: %d", list_size(querys_exec) - 1);
        destruir_query(query);
        break;
    case FINISHED:
        log_info(logger_master, "la query <%d> notifica de su finalizacion, liberando recursos", query->id);
        destruir_query(query);
    default:
        break;
    }

    return NULL;
}

void *worker_handler(void *arg)
{
    worker_t *worker = arg;
    query_t *query = worker->query;
    t_list *list = recv_package(worker->socket, logger_master);
    int opcode = get_opcode(list);

    switch (opcode)
    {
    case DESCONEXION:
        if (worker->state == EXEC)
        {
            finalizar_query(query, ERR_DESC_WORKER);
            destruir_worker(worker);
        }
        else
        {
            log_info(logger_master, "## Se desconecta el Worker <%s> - No habia una query en ejecucion - Cantidad total de Workers: <%d> ", worker->id, list_size(workers) - 1);
            destruir_worker(worker);
        }
    default:
        break;
    }
    return NULL;
}
