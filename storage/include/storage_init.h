#ifndef STORAGE_INIT_H
#define STORAGE_INIT_H

#include "storage_bitmap.h"
#include "storage_fresh_start.h"
#include "storage_globals.h"
#include "storage_locks.h"
#include "utils/pathing.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

void init(char *ruta_config);

#endif