#include "parser.h"

char *instrucciones_l[] = {
    "CREATE",
    "TRUNCATE",
    "WRITE",
    "FLUSH",
    "COMMIT",
    "READ",
    "TAG",
    "DELETE",
    "END"
};

t_list* parsear_archivo(char* path_archivo_query) {
    t_list* instrucciones = list_create();
    FILE* archivo = fopen(path_archivo_query, "r");
    fseek(archivo, 0, SEEK_END);
    long size = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);
    char* contenido = malloc(size + 1);
    contenido[size] = '\0';
    fread(contenido, sizeof(char), size, archivo);
    fclose(archivo);

    char** lineas = string_split(contenido, "\n");
    while (!string_array_is_empty(lineas)) {
        char* linea = string_array_pop(lineas);
        t_instrucion* instruccion = malloc(sizeof(t_instrucion));
        char** tokens = string_split(linea, " ");
        free(linea);
        instruccion->identificador = string_duplicate(tokens[0]);
        instruccion->datos = string_array_new();

        int i = 0;
        while (!string_equals_ignore_case(instruccion->identificador, instrucciones_l[i])){
            i++;
            if (i > END) {
                log_error(logger_worker, "No existe la instruccion %s", instruccion->identificador);
                list_destroy_and_destroy_elements(instrucciones, destruir_instruccion);
                destruir_instruccion(instruccion);
                string_array_destroy(tokens);
                string_array_destroy(lineas);
                free(contenido);
                exit(EXIT_FAILURE);
            }
        }        
        
        instruccion->copi = i; // El valor de i es equivalente al valor del enum set_instrucciones;        
        switch (instruccion->copi) {
            case CREATE:    // Formato: CREATE <NOMBRE_FILE>:<TAG>
            case COMMIT:    // Formato: COMMIT <NOMBRE_FILE>:<TAG>
            case DELETE:    // Formato: DELETE <NOMBRE_FILE>:<TAG>
            case FLUSH:     // Formato: FLUSH <NOMBRE_FILE>:<TAG>
                string_array_push(&instruccion->datos, tokens[1]);
                break;

            case TRUNCATE:  // Formato: TRUNCATE <NOMBRE_FILE>:<TAG> <TAMAÑO>
            case TAG:       // Formato: TAG <NOMBRE_FILE_ORIGEN>:<TAG_ORIGEN> <NOMBRE_FILE_DESTINO>:<TAG_DESTINO>
                string_array_push(&instruccion->datos, tokens[1]);
                string_array_push(&instruccion->datos, tokens[2]);
                break;

            case READ:      // Formato: READ <NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <TAMAÑO>
            case WRITE:     // Formato: WRITE <NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <CONTENIDO>
                string_array_push(&instruccion->datos, tokens[1]);
                string_array_push(&instruccion->datos, tokens[2]);
                string_array_push(&instruccion->datos, tokens[3]);
                break;

            default:        // Formato: END
                break;
        }

        list_add_in_index(instrucciones, 0, instruccion);
    }

    string_array_destroy(lineas);
    free(contenido);

    log_info(logger_worker, "Instrucciones de la query cargadas");
    return instrucciones;
}

void destruir_instruccion(void* arg) {
    t_instrucion* instruccion = (t_instrucion*) arg;
    string_array_destroy(instruccion->datos);
    free(instruccion->identificador);
    free(instruccion);
}