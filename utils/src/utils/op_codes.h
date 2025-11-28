#ifndef _UTILS_OP_CODES_H_
#define _UTILS_OP_CODES_H_

typedef enum opcodes
{
    DESCONEXION = -1,
    HANDSHAKE_QUERY_MASTER,
    HANDSHAKE_WORKER_MASTER,
    HANDSHAKE_WORKER_STORAGE,
    RESULTADO_HANDSHAKE,
    ASIGNACION_WORKER,
    SOLICITUD_STORAGE,
    CONSULTA_INTERRUPCION,
    INSTRUCCION_STORAGE,
    INSTRUCCION_MASTER,
    PAGINA_WORKER,
    LECTURA_MASTER,
    SOLICITUD_EJECUCION,
    QUERY_FINALIZADA,
    NOTIF_QUERY_CONTROL,
    DESALOJO_QUERY,
    QUERY_DESALOJADA,
    INFO_FILE_TAG_STORAGE,
    INFO_FILE_TAG_WORKER,
    MODIFICACIONES_STORAGE,
    RTA_STORAGE_WORKER
} op_code;

typedef enum rta_storage
{
    NOT_DEF_ERR = -1,
    OP_EXITOSA,
    ERR_INEXISTENCIA,           //File / Tag inexistente
    ERR_PREEXISTENCIA,          //File / Tag preexistente
    ERR_ESP_INSUFICIENTE,       // Espacio Insuficiente
    ERR_WRITE_COMMITED,         //Escritura no permitida
    ERR_FUERA_LIMITE            //Lectura o escritura fuera de limite
} rta_storage;

typedef enum notif_query_control
{
    NOTIF_READ,
    NOTIF_FINAL
} notif_query_control;

typedef enum razon_fin
{
    FINALIZACION_CORRECTA,
    ERR_DESC_WORKER,
    ERR_STORAGE
} razon_fin;

#endif