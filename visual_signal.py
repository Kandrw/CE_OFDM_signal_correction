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
    plt.semilogy(db_list, ser_list, label = "Noise channel")
    con = 0
    if 0:
        for i in range(len(db_list)):
            if(i > 30 and i < 35):
                db_list[i] -= con#math.cos(con)
                con += 5.5
                ser_relay[i] += math.tan(con)
            if(i >= 35 and i < 40):
                db_list[i] -= con#math.cos(con)
                con -= 5.5
                ser_relay[i] += math.tan(con)
    plt.semilogy(db_list, ser_relay, c="r", label="Relay channel")
    
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
# 6
def view_graphic_ser_multipath_channel():
    ser_list = []
    zero_forcing_list = []
    snr_list = []
    file = "../data/ser.txt"
    data = 0
    with open(file, "r") as file:
        data = file.readlines()
    for i in range(len(data)):
        aa = (data[i][:-1]).split(" ")
        ser_list.append(float(aa[0]))
        snr_list.append(int(aa[1]))
        zero_forcing_list.append(float(aa[2]))

    plt.semilogy(snr_list, ser_list, label = "ser")
    plt.semilogy(snr_list, zero_forcing_list, c="r", label = "Zero Forcing")
    plt.legend()
    plt.title("BER soft and hard for QAM16")
    plt.xlabel("SNR")
    plt.ylabel("error")
    # filename_s = "../data/samples/rx_50"
    # samples = array_float_to_np_complex(read_file_bin(filename_s, 4))

    # plt.scatter(samples.real, samples.imag)

def view_graph_ofdm_signal_correction():
    filename_samples_tx = "../data/ofdm_signal_correction/samples_modulation_tx.bin"
    filename_samples_rx = "../data/ofdm_signal_correction/samples_modulation_rx.bin"
    
    filename_ofdm_tx = "../data/ofdm_signal_correction/tx_ofdms.bin"
    filename_ofdm_rx = "../data/ofdm_signal_correction/rx_ofdms.bin"

    filename_ber = "../data/ofdm_signal_correction/bers.txt"
    filename_ber_qam_16 = "../data/ofdm_signal_correction/bers_qam16.txt"
    
    ciclic_prefix = 40
    CON_VIEW = 1
    if CON_VIEW == 1:
        id_f = 10
        plt.figure(id_f, figsize=(10,10))
        plt.subplot(2, 2, 1)
        samples = array_float_to_np_complex(
            read_file_bin(filename_samples_tx, 4))
        plt.scatter(samples.real, samples.imag)

        plt.subplot(2, 2, 2)
        samples = array_float_to_np_complex(
            read_file_bin(filename_samples_rx, 4))
        plt.scatter(samples.real, samples.imag)


        data = read_OFDMs(filename_ofdm_tx, 4)
        print("len data - ", len(data))

        view_resourse_grid(ciclic_prefix, data, id_f, (2, 2, 3), "Отправленный ofdm")

        data = read_OFDMs(filename_ofdm_rx, 4)
        print("len data - ", len(data))

        view_resourse_grid(ciclic_prefix, data, id_f, (2, 2, 4), "Принятый ofdm")

        # view_resourse_grid(cyclic_prefix, data[1:], id_f, (2, 2, 3), "Принятый синхронизированный по PSS")
    elif CON_VIEW == 2:
        hard_list = []
        snr_list = []
        with open(filename_ber, "r") as file:
            for data in file:
                aa = (data[:-1]).split(" ")
                snr_list.append(float(aa[0]))
                hard_list.append(float(aa[1]))
        hard_list_qam16 = []
        snr_list_qam16 = []
        with open(filename_ber_qam_16, "r") as file:
            for data in file:
                aa = (data[:-1]).split(" ")
                snr_list_qam16.append(float(aa[0]))
                hard_list_qam16.append(float(aa[1]))
              
        plt.semilogy(snr_list, hard_list, c="r", label = "QPSK")
        plt.semilogy(snr_list_qam16, hard_list_qam16, c="g", label = "QAM16")
        plt.legend()
        plt.title("BER")
        plt.xlabel("SNR")
        plt.ylabel("error")
        if 1:
            
            # plt.legend()
            plt.title("BER")
            plt.xlabel("SNR")
            plt.ylabel("error")
            

