#include "storage_locks.h"

//-- PRIVATE FUNCTIONS --//

char** get_in(char* path, unsigned long mask);
t_lock* create_lock();
t_lock* get_lock_of(char* file, char* tag);
void destroy_lock(char* file, char* tag, t_lock* lock);

//-- PUBLIC FUNCTIONS --//

void init_locks_index() {
    locks_index = dictionary_create();
    sem_init(&sem_locks_index, 0, 1);

    char* path = string_from_format("%s/files", PUNTO_MONTAJE);
    char** files = get_in(path, __S_IFDIR);
    if (string_array_is_empty(files)) {
        free(path);
    }
    
    while (!string_array_is_empty(files)) {

        char* file = string_array_pop(files);
        string_append_with_format(&path, "/%s", file);

        char** tags = get_in(path, __S_IFREG);
        free(path);
        
        t_dictionary* ft_locks = dictionary_create();
        dictionary_put(locks_index, file, ft_locks);
        free(file);

        while (!string_array_is_empty(tags)) {
            char* tag = string_array_pop(tags);
            t_lock* lock = create_lock();
            dictionary_put(ft_locks, tag, lock);
            free(tag);
        }
        
        string_array_destroy(tags);
    }

    string_array_destroy(files);  

    int blocks_count = FS_SIZE / BLOCK_SIZE;
    blk_mutex_list = list_create();

    for (int i = 0; i < blocks_count; i++)
    {
        sem_t* blk_mutex = malloc(sizeof(sem_t));
        sem_init(blk_mutex, 0, 1);
        list_add(blk_mutex_list, blk_mutex);
    }
    
}

void storage_wait(char* file, char* tag) {
    sem_wait(&sem_locks_index);
    
    t_lock* lock = get_lock_of(file, tag);
    if (!lock) { 
        sem_post(&sem_locks_index);
        return; 
    }

    lock->count_waiting++;
    sem_post(&sem_locks_index);
    
    sem_wait(&(lock->sem_file_tag));
    lock->count_waiting--;
}

void storage_signal(char* file, char* tag) {
    sem_wait(&sem_locks_index);
    
    t_lock* lock = get_lock_of(file, tag);
    if (!lock) { 
        sem_post(&sem_locks_index);
        return; 
    }
    
    if ( !(lock->delete) || lock->count_waiting != 0) {
        sem_post(&(lock->sem_file_tag));
    } else {
        destroy_lock(file, tag, lock);
    }
    
    sem_post(&sem_locks_index);
}

void add_lock (char* file, char* tag) {
    sem_wait(&sem_locks_index);
    if (!dictionary_has_key(locks_index, file)) { 
        dictionary_put(locks_index, file, dictionary_create());
    }

    t_dictionary* tags_index = dictionary_get(locks_index, file);
    
    if (!dictionary_has_key(tags_index, tag)) {
        t_lock* lock = create_lock();
        dictionary_put(tags_index, tag, lock);
    }

    sem_post(&sem_locks_index);
}

void remove_lock (char* file, char* tag) {
    sem_wait(&sem_locks_index);

    t_lock* lock = get_lock_of(file, tag);
    if (!lock) { 
        sem_post(&sem_locks_index);
        return; 
    }
    
    if ( lock->count_waiting != 0) {
        lock->delete = true;
    } else {
        destroy_lock(file, tag, lock);
    }

    sem_post(&sem_locks_index);
}

//-- PRIVATE FUNCTIONS --//

char** get_in(char* path, unsigned long mask) { // unsigned long mask -> para que sea compatible con __S_ISTYPE
    char** array = string_array_new();

    DIR *dir = opendir(path);
    if (!dir) {
        log_error(logger_storage, "Error al hacer opendir(%s)", path);
        string_array_destroy(array);
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {

        // Ignorar "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char* path = string_from_format("%s/%s", PUNTO_MONTAJE, entry->d_name);
        struct stat st;
        if (stat(path, &st) == -1){
            free(path);
            continue;
        }

        if (__S_ISTYPE(st.st_mode, mask)) {
            string_array_push(&array, entry->d_name);
        }
    }

    closedir(dir);
    return array;
}

void destroy_lock(char* file, char* tag, t_lock* lock) {
    sem_destroy(&(lock->sem_file_tag));
    //free(&(lock->sem_file_tag));
    t_dictionary* tags_index = dictionary_get(locks_index, file);
    dictionary_remove_and_destroy(tags_index, tag, free);
    if (dictionary_is_empty(tags_index)) {
        dictionary_destroy(dictionary_remove(locks_index, file));
    }
}

t_lock* create_lock() {
    t_lock* lock = (t_lock*) malloc(sizeof(t_lock));
    sem_init(&(lock->sem_file_tag), 0, 1);
    lock->count_waiting = 0;
    lock->delete = false;
    return lock;
}

t_lock* get_lock_of(char* file, char* tag) {
    if (!dictionary_has_key(locks_index, file)) { return NULL; }

    t_dictionary* tags_index = dictionary_get(locks_index, file);
    
    if (!dictionary_has_key(tags_index, tag)) { return NULL; }

    return dictionary_get(tags_index, tag);
}
