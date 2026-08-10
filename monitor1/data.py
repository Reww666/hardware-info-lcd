import psutil 
import serial
import time
import threading
import socket
import os

PORT = '/dev/ttyACM0'
try:
    ser = serial.Serial(PORT, 9600, timeout=1)
    time.sleep(2)
    print(f"Connecté à {PORT}")
except serial.SerialException as e:
    print(f"Erreur : {e}")
    exit(1)
    

GiB = (1024**3)

diskPath = [
    "/",
    "/mnt/data"
]

def get_ip():
    for interface, addrs in psutil.net_if_addrs().items():
        if interface == "lo":
            continue
        for addr in addrs:
            if addr.family == socket.AF_INET:
                return addr.address
    return None

def get_sys_info():
    hostname = socket.gethostname()
    kernel = os.uname().release
    distro = os.uname().sysname
    uptime = (time.time() - psutil.boot_time()) / 3600 # hours

    return hostname, kernel, distro, uptime

def get_temp():
    try:
        temps = psutil.sensors_temperatures()
        # Intel / AMD (via k10temp)
        if 'coretemp' in temps:
            return temps['coretemp'][0].current
        elif 'k10temp' in temps:
            return temps['k10temp'][0].current
        elif 'zenpower' in temps:
            return temps['zenpower'][0].current
        else:
            return None
    except Exception as e:
        print(f"Erreur température : {e}")
        return None

def send():
    while True:
        temp = get_temp()
        cpu = psutil.cpu_percent()

        ram = psutil.virtual_memory()
        ram_used = round(ram.used / GiB, 1)
        ram_total = round(ram.total / GiB)

        ip = get_ip()
        hostname, kernel, distro, uptime = get_sys_info()


        disk_parts = []
        for path in diskPath:
            disk = psutil.disk_usage(path)
            used = round(disk.used / GiB, 1)
            total = round(disk.total / GiB, 1)
            percent = round(disk.percent, 1)
            disk_parts.append(f"{used}|{total}|{percent}|{path}")

        temp_str = round(temp) if temp is not None else 0
        base = f"DATA:{temp_str}|{cpu:.1f}|{ram_used}|{ram_total}|{ip}|{hostname}|{kernel}|{distro}|{uptime:.2f}"
        msg = base + "|" + "|".join(disk_parts) + "\n"

        ser.write(msg.encode())
        time.sleep(2)

thread = threading.Thread(target=send, daemon=True)
thread.start()

while True:
    cmd = input("'k'(CPU/RAM) 'l'(<=)  'm'(=>) :  ")
    if cmd == "m":
        ser.write(b"CMD:M\n")
    elif cmd == "l":
        ser.write(b"CMD:L\n")
    elif cmd == "k":
        ser.write(b"CMD:K\n")
    time.sleep(0.5)