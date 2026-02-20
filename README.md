# Projet-IoT-SolarTracker
un système de suiveur de soleil envoie les données a Azure et un Site web responsable d'afficher les données en temps réel


Capteur ----> Arduino ----> ESP32 ----> Azure IoT ----> BackEnd(Fast API) ----> FrontEnd 
        UART        MQTT            AMQP                    HTTP


les pins arduino                         les pins ESP32
  D4      RX                            16 RX
  D5      TX                            17 TX

pour avoir l'adresse de connexion avec Azure ouvre le terminal et tape cette commande
az iot hub generate-sas-token --device-id SolarTracker1 --hub-name projet-IoT --duration 31536000
