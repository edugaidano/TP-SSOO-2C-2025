#ifndef _UTILS_NETWORKING_H_
#define _UTILS_NETWORKING_H_
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <commons/log.h>
#include <commons/string.h>

#include <utils/paquetes.h>
#include <utils/op_codes.h>

typedef enum resultado_t
{
    ERROR = -1,
    OK
} resultado_t;

/**
 * @brief crea un servidor y levanta un socket
 * @param PORT puerto en el que levanto el server
 * @param log t_log de so-commons de catedra
 * @returns el socket del servidor
 */
int create_server(char *PORT, t_log *log);
/**
 * @brief acepta una conexion y devuelve el socket de la misma
 * @param server_socket socket en el que quiero aceptar la conexion
 * @param log log de so-commons de la catedra
 * @returns el socket de la conexion si funciona, -1 si falla
 */
int accept_connection(int server_socket, t_log *log);
/**
 * @brief conecta un cliente a un servidor existente
 * @param IP ip a la que me quiero conectar
 * @param PORT puerto al que me quiero conectar
 * @param log log de so-commons de la catedra
 * @returns el socket de la conexion si se conecta, -1 si no lo hace
 */
int connect_to_server(char *IP, char *PORT, t_log *log);

#endif
