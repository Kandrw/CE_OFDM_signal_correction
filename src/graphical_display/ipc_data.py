

import matplotlib.animation as animation
import ctypes
import sysv_ipc
import math
import socket
import struct
import matplotlib.pyplot as plt
import numpy as np
import time

from rw_data import *

FILE_SHARED_MEMORY = "."
SIZE_MEMORY = 20000


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
    if not data:
        EXIT(0)
    print(data)
    # if(len(data) < 6):
    #     return header
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
    if(count_arr == 2 and False):
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
        msg_control = []
        while(True):
            msg_control = recv_ipc()
            if(len(msg_control) > 0):
                break

        if(msg_control[0] == 10):
            command_end(None)
        arr = parse_shared_memory()
        
        title = arr[1]
        count_arr = arr[2]

        ax1.clear()
        plt.title(title)
        plt.ylim(-20, 2000)
        if(count_arr == 1):
            arry = arr[3]
            plt.ylabel(arry[1])
            # if()
            # print("type arr: ", type(arry[4]))
            if( np.iscomplexobj(arry[4])):
                plt.plot(abs(arry[4].real))
                # plt.plot(arry[4].real, arry[4].imag)

            else:

                plt.plot(arry[4])
            # arry = arr[3]
        if(count_arr == 2 and False):
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

