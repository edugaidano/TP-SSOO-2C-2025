#include "master_main.h"

int main(int argc, char *argv[])
{
    init(argv[1]);

    log_info(logger_master, "el include esta bien hecho");

    int server_socket = create_server(PUERTO_ESCUCHA, logger_master);

    int conection_socket = accept_connection(server_socket, logger_master);

    return 0;
}