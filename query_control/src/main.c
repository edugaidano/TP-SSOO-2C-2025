#include "query_main.h"

// Private Function //

void recibir_mensaje();
void log_storage_error(rta_storage c_error);

// Main function //

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Uso: %s [archivo_config] [archivo_query] [prioridad]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init(argv[1], argv[2]);
    atexit(notificar_y_liberar);
    signal(SIGINT, exit);

    socket_master = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_query_control);
    log_info(logger_query_control, "## Conexión al Master exitosa. IP: %s, Puerto: %s", IP_MASTER, PUERTO_MASTER);

    paquete_t *paquete_handshake = crear_paquete(HANDSHAKE_QUERY_MASTER);
    agregar_a_paquete(paquete_handshake, argv[2], string_length(argv[2]) + 1);
    agregar_a_paquete(paquete_handshake, argv[3], string_length(argv[3]) + 1);
    enviar_paquete(socket_master, paquete_handshake, logger_query_control);
    log_info(logger_query_control, "## Solicitud de ejecución de Query: %s, prioridad: %s", argv[2], argv[3]);

    char ack[4];
    if (recv(socket_master, ack, 4, MSG_WAITALL) <= 0)
    {
        log_error(logger_query_control, "No se pudo realizar correctamente el handshake con el Master");
        exit(EXIT_FAILURE);
    }
    log_info(logger_query_control, "## handshake con master realizado");

    recibir_mensaje();

    return 0;
}

// Private Function //

void recibir_mensaje()
{
    while (true) 
    { 
        t_list *package = recv_package(socket_master, logger_query_control);
        op_code opcode = get_opcode(package);

        // Casos particulares
        if (opcode != NOTIF_QUERY_CONTROL)
        {
            log_error(logger_query_control, "Se recibio un paquete desconocido");
            list_destroy_and_destroy_elements(package, free);
            exit(EXIT_FAILURE);
        }
        
        // Manejo de notificaciones del Master
        notif_query_control notif = *(notif_query_control*) list_get(package, 0);
        switch (notif)
        {
            case NOTIF_FINAL:
            {
                razon_fin razon = *(razon_fin*) list_get(package, 1);
                switch (razon)
                {
                    case FINALIZACION_CORRECTA:
                        log_info(logger_query_control, "## Query finalizada - La ejecución finalizó correctamente");
                        break;
                    case ERR_DESC_WORKER:
                        log_error(logger_query_control, "## Query finalizada - La ejecución finalizó por desconexión del worker");
                        break;
                    case ERR_STORAGE:
                        rta_storage c_error = *(rta_storage*)list_get(package, 2);
                        log_storage_error(c_error);
                        break;
                    default:
                        log_warning(logger_query_control, "## Query finalizada - motivo desconocido o no definido correctamente");
                        break;
                }
                list_destroy_and_destroy_elements(package, free);
                return;
            }
            case NOTIF_READ:
            {
                char *file = (char *)list_get(package, 1);
                char *tag = (char *)list_get(package, 2);
                char *contenido = (char *)list_get(package, 3);
                log_info(logger_query_control, "## Lectura realizada: Archivo %s:%s, contenido: %s", file, tag, contenido);
                break;
            }
            default:
                log_error(logger_query_control, "Se recibio una notificacion desconocida");
                list_destroy_and_destroy_elements(package, free);
                exit(EXIT_FAILURE);
        }
        list_destroy_and_destroy_elements(package, free);
    }
}

void log_storage_error(rta_storage c_error) {
    switch (c_error)
    {
    case ERR_FUERA_LIMITE:
        log_error(logger_query_control, "## Query finalizada - Lectura o escritura fuera de limite");
        break;
    case ERR_WRITE_COMMITED:
        log_error(logger_query_control, "## Query finalizada - Escritura no permitida");
        break;
    case ERR_ESP_INSUFICIENTE:
        log_error(logger_query_control, "## Query finalizada - Espacio Insuficiente");
        break;
    case ERR_PREEXISTENCIA:
        log_error(logger_query_control, "## Query finalizada - File / Tag preexistente");
        break;
    case ERR_INEXISTENCIA:
        log_error(logger_query_control, "## Query finalizada - File / Tag inexistente");
        break;   
    default:
        log_error(logger_query_control, "## Query finalizada - Error durante la ejecucion no definido");
        break;
    }
}