#include <storage_worker_handler.h>

void *storage_worker_handler(void *arg)
{
    int socket = *(int *)arg;
    free(arg);

    while (1)
    {
        t_list *list = recv_package(socket, logger_storage);
        op_code opcode = get_opcode(list);
        char *id_worker;

        switch (opcode)
        {
        case HANDSHAKE_WORKER_STORAGE:
        {
            id_worker = string_duplicate(list_get(list, 0));
            CANT_WORKERS++;
            send(socket, &BLOCK_SIZE, sizeof(int), 0);

            log_info(logger_storage, "##Se conecta el Worker <%s> - Cantidad de Workers: <%d>", id_worker, CANT_WORKERS);
            break;
        }
        case DESCONEXION:
        {
            CANT_WORKERS--;
            log_info(logger_storage, "##Se desconecta el Worker <%s> - Cantidad de Workers: <%d>", id_worker, CANT_WORKERS);
            close(socket);
            return NULL;
            break;
        }
        default:
            return NULL;
        }
    }

    return NULL;
}