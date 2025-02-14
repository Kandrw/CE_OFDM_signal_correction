import matplotlib.pyplot as plt
import numpy as np
import os
import subprocess
import time
import binascii
import struct
import sys

from rw_data import *
from ipc_data import *
from model_channel import *
from view_data import *

def fast_test():
    tmp = array_float_to_np_complex(read_file_bin("../data/fast_test_1.bin", 4))
    id_f = 10
    plt.figure(id_f, figsize=(10,10))
    print(tmp)
    if 1:
        plt.subplot(2, 2, 1)
        plt.plot(abs(tmp.real))

print("Start visual_signal.py")

PRACTICE = -1

if PRACTICE == 0:
    RUN_ALL = 0
    
    file_names_data = [
    #"192.168.1.1",
        "../data/data_test.txt",
        "../data/modulation.bin",
        "../data/data_bit.txt"
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


targets = {
    "p3":view_graphic_ser,
    "p4":model_soft_solutions,
    "p5":view_graphic_ser_and_relay_channel,
    "p6":view_graphic_ser_multipath_channel,
    "rgr":view_graph_ofdm_signal_correction,
    "processing-commands":processing_commands,
    "vd1":view_data_1,
    "vd2":view_data_2,
    "vd3":view_data_3,
    "vd4":view_data_4,
    "ofdm_model_add_noise": ofdm_model_add_noise,
    "fast_test":fast_test
}


def main():
    key = sys.argv[1]
    if(key in targets):
        targets[key]()
    else:
        print("Not found target program")
        print("targets:")
        for k in targets.keys():
            print("\t", k)
    EXIT(1)


if(len(sys.argv) < 2):
    print("No target program")
    EXIT(0)

main()




print("End visual_signal.py")



EXIT(1)



















