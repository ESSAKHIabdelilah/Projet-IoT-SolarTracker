import time
import json
import random
from azure.iot.device import IoTHubDeviceClient, Message

# ⚠️ Mets ici la chaîne de connexion de l'APPAREIL (pas celle du backend !)


def simulate_esp32():
    try:
        # On crée le client IoT (exactement comme le fera la librairie Arduino)
        client = IoTHubDeviceClient.create_from_connection_string(CONNECTION_STRING)
        print("✅ Connecté à Azure IoT Hub (Simulation ESP32 en cours...)")

        while True:
            # 1. On génère de fausses données de capteurs
            temperature = round(random.uniform(20.0, 30.0), 2)
            humidite = round(random.uniform(40.0, 60.0), 2)
            
            data = {
                "temperature": temperature,
                "humidite": humidite
            }
            
            # 2. On transforme le dictionnaire Python en texte JSON
            message_json = json.dumps(data)
            message = Message(message_json)
            
            # 3. On envoie le message vers Azure
            print(f"📡 Envoi vers Azure : {message_json}")
            client.send_message(message)
            
            # 4. On attend 3 secondes avant le prochain envoi (comme un vrai capteur)
            time.sleep(3)

    except KeyboardInterrupt:
        print("\n🛑 Arrêt de la simulation.")
    finally:
        client.disconnect()

if __name__ == '__main__':
    simulate_esp32()