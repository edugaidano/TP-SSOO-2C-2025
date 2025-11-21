#ifndef STORAGE_FRESH_START_H
#define STORAGE_FRESH_START_H

#include "storage_globals.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Función principal de creación
void storage_fresh_start();

#endif