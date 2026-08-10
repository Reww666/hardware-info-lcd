import serial
import subprocess
import time
import unidecode

PORT = '/dev/ttyACM0'
BAUDRATE = 9600

try:
    ser = serial.Serial(PORT, BAUDRATE, timeout=1)
    time.sleep(2)
    print(f"Connecté à {PORT}")
except serial.SerialException as e:
    print(f"Erreur : {e}")
    exit(1)

def get_metadata():
    try:
        title = subprocess.check_output(
            ["playerctl", "metadata", "--format", "{{ title }}"],
            stderr=subprocess.PIPE, text=True
        ).strip()
        artist = subprocess.check_output(
            ["playerctl", "metadata", "--format", "{{ artist }}"],
            stderr=subprocess.PIPE, text=True
        ).strip()

        # Normalisation UTF-8 → ASCII
        title = unidecode.unidecode(title)
        artist = unidecode.unidecode(artist)

        return title, artist
    except subprocess.CalledProcessError:
        return "Aucune musique", "Aucun artiste"

# Variables pour stocker les valeurs précédentes
last_title = ""
last_artist = ""

while True:
    title, artist = get_metadata()
    print(f"Titre: {title} | Artiste: {artist}")  # Debug

    # Envoi uniquement si changement
    if title != last_title or artist != last_artist:
        ser.write(f"TITLE:{title}\n".encode())
        ser.write(f"ARTIST:{artist}\n".encode())
        last_title = title
        last_artist = artist

    time.sleep(1)  # Vérifie toutes les secondes