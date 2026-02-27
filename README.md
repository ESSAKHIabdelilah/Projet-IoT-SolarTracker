# Projet IoT Complet - Solar Tracker Dashboard

## 📋 Description

Système IoT complet pour la surveillance en temps réel de capteurs ESP32 connectés à Azure IoT Hub. Le projet comprend :

- **Simulation ESP32** : Script Python pour simuler un dispositif IoT
- **Firmware ESP32** : Code Arduino pour ESP32 réel
- **Backend (FastAPI)** : Serveur Python qui reçoit les données d'Azure et les diffuse en temps réel
- **Frontend (HTML/JavaScript)** : Dashboard interactif avec graphiques en temps réel

## 🏗️ Architecture

```
┌─────────────────┐
│   ESP32 / IoT   │  → Envoie température, humidité
└────────┬────────┘
         │
         ▼
┌─────────────────────────────┐
│   Azure IoT Hub / EventHub   │  → Reçoit et stocke
└────────┬────────────────────┘
         │
         ▼
┌──────────────────────┐
│  Backend (FastAPI)   │  → Consomme & diffuse via WebSocket
└────────┬─────────────┘
         │
         ▼
┌──────────────────────┐
│  Frontend (HTML/JS)  │  → Affiche le dashboard
└──────────────────────┘
```

## 📦 Prérequis

### Système
- Python 3.8+ installé
- Node.js/npm (optionnel, pour le développement frontend)
- Un compte Azure IoT Hub configuré

### Clés d'accès Azure
- Connection String du dispositif ESP32
- Connection String Event Hub pour le backend

## ⚙️ Installation

### 1. Cloner/Préparer le projet

```bash
cd projet_complet
```

### 2. Créer un environnement virtuel Python

```bash
# Windows
python -m venv venv
venv\Scripts\activate

# Linux/Mac
python3 -m venv venv
source venv/bin/activate
```

### 3. Installer les dépendances

```bash
# Pour le simulateur et le backend
pip install fastapi uvicorn azure-iot-device azure-eventhub python-dotenv websockets
```

### 4. Configurer les variables d'environnement

Créer/modifier le fichier `.env` à la racine du projet :

```env
# Simulateur ESP32
# Obtenir depuis Azure IoT Hub → Appareils → SolarTracker1 → Chaîne de connexion
CONNECTION_STRING = "HostName=votre-hub.azure-devices.net;DeviceId=SolarTracker1;SharedAccessKey=..."

# Backend
# Point de terminaison compatible Event Hub du IoT Hub
CONNECTION_STR = "Endpoint=sb://ihsuprodparres002dednamespace.servicebus.windows.net/;SharedAccessKeyName=iothubowner;SharedAccessKey=...;EntityPath=iothub-ehub-projet-iot-..."
CONSUMER_GROUP = "$Default"
```

## 🚀 Déploiement

### Option 1 : Mode Développement Local

#### Terminal 1 - Lancer le Backend

```bash
# Activer l'environnement virtuel
venv\Scripts\activate  # Windows
source venv/bin/activate  # Linux/Mac

# Lancer FastAPI
uvicorn "Site_Web - first version/BackEnd/main:app" --reload --host 0.0.0.0 --port 8000
```

Le backend sera accessible sur : `http://localhost:8000`

#### Terminal 2 - Lancer le Simulateur ESP32

```bash
# Activer l'environnement virtuel
venv\Scripts\activate  # Windows
source venv/bin/activate  # Linux/Mac

# Lancer le simulateur
python simuler_ESP32.py
```

#### Terminal 3 - Servir le Frontend

Option A - Avec Python :
```bash
cd "Site_Web - first version\FrontEnd"
python -m http.server 3000
```

Option B - Avec Live Server VS Code :
- Installer l'extension "Live Server"
- Right-click sur `front_test.html` → "Open with Live Server"

Accéder au dashboard : `http://localhost:3000/front_test.html` (ou port Live Server)

### Option 2 : Déploiement en Production

#### Backend (Azure App Service / Heroku / VPS)

1. **Créer `requirements.txt`** :
```bash
pip freeze > requirements.txt
```

2. **Créer `Procfile`** (pour Heroku) :
```
web: uvicorn Site_Web.BackEnd.main:app --host 0.0.0.0 --port $PORT
```

3. **Déployer sur Azure App Service** :
```bash
# Installer Azure CLI
# Login
az login

# Créer le ressource group
az group create --name IoT-RG --location eastus

# Créer un App Service Plan
az appservice plan create --name IoT-Plan --resource-group IoT-RG --sku B1 --is-linux

# Créer l'application
az webapp create --resource-group IoT-RG --plan IoT-Plan --name mon-app-iot --runtime "PYTHON:3.9"

# Déployer
az webapp up --name mon-app-iot --resource-group IoT-RG
```

