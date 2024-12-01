
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

struct bit_sequence{
    int size_buffer;/*byte*/
    int size_data;/*bit*/
    u_char *buffer;
};



struct header_phy {
    u_char command;
    u_int16_t len;
};


struct bit_sequence *generate_packet(u_char *data, int size);

#endif /*GENERATE_PACKET_H*/