def view_data_1():
    filename_corr = "../data/corr_array3.bin"
    filename_corr = "../data/corr_array_convolve.bin" 
    filename_corr2 = "../data/corr_array2.bin"
    
    filename_slots = "../data/slots.bin"

    filename_rx_data = "../data/rx_sample.bin"
    id_f = 10
    if 0:
        plt.figure(id_f-1, figsize=(10,10))
        plt.subplot(2, 2, 1)
        pss = array_float_to_np_complex(read_file_bin("../data/pss.bin", 4))
        plt.scatter(pss.real, pss.imag)

    # corr = read_file_bin(filename_corr, 4)
    corr = array_float_to_np_complex(read_file_bin(filename_corr, 4))
    
    corr = np.array(corr)

    plt.figure(id_f, figsize=(10,10))
    plt.subplot(2, 2, 1)
    corr = np.abs(corr)
    plt.plot(corr)
    print("len corr = ", len(corr))

    corr2 = read_file_bin(filename_corr2, 4)
    corr2 = np.array(corr2)

    plt.subplot(2, 2, 2)
    corr2 = np.abs(corr2)
    plt.plot(corr2, "r")
    # import numpy as np

    # dx, dy = 0.05, 0.05

    # y, x = np.mgrid[slice(1, 5 + dy, dy),
    #                 slice(1, 5 + dx, dx)]

    # z = np.sin(x)**10 + np.cos(10 + y*x) * np.cos(x)

    data = read_OFDM_slots(filename_slots, 4)
    data = data[0]
    print("size pss = ",len(data[0]),"count ofdm = ", len(data[1]))
    # print("PSS:\n",data[0])
    print("1 ofdm:\n", data[1])

    # return -1
    if 1:
        samples = array_float_to_np_complex(read_file_bin("../data/orig_sample.bin", 4))
        samples_noise = array_float_to_np_complex(read_file_bin("../data/model_sample.bin", 4))
        plt.subplot(2, 2, 3)
        plt.plot(samples)
        plt.subplot(2, 2, 4)
        plt.plot(samples_noise)
    if 1:
        plt.figure(id_f+4, figsize=(10,10))
        plt.subplot(2, 2, 1)
        # c1 = array_float_to_np_complex(read_file_bin("../data/auto_corr.bin", 4))
        # plt.plot(abs(c1))
        # plt.subplot(2, 2, 2)
        # c1 = array_float_to_np_complex(read_file_bin("../data/norm_corr.bin", 4))
        # plt.plot(abs(c1))
        # plt.subplot(2, 2, 3)
        c1 = read_file_bin("../data/norm_corr_ofdm.bin", 4)
        c1 = np.array(c1)
        plt.plot(abs(c1))

        ofdms = read_OFDMs("../data/read_ofdms.bin", 4)
        print("kkk", len(ofdms), ofdms)
        
        ofdm = np.fft.fft(ofdms)
        print("kkk", len(ofdm), ofdm)
        ofdm = np.array([ofdm]).T
        # Проверка на пустой массив
        if ofdm.size == 0:
            print("Массив мощности пуст.")
        else:
            # Получение амплитуды
            power_amplitude = np.abs(ofdm)
            # plt.figure(id_f + 5, figsize=(10,10))
            plt.subplot(2, 2, 2)
            # Визуализация plt.figure(figsize=(12, 6))
            plt.imshow(power_amplitude, aspect='auto', cmap='viridis', origin='lower')
            plt.colorbar(label='Мощность (амплитуда)')
            plt.title('Мощность каждой поднесущей в OFDM')
            plt.ylabel('Поднесущие')
            plt.xlabel('Временные символы')
            # plt.xticks(np.arange(0, num_subcarriers, step=8))  # Установка меток по оси X
            # plt.yticks(np.arange(0, num_symbols, step=2))      # Установка меток по оси Y
            plt.grid(False)
            # plt.show()
        
        if 1:
            samples = array_float_to_np_complex(read_file_bin(filename_rx_data, 4))
            plt.subplot(2, 2, 3)
            plt.plot(abs(samples))

    if 0:

        # Параметры OFDM
        # num_subcarriers = 64  # Количество поднесущих
        # num_symbols = 10      # Количество временных символов

        # # Генерация случайной комплексной мощности для каждой поднесущей
        # power = np.random.rand(num_symbols, num_subcarriers) + 1j * np.random.rand(num_symbols, num_subcarriers)
        # print("kkk", len(power), power[0])
        power = data[1][15:]
        print("kkk", len(power), power)
        
        power = np.fft.fft(power)
        print("kkk", len(power), power)
        power = np.array([power]).T
        # Проверка на пустой массив
        if power.size == 0:
            print("Массив мощности пуст.")
        else:
            # Получение амплитуды
            power_amplitude = np.abs(power)
            plt.figure(id_f + 5, figsize=(10,10))
            plt.subplot(2, 2, 1)
            # Визуализация plt.figure(figsize=(12, 6))
            plt.imshow(power_amplitude, aspect='auto', cmap='viridis', origin='lower')
            plt.colorbar(label='Мощность (амплитуда)')
            plt.title('Мощность каждой поднесущей в OFDM')
            plt.ylabel('Поднесущие')
            plt.xlabel('Временные символы')
            # plt.xticks(np.arange(0, num_subcarriers, step=8))  # Установка меток по оси X
            # plt.yticks(np.arange(0, num_symbols, step=2))      # Установка меток по оси Y
            plt.grid(False)
            plt.show()



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
    
    plt.colorbar(label='Мощность (амплитуда)')
    plt.title(title)
    plt.ylabel('Поднесущие')
    plt.xlabel('Временные символы')
    # plt.xticks(np.arange(0, num_subcarriers, step=8))  # Установка меток по оси X
    # plt.yticks(np.arange(0, num_symbols, step=2))      # Установка меток по оси Y
    plt.grid(False)
    if(show):
        plt.show()
