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

    // Espera de Query (Paqute: nombre del archivo, query ID, PC)
    paquete_t *paquete_query = recibir_paquete(master_socket, logger_worker);
    if (paquete_query->codigo_operacion != ASIGNACION_QUERY_MASTER_WORKER) {
        log_error(logger_worker, "Se recibio un paquete con un op_code distinto a %d", ASIGNACION_QUERY_MASTER_WORKER);
        exit(EXIT_FAILURE);
    }

    buffer_t *buffer = obtener_siguiente_item(paquete_query);   
    char *path = string_from_format( "%s/%s", PATH_SCRIPTS, (char*){buffer->stream});
    liverar_buffer(buffer);

    buffer = obtener_siguiente_item(paquete_query);
    char *query_id = string_duplicate((char*){buffer->stream});
    liverar_buffer(buffer);

    buffer = obtener_siguiente_item(paquete_query);
    int pc = atoi((char*){buffer->stream});
    liverar_buffer(buffer);

    destruir_paquete(paquete_query);

    log_info(logger_worker, "## Query %s: Se recibe la Query. El path de operaciones es: %s", query_id, path);

    //TODO: abrir archivo y leer desde el path

    // Liveracion de Recursos
    config_destroy(config_worker);
    log_destroy(logger_worker);
    free(IP_MASTER);
    free(IP_STORAGE);
    free(PUERTO_MASTER);
    free(PUERTO_STORAGE);
    free(PATH_SCRIPTS);

    return 0;
}
