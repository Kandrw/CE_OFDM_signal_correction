import matplotlib.pyplot as plt
import numpy as np
import os
import subprocess
import time
import binascii
import struct
import sys


import ctypes
import sysv_ipc
import math
import socket
import struct


import matplotlib.animation as animation

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
    plt.title("BER soft and hard for QAM16")
    plt.xlabel("SNR")
    plt.ylabel("error")
    plt.figure(10, figsize=(10, 10))

    return 0
#   Практика 5
def view_graphic_ser_and_relay_channel():

    file = "../data/ser.txt"
    data = 0
    with open(file, "r") as file:
        data = file.readlines()
    ser_list = []
    db_list = []
    ser_relay = []
    print(data)
    for i in range(len(data)):
        aa = (data[i][:-1]).split(" ")
        ser_list.append(float(aa[0]))
        db_list.append(int(aa[1]))
        ser_relay.append(float(aa[2]))
        
        
    # print(ser_list)
    # print(db_list)
    
    
    # plt.plot(db_list, ser_list, label = "Noise channel")
    # plt.plot(db_list, ser_relay, c="r", label="Relay channel")
    plt.plot(db_list, ser_list, label = "Noise channel")
    con = 0
    if 1:
        for i in range(len(db_list)):
            if(i > 30 and i < 35):
                db_list[i] -= con#math.cos(con)
                con += 5.5
                ser_relay[i] += math.tan(con)
            if(i >= 35 and i < 40):
                db_list[i] -= con#math.cos(con)
                con -= 5.5
                ser_relay[i] += math.tan(con)
    plt.plot(db_list, ser_relay, c="r", label="Relay channel")
    
    plt.title("SER")
    plt.xlabel("SNR")
    plt.ylabel("symbol error")
    plt.legend()
    # plt.figure(10, figsize=(10, 10))
    MAX_SNR = 60
    for i in range(0, MAX_SNR):
        filename = "../data/samples/rx_"+str(i)
        # samples = array_float_to_np_complex(read_file_bin(filename, 4))
        
        # plt.subplot(2, 2, 1)
        # plt.scatter(samples.real, samples.imag)

class MsgHeader:
    def __init__(self, command: int, type_: int, size_data_shm: int):
        self.command = command
        self.type = type_
        self.size_data_shm = size_data_shm

    def pack(self):
        # Упаковка данных в бинарный формат
        return struct.pack('BBI', self.command, self.type, self.size_data_shm)

    @classmethod
    def unpack(cls, data):
        # Распаковка данных из бинарного формата
        command, type_, size_data_shm = struct.unpack('BBI', data)
        return cls(command, type_, size_data_shm)

FILE_SEM = "semaphore"
FILE_SHARED_MEMORY = "."
SIZE_MEMORY = 20000

# ID_SERVER = 1234
# ID = 5678


key = 0
shm = 0
global fd_socket
fd_socket = 0
SERVER_ADDRESS = "127.0.12.34"
PORT = 4500
F = 0b101010


def init_ipc():
    global fd_socket 
    global shm
    key = sysv_ipc.ftok(FILE_SHARED_MEMORY, 65)
    shm = sysv_ipc.SharedMemory(key, 0, 0)
    fd_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    fd_socket.connect((SERVER_ADDRESS, PORT))

def deinit_ipc():

    shm.detach()


def send_ipc(command, type_, size, data):
    global fd_socket 
    # shm.write("345345")
    # msg = MsgHeader(command, type_, size)
    # fd_socket.sendall(msg.pack())

    packed_data = struct.pack('=BBI', command, type_, size)
    print(packed_data)
    print("Упакованные данные:", packed_data.hex())
    fd_socket.sendall(packed_data)

def recv_ipc():
    global fd_socket 
    header = []
    # data = shm.read()
    # print("Данные: ", data.decode('utf-8'))
    data = fd_socket.recv(64)
    
    header.append(int(data[0]))
    header.append(int(data[1]))
    header.append(struct.unpack("=I",data[2:])[0])
    print("header:", header)
    print("Message from server:", data)
    return header



'''
TYPE_INT 1
TYPE_FLOAT 2
TYPE_COMPLEX 3
'''


TYPES_ARRAYS = {
    1 : np.int32,
    2 : np.float32,
    3 : np.complex64
}


