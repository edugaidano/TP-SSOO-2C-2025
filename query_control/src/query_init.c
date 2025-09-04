#include <query_init.h>

void init(char *config_name)
{

    logger_query_control = log_create("log_file.log", "<query_control>", true, LOG_LEVEL_INFO);
    log_info(logger_query_control, "log inicializado correctamente");

    char *config_path = build_config_path(config_name, logger_query_control);

    t_config *config_query_control = config_create(config_path);

    PUERTO_MASTER = config_get_string_value(config_query_control, "PUERTO_MASTER");
    IP_MASTER = config_get_string_value(config_query_control, "IP_MASTER");
}