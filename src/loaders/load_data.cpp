#include "load_data.hpp"

#include <fstream>


#include <output.hpp>




int *read_bin_type_int(std::string fname, int N){
    FILE *filew = fopen(fname.c_str(), "rb");
    if(!filew){
        print_log(CONSOLE, "[Error load] not found - %s\n", fname);
        return nullptr;
    }
    int *array = new int[N];
    int size = fread(array, sizeof(int), N, filew);
    if(size != N){
        delete[] array;
        fclose(filew);
        print_log(CONSOLE, "[Error read] reading data <%s> error: N = %d, read(data) = %d\n", fname.c_str(), N, size);
        return nullptr;
    }
    fclose(filew);
    return array;
}

double *read_bin_type_double(std::string fname, int N){
    FILE *filew = fopen(fname.c_str(), "rb");
    if(!filew){
        print_log(CONSOLE, "[Error load] not found - %s\n", fname.c_str());
        return nullptr;
    }
    double *array = new double[N];
    int size = fread(array, sizeof(double), N, filew);
    if(size != N){
        delete[] array;
        fclose(filew);
        //output<<"[Error read] reading data<"<<fname<<"> error: N = "<<N<<", read(data) = "<<size<<"\n";
        print_log(CONSOLE, "[Error read] reading data <%s> error: N = %d, read(data) = %d\n", fname.c_str(), N, size);
        return nullptr;
    }
    fclose(filew);
    return array;
}
float *read_bin_type_float(std::string fname, int N){
    FILE *filew = fopen(fname.c_str(), "rb");
    if(!filew){
        //output<<"[Error load] not found - "<<fname<<"\n";
        print_log(CONSOLE, "[Error load] not found - %s\n", fname.c_str());
        return nullptr;
    }
    float *array = new float[N];
    int size = fread(array, sizeof(float), N, filew);
    if(size != N){
        delete[] array;
        fclose(filew);
        //output<<"[Error read] reading data<"<<fname<<"> error: N = "<<N<<", read(data) = "<<size<<"\n";
        print_log(CONSOLE, "[Error read] reading data <%s> error: N = %d, read(data) = %d\n", fname.c_str(), N, size);
        return nullptr;
    }
    fclose(filew);
    return array;
}

u_char *read_file_data(const char *filename, int *ptr_size){
    // print_log(LOG, "[%s:%d] read_file_data:\n", __func__, __LINE__);
    FILE *file = fopen(filename, "rb");

    int size = 0;
    int res;
    if(!file){
        print_log(CONSOLE, "[main.cpp] read_file_data: not found %s\n", filename);
        return NULL;
    }
    res = fread((void*)&size, 1, sizeof(size), file);

#if 1
    if(res <= 0)
        print_log(LOG, "[%s:%d] : res = %d, size = %d\n",
            __func__, __LINE__, res, size);
#endif    
    u_char *buffer = new u_char[size];
    
    fread(buffer, 1, size, file);
    fclose(file);
    if(ptr_size)
        *ptr_size = size;
    return buffer;
}
