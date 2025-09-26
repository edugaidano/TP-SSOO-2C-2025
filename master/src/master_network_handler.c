#include <master_network_handler.h>

void *master_network_handler(void *arg)
{
    int socket_server = *(int *)arg;

    while (1)
    {
        int connection_socket = accept_connection(socket_server, logger_master);
        int *socket_ptr = malloc(sizeof(int));
        *socket_ptr = connection_socket;
        t_list *list = recv_package(connection_socket, logger_master);
        op_code opcode = get_opcode(list);

        switch (opcode)
        {
        case HANDSHAKE_QUERY_MASTER:
        {
            char *archivo = list_get(list, 0);
            char *prioridad = list_get(list, 1);

            query *query = malloc(sizeof(*query));
            query->id = asign_query_id();
            query->pc = 0;
            query->file = archivo;
            query->prioridad = prioridad;
            query->controler_socket = socket_ptr;

            list_add(querys_ready, query);
            enviar_resultado_handshake(connection_socket, (resultado_t){OK}, logger_master);
            // send(connection_socket, "ACK", 4, 0);

            sem_post(&sem_ready);

            log_info(logger_master, "## Se conecta un Query Control para ejecutar la Query <%s> con prioridad <%s>", archivo, prioridad);
            log_info(logger_master, "## Id asignado: <%d>. Nivel multiprocesamiento <CANTIDAD>", query->id);
            break;
        }
        case HANDSHAKE_WORKER_MASTER:
        {
            char *id = list_get(list, 0);

            worker *worker = malloc(sizeof(*worker));
            worker->id = id;
            worker->fd = connection_socket;

            list_add(workers, worker);
            // send(connection_socket, "ACK", 4, 0);
            enviar_resultado_handshake(connection_socket, (resultado_t){OK}, logger_master);

            sem_post(&sem_workers);

            log_info(logger_master, "## Se conecta el Worker <%s> - Cantidad total de Workers: <%d>", id, list_size(workers));
            break;
        }
        default:
            log_warning(logger_master, "Se recibio un paquete no identificado correctamente");
            enviar_resultado_handshake(connection_socket, (resultado_t){ERROR}, logger_master);
            break;;
        }
    }
}