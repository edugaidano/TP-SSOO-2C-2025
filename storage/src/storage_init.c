#include "storage_init.h"

void init(char *config_name)
{

    logger_storage = log_create("log_file.log", "logg_master", true, LOG_LEVEL_INFO);
    log_info(logger_storage, "log inicializado correctamente");

    t_config *config_storage;

    build_config(&config_storage,config_name, logger_storage);

    PUERTO_ESCUCHA = config_get_string_value(config_storage, "PUERTO_ESCUCHA");
    FRESH_START = config_get_int_value(config_storage, "FRESH_START");
    RETARDO_OPERACION = config_get_int_value(config_storage, "RETARDO_OPERACION");
    RETARDO_ACCESO_BLOQUE = config_get_int_value(config_storage, "RETARDO_ACCESO_BLOQUE");

    // config_destroy(config_storage); (?) TODO: revisar donde ponerlo
}