#include <query_main.h>

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Uso: %s [archivo_conf] [archivo_query] [prioridad]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init(argv[1]);

    int master_socket = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_query_control);
    log_info(logger_query_control, "## Conexión al Master exitosa. IP: %s, Puerto: %s", IP_MASTER, PUERTO_MASTER);

    
    log_info(logger_query_control, "## Solicitud de ejecución de Query: %s, prioridad: %s", argv[2], argv[3]);
    // Enviar handshake al Master
    paquete_t* paquete_handshake = crear_paquete(HANDSHAKE_QUERY_MASTER);
    agregar_a_paquete(paquete_handshake, argv[2], string_length(argv[2]) + 1);
    agregar_a_paquete(paquete_handshake, argv[3], string_length(argv[3]) + 1);
    void* handshake = serializar_paquete(paquete_handshake);

    if (send(master_socket, handshake, espacio_paquete_serializado(paquete_handshake), 0) == -1) {
        log_error(logger_query_control, "No se pudo enviar el handshake al Master");
        abort();
    }

    // Verificar handshake
    if (resultado_handshake(master_socket, logger_query_control) == ERROR) {
        log_error(logger_query_control, "No se verificó el handshake con el Master");
        exit(EXIT_FAILURE);
    }

    //TODO
    while (1) {
    }

    log_info(logger_query_control, "## Query Finalizada - <MOTIVO>");
    return 0;
}