#### Frontend (Static Web Apps / GitHub Pages / Netlify)

1. **Azure Static Web Apps** :
```bash
# Créer une static web app qui pointe vers le dossier FrontEnd
# Configuration : App location: Site_Web\ -\ first\ version\FrontEnd
```

2. **Netlify** :
```bash
npm install -g netlify-cli
netlify deploy --prod --dir "Site_Web - first version/FrontEnd"
```

## 🔧 Configuration ESP32 Réel

Si vous utilisez un **ESP32 avec WiFi** (pas la simulation) :

1. Installer les bibliothèques Arduino :
   - `Azure IoT Library for Arduino`
   - `WiFi101` (ou adapté à votre board)

2. Télécharger `Script_ESP32/Script_ESP32.ino` sur votre ESP32

3. Configurer les paramètres WiFi et Azure dans le code :
```cpp
const char* ssid = "Votre_SSID";
const char* password = "Votre_PASSWORD";
const char* iothub_hostname = "votre-hub.azure-devices.net";
const char* device_id = "SolarTracker1";
```

4. Téléverser le code avec l'Arduino IDE

## 📊 Utilisation du Dashboard

### Fonctionnalités
- ✅ Affichage temps réel des données (température, humidité)
- ✅ Graphiques avec Chart.js
- ✅ Mise à jour instantanée via WebSocket
- ✅ Historique des dernières 20 mesures
- ✅ Indicateurs visuels de seuils

### Actions
- **Connexion automatique** : Le dashboard se connecte automatiquement au backend
- **Mise à jour des graphiques** : Chaque donnée reçue met à jour les graphiques
- **Logs en direct** : Console pour déboguer les connexions

## 🐛 Troubleshooting

### Le backend ne reçoit pas les données d'Azure

**Solution** :
- Vérifier la `CONNECTION_STR` dans `.env`
- Vérifier que le Consumer Group existe et que les droits sont OK
- Vérifier les firewall/pare-feu

### Le frontend ne se connecte pas au backend

**Solution** :
- Vérifier que le backend écoute sur `0.0.0.0:8000`
- Vérifier l'URL WebSocket dans le frontend (doit être `ws://localhost:8000/ws`)
- Vérifier la console du navigateur pour les erreurs

### L'ESP32 ne se connecte pas à Azure

**Solution** :
- Vérifier la connexion WiFi
- Vérifier le SAS Token (expiré ?)
- Vérifier les droits du dispositif sur Azure

## 📝 Fichiers Importants

```
projet_complet/
├── README.md                  ← Vous êtes ici
├── .env                       ← Variables d'environnement
├── simuler_ESP32.py           ← Simulation Python du dispositif
├── Script_ESP32/
│   ├── Script_Arduino.ino     ← Code Arduino basique
│   └── Script_ESP32.ino       ← Code ESP32 avec WiFi/Azure
└── Site_Web - first version/
    ├── BackEnd/
    │   ├── main.py           ← Application FastAPI
    │   └── main_V2.py        ← Version alternative
    └── FrontEnd/
        └── front_test.html   ← Dashboard HTML
```

## 🔐 Sécurité

⚠️ **IMPORTANT** :
- **NE JAMAIS** commiter le fichier `.env` avec les vraies clés !
- Ajouter `.env` dans `.gitignore`
- Utiliser des variables d'environnement en production
- Régulièrement renouveler les SAS Tokens Azure
- Utiliser HTTPS/WSS en production

```
# .gitignore
.env
__pycache__/
*.pyc
venv/
.idea/
.vscode/settings.json
```

## 📚 Ressources Utiles

- [Documentation Azure IoT Hub](https://docs.microsoft.com/en-us/azure/iot-hub/)
- [FastAPI Documentation](https://fastapi.tiangolo.com/)
- [Arduino IoT Cloud](https://create.arduino.cc/iot/overview)
- [Chart.js Documentation](https://www.chartjs.org/)

## 👨‍💻 Support et Questions

En cas de problema :
1. Vérifier la section Troubleshooting
2. Consulter les logs du backend : `uvicorn` affiche les erreurs
3. Console navigateur (F12) pour les erreurs frontend
4. Serial Monitor Arduino IDE pour l'ESP32

## 📄 Licence

Projet étudiant - Usage personnel/éducatif

---

**Dernière mise à jour** : Février 2026
**Status** : ✅ Prêt pour déploiement
