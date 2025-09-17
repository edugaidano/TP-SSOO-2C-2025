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

    // Iniciar memoria interna
    init_memoria();
    
    // Espera de Query (Paqute: nombre del archivo, query ID, PC)
    paquete_t *paquete_query = recibir_paquete(master_socket, logger_worker);
    if (paquete_query->codigo_operacion != ASIGNACION_WORKER) {
        log_error(logger_worker, "Se recibio un paquete con un op_code distinto a %d", ASIGNACION_WORKER);
        exit(EXIT_FAILURE);
    }

    buffer_t *buffer = obtener_siguiente_item(paquete_query);   
    char *path = string_from_format( "%s/%s", PATH_SCRIPTS, (char*){buffer->stream});
    liverar_buffer(buffer);

    buffer = obtener_siguiente_item(paquete_query);
    char *query_id = string_duplicate((char*){buffer->stream});
    liverar_buffer(buffer);

    buffer = obtener_siguiente_item(paquete_query);
    int pc = *(int*){buffer->stream};
    liverar_buffer(buffer);

    destruir_paquete(paquete_query);

    log_info(logger_worker, "## Query %s: Se recibe la Query. El path de operaciones es: %s", query_id, path);

    t_list* instrucciones = parsear_archivo(path);
    free(path);

    bool fin = false;
    // Lectura de instrucciones
    while (list_size(instrucciones) > pc && !fin) {
        t_instrucion* instruccion = list_get(instrucciones, pc);
        log_info(logger_worker, "## Query %s: FETCH - Program Counter: %d - %s", query_id, pc, instruccion->identificador);

        switch (instruccion->copi) {
            case CREATE:
                interpretar_CREATE(instruccion, query_id, storage_socket);
                break;
            case COMMIT:
                interpretar_COMMIT(instruccion, query_id, storage_socket);
                break;
            case DELETE:
                interpretar_DELETE(instruccion, query_id, storage_socket);
                break;
            case FLUSH:
                interpretar_FLUSH(instruccion, query_id, storage_socket);
                break;
            case TRUNCATE:
                interpretar_TRUNCATE(instruccion, query_id, storage_socket);
                break;
            case TAG:
                interpretar_TAG(instruccion, query_id,storage_socket);
                break;
            case READ:
                interpretar_READ(instruccion, query_id, storage_socket, master_socket);
                break;
            case WRITE:
                interpretar_WRITE(instruccion, query_id, storage_socket);
                break;
            default:        //END
                interpretar_END(instruccion, query_id, master_socket);
                fin = true; // Sale del while y elimina la query
                break;
        }

        pc ++;
    }

    list_destroy_and_destroy_elements(instrucciones, destruir_instrucciones);

    // Liveracion de Recursos
    config_destroy(config_worker);
    log_destroy(logger_worker);
    // list_destroy_and_destroy_elements(tabla_paginas, destruir_paginas); TODO
    free(memoria);

    return 0;
}
