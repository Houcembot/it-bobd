# 🤖 Simulation Wokwi : IT-Bobd (ESP32-S3 + GC9A01)

Ce dossier contient une configuration complète pour simuler le projet **It-Bobd** avec Wokwi.

## 📁 Structure

- `diagram.json` : Le schéma de câblage virtuel (ESP32-S3 connecté à l'écran GC9A01 via SPI).
- `wokwi.toml` : Fichier de configuration Wokwi pointant vers le firmware compilé.
- `platformio.ini` : Configuration PlatformIO pour compiler le projet.
- `src/main.cpp` : Code source C++ (Arduino) unifié (Yeux + Dashboard).

## 🚀 Comment Lancer la Simulation ?

### Option 1 : VS Code (Recommandé)
1. Installez l'extension **Wokwi for VS Code**.
2. Installez l'extension **PlatformIO**.
3. Ouvrez ce dossier (`wokwi/`) dans VS Code.
4. Compilez le projet avec PlatformIO (`Build`).
5. Ouvrez le fichier `diagram.json`.
6. Cliquez sur le bouton "Play" vert en haut du diagramme.

### Option 2 : Wokwi Web
1. Allez sur [wokwi.com](https://wokwi.com).
2. Créez un nouveau projet "ESP32-S3".
3. Copiez le contenu de `diagram.json` dans l'onglet Diagramme.
4. Copiez le contenu de `src/main.cpp` dans l'onglet Sketch.
5. Ajoutez la librairie `TFT_eSPI` dans le gestionnaire de bibliothèques.
6. Lancez la simulation !

## ⚙️ Câblage (Virtuel & Réel)

| Fonction | ESP32-S3 Pin | GC9A01 Pin |
| :--- | :--- | :--- |
| **VCC** | 3.3V | VCC |
| **GND** | GND | GND |
| **SCLK** | GPIO 12 | SCL |
| **MOSI** | GPIO 11 | SDA |
| **RST** | GPIO 14 | RES |
| **DC** | GPIO 9 | DC |
| **CS** | GPIO 10 | CS |
| **BLK** | 3.3V | BLK |

## ℹ️ Notes
Le code (`src/main.cpp`) alterne automatiquement toutes les 10 secondes entre :
1. **Mode Yeux** (Animation émotionnelle).
2. **Mode Dashboard** (Simulation de vitesse/RPM OBD2).
