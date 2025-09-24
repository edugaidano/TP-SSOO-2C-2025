#ifndef _UTILS_PATHING_H_
#define _UTILS_PATHING_H_

#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
/**
 * @brief crea una ruta absoluta desde el directorio de trabajo (el del tp)
 * @param relative_path la ruta al archivo desde el directorio
 * @param logger un logger de la libreria commons, necesario para loguear errores
 * @return la ruta absoluta al archivo
 */
char *build_path(char *relative_path, t_log *logger);

/**
 * @brief construye una ruta al archivo de configuraciones en el directorio de cada componente.
 * Se asume que hay una carpeta llamada "config" en cada componente.
 * Se asume que los archivos vienen en formato .conf.
 * Se le pone automaticamente la extension a la ruta.
 *
 * @param file_name el nombre del archivo de configuracion sin extension
 * @param logger un logger de la libreria commons, necesario para loguear errores
 *
 * @return la ruta absoluta al archivo de configuracion
 */
char *build_config_path(char *file_name, t_log *logger);

#endif
