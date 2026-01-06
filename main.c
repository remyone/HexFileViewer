/*
                   ___ _ _            _                         
  /\  /\_____  __ / __(_) | ___/\   /(_) _____      _____ _ __  
 / /_/ / _ \ \/ // _\ | | |/ _ \ \ / / |/ _ \ \ /\ / / _ \ '__| 
/ __  /  __/>  </ /   | | |  __/\ V /| |  __/\ V  V /  __/ |    
\/ /_/ \___/_/\_\/    |_|_|\___| \_/ |_|\___| \_/\_/ \___|_|                                                                                                                                    
                    made by remy
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define OFFSET 16
#define HEX_ADDRESS_CHAR 9

#define reset "\x1b[0m"
#define red "\x1b[31m"
#define green "\x1b[92m"
#define yellow "\x1b[93m"
#define blue "\x1b[96m"

int min(int a, int b) {
    if (a < b)
        return a;
    return b;
}

long GetFileSize(FILE *stream) {
    if (fseek(stream, 0, SEEK_END) != -1) {
        long FileSize = ftell(stream);
        rewind(stream);
        return FileSize;
    }
    return -1l;
}

char *GetOffset(ssize_t address_length, long offset) {
    int alloc_size = sizeof(char) * address_length;
    char *hex_offset = malloc(alloc_size);

    if (hex_offset == NULL) {
        printf(red"ERROR: [hex offset allocation failed]"reset);
        exit(-1);
    }

    snprintf(hex_offset, alloc_size, "%x0", offset);
    return hex_offset;
}
/*
 =========================
|     Modify Address     |
=========================
*/
void ConvertOffsetToAddress(char *hex_address, long offset) {
    ssize_t address_length = snprintf(NULL, 0, "%x0", offset);
    for (int i = 0; i < HEX_ADDRESS_CHAR - address_length; ++i)
        *(hex_address+i) = '0';
    *(hex_address + HEX_ADDRESS_CHAR - address_length) = '\0';
    char *hex_offset = GetOffset(address_length, offset);
    strcat(hex_address, hex_offset);
    *(hex_address+HEX_ADDRESS_CHAR) = '\0';
    free(hex_offset);
}

void findAndModifyNewLine(char *buffer, long buffer_size) {
    for (int i = 0; i < buffer_size; ++i) {
        if (*(buffer+i) == '\n')
            *(buffer+i) = '.';
    }
}

void PrintReadLine(char *hex_address, char *buffer, long FileSize, long offset, int minimum_remaining_bytes) {
    uint16_t opcode;

    printf("%s: ", hex_address);

    if (minimum_remaining_bytes & 1)
        minimum_remaining_bytes--;


    for (int i = 0; i < minimum_remaining_bytes ; i += 2) {
        opcode = (buffer[i + offset] << 8) | buffer[i + offset + 1];
        if ((opcode >> 8) == 10) {
            printf(yellow"0%x"reset, opcode >> 8);
            printf(green"%x "reset, opcode & 0xFF);
        } else if ((opcode & 0xFF) == 10) {
            printf(green"%x"reset, opcode >> 8);
            printf(yellow"0%x "reset, opcode & 0xFF);
        }
        else
            printf(green"%x "reset, opcode);
    }

    if (FileSize & 1) {
        if ((minimum_remaining_bytes < 16) && (buffer[offset+minimum_remaining_bytes+1] == buffer[FileSize]))
            printf(yellow"0a "reset);
        else
            printf(green"%x "reset, opcode);
        //for (int i = 0; i < (17 - minimum_remaining_bytes) * 2 - 1)
    }

    findAndModifyNewLine(buffer, sizeof(char) * FileSize + 1);

    printf(blue" %s\n"reset, buffer + offset);

}

void ReadFile(char *filename, long *offset) {
    char *hex_address = malloc(sizeof(char) * HEX_ADDRESS_CHAR);
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        printf(red"ERROR: [file open failed]\n"reset);
        exit(-1);
    }

    long FileSize;
    if ((FileSize = GetFileSize(file)) == -1l)
        printf(red"ERROR: [getting file size failed]\n"reset);

    char *buffer = malloc(sizeof(char) * FileSize + 1);

    if (buffer == NULL) {
        printf(red"ERROR: [buffer allocation failed]\n"reset);
        exit(-1);
    }

    int minimum_remaining_bytes;
    long remaining_bytes;
    rewind(file);

    /*
    
    */

    while (*offset < FileSize) {
        remaining_bytes = FileSize - *offset;

        minimum_remaining_bytes = min(OFFSET, remaining_bytes);

        fseek(file, *offset, SEEK_SET); //move on offset (e.g 16, 32, ...) bytes from the beginning of the file
        fread((buffer + *offset), 1, minimum_remaining_bytes, file);

        ConvertOffsetToAddress(hex_address, *offset);

        PrintReadLine(hex_address, buffer, FileSize, *offset, minimum_remaining_bytes);

        *offset += minimum_remaining_bytes;

        memset(hex_address, 0, HEX_ADDRESS_CHAR);
    }

    rewind(file);
    free(buffer);
    free(hex_address);
    fclose(file);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf(red"Usage: %s filename\n"reset, *argv);
        return -1;
    }

    long offset = 0;

    ReadFile(argv[1], &offset);
    
    return 0;
}