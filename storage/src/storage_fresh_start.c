#include "storage_fresh_start.h"

void storage_fresh_start(const char* mount_point, int fs_size, int block_size)
{
    log_info(logger_storage, "Iniciando FRESH_START en %s", mount_point);

    // Crear directorios base
    mkdir(mount_point, 0777);
    chdir(mount_point);
    mkdir("physical_blocks", 0777);
    mkdir("files", 0777);

    // Crear blocks_hash_index.config vacío
    FILE* hash_index = fopen("blocks_hash_index.config", "w");
    if (hash_index) fclose(hash_index);

    // Crear superblock.config
    FILE* superblock = fopen("superblock.config", "w");
    if (superblock) {
        fprintf(superblock, "FS_SIZE=%d\n", fs_size);
        fprintf(superblock, "BLOCK_SIZE=%d\n", block_size);
        fclose(superblock);
    }

    // Calcular cantidad de bloques
    int blocks_count = fs_size / block_size;

    // Crear bitmap.bin
    int bitmap_bytes = (blocks_count + 7) / 8;
    FILE* bitmap = fopen("bitmap.bin", "w+b");
    if (!bitmap) {
        log_error(logger_storage, "No se pudo crear bitmap.bin");
        return;
    }

    int bitmap_fd = fileno(bitmap); 

    ftruncate(bitmap_fd, bitmap_bytes);

    char* bit_ptr = (char*)mmap(NULL, bitmap_bytes, PROT_WRITE | PROT_READ, MAP_SHARED, bitmap_fd, 0);
    if (bit_ptr == MAP_FAILED)
    {
        log_error(logger_storage, "Error al mapear bitmap.bin");
        fclose(bitmap);
        return;
    }

    t_bitarray* bitarray_ptr = bitarray_create_with_mode(bit_ptr, bitmap_bytes, LSB_FIRST);

    for (int i = 0; i < (bitmap_bytes * 8); i++) {
        bitarray_clean_bit(bitarray_ptr, i);
    }

    msync(bit_ptr, bitmap_bytes, MS_SYNC);

    bitarray_destroy(bitarray_ptr);
    munmap(bit_ptr, bitmap_bytes);
    fclose(bitmap);

    log_info(logger_storage, "bitmap.bin creado (%d bytes)", bitmap_bytes);

    // Crear bloque físico inicial
    mkdir("physical_blocks", 0777);
    char block_path[128];
    snprintf(block_path, sizeof(block_path), "physical_blocks/block%04d.dat", 0);
    FILE* block = fopen(block_path, "wb");
    if (block) {
        char zero_block[block_size];
        memset(zero_block, 0, block_size);
        fwrite(zero_block, 1, block_size, block);
        fclose(block);
    }

    // Crear estructura de archivos inicial
    mkdir("files/BASE", 0777);
    mkdir("files/BASE/BASE", 0777);
    mkdir("files/BASE/BASE/logical_blocks", 0777);

    // Hard link desde logical_blocks/00000.dat al bloque físico 0
    char logical_path[256];
    snprintf(logical_path, sizeof(logical_path), "files/BASE/BASE/logical_blocks/00000.dat");
    link(block_path, logical_path);

    // Crear metadata.config
    FILE* meta = fopen("files/BASE/BASE/metadata.config", "w");
    if (meta) {
        fprintf(meta, "TAMAÑO=0\n");
        fprintf(meta, "BLOCKS=[]\n");
        fprintf(meta, "ESTADO=WORK_IN_PROGRESS\n");
        fclose(meta);
    }

    log_info(logger_storage, "FRESH_START completado correctamente.");
    log_info(logger_storage, "Directorio base creado en %s", mount_point);
    chdir("..");
}