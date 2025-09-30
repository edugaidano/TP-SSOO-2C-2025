#include <utils/networking.h>

int create_server(char *PORT, t_log *log)
{

    int socket_server;
    struct addrinfo hints, *server_info, *pointer;
    int return_value;
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    return_value = getaddrinfo(NULL, PORT, &hints, &server_info);
    if (return_value != 0)
    {
        log_error(log, "getaddrinfo %s", gai_strerror(return_value));
        exit(-1);
    }

    for (pointer = server_info; pointer != NULL; pointer = pointer->ai_next)
    {
        socket_server = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);
        if (socket_server == -1)
        {
            log_warning(log, "server: socket %s, intentando nuevamente", strerror(errno));
            continue;
        }

        return_value = setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (return_value == -1)
        {
            log_error(log, "setsockopt %s", strerror(errno));
            exit(-1);
        }

        return_value = bind(socket_server, pointer->ai_addr, pointer->ai_addrlen);
        if (return_value == -1)
        {
            close(socket_server);
            log_error(log, "server: bind %s, intentando nuevamente", strerror(errno));
            continue;
        }
        break;
    }
    freeaddrinfo(server_info);

    if (pointer == NULL)
    {
        log_error(log, "server: no se pudo levantar el servidor");
        exit(-1);
    }

    return_value = listen(socket_server, 10);
    if (return_value == -1)
    {
        close(socket_server);
        log_error(log, "server: listen %s", strerror(errno));
        exit(-1);
    }

    log_info(log, "Servidor levantado, escuchando conexiones");
    return socket_server;
}

int accept_connection(int server_socket, t_log *log)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;

    addr_size = sizeof(their_addr);

    log_info(log, "server: esperando conexion...");
    int new_socket = accept(server_socket, &their_addr, &addr_size);
    if (new_socket == -1)
    {
        log_error(log, "server: accept %s", strerror(errno));
        return -1;
    }

    log_info(log, "server: modulo conectado exitosamente");
    return new_socket;
}

int connect_to_server(char *IP, char *PORT, t_log *log)
{
    int socket_client;
    struct addrinfo hints, *server_info, *pointer;
    int return_value;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((return_value = getaddrinfo(IP, PORT, &hints, &server_info)) != 0)
    {
        log_error(log, "getaddrinfo %s", gai_strerror(return_value));
        exit(-1);
    }

    for (pointer = server_info; pointer != NULL; pointer = pointer->ai_next)
    {
        if ((socket_client = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol)) == -1)
        {
            log_warning(log, "client: socket %s, intentando nuevamente", strerror(errno));
            continue;
        }

        if ((return_value = connect(socket_client, pointer->ai_addr, pointer->ai_addrlen)) == -1)
        {
            log_warning(log, "client: connect %s, intentando nuevamente", strerror(errno));
            close(socket_client);
            continue;
        }

        break;
    }

    if (pointer == NULL)
    {
        log_error(log, "client: no se pudo conectar el cliente");
        freeaddrinfo(server_info);
        return -1;
    }

    log_info(log, "el cliente se conecto exitosamente IP:%s, PORT:%s", IP, PORT);
    freeaddrinfo(server_info);

    return socket_client;
}

void enviar_resultado_handshake(int fd_conexion, resultado_t resultado, t_log *logger)
{
    int size = sizeof(op_code) + sizeof(resultado_t);
    void *stream = malloc(size);

    memcpy(stream, &(op_code){RESULTADO_HANDSHAKE}, sizeof(op_code));
    memcpy(stream + sizeof(op_code), &resultado, sizeof(resultado_t));

    if (send(fd_conexion, stream, size, 0) <= 0)
    {
        log_error(logger, "Error en send (fd %d)", fd_conexion);
        exit(EXIT_FAILURE);
    }

    free(stream);
}

resultado_t resultado_handshake(int fd_conexion, t_log *logger)
{
    op_code codigo_operacion;
    resultado_t resultado;
    if (recv(fd_conexion, &codigo_operacion, sizeof(op_code), MSG_WAITALL) <= 0)
    {
        log_error(logger, "Error en recv (fd %d)", fd_conexion);
        exit(EXIT_FAILURE);
    }

    if (codigo_operacion != RESULTADO_HANDSHAKE)
    {
        log_error(logger, "No se pudo recibir el resultado del handshake");
        exit(EXIT_FAILURE);
    }

    if (recv(fd_conexion, &resultado, sizeof(resultado_t), MSG_WAITALL) <= 0)
    {
        log_error(logger, "Error en recv (fd %d)", fd_conexion);
        exit(EXIT_FAILURE);
    }
    return resultado;
}
