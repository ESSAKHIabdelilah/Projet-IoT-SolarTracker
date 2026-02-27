#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "time.h"

// --- 1. TES INFOS WIFI ---
// Correction de la syntaxe : Remplacement de la virgule par un point-virgule
const char* ssid = ""; 
const char* password = ""; 

// --- 2. TES INFOS AZURE ---
const char* iothub_hostname = ""; 
const char* device_id = ""; 

// Ton SAS TOKEN valide
const char* sas_token = "";

// --- CONFIGURATION TECHNIQUE ---
const char* ntpServer = "pool.ntp.org";
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Pin RX2 et TX2 de l'ESP32
#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200); 
  
  // C'est cette ligne qui fait le lien avec l'Arduino !
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); 

  // 1. Connexion WiFi
  setupWifi();
  
  // 2. Réglage de l'heure
  configTime(0, 0, ntpServer);
  Serial.print("Synchronisation de l'heure");
  while (time(nullptr) < 1000000000) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nHeure OK !");

  // 3. Configuration Azure (SSL)
  espClient.setInsecure(); 
  client.setServer(iothub_hostname, 8883); 
  client.setCallback(callback);
}

void loop() {
  // 1. Vérification connexion Azure
  if (!client.connected()) {
    reconnectAzure();
  }
  client.loop();

  // 2. Lecture des données venant de l'Arduino (UART)
  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    data.trim(); 
    
    // Si on reçoit des données valides (commençant par { )
    if (data.length() > 0 && data.startsWith("{")) {
      Serial.print("Reçu de Arduino: ");
      Serial.println(data);
      
      // Envoi à Azure
      sendToAzure(data);
    }
  }
}

// --- FONCTIONS ---

void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connexion WiFi à ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int tentatives = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tentatives++;
    if(tentatives > 20) {
        Serial.println("\nErreur WiFi: Vérifie ton mot de passe !");
        delay(5000);
        tentatives = 0;
    }
  }
  Serial.println("\nWiFi Connecté !");
}

void reconnectAzure() {
  while (!client.connected()) {
    Serial.print("Connexion MQTT Azure...");
    
    // Création du UserID Azure
    String username = String(iothub_hostname) + "/" + String(device_id) + "/?api-version=2021-04-12";
    
    if (client.connect(device_id, username.c_str(), sas_token)) {
      Serial.println("Connecté !");
    } else {
      Serial.print("Echec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5s");
      delay(5000);
    }
  }
}

void sendToAzure(String jsonPayload) {
  String topic = "devices/" + String(device_id) + "/messages/events/";
  
  if (client.publish(topic.c_str(), jsonPayload.c_str())) {
    Serial.println("-> Données envoyées à Azure succès !");
  } else {
    Serial.print("-> Erreur d'envoi. Etat: ");
    Serial.println(client.state());
  }
}

void callback(char* topic, byte* payload, unsigned int length) {}