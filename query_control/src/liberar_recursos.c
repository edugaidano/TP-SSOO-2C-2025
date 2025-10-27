#include "liberar_recursos.h"

// Pivate Functions //

void notificar_master();
void liberar_recursos();

// Public Functions //

void notificar_y_liberar() {
    log_info(logger_query_control, "Notificando desconexion al Master");
    notificar_master();
    log_info(logger_query_control, "Libernado recursos");
    liberar_recursos();
}

// Pivate Functions //

void notificar_master() {
    paquete_t* paquete = crear_paquete(DESCONEXION);
    void* paquete_s = serializar_paquete(paquete);

    if (socket_master != -1) {
        send(socket_master, paquete_s, espacio_paquete_serializado(paquete), 0);
    }
    
    destruir_paquete(paquete);
    free(paquete_s);
}

void liberar_recursos() {
    config_destroy(config_query_control);
    close(socket_master);
    log_destroy(logger_query_control);
}