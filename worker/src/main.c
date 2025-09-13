#include <worker_main.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Uso: %s [archivo_conf] [ID Worker]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Iniciar Logs y Configs
    init(argv[1]);

    // Conexiones con Storage y Master
    int storage_socket = connect_to_server(IP_STORAGE, PUERTO_STORAGE, logger_worker);
    enviar_handshake(HANDSHAKE_WORKER_STORAGE, storage_socket, "Storage", argv[2]);
    verificar_resultado_handshake(storage_socket, "Storage");
    
    int master_socket = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_worker);
    enviar_handshake(HANDSHAKE_WORKER_MASTER, master_socket, "Master", argv[2]);
    verificar_resultado_handshake(master_socket, "Master");

    // Test
    paquete_t *paquete_path = recibir_paquete(master_socket, logger_worker);
    buffer_t *buffer = obtener_siguiente_item(paquete_path);
    char *path = (char*){buffer->stream};

    log_info(logger_worker, "## Query <QUERY_ID>: Se recibe la Query. El path de operaciones es: %s", path);

    config_destroy(config_worker);
    return 0;
}
