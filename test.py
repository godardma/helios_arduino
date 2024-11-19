import serial
import time

# Configuration du port série
port = '/dev/narval_motors'
baudrate = 115200

# Initialisation de la connexion série
try:
    ser = serial.Serial(port, baudrate, timeout=1)
    time.sleep(2)  # Attendre que la connexion soit établie
except serial.SerialException as e:
    print(f"Erreur de connexion : {e}")
    exit()

# Message à envoyer
message = "40 60\n" 

try:
    # Envoi du message
    ser.write(message.encode('utf-8'))
    print(f"Message envoyé : {message}")

    time.sleep(0.1)
    
    # Lecture de la réponse
    response = ser.read_until(b'\n').decode('utf-8').strip()
    print(f"Réponse reçue : {response}")


except Exception as e:
    print(f"Erreur de communication : {e}")

finally:
    # Fermeture de la connexion série
    ser.close()
