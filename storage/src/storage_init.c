#include "storage_init.h"

void init(char *config_name)
{

    logger_storage = log_create("log_file.log", "logg_storage", true, LOG_LEVEL_INFO);
    log_info(logger_storage, "log inicializado correctamente");

    char *config_path = build_config_path(config_name, logger_storage);
    t_config *config_storage = config_create(config_path);

    PUERTO_ESCUCHA = config_get_string_value(config_storage, "PUERTO_ESCUCHA");
    FRESH_START = config_get_int_value(config_storage, "FRESH_START");
    RETARDO_OPERACION = config_get_int_value(config_storage, "RETARDO_OPERACION");
    RETARDO_ACCESO_BLOQUE = config_get_int_value(config_storage, "RETARDO_ACCESO_BLOQUE");
    FS_SIZE = config_get_int_value(config_storage, "FS_SIZE");
    BLOCK_SIZE = config_get_int_value(config_storage, "BLOCK_SIZE");

    CANT_WORKERS = 0;

    // config_destroy(config_storage); (?) TODO: revisar donde ponerlo
}