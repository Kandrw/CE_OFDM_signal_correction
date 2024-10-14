




import ctypes
import sysv_ipc

# создаем IPC ключ
key = sysv_ipc.ftok('shmfile', 65)

# открываем сегмент памяти
shm = sysv_ipc.SharedMemory(key)

# читаем из памяти
data = shm.read()
print("Данные: ", data.decode('utf-8'))
shm.write("345345")
# чистим сегмент
shm.detach()








