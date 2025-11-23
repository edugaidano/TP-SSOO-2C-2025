#include "storage_operaciones.h"

resultado_t desglozar_instruccion(t_list* pkg) {
    set_instrucciones copi =*(set_instrucciones*)list_get(pkg, 0);
    char* file = (char*)list_get(pkg, 1);
    char* tag = (char*)list_get(pkg, 2);
    resultado_t result;

    //sem_t* sem_ft = get_sem_file_tag(file, tag) -> busca en una estructura el sem_t* del file tag con el que se quiera interactuar
    // if else: si no existe la entrada (sem_ft == NULL), y el copi == CREATE entonces le crea una entrada en la estructura (si no tira error por no existir)

    //sem_wait(sem_ft)

    switch (copi)
    {
    case CREATE:
        result = storage_create(file, tag);
        break;
    case COMMIT:
    {
        // Por ahora lo puse aca, dsp hacer una funcion commit bien XD
        t_metadata_file* metadata = storage_metadata_read(file, tag);
        free(metadata->estado);
        metadata->estado = string_duplicate("COMMITED");
        storage_metadata_write(file, tag, metadata);
        storage_metadata_destroy(metadata);
        //TODO: liberar bloques fisicos que tengan bloques logicos del file tag (usar las funciones de hash)
        result = OK;
        break;
    }
    case DELETE:
        //detele()
        result = OK;
        break;
    case FLUSH:
    {
        int total_pagians = (list_size(pkg) - 3)/2;
        for (int i = 0; i < total_pagians; i++) {
            int nro_pagina = *(int*)list_get(pkg, i + 3);
            char* data = (char*)list_get(pkg, i + 4);
            // Por ahora lo puse aca, dsp hacer una funcion flush bien XD
            int r = storage_write(file, tag, nro_pagina, data);
            if (r != OK) { return r; }
        }
        result = OK;  
        break;
    }
    case TRUNCATE:
    {
        int size = *(int*)list_get(pkg, 3);
        result = storage_truncate(file, tag, size);
        break;
    }
    case TAG:
    {
        char* file2 = (char*)list_get(pkg, 3);
        char* tag2 = (char*)list_get(pkg, 4);
        // Por ahora lo puse aca, dsp hacer una funcion tag bien XD
        result = storage_create(file2, tag2);
        if(result == ERROR) {return result;}

        t_metadata_file* metadata_ft1 = storage_metadata_read(file, tag);
        free(metadata_ft1->estado);
        metadata_ft1->estado = string_duplicate("WORK_IN_PROGRESS");
        storage_metadata_write(file2, tag2, metadata_ft1);

        result = storage_truncate(file2, tag2, metadata_ft1->tamanio);
        if(result == ERROR) {return result;}
        storage_metadata_destroy(metadata_ft1);
        // TODO: copiar el contenido del otro file tag
        break;
    }
    default:
    {
        log_error(logger_storage, "se recivio un paquete con una instruccion desconocida");
        result = ERROR;
        break;
    }
    }

    //sem_post(sem_ft)

    return result;
}