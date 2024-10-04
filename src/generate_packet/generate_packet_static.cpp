#include "generate_packet.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>



int read_data_from_file(u_char *buffer, char *filename){
    FILE *file = fopen(filename, "r");
    int size;
    if(!file){
        printf("the file '%s' could not be opened\n", filename);
        return -1;
    }
    size = fread(buffer, sizeof(char), SIZE_BUFFER, file);
    if(size < 0){
        perror("read_data_from_file, fread");
    }
    fclose(file);
    return size;
}
int write_data_to_file(u_char *buffer, char *filename){
    FILE *file = fopen(filename, "w");
    int size;
    if(!file){
        printf("the file '%s' could not be opened\n", filename);
        return -1;
    }
    size = fwrite(buffer, sizeof(char), SIZE_BUFFER, file);
    if(size < 0){
        perror("write_data_to_file, fread");
    }
    fclose(file);
    return size;
}

void print_bit(u_char *buffer, int size){
    int i, i2;
    u_char iter;
    for(i = 0; i < size; ++i){
        iter = buffer[i];
        for(i2 = 0; i2 < BYTE; ++i2){
            switch(iter & 128){
            case 0:
                printf("0");
                break;
            default:
                printf("1");
                break;
            } 
            iter <<= 1;  
        }
        printf(" ");
    }
    printf("\n");
}
void print_bit_to_stream(u_char *buffer, int size, FILE *stream){
    int i, i2;
    u_char iter;
    for(i = 0; i < size; ++i){
        iter = buffer[i];
        for(i2 = 0; i2 < BYTE; ++i2){
            switch(iter & 128){
            case 0:
                fprintf(stream, "0");
                break;
            default:
                fprintf(stream, "1");
                break;
            } 
            iter <<= 1;  
        }
    }
}
void print_data(u_char *buffer, int size){
    int i;
    for(i = 0; i < size; ++i){
       printf("%3d ", buffer[i]);
        
    }
    printf("\n");
}

void print_polinom_crc(u_char *polinom){
    int i;
    for(i = 0; i < SIZE_CRC; ++i){
       printf("%3d ", polinom[i]);
        
    }
    printf("\n");
}
void shift_bit(u_char *bytes, int size){
    if(size < 2){
        *bytes = *bytes << 1;
        return;
    }
    int i;
    for(i = 0; i < size - 1; ++i){
        bytes[i] <<= 1;
        bytes[i] |= (bytes[i+1] >> (BYTE - 1));
    }
    bytes[i] <<= 1;
    return;
}
void xor_bytes(u_char *arg1, u_char *arg2, int size){
    int i;
    for(i = 0; i < size; ++i){
        arg1[i] = arg1[i] ^ arg2[i]; 
    }
}
int read_sequence_gold(u_char *seq_gold, char *filename){
    FILE *file = fopen(filename, "r");
    int size;
    if(!file){
        printf("the file '%s' could not be opened\n", filename);
        return -1;
    }
#ifdef BIN_READ_SEQ
    size = fwrite(seq_gold, sizeof(char), SIZE_SEQ_GOLD, file);
    if(size < 0){
        perror("write_data_to_file, fwrite");
    }
#else
    char bit_str;
    char i = 0, i2 = 0; 
    char run = 1;
    u_char byte = 0;
    while ( run && (bit_str = getc(file)) != EOF && i < (SIZE_SEQ_GOLD * BYTE)){
        // printf("%c", bit_str);
        switch (bit_str){
        case '1':
            byte |= 1;
            break;
        case '0':
            break;
        default:
            printf("Incorrect char: [%c] - %d, correct: 0 or 1\n", bit_str, bit_str);
            run = 0;
            break;
        }
        if( (i + 1) % BYTE == 0){
            seq_gold[i2] = byte;
            ++i2;
            byte = 0;
        }
        byte <<= 1;
        ++i;
    } 
    if((i) % BYTE != 0){
        while((i + 1) % BYTE != 0){
            byte <<= 1;
            // print_bit(&byte, 1);
            ++i;
        }
        seq_gold[i2] = byte;
        ++i2;
    }
    size = i2;
#endif
    // print_bit(seq_gold, size);
    fclose(file);
    return size;
}
// #define DEBUG_CALC_CRC

