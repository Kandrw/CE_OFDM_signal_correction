import matplotlib.pyplot as plt
import numpy as np
import math

from rw_data import *




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
    CON_VIEW = 2
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

    if 0:
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
    if 0:
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
    plt.figure(id_f, figsize=(10,10))
    if 1:
        data = read_OFDM_slots(filename_slots_tx, 4)
        print("len data - ", len(data))
        for i in range(len(data)):
            print(f"len data[{i}] - ", len(data[i]))
        data0 = data[0]
        print("size pss = ",len(data0[0]),"count ofdm = ", len(data0[1]))
        plt.subplot(2, 2, 1)
        print(data0[1:][0])
        tmp = np.array([data0[3:][0]]).T
        plt.plot(abs(tmp))
        # view_resourse_grid(cyclic_prefix, data0[1:], id_f, (2, 2, 1), "Отправленный OFDM")














