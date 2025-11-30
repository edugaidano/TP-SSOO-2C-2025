#ifndef _STORAGE_LOCKS_H
#define _STORAGE_LOCKS_H

#include "storage_globals.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <sys/stat.h>

void init_locks_index();

void storage_wait(char* file, char* tag);
void storage_signal(char* file, char* tag);

void add_lock (char* file, char* tag);
void remove_lock (char* file, char* tag);

#endif