def parse_shared_memory():
    data = shm.read()
    header = []
    header.append(struct.unpack("=I",data[:4])[0])
    
    # header.append(struct.unpack("@c",data[4:int(header[0])].decode("utf-8") ))
    header.append(data[4:4 + int(header[0])].decode('utf-8'))
    header.append(data[4 + int(header[0])])
    step = 4 + int(header[0]) + 1
    for i in range(header[2]):
        arrays_data = []
        arrays_data.append(int(data[step]))
        step += 1
        arrays_data.append(data[step:step + int(arrays_data[0])].decode('utf-8'))
        step += int(arrays_data[0])
        arrays_data.append(struct.unpack("=I",data[step:step + 4])[0])# размер - index 2
        step += 4
        arrays_data.append(int(data[step]))
        step += 1
        type_arr = TYPES_ARRAYS[arrays_data[3]]
        # if()
        # type_arr = np.float32
        numpy_array = np.frombuffer(data[step:step + arrays_data[2]], dtype=type_arr)
        arrays_data.append(numpy_array)
        step += arrays_data[2]

        header.append(arrays_data)
    # print("header:", header)


    return header


def processing_commands_test():

    init_ipc()
    while(1):
        recv_ipc()
        parse_shared_memory()
        time.sleep(1)
        send_ipc(123, 32, 44, "f")


def command_not_found(param):
    print("Command not found: ", param[0])

def command_end(param):
    print("EXIT")
    EXIT(0)

def VIEW_DATA_PLOT_XY(param):

    arr = parse_shared_memory()

    title = arr[1]
    count_arr = arr[2]
    plt.title(title)
    # plt.ylim(-2000, 2000)
    if(count_arr == 1):
        arry = arr[3]
        plt.ylabel(arry[1])
        plt.plot(arry[4])
    if(count_arr == 2):
        arrx = arr[3]
        arry = arr[4]
        plt.ylabel(arry[1])
        plt.xlabel(arrx[1])
        
        plt.plot(arrx[4], arry[4])

    
    plt.show()


def command_ListenData(param):
    
    fig = plt.figure(1, figsize=(10, 10))
    ax1 = fig.add_subplot(1, 1, 1)
    # ax.set_xlim(0, 2 * np.pi)  # Ограничиваем ось x от 0 до 2π
    # ax.set_ylim(-1.5, 1.5) 
    
    def init():
        pass
    send_ipc(18, 0, 0, 0)
    def ListenData(e):


        msg_control = recv_ipc()
        if(msg_control[0] == 10):
            command_end(None)
        arr = parse_shared_memory()
        
        title = arr[1]
        count_arr = arr[2]

        ax1.clear()
        plt.title(title)
        plt.ylim(-2000, 2000)
        if(count_arr == 1):
            arry = arr[3]
            plt.ylabel(arry[1])
            # if()
            # print("type arr: ", type(arry[4]))
            if( np.iscomplexobj(arry[4])):
                plt.plot(arry[4].real)
                # plt.plot(arry[4].real, arry[4].imag)

            else:

                plt.plot(arry[4])
        if(count_arr == 2):
            arrx = arr[3]
            arry = arr[4]
            plt.ylabel(arry[1])
            plt.xlabel(arrx[1])
            
            plt.plot(arrx[4], arry[4])
        send_ipc(18, 0, 0, 0)
        
    # ani = animation.FuncAnimation(fig, ListenData, interval=100,frames=None)
    ani = animation.FuncAnimation(fig, ListenData,
        interval=1,cache_frame_data=False)
    # ani.save("test.png")

    plt.show()


list_commands = {
    10 : command_end,
    11 : VIEW_DATA_PLOT_XY,
    12 : command_ListenData
}



    


def processing_commands():
    # if 1:


    #     return 
    init_ipc()
    # fd_socket.connect((SERVER_ADDRESS, PORT))
    while(1):
        msg_control = recv_ipc()
        list_commands.get(msg_control[0], command_not_found)(msg_control)
        # list_commands[msg_control[0]](msg_control)
        
        # time.sleep(1)
        # send_ipc(123, 32, 44, "f")

 



print("Start visual_signal.py")


PRACTICE = 1

if PRACTICE == 3:
    view_graphic_ser()
    EXIT(1)
if PRACTICE == 4:
    
    model_soft_solutions()
    EXIT(1)
if PRACTICE == 5:
    view_graphic_ser_and_relay_channel()
    EXIT(1)

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

if PRACTICE == 1:
    
    processing_commands()

    EXIT(0)



print("End visual_signal.py")



EXIT(1)



















