#include "liberar_recursos.h"

// Private Functions //

void notificar_modulos();
void liberar_recursos();

// Public Functions //

void notificar_y_liberar() {
    log_info(logger_worker, "Notificando desconexion a los otros modulos");
    notificar_modulos();
    log_info(logger_worker, "Libernado recursos");
    liberar_recursos();
}

// Private Functions //

void notificar_modulos() {
    paquete_t* paquete = crear_paquete(DESCONEXION);
    void* paquete_s = serializar_paquete(paquete);
    
    if (storage_socket != -1) {
        send(storage_socket, paquete_s, espacio_paquete_serializado(paquete), 0);
    }
    if (master_socket != -1) {
        send(master_socket, paquete_s, espacio_paquete_serializado(paquete), 0);
    }
    
    destruir_paquete(paquete);
    free(paquete_s);
}

void liberar_recursos() {
    if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "LRU")) {
        temporal_destroy(cronometro);
    }
    config_destroy(config_worker);
    list_destroy_and_destroy_elements(file_tag_pages, free_file_tag);
    list_destroy_and_destroy_elements(marco, free);
    free(memoria);
    free(bit_map_marco);
    close(master_socket);
    close(storage_socket);
    log_destroy(logger_worker);
}