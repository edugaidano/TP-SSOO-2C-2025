#include <worker_main.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Uso: %s [archivo_conf] [ID Worker]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init(argv[1]);

    int master_socket = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_worker);

    // TODO: Enviar handshake al Storage
    
    // Enviar handshake al Master
    paquete_t *paquete_handshake = crear_paquete(HANDSHAKE_WORKER_MASTER);
    agregar_a_paquete(paquete_handshake, argv[2], string_length(argv[2]) + 1);
    void* handshake = serializar_paquete(paquete_handshake);

    if (send(master_socket, handshake, espacio_paquete_serializado(paquete_handshake), 0) == -1) {
        log_error(logger_worker, "No se pudo enviar el handshake al Master");
        abort();
    }

    // Verificar handshake
    if (resultado_handshake(master_socket, logger_worker) == ERROR) {
        log_error(logger_worker, "No se verificó el handshake con el Master");
        exit(EXIT_FAILURE);
    }

    // Test
    paquete_t *paquete_path = recibir_paquete(master_socket, logger_worker);
    buffer_t *buffer = obtener_siguiente_item(paquete_path);
    char *path = (char*){buffer->stream};

    log_info(logger_worker, "## Query <QUERY_ID>: Se recibe la Query. El path de operaciones es: %s", path);

    return 0;
}
