#include <query_main.h>

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Uso: %s [archivo_conf] [archivo_query] [prioridad]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init(argv[1]);

    int master_socket = connect_to_server(IP_MASTER, PUERTO_MASTER, logger_query_control);

    return 0;
}
