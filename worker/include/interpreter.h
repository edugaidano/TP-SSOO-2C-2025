#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <worker_globals.h>
#include <memoria.h>
#include <math.h>
#include <utils/paquetes.h>

void interpretar_CREATE(t_instrucion* instruccion, char* query_id, int fd_storage);
void interpretar_COMMIT(t_instrucion* instruccion, char* query_id, int fd_storage);
void interpretar_DELETE(t_instrucion* instruccion, char* query_id, int fd_storage);
void interpretar_FLUSH(t_instrucion* instruccion, char* query_id, int fd_storage);
void interpretar_TRUNCATE(t_instrucion* instruccion, char* query_id, int fd_storage);
void interpretar_TAG(t_instrucion* instruccion, char* query_id, int fd_storage);
void interpretar_READ(t_instrucion* instruccion, char* query_id, int fd_storage, int fd_master);
void interpretar_WRITE(t_instrucion* instruccion, char* query_id, int fd_storage);
void interpretar_END(t_instrucion* instruccion, char* query_id, int fd_master);

#endif