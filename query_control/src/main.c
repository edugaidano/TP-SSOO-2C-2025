#include <query_main.h>

void recibir_mensaje(int socket)
{
    while (true) 
    { 
        t_list *package = recv_package(socket, logger_query_control);
        op_code opcode = get_opcode(package);

        // Casos particulares
        if (opcode != NOTIF_QUERY_CONTROL && opcode != DESCONEXION)
        {
            log_error(logger_query_control, "Se recibio un paquete desconocido");
            exit(EXIT_FAILURE);
        }
        
        if (opcode == DESCONEXION)
        {
            log_error(logger_query_control, "El master se desconecto");
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
                log_info(logger_query_control, "## Query finalizada - <La ejecución finalizó correctamente>");
                break;
            case ERR_DESC_WORKER:
                log_info(logger_query_control, "## Query finalizada - <La ejecución finalizó por desconexión del worker>");
                break;
            default:
                log_info(logger_query_control, "Query finalizada - motivo desconocido o no definido correctamente");
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
            log_info(logger_query_control, "## Lectura realizada: Archivo <%s:%s>, contenido: <%s>", file, tag, contenido);
            break;
        }
        default:
            log_error(logger_query_control, "Se recibio una notificacion desconocida");
            exit(EXIT_FAILURE);
            break;
        }
        
        list_destroy_and_destroy_elements(package, free);
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
    enviar_paquete(socket_master, paquete_handshake, logger_query_control);
    log_info(logger_query_control, "## Solicitud de ejecución de Query: %s, prioridad: %s", argv[2], argv[3]);

    char ack[4];
    recv(socket_master, ack, 4, MSG_WAITALL);
    log_info(logger_query_control, "## handshake con master realizado");

    recibir_mensaje(socket_master);
    //TODO: liberar recursos y atexit()
    paquete_t *paquete_desconexion = crear_paquete(DESCONEXION);
    enviar_paquete(socket_master, paquete_desconexion, logger_query_control);
    close(socket); // TODO: agrgar a liberar recursos
    return 0;
}