def view_data_2():
    filename_corr = "../data/corr_array3.bin"
    filename_corr = "../data/corr_array_convolve.bin" 
    filename_corr2 = "../data/corr_array2.bin"
    
    filename_slots = "../data/slots.bin"
    filename_slots_tx = "../data/slots_tx.bin"
    filename_rx_data = "../data/rx_sample.bin"
    filename_rx_ofdms = "../data/read_ofdms.bin"
    filename_test_rx_ofdms = "../data/test_ofdm_rx.bin"
    filename_samples_tx = "../data/samples_tx.bin"
    filename_corr_test = "../data/corr_array_test.bin"
    config = read_config("../configs/config.txt")
    print("config:\n", config)
    cyclic_prefix = int(config["cyclic_prefix"])
    count_subcarriers = int(config["count_subcarriers"])
    id_f = 10
    plt.figure(id_f, figsize=(10,10))
    if 1:
        samples = array_float_to_np_complex(read_file_bin(filename_rx_data, 4))
        plt.subplot(2, 2, 1)
        plt.title("Буфер")
        plt.plot(abs(samples))
        if 1:
            data = read_OFDMs(filename_rx_ofdms, 4)
            print("len data - ", len(data))
            # fft = []
            # for ofdm in data:
            #     fft1 = np.fft.fft(ofdm)
            #     fft += fft1
            plt.subplot(2, 2, 2)
            plt.title("Пример спектра одного OFDM символа")
            plt.plot(abs(np.fft.fft(data[0])))
    if 1:
        samples = array_float_to_np_complex(read_file_bin(filename_samples_tx, 4))
        plt.subplot(2, 2, 3)
        plt.title("Отправленный")
        plt.plot(abs(samples))
    if 1:
        corr = read_file_bin(filename_corr_test, 4)
        plt.subplot(2, 2, 4)
        plt.title("Корреляция")
        plt.plot(corr)

    id_f += 1
    plt.figure(id_f, figsize=(10,10))
    if 1:
        data = read_OFDM_slots(filename_slots_tx, 4)
        
        print("len data - ", len(data))
        for i in range(len(data)):
            print(f"len data[{i}] - ", len(data[i]))
        data_ofdms_no_pss = []
        for i in range(len(data)):
            for j in range(1, len(data[i])):
                # if(len(data[i][j]) < count_subcarriers):
                #     f_len_arr = (count_subcarriers - len(data[i][j])) // 2
                #     data[i][j] = np.concatenate((data[i][j], [0 for i in range(f_len_arr + 2 + cyclic_prefix)]))
                #     data[i][j] = np.concatenate(([0 for i in range(f_len_arr)], data[i][j]))
                #     data[i][j] = data[i][j][:count_subcarriers + cyclic_prefix]
                # data_ofdms_no_pss.append(data[i][j])
                # print(f"len s - {len(data[i][j])}")
                data_ofdms_no_pss.append(data[i][j])
        data0 = data[0]
        print("size pss = ",len(data0[0]),"count ofdm = ", len(data0[1]))
        print(type(data0[1:]))
        view_resourse_grid(cyclic_prefix, data_ofdms_no_pss, id_f, (2, 2, 1), "Отправленный")
    if 1:
        data = read_OFDMs(filename_rx_ofdms, 4)
        print("len data - ", len(data))

        # view_resourse_grid(0, data[1:], id_f, (2, 2, 2), "Принятый")
        view_resourse_grid(0, data, id_f, (2, 2, 2), "Принятый")
        
    if 0:
        data = read_OFDMs(filename_test_rx_ofdms, 4)
        print("len data - ", len(data))

        view_resourse_grid(cyclic_prefix, data[1:], id_f, (2, 2, 3), "Принятый синхронизированный по PSS")
    
    id_f += 1



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

