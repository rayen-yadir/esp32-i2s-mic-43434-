import serial
import time

# Change COM8 par ton port (COM5, COM8... sur Windows)
PORT = "COM8"
BAUD = 921600
OUTPUT_FILE = "enregistrement.wav"

SAMPLE_RATE = 16000
RECORD_SECONDS = 5
EXPECTED_BYTES = SAMPLE_RATE * RECORD_SECONDS * 2  # 16 bits = 2 bytes/echantillon

print(f"Connexion sur {PORT}...")
ser = serial.Serial(PORT, BAUD, timeout=5)
time.sleep(2)

print("En attente de l'ESP32...")

# Attend le signal READY
while True:
    line = ser.readline().decode('utf-8', errors='ignore').strip()
    print(line)
    if "READY" in line:
        break

print("ESP32 connecte ! En attente du debut...")

# Affiche le compte a rebours
while True:
    line = ser.readline().decode('utf-8', errors='ignore').strip()
    print(line)
    if "GO" in line:
        break

print("Enregistrement en cours... Parle !")

# Recoit les donnees WAV
wav_data = bytearray()
end_marker = b"ENDOFWAV"

while True:
    chunk = ser.read(1024)
    if not chunk:
        break
    wav_data += chunk
    if end_marker in wav_data:
        idx = wav_data.index(end_marker)
        wav_data = wav_data[:idx]
        break

# Sauvegarde le fichier WAV
with open(OUTPUT_FILE, 'wb') as f:
    f.write(wav_data)

actual_data_bytes = len(wav_data) - 44  # moins l'entete WAV (44 bytes)

print(f"\nFichier sauvegarde : {OUTPUT_FILE}")
print(f"Taille totale : {len(wav_data)} bytes")
print(f"Donnees audio : {actual_data_bytes} / {EXPECTED_BYTES} bytes attendus")

if actual_data_bytes < EXPECTED_BYTES * 0.98:
    manque = EXPECTED_BYTES - actual_data_bytes
    print(f"ATTENTION: il manque ~{manque} bytes ({manque // 2} echantillons)")
    print("-> perte de donnees pendant la transmission serie, cause probable du bruit de grésillement")
else:
    print("OK: taille conforme, pas de perte de donnees detectee.")

ser.close()