void calc_crc(u_char *buffer, u_char *polinom, u_char *crc, int size_buffer){
    int i, i2;
    u_char iter[SIZE_CRC];
    u_char byte;
    u_char polinom_0[SIZE_CRC];
    memset((void*) polinom_0, 0, SIZE_CRC);
    memcpy((void*)iter, (void*)buffer, SIZE_CRC);
#ifdef DEBUG_CALC_CRC
    print_bit((u_char*)iter, SIZE_CRC);
#endif //DEBUG_CALC_CRC
    for(i = SIZE_CRC; i < size_buffer; ++i){
#ifdef DEBUG_CALC_CRC
        printf("i = %2d\n", i);
#endif //DEBUG_CALC_CRC
        byte = buffer[i];
        for(i2 = 0; i2 < BYTE; ++i2){
            if(iter[0] & 128)
                xor_bytes(iter, polinom, SIZE_CRC);
            else
                xor_bytes(iter, polinom_0, SIZE_CRC);
            if(i == size_buffer-1 && i2 == BYTE-1)
                break;
            shift_bit(iter, SIZE_CRC);
            iter[SIZE_CRC - 1] |= (byte >> (BYTE - 1));
#ifdef DEBUG_CALC_CRC
            printf("%d)\t", i2); print_bit(iter, SIZE_CRC);
#endif //DEBUG_CALC_CRC
            byte <<= 1;
        }
    }
    shift_bit(iter, SIZE_CRC);
#ifdef DEBUG_CALC_CRC
    printf("res:\n");
    print_bit(iter, SIZE_CRC);
#endif
    if(crc){
        memcpy((void*)crc, (void*)iter, SIZE_CRC); 
    }
}
void read_polinom(u_char *polinom, char *filename){
    polinom[0] = 172;
#if SIZE_CRC > 1
    polinom[1] = 17;
#endif
#if SIZE_CRC > 2
    polinom[2] = 57;
#endif
#if SIZE_CRC > 3
    polinom[3] = 18;
#endif
}
//b main
//r <аргументы>
//disassemble
//x/10bb $esp+0x10a
//x/8bt $esp
//x/10bt 0x61fe1c - адресс смотреть в регистрах
//x/10 &buffer - или вывести напрямую
//x/4ht &size

int generate_packet(struct data_packet_v2 *packet, int argc, char *argv[]){


    // u_char buffer[SIZE_BUFFER];
    u_char *buffer = packet->buffer;
    
    u_char polinom_crc[SIZE_CRC];
    u_char crc[SIZE_CRC];
    u_char gold_first[SIZE_SEQ_GOLD];
    u_char gold_end[SIZE_SEQ_GOLD];
    
    u_char *data = buffer + SIZE_SEQ_GOLD;
    int size;

    if(argc < COUNT_ARGV){
        printf("Not enough arguments\n");
        return 0;
    }
    memset(buffer, 0, SIZE_BUFFER);
    memset((void*) gold_first, 0, SIZE_SEQ_GOLD);
    memset((void*) gold_end, 0, SIZE_SEQ_GOLD);
    
    size = read_data_from_file(data, argv[DATA]);
#ifdef DEBUG
    printf("File: %s\n", argv[DATA]);
    printf("read data: %d\n", size);
#endif
    if(size == -1){
        return -1;
    }
    if(!size){
        printf("file '%s' empty\n", argv[DATA]);
        return 0;
    }
    read_polinom(polinom_crc, NULL);
    if(read_sequence_gold(gold_first, argv[SEQUENCE_FIRST]) == -1){
        return -1;
    }
    if(read_sequence_gold(gold_end, argv[SEQUENCE_END]) == -1){
        return -1;
    }
#ifdef DEBUG
    printf("sequence gold first: \n");
    print_bit(gold_first, SIZE_SEQ_GOLD);
    printf("sequence gold end: \n");
    print_bit(gold_end, SIZE_SEQ_GOLD);
#endif
#ifdef DEBUG
    printf("Data:\n");
    print_data(data, size);
    print_bit(data, size);
    printf("Data + 0(%d):\n", SIZE_CRC-1);
    print_bit(data, size + SIZE_CRC);
    print_polinom_crc(polinom_crc);
    print_bit(polinom_crc, SIZE_CRC);
    printf("\n\n");
#endif
#if 0
    shift_bit(polinom_crc, SIZE_CRC);
    print_bit(polinom_crc, SIZE_CRC);
    return 0;
#endif
#if 0
    xor_bytes(data, polinom_crc, SIZE_CRC);
    print_bit(data, SIZE_CRC);
    return 0;
#endif
    calc_crc(data, polinom_crc, crc, size + SIZE_CRC);
    memcpy( (void*)(data + size), (void*)crc, SIZE_CRC);
#ifdef DEBUG
    printf("CRC:\n");
    print_bit(crc, SIZE_CRC);
    printf("Data + CRC:\n");
    print_bit(data, size + SIZE_CRC);
#endif
#ifdef DEBUG_CALC_CRC
    // data[1] |= 64;
    // printf("Data + CRC (trash):\n");
    // print_bit(data, size + SIZE_CRC);
    calc_crc(data, polinom_crc, crc, size + SIZE_CRC);
    printf("CRC:\n");
    print_bit(crc, SIZE_CRC);
    printf("End\n");
    return 0;
#endif //DEBUG_CALC_CRC
    memcpy(buffer, gold_first, SIZE_SEQ_GOLD);
    memcpy(data + size + SIZE_CRC, gold_end, SIZE_SEQ_GOLD);
#ifdef DEBUG
    printf("seq gold + data + CRC:\n");
    print_bit(buffer, SIZE_SEQ_GOLD + size + SIZE_CRC);
    printf("seq gold + data + CRC + seq gold end:\n");
    print_bit(buffer, SIZE_SEQ_GOLD + size + SIZE_CRC + SIZE_SEQ_GOLD);
#endif
    FILE *file = fopen(argv[RESULT], "w");
    print_bit_to_stream(buffer, SIZE_SEQ_GOLD + size + SIZE_CRC + SIZE_SEQ_GOLD, file);
    packet->size = SIZE_SEQ_GOLD + size + SIZE_CRC + SIZE_SEQ_GOLD;
    fclose(file);
    return 0;
}