def ofdm_model_add_noise():
    filename_corr_pss = "../data/corr_array_convolve.bin" 
    filename_test_corr = "../data/corr_array2.bin"

    filename_slots_tx = "../data/ofdm_model_add_noise/slots_tx.bin"
    filename_signal_tx = "../data/ofdm_model_add_noise/signal_tx.bin"
    filename_signal_rx = "../data/ofdm_model_add_noise/signal_rx.bin"
    filename_test_ofdm_rx = "../data/test_ofdm_rx.bin"
    filename_rx_ofdms = "../data/read_ofdms.bin"
    filename_noise = "../data/noise.bin"

    config = read_config("../data/ofdm_model_add_noise/config.txt")
    cyclic_prefix = int(config["cyclic_prefix"])
    id_f = 10
    plt.figure(id_f, figsize=(10,10))

    if 1:
        pss = array_float_to_np_complex(read_file_bin(filename_corr_pss, 4))
        plt.subplot(2, 2, 1)
        plt.title("Корреляция PSS")
        plt.plot(pss)
        if 1:
            pss = read_file_bin(filename_test_corr, 4)
            plt.subplot(2, 2, 2)
            plt.title("тестоввая корреляция PSS")
            plt.plot(pss)

    id_f += 1
    plt.figure(id_f, figsize=(10,10))

    if 1:
        data = read_OFDM_slots(filename_slots_tx, 4)
        print("len data - ", len(data))
        for i in range(len(data)):
            print(f"len data[{i}] - ", len(data[i]))
        data0 = data[0]
        print("size pss = ",len(data0[0]),"count ofdm = ", len(data0[1]))

        view_resourse_grid(cyclic_prefix, data0[1:], id_f, (2, 2, 1), "Отправленный OFDM")
    if 1:
        data = read_OFDMs(filename_rx_ofdms, 4)
        print("len data - ", len(data))

        view_resourse_grid(0, data[1:], id_f, (2, 2, 2), "Принятый")
    if 1:
        data = read_OFDMs(filename_test_ofdm_rx, 4)
        print("len data - ", len(data))

        view_resourse_grid(cyclic_prefix, data[1:], id_f, (2, 2, 3), "Принятый 2")
    id_f += 1
    plt.figure(id_f, figsize=(10,10))
    if 1:
        
        samples = array_float_to_np_complex(read_file_bin(filename_signal_tx, 4))
        samples100 = samples[:50]
        print(samples100)
        plt.subplot(2, 2, 3)
        plt.title("Отправленный сигнал во временной области")
        # plt.plot(abs(samples))
        plt.plot(abs(samples))
        
    if 1:
        samples = array_float_to_np_complex(read_file_bin(filename_signal_rx, 4))
        plt.subplot(2, 2, 4)
        plt.title("Принятый сигнал во временной области")
        plt.plot(abs(samples))
    if 1:
        noise = array_float_to_np_complex(read_file_bin(filename_noise, 4))
        plt.subplot(2, 2, 1)
        plt.title("Шум")
        plt.plot(noise)
    id_f += 1


