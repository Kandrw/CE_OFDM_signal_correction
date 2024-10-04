
#ifndef GENERATE_PACKET_H
#define GENERATE_PACKET_H
#define SIZE_DATA 1000//256 //byte
#define SIZE_CRC 4// byte
#define SIZE_SEQ_GOLD 4 //byte
#define SIZE_BUFFER ( SIZE_SEQ_GOLD + SIZE_DATA + SIZE_CRC) //byte
#define BYTE 8 //bit

typedef unsigned char u_char;

enum position_argv{
    DATA = 1,
    SEQUENCE_FIRST,
    SEQUENCE_END,
    RESULT,
    COUNT_ARGV
};

struct data_packet_v2{
    int size;
    u_char buffer[SIZE_BUFFER];
};


int generate_packet(struct data_packet_v2 *packet, int argc, char *argv[]);

#endif /*GENERATE_PACKET_H*/







