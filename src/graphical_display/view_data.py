
import matplotlib.pyplot as plt
import numpy as np
import math

from rw_data import *



            

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






def view_data_3():
    filename_corr = "../data/corr_array3.bin"
    filename_corr = "../data/corr_array_convolve.bin" 
    filename_corr2 = "../data/corr_array2.bin"
    
    filename_slots = "../data/slots.bin"
    filename_slots_tx = "../data/slots_tx.bin"
    filename_rx_data = "../data/rx_sample.bin"
    filename_rx_ofdms = "../data/read_ofdms.bin"
    filename_rx_ofdms_no_cfo = "../data/read_ofdms_no_cfo.bin"
    
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
            print("\n\n\n\n")
            ofdm = data[0]
            ofdm = np.fft.fft(ofdm)
            ofdm = np.fft.fftshift(ofdm)

            

            print("len data - ", len(data))
            plt.subplot(2, 2, 2)
            plt.xlabel("Гц")
            plt.ylabel("Амплитуда")
            
            plt.title("Пример спектра одного OFDM символа 1")
            plt.plot(abs(ofdm))
    
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
    if 1:
        data = read_OFDMs(filename_rx_ofdms_no_cfo, 4)
        print("len data - ", len(data))

        # view_resourse_grid(0, data[1:], id_f, (2, 2, 2), "Принятый")
        view_resourse_grid(0, data, id_f, (2, 2, 3), "Принятый")
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
    # id_f += 1
    # plt.figure(id_f, figsize=(10,10))
    if 0:
        filename_est_cfo = "../data/est_cfo.bin"
        est = read_file_bin(filename_est_cfo, 4)
        plt.subplot(2, 2, 1)
        plt.title("est cfo")
        plt.plot(est)
        id_f += 1
        plt.figure(id_f, figsize=(10,10))
    if 0:
        data = read_OFDM_slots(filename_slots_tx, 4)
        pss = data[0][0]
        print(pss)
        pss = np.array(pss)
        plt.subplot(2, 2, 1)
        plt.title("Пример PSS")
        plt.xlabel("real")
        plt.ylabel("imag")        
        plt.scatter(pss.real, pss.imag)
        id_f += 1
    if 0:
        data = read_OFDM_slots(filename_slots_tx, 4)
        
        print("len data - ", len(data))
        for i in range(len(data)):
            print(f"len data[{i}] - ", len(data[i]))
        data_ofdms_no_pss = []
        for i in range(len(data)):
            if 1:
                print("\t len - ", len(data[i][1]))
                pss = data[i][0]
                f_len_arr = (count_subcarriers - len(data[i][j])) // 2
                val = -100+-100j
                val = 0
                pss = np.concatenate((pss, [val for i in range(52)]))
                pss = np.concatenate(([val for i in range(53)], pss))
                pss = pss / 10
                print("\t len - ", len(pss))
                print(pss)
                data_ofdms_no_pss.append(pss)
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
    


def view_data_4():
    filename_1 = "../data/dump_data/slots_rx.bin"
    
    filename_2 = "../data/dump_data/demod_ofdm.bin"
    filename_3 = "../data/dump_data/slots_tx.bin"
    filename_4 = "../data/dump_data/slots_tx_2.bin"
    f_5 = "../data/dump_data/rx_sample.bin"
    f_6 = "../data/dump_data/pilot_tx.bin"
    f_7 = "../data/dump_data/pilot_rx.bin"
    f_8 = "../data/dump_data/linearInterpolation.bin"
    f_9 = "../data/dump_data/ofdm_1.bin"
    f_10 = "../data/dump_data/ofdm_eq.bin"
    f_11 = "../data/dump_data/pilot_h.bin"
    id_f = 1
    plt.figure(id_f, figsize=(10,10))
    if 1:
        samples = array_float_to_np_complex(read_file_bin(filename_2, 4))
        plt.subplot(2, 2, 3)
        plt.title("QPSK")
        plt.scatter(samples.real, samples.imag)

    id_f += 1
    plt.figure(id_f, figsize=(10,10))


    if 1:
        samples = array_float_to_np_complex(read_file_bin(f_5, 4))
        plt.subplot(2, 2, 1)
        plt.title("Буфер")
        plt.plot(abs(samples))
    id_f += 1
    plt.figure(id_f, figsize=(10,10))

    if 1:
        data = read_OFDMs(filename_3, 4)
        print("len data - ", len(data))
        view_resourse_grid(0, data, id_f, (2, 2, 1), "Tx")
    if 1:
        data = read_OFDMs(filename_1, 4)
        print("len data - ", len(data))
        view_resourse_grid(0, data, id_f, (2, 2, 2), "Rx")

    
    if 1:
        cyclic_prefix = 40
        data = read_OFDM_slots(filename_4, 4)
        
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
        view_resourse_grid(cyclic_prefix, data_ofdms_no_pss, id_f,
                            (2, 2, 3), "Отправленный")

    id_f += 1
    plt.figure(id_f, figsize=(10,10))
    if 1:
        
        ptx = array_float_to_np_complex(read_file_bin(f_6, 4))
        plt.subplot(2, 2, 1)
        plt.title("Оценка канала")
        plt.plot(ptx, label = "pilot tx")
        prx = array_float_to_np_complex(read_file_bin(f_7, 4))
        plt.plot(prx, label = "pilot rx")
        ph = array_float_to_np_complex(read_file_bin(f_11, 4))
        plt.plot(ph, label = "pilot rx/tx - Оценка канала")
        plt.legend()

    if 1:
        li = array_float_to_np_complex(read_file_bin(f_8, 4))
        plt.subplot(2, 2, 2)
        plt.title("Линейная интерполяция к оценки канала")
        plt.plot(li)
        ofdm1 = array_float_to_np_complex(read_file_bin(f_9, 4))
        plt.subplot(2, 2, 3)
        plt.title("Принятый сигнал")
        # plt.title("T")
        plt.plot(ofdm1)
        ofdm_heq = array_float_to_np_complex(read_file_bin(f_10, 4))
        plt.subplot(2, 2, 4)
        plt.title("Коррекция сигнала")
        plt.plot(ofdm_heq)
    
