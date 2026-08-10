# 📟 LCD Server Monitor

> Arduino-based display system showing real-time server metrics (CPU, RAM, disks) and currently playing music on a 16x2 LCD screen.
 
> [!NOTE]
> **Monitor1** handles **stats**
> 
> **Monitor2** handles **music** — *see each folder's script*

## 📋 Table of Contents
- [Hardware](#-hardware)
- [Features](#-features)
- [Installation](#-installation)
  - [Monitor 1 — Stats](#monitor1--stats-display)
  - [Monitor 2 — Music](#monitor2--music-display)

## 📦 Hardware

- 2x Arduino (Mega 2560 and/or Uno)
- 16x2 LCD with I2C backpack
- 2 push buttons (mode navigation)
- 10kΩ resistors (optional, internal pull-up used)
- PC / server running Python scripts

> [!TIP]
> [Wiring diagram (with 2 Arduino uno)](wiring_diagram.pdf)


## 🔧 Features

- **Mode 0** : CPU usage, temperature, RAM usage
- **Modes 1 to N** : disk usage per mount point (used/total/percentage)
- **Last mode** : system info (IP, hostname, kernel, uptime)
- **Navigation** : physical buttons to switch between modes
- **Music display** : scrolls title and artist with pause/resume


## 🐍 Python side (data sender)

- Collects system metrics with `psutil`
- Sends data over serial to Arduino
- Format : `DATA:temp|cpu|ram_used|ram_total|ip|hostname|kernel|distro|uptime|disk1_used|disk1_total|disk1_percent|disk1_path|...`
- Fetches current music metadata using `playerctl`


## 🔌 Installation

### Monitor1 — Stats display

1. **Arduino** : upload `monitor1/monitor1.ino` using [arduino-cli](https://github.com/arduino/arduino-cli)
2. **PC** : install Python dependencies
```bash
   pip install psutil pyserial
```
3. Edit the `PORT` variable in `monitor1/data.py` to match your Arduino's serial port
4. Run the sender script
```bash
   python monitor1/data.py
```

### Monitor2 — Music display

1. **Arduino** : upload `monitor2/monitor2.ino` using [arduino-cli](https://github.com/arduino/arduino-cli)
2. **PC / NAS** : install [Python dependencies](https://pypi.org/project/pip/)
```bash
   pip install unidecode requests flask pyserial
   sudo pacman -S playerctl  # on Arch/ Arch based
   sudo apt install playerctl  # on Ubuntu / Debian based
```
3. Edit the `SERVER_URL` in `monitor2/data.py` and the serial port in `monitor2/serverFlask.py` to match your setup
4. Run the Flask server (on the machine connected to the Arduino)
```bash
   python monitor2/serverFlask.py
```
5. Run the metadata sender (on the machine playing music, can be the same or different)
```bash
   python monitor2/data.py
```
