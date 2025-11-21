#include "storage_fresh_start.h"

void storage_fresh_start()
{
    log_info(logger_storage, "Iniciando FRESH_START en %s", PUNTO_MONTAJE);

    // Crear directorios base
    int result_mkdir = mkdir(PUNTO_MONTAJE, 0777);
    chdir(PUNTO_MONTAJE);
    if (result_mkdir != 0 && errno == EEXIST) {
        system("rm -R ./files/*");
        system("rm -R ./physical_blocks/*");
    }
    mkdir("physical_blocks", 0777);
    mkdir("files", 0777);

    // superblock.config
    config_super_block = config_create("superblock.config");
    if (!config_super_block) {
        FILE* superblock = fopen("superblock.config", "w");
        fprintf(superblock, "FS_SIZE=%d\n", FS_SIZE);
        fprintf(superblock, "BLOCK_SIZE=%d\n", BLOCK_SIZE);
        fclose(superblock);
        config_super_block = config_create("superblock.config");
    } else {
        FS_SIZE = config_get_int_value(config_super_block, "FS_SIZE");
        BLOCK_SIZE = config_get_int_value(config_super_block, "BLOCK_SIZE");
    } 

    // Crear blocks_hash_index.config vacío
    FILE* hash_index = fopen("blocks_hash_index.config", "w");
    if (hash_index) fclose(hash_index);

    // Calcular cantidad de bloques
    int blocks_count = FS_SIZE / BLOCK_SIZE;

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

    log_info(logger_storage, "bitmap.bin creado (%d bytes)", bitmap_bytes);

    // Crear bloque físico inicial
    mkdir("physical_blocks", 0777);
    char *zero_block = string_repeat('0', BLOCK_SIZE);
    for (int i = 0; i < blocks_count; i++) {
        char* block_path = string_from_format("physical_blocks/block%04d.dat", i);

        FILE* block = fopen(block_path, "wb");
        int block_fd = fileno(block); 
        ftruncate(block_fd, BLOCK_SIZE);
        if (i == 0) {
            fwrite(zero_block, 1, BLOCK_SIZE, block);
            free(zero_block);
            // TODO: hash
        }
        fclose(block);
    }    

    // Crear estructura de archivos inicial
    mkdir("files/initial_file", 0777);
    mkdir("files/initial_file/BASE", 0777);
    mkdir("files/initial_file/BASE/logical_blocks", 0777);

    // Hard link desde logical_blocks/00000.dat al bloque físico 0
    link("physical_blocks/block0000.dat", "files/initial_file/BASE/logical_blocks/00000.dat");
    bitarray_set_bit(bitarray_ptr, 0);
    msync(bit_ptr, bitmap_bytes, MS_SYNC);

    bitarray_destroy(bitarray_ptr);
    munmap(bit_ptr, bitmap_bytes);
    fclose(bitmap);

    // Crear metadata.config
    FILE* meta = fopen("files/initial_file/BASE/metadata.config", "w");
    if (meta) {
        fprintf(meta, "TAMAÑO=0\n");
        fprintf(meta, "BLOCKS=[]\n");
        fprintf(meta, "ESTADO=WORK_IN_PROGRESS\n");
        fclose(meta);
    }

    log_info(logger_storage, "FRESH_START completado correctamente.");
    log_info(logger_storage, "Directorio base creado en %s", PUNTO_MONTAJE);
    chdir("..");
}