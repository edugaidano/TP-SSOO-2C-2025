#include "storage_operaciones.h"

rta_storage desglozar_instruccion(t_list* pkg) {
    set_instrucciones copi =*(set_instrucciones*)list_get(pkg, 0);
    char* file = (char*)list_get(pkg, 1);
    char* tag = (char*)list_get(pkg, 2);
    char* query_id = (char*)list_get(pkg, 3);
    rta_storage result;

    //sem_t* sem_ft = get_sem_file_tag(file, tag) -> busca en una estructura el sem_t* del file tag con el que se quiera interactuar
    // if else: si no existe la entrada (sem_ft == NULL), y el copi == CREATE entonces le crea una entrada en la estructura (si no tira error por no existir)

    //sem_wait(sem_ft)

    switch (copi)
    {
    case CREATE:
    {
        result = storage_create(file, tag, query_id);
        break;
    }
    case COMMIT:
    {
        result = storage_commit(file, tag, query_id);
        break;
    }
    case DELETE:
    {
        result = storage_delete(file, tag, query_id);
        break;
    }
    case TRUNCATE:
    {
        int size = *(int*)list_get(pkg, 4);
        result = storage_truncate(file, tag, size, query_id);
        break;
    }
    case TAG:
    {
        char* file2 = (char*)list_get(pkg, 4);
        char* tag2 = (char*)list_get(pkg, 5);
        result = storage_tag(file, tag, file2, tag2, query_id);
        break;
    }
    default:
    {
        log_error(logger_storage, "se recivio un paquete con una instruccion desconocida");
        result = NOT_DEF_ERR;
        break;
    }
    }

    //sem_post(sem_ft)

    return result;
}