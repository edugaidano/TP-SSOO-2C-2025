#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "memoria.h"
#include "utils/paquetes.h"
#include <math.h>

/*
 * Formato: CREATE <FILE>:<TAG>
 * Solicita al Storage la creación de un nuevo File:Tag con tamaño 0.
 */
void interpretar_CREATE(t_instrucion* instruccion);

/*
 * Formato: COMMIT <FILE>:<TAG>
 * Le indica al Storage que no se realizarán más cambios sobre el File:Tag pasados por parámetro.
 */
void interpretar_COMMIT(t_instrucion* instruccion);

/*
 * Formato: DELETE <FILE>:<TAG>
 * Solicitará al Storage la eliminación del File:Tag correspondiente.
 */
void interpretar_DELETE(t_instrucion* instruccion);

/*
 * Formato: FLUSH <FILE>:<TAG>
 * Persistirá todas las modificaciones realizadas en Memoria Interna de un File:Tag en el Storage.
 */
void interpretar_FLUSH(t_instrucion* instruccion);

/*
 * Formato: TRUNCATE <FILE>:<TAG> <TAMAÑO>
 * Solicita al Storage la modificación del tamaño del File:Tag (el tamaño deberá ser múltiplo del tamaño de bloque).
 */
void interpretar_TRUNCATE(t_instrucion* instruccion);

/*
 * Formato: TAG <FILE_ORIGEN>:<TAG_ORIGEN> <FILE_DESTINO>:<TAG_DESTINO>
 * La instrucción TAG solicitará al Storage la creación un nuevo File:Tag a partir del File:Tag origen.
 */
void interpretar_TAG(t_instrucion* instruccion);

/*
 * Formato: READ <FILE>:<TAG> <DIRECCIÓN BASE> <TAMAÑO>
 * Lee de la Memoria Interna los bytes correspondientes a partir de la dirección base del File:Tag, y la enviar al Master.
 */
void interpretar_READ(t_instrucion* instruccion);

/*
 * Formato: WRITE <FILE>:<TAG> <DIRECCIÓN BASE> <CONTENIDO>
 * Escribe en la Memoria Interna los bytes correspondientes a partir de la dirección base del File:Tag. 
 */
void interpretar_WRITE(t_instrucion* instruccion);

/*
 * Formato: END
 * Da por finalizada la Query y le informa al módulo Master el fin de la misma.
 */
void interpretar_END(t_instrucion* instruccion);

#endif