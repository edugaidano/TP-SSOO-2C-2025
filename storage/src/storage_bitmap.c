#include "storage_bitmap.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


t_bitarray* bitmap = NULL;
pthread_rwlock_t bitmap_lock = PTHREAD_RWLOCK_INITIALIZER; // lo mismo q comente en el storage_bitmap.h
static int bitmap_fd = -1;
static size_t bitmap_size = 0;
static void* bitmap_data = NULL;

int bitmap_init(const char* mount_point, int blocks_count)
{
    char* path = string_from_format("%s/bitmap.bin", mount_point);

    bitmap_size = (blocks_count + 7) / 8;

    bitmap_fd = open(path, O_RDWR);
    if (bitmap_fd < 0)
    {
        log_error(logger_storage, "No se pudo abrir %s", path);
        return -1;
    }

    bitmap_data = mmap(NULL, bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, bitmap_fd, 0);
    if (bitmap_data == MAP_FAILED)
    {
        log_error(logger_storage, "Error al mapear %s", path);
        close(bitmap_fd);
        return -1;
    }

    bitmap = bitarray_create_with_mode(bitmap_data, bitmap_size, LSB_FIRST);
    log_info(logger_storage, "Bitmap inicializado (%d bloques, %zu bytes)", blocks_count, bitmap_size);
    return 0;
}

void bitmap_destroy(void)
{
    if (bitmap)
    {
        msync(bitmap_data, bitmap_size, MS_SYNC);
        bitarray_destroy(bitmap);
        munmap(bitmap_data, bitmap_size);
        close(bitmap_fd);
        bitmap = NULL;
    }
}

int find_free_block()
{
    pthread_rwlock_wrlock(&bitmap_lock);
    int free_block = -1;

    for (int i = 1; i < bitarray_get_max_bit(bitmap); i++)
    {
        if (!bitarray_test_bit(bitmap, i))
        {
            bitarray_set_bit(bitmap, i);
            msync(bitmap_data, bitmap_size, MS_SYNC);
            free_block = i;
            break;
        }
    }

    pthread_rwlock_unlock(&bitmap_lock);

    if (free_block == -1)
        log_error(logger_storage, "No hay bloques libres en el bitmap");

    return free_block;
}

void mark_block_used(int block_num)
{
    pthread_rwlock_wrlock(&bitmap_lock);
    bitarray_set_bit(bitmap, block_num);
    msync(bitmap_data, bitmap_size, MS_SYNC);
    pthread_rwlock_unlock(&bitmap_lock);
}

void mark_block_free(int block_num, char* query_id)
{
    pthread_rwlock_wrlock(&bitmap_lock);
    bitarray_clean_bit(bitmap, block_num);
    msync(bitmap_data, bitmap_size, MS_SYNC);
    pthread_rwlock_unlock(&bitmap_lock);
    log_info(logger_storage, "## %s - Bloque Físico Liberado - Número de Bloque: %d", query_id, block_num);
}