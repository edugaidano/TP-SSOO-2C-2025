#include <query_main.h>

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
    enviar_paquete(socket_master, paquete_handshake, logger_query_control, "Master");
    log_info(logger_query_control, "## Solicitud de ejecución de Query: %s, prioridad: %s", argv[2], argv[3]);

    char ack[4];
    recv(socket_master, ack, 4, MSG_WAITALL);
    log_info(logger_query_control, "## handshake con master realizado");

    recv(socket_master, ack, 4, MSG_WAITALL);
    log_info(logger_query_control, "## Query Finalizada - <MOTIVO>");

    return 0;
}