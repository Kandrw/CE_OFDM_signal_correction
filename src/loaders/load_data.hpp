
#include <fstream>

int *read_bin_type_int(std::string fname, int N);
double *read_bin_type_double(std::string fname, int N);
float *read_bin_type_float(std::string fname, int N);
u_char *read_file_data(const char *filename, int *ptr_size);