#include <utils/rutas.h>

char *build_path(char *relative_path, t_log *logger)
{
    char *current_working_directory = getcwd(NULL, 0);
    char *virtual_path = string_from_format("%s/../%s", current_working_directory, relative_path);
    char *path = realpath(virtual_path, NULL);

    if (path == NULL)
    {
        log_error(logger, "ERROR, la ruta %s no existe", virtual_path);
    };

    free(virtual_path);
    free(current_working_directory);
    return path;
}

char *build_config_path(char *file_name, t_log *logger)
{
    log_info(logger, "nombre del config: %s", file_name);
    char *relative_config_path = string_from_format("config/%s.conf", file_name);
    log_info(logger, "ruta relativa: %s", relative_config_path);
    char *config_path = build_path(relative_config_path, logger);
    log_info(logger, "ruta absoluta: %s", config_path);

    free(relative_config_path);
    return config_path;
}