#include "utils/pathing.h"

char *build_path(char *relative_path)
{
    char *current_working_directory = getcwd(NULL, 0);
    char *virtual_path = string_from_format("%s/../%s", current_working_directory, relative_path);
    char *path = realpath(virtual_path, NULL);

    if (path == NULL)
    {
        printf("ERROR, la ruta %s no existe\n", virtual_path);
        exit(EXIT_FAILURE);
    }

    free(virtual_path);
    free(current_working_directory);

    return path;
}

char *build_config_path(char *file_name)
{
    printf("nombre del config: %s\n", file_name);
    char *relative_config_path = string_from_format("config/%s.config", file_name);
    printf("ruta relativa: %s\n", relative_config_path);
    char *config_path = build_path(relative_config_path);
    printf("ruta absoluta: %s\n", config_path);

    free(relative_config_path);
    return config_path;
}