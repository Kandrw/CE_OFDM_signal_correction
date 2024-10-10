#include <sstream>
#define CONSOLE 1
#define LOG 2
#define LOG_DATA 3
#define DEBUG 4
#define ERROR_OUT 5
#define LOG_DEVICE 6

// class Output{
// public:
//     Output();
//     static std::stringstream buffer;
//     static bool stream_cout; 
//     void print();
//     void clear();
//     Output *setColorStream(char fcolor, char bgcolor);
// };
// template<typename T> Output& operator << (Output &output, T t){
//     output.buffer<<t;
//     output.print();
//     output.clear();
//     return output;
// }


//extern Output output;

// void print_log(int out, const char* format, ...);
void print_log(int out, const char* format, ...);

int init_log(const char *log_filename);
int deinit_log();
int write_file_bin_data(const char *filename, const void *data, const int size);

void time_counting_start();
void time_counting_end(int out);


void print_to_file(const char *filename, 
                const char* key, 
                const char* format, ...);









