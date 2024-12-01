from context import *
import numpy as np
import matplotlib.pyplot as plt


def freq_cp_offset(data, cp, Nfft): # приходит окоеллированный массив по псс
    
    print("len_data: ",len(data))
    data = data[:5600]
    data = del_pss_in_frame(data, Nfft,cp)
    symbol_length = Nfft + cp
    num_symbols = len(ofdm) // symbol_length

    # Список для хранения оценок CFO для каждого символа
    cfo_estimates = []

    # Оценка CFO для каждого OFDM символа
    for i in range(num_symbols):
        # Извлекаем один OFDM символ
        symbol = ofdm[i * symbol_length : (i + 1) * symbol_length]
        
        # циклический префикс и  часть символа
        cp_part = symbol[:cp]  # Циклический префикс
        data_part = symbol[Nfft:Nfft + cp]  # часть символа OFDM

        # Вычисляем CFO для текущего символа

        cfo_estimate = (1 / (2 * np.pi)) * np.angle(np.sum(np.conj(cp_part) * data_part))
        cfo_estimates.append(cfo_estimate)

    # sum_estim = sum_estim
    average_cfo = np.mean(cfo_estimates)
    t = np.arange(0,len(data))
    t = t/Nfft

    return   data * np.exp(-1j * 2 * np.pi * average_cfo * t)

def freq_cp_offset2(data, cp, Nfft): # приходит окоеллированный массив по псс
    
    print("len_data: ",len(data))
    data = data[:5600]
    data = del_pss_in_frame(data, Nfft,cp)
    # resource_grid(data,5,5, N_fft, CP)
    mass_estim = []
    for i in range(len(data)//(Nfft+cp)):
        cfo_estim = np.sum(np.angle(data[i*(N_fft+cp):i*(N_fft+cp)+cp] * np.conj(data[i*(N_fft+cp)+N_fft:i*(N_fft+cp)+(cp+Nfft)])))
        mass_estim.append(abs(cfo_estim))

    sum_estim = 0

    data_cfo = np.zeros(0)
    t = np.arange(0,Nfft+cp)
    t = t 
    for i in range(len(mass_estim)):

        data_cor = data[i*160:(i*160)+160] * np.exp(-1j * 2 * np.pi * mass_estim[i] * t)
        data_cfo = np.concatenate([data_cfo, data_cor])

    return   data_cfo


def estimate_cfo(ofdm, N=128, N_G=32):
    
    ofdm = ofdm[:5600]

    symbol_length = N + N_G
    num_symbols = len(ofdm) // symbol_length

    cfo_estimates = []

    for i in range(num_symbols):

        symbol = ofdm[i * symbol_length : (i + 1) * symbol_length]

        cp_part = symbol[:N_G] 
        data_part = symbol[N:N + N_G] 

        # Вычисляем CFO для текущего символа
        cfo_estimate = (1 / (2 * np.pi)) * np.angle(np.sum(np.conj(cp_part) * data_part))
        cfo_estimates.append(cfo_estimate)

    # Усредняем оценки CFO по всем символам для снижения шума
    average_cfo = np.mean(cfo_estimates)
   
    t = np.arange(0,len(ofdm))
    t = t / N

    return   ofdm * np.exp(-1j * 2 * np.pi * average_cfo * t)

def sigma(snr,samples):

    h2 = 10**(0.1*snr)
    Ps = np.sum(abs(samples)**2)/len(samples)
    sigma = np.sqrt(Ps/h2)

    return sigma

N_fft = 128
GB_len = 55
CP = 32
N_pilot = 6

FILE = 1

if FILE == 1:
    path = "C:\\Users\\Ivan\\Desktop\\lerning\\OFDM\\OFDM_TX_RX\\test\\resurs_tx\\tx_file.py"


bits = converted_file_to_bits(path)

len_packet = len(bits)
print("len_bit_tx: ", len_packet)

QAM = QPSK(bits)

ofdm = modulation(N_fft, CP,GB_len, QAM, N_pilot, amplitude_all= 1, amplitude_data=1, amplitude_pss = 1, amplitude_pilots = 1)
resource_grid(ofdm,5,6, N_fft, CP)
ofdm = np.concatenate([ofdm, np.zeros(2000)])
snr = 27
sig = sigma(snr, ofdm)
print("snr = ",snr ,"sigma = ", sig)

noise = np.random.normal(0,sig, len(ofdm)) + 1j * np.random.normal(0,sig, len(ofdm))
# print(noise)

rx = (ofdm + noise)

offset = -3245
t = np.arange(0,len(rx))

t = t / 1920000


data_offset = rx * np.exp(-1j * 2 * np.pi * offset * t)

resource_grid(data_offset,5,6, N_fft, CP)

data = corr_pss_time(data_offset,N_fft)

# data_cor = calculate_correlation(N_fft, data, 15000) #

# data_cor = freq_cp_offset(data, CP, N_fft)
# data_cor2 = freq_cp_offset2(data, CP, N_fft)
data_cor = estimate_cfo(data)
# print("Средняя оценка частотного смещения (CFO):", cfo)
# print("datacor = ",len(data_cor))

resource_grid(data_cor,5,6, N_fft, CP)
# resource_grid(data_cor2,5,6, N_fft, CP)
# # data_cor = data_cor[(N_fft + CP):]

data1 = data_cor[:(N_fft+CP) * 5 + CP]

print("data 1 ",len(data1))

count_slots = get_inform_slot_bit10(data1, N_fft, N_pilot, GB_len, 2, CP)

print("count slot= ", count_slots)

ofdm2 = data_cor[:((960*count_slots)+800)]

ofdm2 = del_pss_in_frame(ofdm2, N_fft,CP)

slots, broken_slot = decode_slots_bit10(ofdm2, N_fft, CP, GB_len,count_slots,N_pilot)
print(broken_slot)
bits = slots

print("len_bit_rx: ", len(bits))

if FILE == 1:   
    path_final_file = "C:\\Users\\Ivan\\Desktop\\lerning\\OFDM\\OFDM_TX_RX\\test\\resurs_rx\\rx_file.py"
if FILE == 2:
    path_final_file = "C:\\Users\\Ivan\\Desktop\\lerning\\OFDM\\OFDM_TX_RX\\test\\resurs_rx\\imag_rx.jpg"
if FILE == 3:
    path_final_file = "C:\\Users\\Ivan\\Desktop\\lerning\\OFDM\\OFDM_TX_RX\\test\\resurs_rx\\rx.jpg"



converted_bits_to_file(bits, path_final_file)