#ifdef TEST
int main(int argc, char *argv[]){

    // char p[4] = {1, 1, 1, 1};
    // int *r = (int*)(p);
    // *r = 0xaabbccdd;
    // printf("%x %x %x %x\n", (unsigned char)p[0], 
    // (unsigned char)p[1], (unsigned char)p[2], (unsigned char)p[3]);//dd cc bb aa
    // return 0;
    u_char buffer[SIZE_BUFFER];
    u_char polinom_crc[SIZE_CRC];
    u_char crc[SIZE_CRC];
    u_char gold_first[SIZE_SEQ_GOLD];
    u_char gold_end[SIZE_SEQ_GOLD];
    
    u_char *data = buffer + SIZE_SEQ_GOLD;
    int size;

    if(argc < COUNT_ARGV){
        printf("Not enough arguments\n");
        return 0;
    }
    memset(buffer, 0, SIZE_BUFFER);
    memset((void*) gold_first, 0, SIZE_SEQ_GOLD);
    memset((void*) gold_end, 0, SIZE_SEQ_GOLD);
    
    size = read_data_from_file(data, argv[DATA]);
#ifdef DEBUG
    printf("File: %s\n", argv[DATA]);
    printf("read data: %d\n", size);
#endif
    if(size == -1){
        return -1;
    }
    if(!size){
        printf("file '%s' empty\n", argv[DATA]);
        return 0;
    }
    read_polinom(polinom_crc, NULL);
    if(read_sequence_gold(gold_first, argv[SEQUENCE_FIRST]) == -1){
        return -1;
    }
    if(read_sequence_gold(gold_end, argv[SEQUENCE_END]) == -1){
        return -1;
    }
#ifdef DEBUG
    printf("sequence gold first: \n");
    print_bit(gold_first, SIZE_SEQ_GOLD);
    printf("sequence gold end: \n");
    print_bit(gold_end, SIZE_SEQ_GOLD);
#endif
#ifdef DEBUG
    printf("Data:\n");
    print_data(data, size);
    print_bit(data, size);
    printf("Data + 0(%d):\n", SIZE_CRC-1);
    print_bit(data, size + SIZE_CRC);
    print_polinom_crc(polinom_crc);
    print_bit(polinom_crc, SIZE_CRC);
    printf("\n\n");
#endif
#if 0
    shift_bit(polinom_crc, SIZE_CRC);
    print_bit(polinom_crc, SIZE_CRC);
    return 0;
#endif
#if 0
    xor_bytes(data, polinom_crc, SIZE_CRC);
    print_bit(data, SIZE_CRC);
    return 0;
#endif
    calc_crc(data, polinom_crc, crc, size + SIZE_CRC);
    memcpy( (void*)(data + size), (void*)crc, SIZE_CRC);
#ifdef DEBUG
    printf("CRC:\n");
    print_bit(crc, SIZE_CRC);
    printf("Data + CRC:\n");
    print_bit(data, size + SIZE_CRC);
#endif
#ifdef DEBUG_CALC_CRC
    // data[1] |= 64;
    // printf("Data + CRC (trash):\n");
    // print_bit(data, size + SIZE_CRC);
    calc_crc(data, polinom_crc, crc, size + SIZE_CRC);
    printf("CRC:\n");
    print_bit(crc, SIZE_CRC);
    printf("End\n");
    return 0;
#endif //DEBUG_CALC_CRC
    memcpy(buffer, gold_first, SIZE_SEQ_GOLD);
    memcpy(data + size + SIZE_CRC, gold_end, SIZE_SEQ_GOLD);
#ifdef DEBUG
    printf("seq gold + data + CRC:\n");
    print_bit(buffer, SIZE_SEQ_GOLD + size + SIZE_CRC);
    printf("seq gold + data + CRC + seq gold end:\n");
    print_bit(buffer, SIZE_SEQ_GOLD + size + SIZE_CRC + SIZE_SEQ_GOLD);
#endif
    FILE *file = fopen(argv[RESULT], "w");
    print_bit_to_stream(buffer, SIZE_SEQ_GOLD + size + SIZE_CRC + SIZE_SEQ_GOLD, file);
    fclose(file);
    return 0;
}
#endif /*TEST*/