def view_data_3():
    filename_corr = "../data/corr_array3.bin"
    filename_corr = "../data/corr_array_convolve.bin" 
    filename_corr2 = "../data/corr_array2.bin"
    
    filename_slots = "../data/slots.bin"
    filename_slots_tx = "../data/slots_tx.bin"
    filename_rx_data = "../data/rx_sample.bin"
    filename_rx_ofdms = "../data/read_ofdms.bin"
    filename_test_rx_ofdms = "../data/test_ofdm_rx.bin"
    filename_samples_tx = "../data/samples_tx.bin"
    filename_corr_test = "../data/corr_array_test.bin"
    filename_mod_tx = "../data/samples_modulation_tx.bin"
    filename_mod_rx = "../data/samples_modulation_rx.bin"
    
    config = read_config("../configs/config.txt")
    print("config:\n", config)
    cyclic_prefix = int(config["cyclic_prefix"])
    count_subcarriers = int(config["count_subcarriers"])
    id_f = 10
    plt.figure(id_f, figsize=(10,10))
    if 1:
        samples = array_float_to_np_complex(read_file_bin(filename_rx_data, 4))
        plt.subplot(2, 2, 1)
        plt.title("Буфер")
        plt.plot(abs(samples))
        if 1:
            data = read_OFDMs(filename_rx_ofdms, 4)
            print("len data - ", len(data))
            plt.subplot(2, 2, 2)
            plt.title("Пример спектра одного OFDM символа")
            plt.plot(abs(np.fft.fft(data[0])))
    if 1:
        samples = array_float_to_np_complex(read_file_bin(filename_samples_tx, 4))
        plt.subplot(2, 2, 3)
        plt.title("Отправленный")
        plt.plot(abs(samples))
    if 1:
        corr = read_file_bin(filename_corr_test, 4)
        plt.subplot(2, 2, 4)
        plt.title("Корреляция")
        plt.plot(corr)

    id_f += 1
    plt.figure(id_f, figsize=(10,10))
    if 1:
        data = read_OFDM_slots(filename_slots_tx, 4)
        
        print("len data - ", len(data))
        for i in range(len(data)):
            print(f"len data[{i}] - ", len(data[i]))
        data_ofdms_no_pss = []
        for i in range(len(data)):
            for j in range(1, len(data[i])):
                # if(len(data[i][j]) < count_subcarriers):
                #     f_len_arr = (count_subcarriers - len(data[i][j])) // 2
                #     data[i][j] = np.concatenate((data[i][j], [0 for i in range(f_len_arr + 2 + cyclic_prefix)]))
                #     data[i][j] = np.concatenate(([0 for i in range(f_len_arr)], data[i][j]))
                #     data[i][j] = data[i][j][:count_subcarriers + cyclic_prefix]
                # data_ofdms_no_pss.append(data[i][j])
                # print(f"len s - {len(data[i][j])}")
                # data[i][j] = np.fft.fftshift(data[i][j])
                data_ofdms_no_pss.append(data[i][j])
        data0 = data[0]
        print("size pss = ",len(data0[0]),"count ofdm = ", len(data0[1]))
        print(type(data0[1:]))
        view_resourse_grid(cyclic_prefix, data_ofdms_no_pss, id_f, (2, 2, 1), "Отправленный")
    if 1:
        data = read_OFDMs(filename_rx_ofdms, 4)
        print("len data - ", len(data))

        # view_resourse_grid(0, data[1:], id_f, (2, 2, 2), "Принятый")
        view_resourse_grid(0, data, id_f, (2, 2, 2), "Принятый")
        
    if 0:
        data = read_OFDMs(filename_test_rx_ofdms, 4)
        print("len data - ", len(data))

        view_resourse_grid(cyclic_prefix, data[1:], id_f, (2, 2, 3), "Принятый синхронизированный по PSS")
    
    # id_f += 1
    # plt.figure(id_f, figsize=(10,10))
    if 1:
        if 0:
            samples = array_float_to_np_complex(read_file_bin(filename_mod_tx, 4))
            plt.subplot(2, 2, 3)
            plt.title("Отправленные QPSK")
            plt.scatter(samples.real, samples.imag)

        samples = array_float_to_np_complex(read_file_bin(filename_mod_rx, 4))
        plt.subplot(2, 2, 4)
        plt.title("Принятые QPSK")
        plt.scatter(samples.real, samples.imag)
    id_f += 1
    # plt.figure(id_f, figsize=(10,10))
    if 0:
        filename_est_cfo = "../data/est_cfo.bin"
        est = read_file_bin(filename_est_cfo, 4)
        plt.subplot(2, 2, 1)
        plt.title("est cfo")
        plt.plot(est)


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
    "ofdm_model_add_noise": ofdm_model_add_noise

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



















