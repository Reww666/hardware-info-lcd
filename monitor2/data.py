import subprocess
import time
import unidecode
import requests

SERVER_URL = "http://192.168.1.16:5000/update" # NAS IP

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

# Variables for storing previous values
last_title = ""
last_artist = ""

while True:
    title, artist = get_metadata()

    # Send only if there is a change
    if title != last_title or artist != last_artist:
        message = f"TITLE:{title}\nARTIST:{artist}"
        try:
            requests.post(SERVER_URL, json={"message": message})
            print(f"Envoyé : {title} - {artist}")
        except Exception as e:
            print(f"Erreur envoi: {e}")

        last_title = title
        last_artist = artist

    time.sleep(1)  # Check every second