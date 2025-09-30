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
            query->controler_socket = connection_socket;
            query->worker = NULL;
            query->is_exec = false;

            list_add(querys_ready, query);
            send(connection_socket, "ACK", 4, 0);

            sem_post(&sem_ready);

            log_info(logger_master, "## Se conecta un Query Control para ejecutar la Query <%s> con prioridad <%d>", archivo, prioridad);
            log_info(logger_master, "## Id asignado: <%d>. Nivel multiprocesamiento <%d>", query->id, list_size(workers));

            pthread_t query_keepalive;
            pthread_create(&query_keepalive, NULL, &keepalive, query);
            break;
        }
        case HANDSHAKE_WORKER_MASTER:
        {
            char *id = string_duplicate(list_get(list, 0));

            worker_t *worker = malloc(sizeof(*worker));
            worker->is_free = true;
            worker->id = id;
            worker->fd = connection_socket;

            list_add(workers, worker);
            send(connection_socket, "ACK", 4, 0);

            sem_post(&sem_workers);

            log_info(logger_master, "## Se conecta el Worker <%s> - Cantidad total de Workers: <%d>", id, list_size(workers));
            break;
        }
        default:;
        }
        list_destroy_and_destroy_elements(list, free);
    }
}

void *keepalive(void *arg)
{
    query_t *query = arg;
    int buffer[1];
    while (1)
    {
        int res = recv(query->controler_socket, buffer, sizeof(int), MSG_PEEK);
        if (res == 0)
        {
            log_info(logger_master, "## Se desconecta un Query Control. Se finaliza la Query <%d> con prioridad <%d>. Nivel multiprocesamiento <%d>", query->id, query->prioridad, list_size(workers));
            if (query->is_exec)
            {
                log_info(logger_master, "## Se desaloja la Query <%d> del Worker <%s>", query->id, query->worker->id);
                liberar_worker(query->worker);
            }
            destruir_query(query);
            break;
        }
    }
    return 0;
}