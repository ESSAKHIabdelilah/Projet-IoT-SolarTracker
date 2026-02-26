/*
  Keyestudio Sun Follower - Version Optimisée
  Améliorations : Non-bloquant, fluide, correction ISR, mode nuit.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BH1750.h>
#include <dht11.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// --- Objets ---
LiquidCrystal_I2C lcd(0x27, 16, 2);
BH1750 lightMeter;
dht11 DHT;
Servo lr_servo; // Gauche-Droite (Horizontal)
Servo ud_servo; // Haut-Bas (Vertical)

// --- Communication ESP32 ---
// RX sur pin 4, TX sur pin 5 (car la pin 10 est utilisée par le servo)
SoftwareSerial espSerial(4, 5);

// --- Pins ---
#define DHT11_PIN 7

const byte interruptPin = 2; 
const byte buzzer = 6;
const byte lr_servopin = 9;
const byte ud_servopin = 10;
const int l_sensor = A0;
const int r_sensor = A1;
const int u_sensor = A2;
const int d_sensor = A3;

// --- Variables de configuration ---
int lr_angle = 90;
int ud_angle = 45; // 45 est souvent mieux que 10 pour démarrer
byte error_margin = 25; // Marge d'erreur un peu plus large pour éviter les vibrations
byte resolution = 1; 

// --- Variables système ---
volatile bool buttonPressed = false; // "volatile" est obligatoire pour les interruptions
unsigned long lastServoTime = 0;
unsigned long lastScreenTime = 0;
unsigned long lastDebounceTime = 0;
unsigned long lastUartTime = 0; // <-- AJOUT POUR LE CHRONO ESP32

// Variables de stockage
int temperature = 0;
int humidity = 0;
uint16_t lux = 0;

void setup() {
  
  Serial.begin(9600);

  espSerial.begin(115200);   // Communication avec l'ESP32 (Même vitesse)

  Wire.begin();
  
  // Initialisation capteurs
  lightMeter.begin();
  lcd.init();
  lcd.backlight();
  
  // Initialisation Servos
  lr_servo.attach(lr_servopin);
  ud_servo.attach(ud_servopin);
  
  // Position initiale
  lr_servo.write(lr_angle);
  ud_servo.write(ud_angle);
  
  // Pins
  pinMode(l_sensor, INPUT);
  pinMode(r_sensor, INPUT);
  pinMode(u_sensor, INPUT);
  pinMode(d_sensor, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  
  // Interruption
  attachInterrupt(digitalPinToInterrupt(interruptPin), isr_button, FALLING);
  
  lcd.setCursor(0,0);
  lcd.print("Systeme Pret");
  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. GESTION DU BOUTON (Prioritaire mais non bloquant)
  if (buttonPressed) {
    if (currentMillis - lastDebounceTime > 200) { // Anti-rebond
      changeResolution();
      lastDebounceTime = currentMillis;
    }
    buttonPressed = false; // Reset du drapeau
  }

  // 2. GESTION DES SERVOS (Rapide : toutes les 30ms)
  if (currentMillis - lastServoTime >= 30) {
    lastServoTime = currentMillis;
    
    // On ne bouge que s'il y a assez de lumière (Mode économie nuit)
    if (lux > 10) { 
       moveSolarTracker();
    }
  }

  // 3. GESTION ECRAN & CAPTEURS LENTS (Lente : toutes les 1000ms)
  if (currentMillis - lastScreenTime >= 1000) {
    lastScreenTime = currentMillis;
    readEnvironment();
    updateLCD();
  }

  
  // 4. ENVOI DES DONNÉES À L'ESP32 (Toutes les 5 secondes) <-- AJOUT !
  if (currentMillis - lastUartTime >= 5000) {
    lastUartTime = currentMillis;
    
    // Fabrication du message JSON avec toutes tes variables !
    String json = "{\"lux\": " + String(lux) + 
                  ", \"temperature\": " + String(temperature) + 
                  ", \"humidity\": " + String(humidity) + 
                  ", \"angle_h\": " + String(lr_angle) + 
                  ", \"angle_v\": " + String(ud_angle) + "}";
                  
    // Envoi via les pins 4 et 5
    espSerial.println(json);
    
    // Affichage sur le PC pour contrôler
    Serial.println("Envoye a ESP32 : " + json);
  }
}


// --- Logique du Suiveur Solaire ---
void moveSolarTracker() {
  // Lecture avec moyenne pour stabiliser
  int L = (analogRead(l_sensor) + analogRead(l_sensor)) / 2;
  int R = (analogRead(r_sensor) + analogRead(r_sensor)) / 2;
  int U = (analogRead(u_sensor) + analogRead(u_sensor)) / 2;
  int D = (analogRead(d_sensor) + analogRead(d_sensor)) / 2;

  // --- Horizontal (Gauche / Droite) ---
  int diffLR = abs(L - R);
  if (diffLR > error_margin) {
    if (L > R) {
      lr_angle -= resolution;
    } else {
      lr_angle += resolution;
    }
    // Contraintes physiques
    lr_angle = constrain(lr_angle, 0, 180);
    lr_servo.write(lr_angle);
  }

  // --- Vertical (Haut / Bas) ---
  int diffUD = abs(U - D);
  if (diffUD > error_margin) {
    if (U > D) {
      ud_angle -= resolution;
    } else {
      ud_angle += resolution;
    }
    // Contraintes physiques (souvent 0-90 suffisent pour le vertical)
    ud_angle = constrain(ud_angle, 0, 90); 
    ud_servo.write(ud_angle);
  }
}

// --- Lecture Capteurs Lents ---
void readEnvironment() {
  lux = lightMeter.readLightLevel();
  
  int chk = DHT.read(DHT11_PIN);
  if (chk == DHTLIB_OK) {
    temperature = DHT.temperature;
    humidity = DHT.humidity;
  }
}

// --- Mise à jour LCD ---
void updateLCD() {
  // Ligne 1 : Lumière
  lcd.setCursor(0, 0);
  lcd.print("Lux:");
  lcd.print(lux);
  lcd.print("   "); // Efface les vieux chiffres

  // Ligne 2 : Temp/Hum et Resolution
  lcd.setCursor(0, 1);
  lcd.print(temperature);
  lcd.print("C ");
  lcd.print(humidity);
  lcd.print("% R:");
  lcd.print(resolution);
}

// --- Gestion du Bouton ---
void isr_button() {
  // L'ISR doit être la plus courte possible !
  buttonPressed = true;
}

void changeResolution() {
  tone(buzzer, 1000, 100); // Petit bip
  resolution++;
  if (resolution > 5) resolution = 1;
}