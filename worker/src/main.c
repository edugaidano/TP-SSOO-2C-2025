#include <worker_main.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Uso: %s [archivo_conf] [ID Worker]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init(argv[1]);

    int master_socket = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_worker);
    // Enviar handshake al Master
    paquete_t *handshake = crear_paquete(HANDSHAKE_WORKER_MASTER);
    agregar_a_paquete(handshake, argv[1], sizeof(argv[1]));
    enviar_paquete(master_socket, handshake);

    char ack[4];
    recv(master_socket, ack, 4, MSG_WAITALL);
    log_info(logger_worker, "## handshake con master realizado");

    return 0;
}
