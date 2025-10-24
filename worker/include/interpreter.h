#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <worker_globals.h>
#include <memoria.h>
#include <math.h>
#include <utils/paquetes.h>

void interpretar_CREATE(t_instrucion* instruccion);
void interpretar_COMMIT(t_instrucion* instruccion);
void interpretar_DELETE(t_instrucion* instruccion);
void interpretar_FLUSH(t_instrucion* instruccion);
void interpretar_TRUNCATE(t_instrucion* instruccion);
void interpretar_TAG(t_instrucion* instruccion);
void interpretar_READ(t_instrucion* instruccion);
void interpretar_WRITE(t_instrucion* instruccion);
void interpretar_END(t_instrucion* instruccion);

#endif