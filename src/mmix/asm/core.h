#ifndef __MMIX_ASM_CORE_H__
#define __MMIX_ASM_CORE_H__


static size_t BUFFER_SIZE = 250;

typedef enum {
    OK,
    FILE_NOT_FOUND
} asm_error_t;


int file_assemble(const char* src, const char* dest) {
    FILE* src_file = fopen(src, "rb");
    if(src_file == NULL) {
        return FILE_NOT_FOUND;
    }

    buffer_t buff = {src_file, BUFFER_SIZE, 0}

    fclose(src_file);
    return 0;
}

#endif