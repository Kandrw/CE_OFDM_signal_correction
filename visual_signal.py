import matplotlib.pyplot as plt
import numpy as np
import os
import subprocess
import time
import binascii
import struct
import sys

def EXIT(show = 0):
    #plt.ion()
    #plt.pause(100)
    if(show):
        plt.show()
    sys.exit()

def read_file_bin(filename, size_type):
    size = 0
    array = []
    with open(filename, "rb") as file:
        size = file.read(4)

        size = int.from_bytes(size, byteorder='little')
        byte_array = file.read(size)
        for i in range(0, size, size_type):
            # array.append(float.from_bytes(byte_array[i:i+size_type], byteorder='little'))
            array.append(struct.unpack('f',byte_array[i:i+size_type]))
    print("size =", size)
    # print(array)
    return array

def array_float_to_np_complex(array):
    array_complex = []
    for i in range(0, len(array), 2):
        array_complex.append(array[i][0] + 1j * array[i+1][0])
    return np.array(array_complex)
def read_file_to_list_str(filename):
    with open(filename, "r") as file:
        data = file.readlines()
        data = data[0]
        return data.split(" ")



#   Практика 3
def view_graphic_ser():

    file = "../data/ser.txt"
    data = 0
    with open(file, "r") as file:
        data = file.readlines()
    ser_list = []
    db_list = []
    print(data)
    for i in range(len(data)):
        aa = (data[i][:-1]).split(" ")
        ser_list.append(float(aa[0]))
        db_list.append(int(aa[1]))
        
    print(ser_list)
    print(db_list)
    
    
    plt.plot(db_list, ser_list)
    plt.title("SER")
    plt.xlabel("SNR")
    plt.ylabel("symbol error")
    plt.figure(10, figsize=(10, 10))
    for i in range(0, 25):
        filename = "../data/samples/rx_"+str(i)
        samples = array_float_to_np_complex(read_file_bin(filename, 4))
        
        # plt.subplot(2, 2, 1)
        plt.scatter(samples.real, samples.imag)

# Практика 4
def model_soft_solutions():
    filename_s = "../data/practice5/s.bin"
    filename_s_noise = "../data/practice5/s_noise.bin"

    samples = array_float_to_np_complex(read_file_bin(filename_s, 4))
    samples_noise = array_float_to_np_complex(read_file_bin(filename_s_noise, 4))
    


    plt.figure(10, figsize = (10, 10))

    if 0:
        plt.subplot(2, 2, 1)
        plt.plot(samples)
        plt.subplot(2, 2, 2)
        plt.plot(samples_noise)
        plt.subplot(2, 2, 3)
        plt.scatter(samples.real, samples.imag)
        plt.subplot(2, 2, 4)
        
        plt.scatter(samples_noise.real, samples_noise.imag)

    file = "../data/practice5/ber_soft.txt"
    data = 0
    with open(file, "r") as file:
        data = file.readlines()
    soft_list = []
    hard_list = []
    db_list = []
    print(data)
    for i in range(len(data)):
        aa = (data[i][:-1]).split(" ")
        soft_list.append(float(aa[0]))
        hard_list.append(float(aa[1]))
        db_list.append(int(aa[2]))
        
    print(soft_list)
    print(hard_list)
    print(db_list)
    plt.semilogy(db_list, soft_list, label = "Softbit")
    plt.semilogy(db_list, hard_list, c="r", label = "Hardbit")
    plt.legend()

    
    #plt.plot(db_list, ser_list)
    plt.title("BER soft and hard")
    plt.xlabel("SNR")
    plt.ylabel("symbol error")
    plt.figure(10, figsize=(10, 10))

    return 0







print("Start visual_signal.py")


PRACTICE = 4

if PRACTICE == 3:
    view_graphic_ser()
    EXIT(1)
if PRACTICE == 4:
    
    model_soft_solutions()
    EXIT(1)

if PRACTICE == 0:
    RUN_ALL = 0
    
    file_names_data = [
    #"192.168.1.1",
    "data/data_test.txt",
    "data/modulation.bin",
    "data/data_bit.txt"
    ]
    if RUN_ALL == 1:

        #os.system('mingw32-make -j16')
        #filename_program = "Program.exe"
        os.system('make -j16')
        
        filename_program = "./program"
            
        if(os.path.exists(filename_program)):
            subprocess.call(filename_program)
            #subprocess.call(filename_program + " " + file_names_data[0])
        else:
            print("Not found:", filename_program)
            EXIT(0)

    samples = array_float_to_np_complex(read_file_bin(file_names_data[1], 4))
    data_an = read_file_to_list_str(file_names_data[2])
    plt.figure(10, figsize=(10, 10))
    plt.subplot(2, 2, 1)
    plt.scatter(samples.real, samples.imag)
    if 1:
        for i in range(len(samples)):
            plt.text( samples[i].real, samples[i].imag, data_an[i])
            # print(samples[i].real, samples[i].imag, data_an[i])





print("End visual_signal.py")



EXIT(1)



















