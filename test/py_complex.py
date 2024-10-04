

import math
import cmath

import numpy as np






d1 = 1 + 1j

print(d1)
print(d1 **2)



d1 = -3 + 3j

print(d1)
print(d1 **2)

d2 = d1 / 2

print("d1", d1, "d1 / 2:", d2)

d2 = cmath.sqrt( d1 / 2)

print("d2:", d2)



dm1 = [1 + 1j for i in range(5)]
print(dm1)


dsum = sum(dm1)


print("sum = ", dsum)




d1 = -3 + 4j

print("abs() = ", abs(d1))



data = [1 + 1j, 30 + 3j]

data = np.abs(data)

data = data**2

print(data)

