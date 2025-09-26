#include <query_main.h>

void *element_destroyer(void *arg)
{
    free(arg);
    return 0;
}

void recibir_mensaje(int socket)
{
    t_list *list = recv_package(socket, logger_query_control);
    op_code opcode = get_opcode(list);

    switch (opcode)
    {
    case NOTIF_QUERY_CONTROL:
        notif_query_control notif = *(notif_query_control *)list_get(list, 0);
        switch (notif)
        {
        case FINALIZACION:
            log_info(logger_query_control, "## Query finalizada - <La ejecución finalizó correctamente>");
            list_destroy_and_destroy_elements(list, &element_destroyer);
            break;
        case READ:
            char *file = (char *)list_get(list, 1);
            char *tag = (char *)list_get(list, 2);
            char *contenido = (char *)list_get(list, 3);
            log_info(logger_query_control, "## Lectura realizada: Archivo <%s:%s>, contenido: <%s>", file, tag, contenido);
            list_destroy_and_destroy_elements(list, &element_destroyer);
            recibir_mensaje(socket);
            break;
        default:;
        }
    default:;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Uso: %s [archivo_conf] [archivo_query] [prioridad]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init(argv[1]);

    int socket_master = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_query_control);
    log_info(logger_query_control, "## Conexión al Master exitosa. IP: %s, Puerto: %s", IP_MASTER, PUERTO_MASTER);

    paquete_t *paquete_handshake = crear_paquete(HANDSHAKE_QUERY_MASTER);
    agregar_a_paquete(paquete_handshake, argv[2], string_length(argv[2]) + 1);
    agregar_a_paquete(paquete_handshake, argv[3], string_length(argv[3]) + 1);
    enviar_paquete(socket_master, paquete_handshake);
    log_info(logger_query_control, "## Solicitud de ejecución de Query: %s, prioridad: %s", argv[2], argv[3]);

    char ack[4];
    recv(socket_master, ack, 4, MSG_WAITALL);
    log_info(logger_query_control, "## handshake con master realizado");

    recibir_mensaje(socket_master);
    return 0;
}