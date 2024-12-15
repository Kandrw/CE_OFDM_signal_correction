
import numpy as np
import matplotlib.pyplot as plt




N = 128

ofdm = [i for i in range(100)]

t = np.arange(0,len(ofdm))
t = t / N

print(t)

average_cfo = 0.9

rrr =  np.exp(-1j * 2 * np.pi * average_cfo * t)


print(rrr)

print(len(rrr))




t = np.arange(0,12)

print(t)





