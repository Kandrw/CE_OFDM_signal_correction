import numpy as np
import struct
import matplotlib.pyplot as plt
import sys

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

def read_file_to_list_str(filename):
    with open(filename, "r") as file:
        data = file.readlines()
        data = data[0]
        return data.split(" ")

def read_OFDM_one_slot(filename, size_type):
    data = []
    with open(filename, "rb") as file:

        size = file.read(4)
        if(size == ""):
            return data
        
        slot = []
        size = int.from_bytes(size, byteorder='little')
        print("|size = ", size)
        byte_array = file.read(size * size_type * 2)
        numpy_array = np.frombuffer(byte_array, dtype=np.complex64)
        slot.append(numpy_array)
        size_sub = file.read(4)
        size_sub = int.from_bytes(size_sub, byteorder='little')
        print("||size = ", size_sub)
        size = file.read(4)
        size = int.from_bytes(size, byteorder='little')
        print("|||size = ", size)
        for i in range(0, size):
            byte_array = file.read(size_sub * size_type * 2)
            numpy_array = np.frombuffer(byte_array, dtype=np.complex64)
            print("|||size numpy_array = ", len(numpy_array))
            slot.append(numpy_array)
        data.append(slot)
    return data

def read_OFDM_slots(filename, size_type):
    data = []
    with open(filename, "rb") as file:

        count_slots = file.read(4)
        
        count_slots = int.from_bytes(count_slots, byteorder='little')
        print("count_slots =", count_slots)
        for k in range(count_slots):


            size = file.read(4)
            if(size == ""):
                return data
            
            slot = []
            size = int.from_bytes(size, byteorder='little')
            print("|size = ", size)
            byte_array = file.read(size * size_type * 2)
            numpy_array = np.frombuffer(byte_array, dtype=np.complex64)
            slot.append(numpy_array)
            size_sub = file.read(4)
            size_sub = int.from_bytes(size_sub, byteorder='little')
            print("||size = ", size_sub)
            size = file.read(4)
            size = int.from_bytes(size, byteorder='little')
            print("|||size = ", size)
            for i in range(0, size):
                byte_array = file.read(size_sub * size_type * 2)
                numpy_array = np.frombuffer(byte_array, dtype=np.complex64)
                print("|||size numpy_array = ", len(numpy_array))
                slot.append(numpy_array)
            data.append(slot)
    return data


def read_OFDMs(filename, size_type):
    data = []
    with open(filename, "rb") as file:

        size_sub = file.read(4)
        if(size_sub == ""):
            return data
        size_sub = int.from_bytes(size_sub, byteorder='little')
        print("||size = ", size_sub)
        size = file.read(4)
        size = int.from_bytes(size, byteorder='little')
        print("|||size = ", size)
        for i in range(0, size):
            byte_array = file.read(size_sub * size_type * 2)
            numpy_array = np.frombuffer(byte_array, dtype=np.complex64)
            print("|||size numpy_array = ", len(numpy_array))
            data.append(numpy_array)
    return data

def read_config(filename):
    config = {}
    with open(filename, "r") as file:
        for line in file:
            line = line[:-1]
            line = line.split(":")
            line[0] = line[0].lstrip()
            if(len(line) > 1):
                line[1] = line[1][1:]
            if(len(line) > 1 and line[1] == "{"):
                pass
            elif len(line) == 1:
                pass
            else:
                config[line[0]] = line[1]
    print(config)
    return config

def array_float_to_np_complex(array):
    array_complex = []
    for i in range(0, len(array), 2):
        array_complex.append(array[i][0] + 1j * array[i+1][0])
    return np.array(array_complex)


def view_resourse_grid(del_cyclic_prefix, data, \
                          id_fig = 1, subplot = "None",\
                          title = "Мощность каждой поднесущей в OFDM",\
                          show = False,
                          ):
    table = []
    for i in range(len(data)):
        # if i > 1000 and i < 1200:
        #     table.append( np.fft.fft(data[i][del_cyclic_prefix:] * 1.3) )
        # else:
        res = np.fft.fft(data[i][del_cyclic_prefix:])
        res = np.fft.fftshift(res)
        table.append( res )
    table = np.array([table]).T
    power_amplitude = np.abs(table)
    plt.figure(id_fig, figsize=(10,10))
    if subplot != "None":
        plt.subplot(*subplot)
    # plt.imshow(power_amplitude, aspect='auto', cmap='viridis', origin='lower')
    if(len(power_amplitude) == 0):
        print("view_resourse_grid: Error: len data = 0 ")
        return
    plt.imshow(power_amplitude, aspect='auto', cmap='jet', origin='lower')
    
    plt.colorbar(label='Амплитуда')
    plt.title(title)
    plt.ylabel('Гц')
    plt.xlabel('Временные символы')
    # plt.xticks(np.arange(0, num_subcarriers, step=8))  # Установка меток по оси X
    # plt.yticks(np.arange(0, num_symbols, step=2))      # Установка меток по оси Y
    plt.grid(False)
    if(show):
        plt.show()


def EXIT(show = 0):
    #plt.ion()
    #plt.pause(100)
    if(show):
        plt.show()
    sys.exit()
