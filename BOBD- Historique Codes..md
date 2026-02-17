# Onglet 1

---

# **📑 \[VOXEL STUDIO\] \- ARCHIVE TECHNIQUE : CAR ASSISTANT**

**Projet :** Assistant Intelligent pour VW Polo 9N (2005)

**Auteur :** Houcemedinne – Voxel Studio

**Période :** Janvier 2026

---

### **PAGE 1 : Introduction et Vision du Projet**

Le projet "Car Assistant" de Voxel Studio vise à transformer l'expérience de conduite d'une Volkswagen Polo 9N en intégrant un compagnon robotisé capable de surveiller la télémétrie moteur en temps réel. L'objectif est de pallier l'absence d'ordinateur de bord d'origine via un écran LCD rond de 1.28 pouce. La vision s'articule autour de trois phases : la validation logicielle (Pi Zero 2), la miniaturisation hardware (ESP32-C3) et l'intégration émotionnelle (Audio/IA).

### **PAGE 2 : Le Mur du "UNABLE TO CONNECT" (06 \- 12 Janvier)**

La phase initiale a été marquée par l'échec systématique des connexions OBD standards. L'ECU Magneti Marelli de la Polo 9N rejetait toutes les tentatives via les protocoles automatiques. Cette période de test a permis d'éliminer les méthodes classiques pour se concentrer sur les spécificités des protocoles propriétaires du groupe VAG.

### **PAGE 3 : La Percée \- Le Handshake Volkswagen (15 \- 16 Janvier)**

Le 16 Janvier à 22h45, le projet a franchi une étape décisive avec la validation de la séquence d'initialisation forcée :

* AT Z (Reset)  
* AT SP 5 (Protocole ISO 14230-4 KWP)  
* AT SH 81 10 F1 (Header d'adressage ECU moteur)  
  Cette commande a permis la première lecture stable du régime moteur (RPM).

### **PAGE 4 : Design de l'Interface et Gestion du LCD (20 \- 22 Janvier)**

Développement d'un dashboard dynamique 240x240 pixels sous Python (PIL). L'interface utilise un rendu d'arc de cercle pour la vitesse et des couleurs contrastées pour une visibilité optimale. Le rafraîchissement a été optimisé pour garantir la fluidité sur le bus SPI du Raspberry Pi.

### **PAGE 5 : Intelligence et Coach de Conduite (24 \- 25 Janvier)**

Implémentation du "Coach Trip". Un algorithme analyse l'historique de la charge moteur (LOAD) pour attribuer une note de conduite sur 10\. L'assistant guide le conducteur vers une conduite plus "ZEN" en temps réel.

### **PAGE 6 : Ergonomie \- Le Défi du Tactile (26 \- 27 Janvier)**

Optimisation du capteur TTP223. Pour supprimer les lags, la vérification du toucher a été insérée entre chaque requête OBD. Le délai de réponse a été réduit à 0.15s, rendant le passage entre les 7 pages instantané.

### **PAGE 7 : Analyse Hardware \- La Puce PIC18F25K80 (28 Janvier)**

Validation matérielle suite au démontage de l'ELM327. La présence de la puce PIC18F25K80 garantit la compatibilité avec les commandes AT complexes et la stabilité nécessaire pour un usage automobile permanent.

### **PAGE 8 : Gestion d'Énergie "Zero Drain" (28 Janvier)**

Étude de la commande AT LP (Low Power). Cette fonction permet de mettre l'adaptateur en sommeil profond pour protéger la batterie de la Polo 9N, car la prise OBD reste alimentée même moteur coupé.

### **PAGE 9 : Migration Phase 2 \- ESP32-C3 (29 Janvier)**

Réception du kit ESP32-C3 à Tunis. Décision de passer à une liaison filaire physique via un **câble CAT6** (1 mètre) pour éliminer les problèmes d'appairage Bluetooth et assurer une latence zéro.

### **PAGE 10 : Vision du Futur \- Phase 3 (Audio et Émotions)**

Planification de l'intégration audio via les modules **MAX98357A** et **INMP441**. L'assistant pourra prochainement parler et écouter, transformant le dashboard en une véritable interface humanoïde.

### **PAGE 11 : Annexe A \- Lexique des Commandes AT**

* AT SP 5 : Fixe le protocole ISO 14230-4.  
* AT SH 81 10 F1 : Définit l'adressage ECU.  
* 01 0C : RPM | 01 0D : Vitesse | 01 05 : Temp. Eau.

### **PAGE 12 : Annexe B \- Ressources et Liens**

* [Guide Commandes AT](https://www.scribd.com/document/504218143/ELM327)  
* [Spécifications PIC18F25K80](https://www.scribd.com/document/645200866/OBD2-Scanner-ELM327-V1-5-PIC18F25K80-Bluetooth-Compatible-ELM-327-V2)

### **PAGE 13 : Annexe C \- Code Source Final (obd\_tactile.py)**

*\#\!/usr/bin/env python3*

*\# PROJET : Voxel Assistant \- Polo 9N*

*\# DATE : 29/01/2026 | VERSION : 1.4 (Optimisation Tactile & Coach Trip)*

*\# DEVELOPPEUR : Houcemedinne / Voxel Studio*

*import sys*

*import time*

*import os*

*import serial*

*import subprocess*

*import re*

*from PIL import Image, ImageDraw, ImageFont*

*import lgpio*

*\# \--- CONFIGURATION MATÉRIELLE \---*

*BLUETOOTH\_MAC \= "00:1D:A5:06:22:DF"*

*SERIAL\_PORT \= "/dev/rfcomm0"*

*TOUCH\_PIN \= 4* 

*\# Initialisation LCD Waveshare 1.28"*

*sys.path.append('/home/pi/LCD\_Module\_code/LCD\_Module\_RPI\_code/RaspberryPi/python/lib')*

*import LCD\_1inch28*

*LCD \= LCD\_1inch28.LCD\_1inch28()*

*LCD.Init()*

*\# Initialisation GPIO pour le tactile (TTP223)*

*h\_chip \= lgpio.gpiochip\_open(0)*

*lgpio.gpio\_claim\_input(h\_chip, TOUCH\_PIN, lgpio.SET\_PULL\_DOWN)*

*\# \--- VARIABLES D'ÉTAT & HISTORIQUE \---*

*page\_index \= 0*

*last\_touch\_time \= 0*

*load\_history \= \[\]*

*conso\_history \= \[\]*

*\# Polices (A ajuster selon ton chemin)*

*font\_path \= "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"*

*font\_data \= ImageFont.truetype(font\_path, 45\)*

*font\_unit \= ImageFont.truetype(font\_path, 20\)*

*font\_msg \= ImageFont.truetype(font\_path, 22\)*

*\# \--- GESTION TACTILE ULTRA-RÉACTIVE \---*

*def verifier\_toucher():*

    *global page\_index, last\_touch\_time*

    *if lgpio.gpio\_read(h\_chip, TOUCH\_PIN) \== 1:*

        *current\_time \= time.time()*

        *\# Délai réduit à 0.15s pour une sensation de réactivité immédiate*

        *if current\_time \- last\_touch\_time \> 0.15:* 

            *page\_index \= (page\_index \+ 1\) % 7* 

            *last\_touch\_time \= current\_time*

            *return True*

    *return False*

*def afficher\_message(titre, sous\_titre="", bg=(20, 20, 20), duree=1):*

    *img \= Image.new("RGB", (240, 240), bg)*

    *draw \= ImageDraw.Draw(img)*

    *\# Centrage automatique du texte*

    *w1 \= draw.textbbox((0,0), titre, font=font\_msg)\[2\]*

    *draw.text(((240-w1)/2, 90), titre, fill=(255,255,255), font=font\_msg)*

    *w2 \= draw.textbbox((0,0), sous\_titre, font=font\_unit)\[2\]*

    *draw.text(((240-w2)/2, 130), sous\_titre, fill=(180,180,180), font=font\_unit)*

    *LCD.ShowImage(img)*

    *\# On continue de surveiller le tactile même pendant l'affichage d'un message*

    *t\_end \= time.time() \+ duree*

    *while time.time() \< t\_end:*

        *verifier\_toucher()*

        *time.sleep(0.05)*

*\# \--- PARSING DES RÉPONSES ECU \---*

*def extraire\_valeur(raw\_data, type\_donnee):*

    *if not raw\_data or "\>" not in raw\_data: return None*

    *try:*

        *\# Nettoyage de la chaîne Hexadécimale*

        *clean \= "".join(re.findall(r'\[0-9A-F\]', raw\_data.upper()))*

        *if type\_donnee \== "RPM" and "410C" in clean:*

            *pos \= clean.find("410C") \+ 4*

            *return int(((int(clean\[pos:pos+2\], 16\) \* 256\) \+ int(clean\[pos+2:pos+4\], 16)) / 4\)*

        *if type\_donnee \== "SPD" and "410D" in clean:*

            *return int(clean\[clean.find("410D")+4:clean.find("410D")+6\], 16\)*

        *if type\_donnee \== "TEMP" and "4105" in clean:*

            *return int(clean\[clean.find("4105")+4:clean.find("4105")+6\], 16\) \- 40*

        *if type\_donnee \== "LOAD" and "4104" in clean:*

            *return int((int(clean\[clean.find("4104")+4:clean.find("4104")+6\], 16\) \* 100\) / 255\)*

    *except: return None*

    *return None*

*def run\_assistant():*

    *global page\_index, load\_history*

    *while True:*

        *\# Nettoyage RFCOMM au démarrage*

        *os.system("sudo rfcomm release 0 2\>/dev/null")*

        

        *\# \--- PHASE DE CONNEXION \---*

        *bt\_connected \= False*

        *while not bt\_connected:*

            *afficher\_message("CONNEXION", "POLO 9N...", bg=(30, 0, 0))*

            *if subprocess.run(\["hcitool", "name", BLUETOOTH\_MAC\], capture\_output=True, text=True).stdout.strip():*

                *afficher\_message("BT DÉTECTÉ", "BINDING...", bg=(0, 40, 0))*

                *os.system(f"sudo rfcomm bind 0 {BLUETOOTH\_MAC} 1")*

                *time.sleep(1)*

                *bt\_connected \= True*

        *try:*

            *ser \= serial.Serial(SERIAL\_PORT, baudrate=38400, timeout=0.8)*

            

            *\# \--- HANDSHAKE VOLKSWAGEN (CRITIQUE) \---*

            *for cmd in \[b"AT Z\\r", b"AT E0\\r", b"AT SP 5\\r", b"AT SH 81 10 F1\\r"\]:*

                *ser.write(cmd)*

                *ser.read\_until(b"\>")*

                *verifier\_toucher() \# Reste réactif pendant l'init*

            *\# Valeurs par défaut*

            *data \= {"RPM": 0, "SPD": 0, "TEMP": 0, "LOAD": 0}*

            *while True:*

                *verifier\_toucher()*

                *ser.reset\_input\_buffer()*

                *\# \--- LECTURE SÉQUENTIELLE (OPTIMISÉE) \---*

                *\# On intercale verifier\_toucher() pour supprimer tout lag tactile*

                *ser.write(b"01 0C\\r")*

                *data\["RPM"\] \= extraire\_valeur(ser.read\_until(b"\>").decode(errors='ignore'), "RPM") or data\["RPM"\]*

                *verifier\_toucher()*

                *ser.write(b"01 0D\\r")*

                *data\["SPD"\] \= extraire\_valeur(ser.read\_until(b"\>").decode(errors='ignore'), "SPD") or data\["SPD"\]*

                *verifier\_toucher()*

                *ser.write(b"01 04\\r")*

                *data\["LOAD"\] \= extraire\_valeur(ser.read\_until(b"\>").decode(errors='ignore'), "LOAD") or data\["LOAD"\]*

                

                *\# Mise à jour de l'historique pour le Coach*

                *if data\["RPM"\] \> 500:*

                    *load\_history.append(data\["LOAD"\])*

                    *if len(load\_history) \> 500: load\_history.pop(0)*

                *\# \--- GÉNÉRATION DE L'IMAGE \---*

                *img \= Image.new("RGB", (240, 240), (0, 0, 10))*

                *draw \= ImageDraw.Draw(img)*

                *if page\_index \== 0: \# Page RPM*

                    *draw.text((60, 80), f"{data\['RPM'\]}", fill=(0, 255, 255), font=font\_data)*

                    *draw.text((95, 140), "RPM", fill=(200, 200, 200), font=font\_unit)*

                

                *elif page\_index \== 6: \# Page COACH (Nouveauté)*

                    *moyenne \= sum(load\_history)/len(load\_history) if load\_history else 0*

                    *note \= max(0, min(10, 10 \- int(moyenne/10)))*

                    *statut \= "ZEN" if note \> 7 else "NERVEUSE"*

                    *color \= (0, 255, 0\) if note \> 7 else (255, 0, 0\)*

                    *draw.text((50, 40), "MON TRIP", fill=(255,255,255), font=font\_unit)*

                    *draw.text((85, 80), f"{note}/10", fill=color, font=font\_data)*

                    *draw.text((65, 150), statut, fill=color, font=font\_msg)*

                

                *\# ... \[Ajouter ici les autres pages selon tes besoins\] ...*

                *LCD.ShowImage(img)*

                *time.sleep(0.01)*

        *except Exception as e:*

            *if 'ser' in locals(): ser.close()*

            *time.sleep(2)*

*if \_\_name\_\_ \== "\_\_main\_\_":*

    *run\_assistant()*

---

eps32s3 zero:

**code: eyes au debut 5sec \+commandes reel \+cadran:**

/\*\*  
 \* RoboOBD\_Final\_Integrated\_Eyes.ino  
 \* 1\. Animation Yeux (Code fourni) pendant 5s.  
 \* 2\. Initialisation OBD réelle (2 tentatives, affichage version, erreur critique si fail).  
 \* 3\. Dashboard OBD complet.  
 \* Pour GC9A01 \+ ESP32-S3-Zero.  
 \*/

\#include \<TFT\_eSPI.h\>

TFT\_eSPI tft \= TFT\_eSPI();  
TFT\_eSprite spr \= TFT\_eSprite(\&tft);

// \--- CONFIGURATION GLOBALE \---  
\#define BGCOLOR       TFT\_BLACK  
\#define EYECOLOR      TFT\_WHITE  
\#define BG\_DARK\_BLUE  0x000A  
\#define V\_DARK        0x0110  
\#define TFT\_YELLOW    0xFFE0 // Correction pour éviter l'erreur 'YELLOW'

// \--- PARAMÈTRES YEUX (Tirés de votre code) \---  
const int SCREEN\_W \= 240;  
const int SCREEN\_H \= 240;  
int eyeW \= 56;              
int eyeH\_Max \= 56;          
int borderRadius \= 16;      
int spaceBetween \= 24;  
int blinkThickness \= 6;      
int blinkWidth \= 64;        
float blinkThreshold \= 12.0;

// Variables d'animation Yeux  
float curLx, curLy, curLh;  
float targetLx, targetLy, targetLh;

enum Mood { NORMAL, TIRED, ANGRY, HAPPY };  
Mood currentMood \= NORMAL;

unsigned long blinkTimer \= 0;  
unsigned long idleTimer \= 0;  
unsigned long moodTimer \= 0;

// \--- VARIABLES OBD & DASHBOARD \---  
float realRPM \= 0, realSPD \= 0, realTMP \= 0;

// \--- FONCTIONS UTILITAIRES \---  
void printCentered(String s, int y, int size, uint16\_t color) {  
  spr.setTextSize(size);  
  spr.setTextColor(color);  
  int x \= 120 \- (s.length() \* 6 \* size / 2);  
  spr.setCursor(x, y);  
  spr.print(s);  
}

// \--- GESTION OBD RÉELLE (AVEC ATTENTE RÉPONSE) \---  
bool sendOBD(const char\* cmd, String \*responseStr) {  
  Serial.println(cmd); // Envoi de la commande  
   
  \*responseStr \= ""; // Reset de la réponse  
  unsigned long timeout \= millis();  
  bool promptFound \= false;  
   
  while (millis() \- timeout \< 2000) { // Timeout 2 secondes  
    if (Serial.available()) {  
      char c \= Serial.read();  
      // On ignore les retours chariot et sauts de ligne pour le nettoyage, mais on les garde dans le buffer si besoin  
      \*responseStr \+= c;  
       
      if (c \== '\>') { // Le prompt ELM327 indique que la commande est finie  
        promptFound \= true;  
        break;  
      }  
    }  
  }  
   
  if (promptFound) {  
    // Nettoyage de la chaîne pour afficher la version (enlève le '\>' final et les \\r\\n)  
    responseStr\-\>trim();  
    return true;  
  }  
  return false;  
}

bool initOBDCommand(const char\* cmd, const char\* title, bool showVersion) {  
  String response \= "";  
   
  for (int attempt \= 1; attempt \<= 2; attempt++) {  
    // Affichage ENVOI  
    spr.fillSprite(0x9009); // Purple  
    printCentered("SENDING " \+ String(attempt) \+ "/2", 60, 2, TFT\_WHITE);  
    printCentered(String(cmd), 110, 3, TFT\_WHITE);  
    printCentered(String(title), 160, 2, TFT\_WHITE);  
    spr.pushSprite(0, 0);  
     
    if (sendOBD(cmd, \&response)) {  
      // Succès \!  
      spr.fillSprite(0x04A9); // Green  
      printCentered("OK", 90, 4, TFT\_WHITE);  
       
      if (showVersion && response.length() \> 0) {  
         // Affiche la version (ex: "ELM327 v1.5") si c'est la commande AT Z  
         printCentered(response, 130, 2, TFT\_WHITE);  
      }  
       
      spr.pushSprite(0, 0);  
      delay(800); // Pause pour voir le OK  
      return true;  
    }  
    // Échec, on retente si attempt \< 2  
    delay(500);  
  }  
  return false;  
}

void showError() {  
  spr.fillSprite(TFT\_RED);  
  printCentered("NOT CONNECTED", 110, 2, TFT\_WHITE);  
  spr.pushSprite(0, 0);  
  while(1); // Arrêt total  
}

// \--- LOGIQUE DESSIN YEUX (Votre code) \---  
void drawEyesToSprite() {  
  spr.fillSprite(BGCOLOR);

  int drawW, drawH, drawRadius, finalY;  
  int xL, xR;

  if (curLh \< blinkThreshold) {  
    // PHASE TRAIT  
    drawW \= blinkWidth;  
    drawH \= blinkThickness;  
    drawRadius \= 3;  
    finalY \= (int)curLy \+ (eyeH\_Max / 2) \- (blinkThickness / 2);  
    xL \= (int)curLx \- (blinkWidth \- eyeW) / 2;  
    xR \= xL \+ blinkWidth \+ (spaceBetween \- (blinkWidth \- eyeW));  
  } else {  
    // PHASE NORMALE  
    drawW \= eyeW;  
    drawH \= (int)curLh;  
    drawRadius \= borderRadius;  
    finalY \= (int)curLy \+ (eyeH\_Max \- drawH) / 2;  
    xL \= (int)curLx;  
    xR \= xL \+ eyeW \+ spaceBetween;  
  }

  // Dessin  
  spr.fillRoundRect(xL, finalY, drawW, drawH, drawRadius, EYECOLOR);  
  spr.fillRoundRect(xR, finalY, drawW, drawH, drawRadius, EYECOLOR);

  // ÉMOTIONS  
  if (curLh \>= blinkThreshold) {  
    int mH \= drawH / 2;  
    if (currentMood \== ANGRY) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL \+ eyeW, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR, finalY \+ mH, BGCOLOR);  
    }  
    else if (currentMood \== TIRED) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR \+ eyeW, finalY \+ mH, BGCOLOR);  
    }  
    else if (currentMood \== HAPPY) {  
      spr.fillRoundRect(xL \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
      spr.fillRoundRect(xR \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
    }  
  }  
}

void updateEyesLogic() {  
  unsigned long now \= millis();  
   
  // Cycle des émotions  
  if (now \> moodTimer) {  
    if (currentMood \== NORMAL) currentMood \= TIRED;  
    else if (currentMood \== TIRED) currentMood \= ANGRY;  
    else if (currentMood \== ANGRY) currentMood \= HAPPY;  
    else currentMood \= NORMAL;  
    moodTimer \= now \+ 4000;  
  }

  // Clignement  
  if (now \> blinkTimer) {  
    targetLh \= 0;  
    blinkTimer \= now \+ random(2000, 5000);  
  }

  // Mouvements  
  if (now \> idleTimer) {  
    targetLx \= random(40, 90);  
    targetLy \= random(70, 105);  
    idleTimer \= now \+ random(2000, 6000);  
  }

  // Interpolation  
  float speed \= (targetLh \== 0 || curLh \< 10) ? 0.40 : 0.20;  
  curLx \+= (targetLx \- curLx) \* 0.20;  
  curLy \+= (targetLy \- curLy) \* 0.20;  
  curLh \+= (targetLh \- curLh) \* speed;  
   
  if (curLh \< 1.0) targetLh \= eyeH\_Max;  
}

// \--- DASHBOARD OBD \---  
uint16\_t getSpeedColor(float spd) {  
  if (spd \< 40) return TFT\_BLUE;  
  if (spd \< 80) return TFT\_YELLOW;  
  return TFT\_GREEN;  
}

uint16\_t getRPMColor(float rpm) {  
  if (rpm \< 2000) return TFT\_YELLOW;  
  if (rpm \< 4000) return 0xFA20; // Orange  
  return TFT\_RED;  
}

void drawDashboard(float rpm, float spd, float tmp) {  
  spr.fillSprite(BG\_DARK\_BLUE);  
   
  // Rails de fond (drawArc standard)  
  // x, y, r\_out, r\_in, start, end, color, bg  
  spr.drawArc(120, 120, 115, 100, 150, 390, V\_DARK, BG\_DARK\_BLUE);  
  spr.drawArc(120, 120, 95, 85, 150, 390, V\_DARK, BG\_DARK\_BLUE);

  // Arcs dynamiques  
  int endAngleSpd \= 150 \+ (int)((spd / 120.0) \* 240.0);  
  int endAngleRpm \= 150 \+ (int)((rpm / 5000.0) \* 240.0);  
   
  if(spd \> 0.1) spr.drawArc(120, 120, 115, 100, 150, endAngleSpd, getSpeedColor(spd), BG\_DARK\_BLUE);  
  if(rpm \> 0.1) spr.drawArc(120, 120, 95, 85, 150, endAngleRpm, getRPMColor(rpm), BG\_DARK\_BLUE);

  // Textes  
  spr.setTextColor(TFT\_CYAN); spr.setTextSize(3);  
  spr.setCursor(85, 50); spr.print((int)tmp); spr.setTextSize(1); spr.print(" C");  
   
  spr.setTextColor(TFT\_WHITE); spr.setTextSize(5);  
  int xRpm \= (rpm \< 1000) ? 95 : 75;  
  spr.setCursor(xRpm, 100); spr.print((int)rpm);  
   
  spr.setTextColor(TFT\_YELLOW); spr.setTextSize(4);  
  int xSpd \= (spd \< 100) ? 95 : 75;  
  spr.setCursor(xSpd, 160); spr.print((int)spd);  
  spr.setTextSize(1); spr.print(" km/h");

  spr.pushSprite(0, 0);  
}

// \--- SETUP PRINCIPAL \---  
void setup() {  
  // Reset Ecran  
  pinMode(1, OUTPUT);  
  digitalWrite(1, LOW); delay(100);  
  digitalWrite(1, HIGH); delay(500);

  Serial.begin(115200); // Pour OBD  
  tft.init();  
  tft.setRotation(0);  
  spr.createSprite(SCREEN\_W, SCREEN\_H);  
  spr.setSwapBytes(true);

  // \--- PHASE 1 : YEUX (5 SECONDES) \---  
  targetLh \= eyeH\_Max;  
  targetLx \= (SCREEN\_W \- (eyeW \* 2 \+ spaceBetween)) / 2;  
  targetLy \= (SCREEN\_H \- eyeH\_Max) / 2;  
  curLx \= targetLx; curLy \= targetLy; curLh \= eyeH\_Max;  
   
  blinkTimer \= millis() \+ 2000;  
  idleTimer \= millis() \+ 4000;  
  moodTimer \= millis() \+ 4000;

  unsigned long startEyes \= millis();  
  while (millis() \- startEyes \< 5000) {  
    updateEyesLogic();  
    drawEyesToSprite();  
    spr.pushSprite(0, 0);  
    delay(16);  
  }

  // \--- PHASE 2 : INITIALISATION OBD (AVEC ATTENTE RÉELLE) \---  
  // AT Z affiche la version, donc true pour showVersion  
  if (\!initOBDCommand("AT Z", "RESET", true)) { showError(); }  
   
  // Les autres commandes, on ne s'attend pas forcément à une version spécifique, mais on attend le OK  
  if (\!initOBDCommand("AT E0", "ECHO OFF", false)) { showError(); }  
  if (\!initOBDCommand("AT SP 0", "AUTO PROTOCOL", false)) { showError(); }  
  // Optionnel: AT SH 81 10 F1 pour VW si besoin, décommentez ci-dessous  
  // if (\!initOBDCommand("AT SH 81 10 F1", "VW HEADER", false)) { showError(); }

  // \--- PHASE 3 : ANIMATION LANCEMENT DASHBOARD \---  
  for (int i \= 0; i \<= 100; i \+= 2) {  
    drawDashboard(i \* 50, i \* 1.2, i);  
    delay(20);  
  }  
}

// \--- LOOP PRINCIPAL \---  
void loop() {  
  // SIMULATION DES DONNÉES RÉELLES  
  // Remplacez cette partie par votre logique de lecture Serial.parseOBD()  
   
  realRPM \= random(800, 4800);  
  realSPD \= map(realRPM, 800, 4800, 0, 120);  
  realTMP \= 92;

  drawDashboard(realRPM, realSPD, realTMP);  
  delay(50);  
}

# Onglet 2

eps32s3 zero:

**code: eyes au debut 5sec \+commandes reel \+cadran:**

/\*\*  
 \* RoboOBD\_Final\_Integrated\_Eyes.ino  
 \* 1\. Animation Yeux (Code fourni) pendant 5s.  
 \* 2\. Initialisation OBD réelle (2 tentatives, affichage version, erreur critique si fail).  
 \* 3\. Dashboard OBD complet.  
 \* Pour GC9A01 \+ ESP32-S3-Zero.  
 \*/

\#include \<TFT\_eSPI.h\>

TFT\_eSPI tft \= TFT\_eSPI();  
TFT\_eSprite spr \= TFT\_eSprite(\&tft);

// \--- CONFIGURATION GLOBALE \---  
\#define BGCOLOR       TFT\_BLACK  
\#define EYECOLOR      TFT\_WHITE  
\#define BG\_DARK\_BLUE  0x000A  
\#define V\_DARK        0x0110  
\#define TFT\_YELLOW    0xFFE0 // Correction pour éviter l'erreur 'YELLOW'

// \--- PARAMÈTRES YEUX (Tirés de votre code) \---  
const int SCREEN\_W \= 240;  
const int SCREEN\_H \= 240;  
int eyeW \= 56;              
int eyeH\_Max \= 56;          
int borderRadius \= 16;      
int spaceBetween \= 24;  
int blinkThickness \= 6;      
int blinkWidth \= 64;        
float blinkThreshold \= 12.0;

// Variables d'animation Yeux  
float curLx, curLy, curLh;  
float targetLx, targetLy, targetLh;

enum Mood { NORMAL, TIRED, ANGRY, HAPPY };  
Mood currentMood \= NORMAL;

unsigned long blinkTimer \= 0;  
unsigned long idleTimer \= 0;  
unsigned long moodTimer \= 0;

// \--- VARIABLES OBD & DASHBOARD \---  
float realRPM \= 0, realSPD \= 0, realTMP \= 0;

// \--- FONCTIONS UTILITAIRES \---  
void printCentered(String s, int y, int size, uint16\_t color) {  
  spr.setTextSize(size);  
  spr.setTextColor(color);  
  int x \= 120 \- (s.length() \* 6 \* size / 2);  
  spr.setCursor(x, y);  
  spr.print(s);  
}

// \--- GESTION OBD RÉELLE (AVEC ATTENTE RÉPONSE) \---  
bool sendOBD(const char\* cmd, String \*responseStr) {  
  Serial.println(cmd); // Envoi de la commande  
   
  \*responseStr \= ""; // Reset de la réponse  
  unsigned long timeout \= millis();  
  bool promptFound \= false;  
   
  while (millis() \- timeout \< 2000) { // Timeout 2 secondes  
    if (Serial.available()) {  
      char c \= Serial.read();  
      // On ignore les retours chariot et sauts de ligne pour le nettoyage, mais on les garde dans le buffer si besoin  
      \*responseStr \+= c;  
       
      if (c \== '\>') { // Le prompt ELM327 indique que la commande est finie  
        promptFound \= true;  
        break;  
      }  
    }  
  }  
   
  if (promptFound) {  
    // Nettoyage de la chaîne pour afficher la version (enlève le '\>' final et les \\r\\n)  
    responseStr\-\>trim();  
    return true;  
  }  
  return false;  
}

bool initOBDCommand(const char\* cmd, const char\* title, bool showVersion) {  
  String response \= "";  
   
  for (int attempt \= 1; attempt \<= 2; attempt++) {  
    // Affichage ENVOI  
    spr.fillSprite(0x9009); // Purple  
    printCentered("SENDING " \+ String(attempt) \+ "/2", 60, 2, TFT\_WHITE);  
    printCentered(String(cmd), 110, 3, TFT\_WHITE);  
    printCentered(String(title), 160, 2, TFT\_WHITE);  
    spr.pushSprite(0, 0);  
     
    if (sendOBD(cmd, \&response)) {  
      // Succès \!  
      spr.fillSprite(0x04A9); // Green  
      printCentered("OK", 90, 4, TFT\_WHITE);  
       
      if (showVersion && response.length() \> 0) {  
         // Affiche la version (ex: "ELM327 v1.5") si c'est la commande AT Z  
         printCentered(response, 130, 2, TFT\_WHITE);  
      }  
       
      spr.pushSprite(0, 0);  
      delay(800); // Pause pour voir le OK  
      return true;  
    }  
    // Échec, on retente si attempt \< 2  
    delay(500);  
  }  
  return false;  
}

void showError() {  
  spr.fillSprite(TFT\_RED);  
  printCentered("NOT CONNECTED", 110, 2, TFT\_WHITE);  
  spr.pushSprite(0, 0);  
  while(1); // Arrêt total  
}

// \--- LOGIQUE DESSIN YEUX (Votre code) \---  
void drawEyesToSprite() {  
  spr.fillSprite(BGCOLOR);

  int drawW, drawH, drawRadius, finalY;  
  int xL, xR;

  if (curLh \< blinkThreshold) {  
    // PHASE TRAIT  
    drawW \= blinkWidth;  
    drawH \= blinkThickness;  
    drawRadius \= 3;  
    finalY \= (int)curLy \+ (eyeH\_Max / 2) \- (blinkThickness / 2);  
    xL \= (int)curLx \- (blinkWidth \- eyeW) / 2;  
    xR \= xL \+ blinkWidth \+ (spaceBetween \- (blinkWidth \- eyeW));  
  } else {  
    // PHASE NORMALE  
    drawW \= eyeW;  
    drawH \= (int)curLh;  
    drawRadius \= borderRadius;  
    finalY \= (int)curLy \+ (eyeH\_Max \- drawH) / 2;  
    xL \= (int)curLx;  
    xR \= xL \+ eyeW \+ spaceBetween;  
  }

  // Dessin  
  spr.fillRoundRect(xL, finalY, drawW, drawH, drawRadius, EYECOLOR);  
  spr.fillRoundRect(xR, finalY, drawW, drawH, drawRadius, EYECOLOR);

  // ÉMOTIONS  
  if (curLh \>= blinkThreshold) {  
    int mH \= drawH / 2;  
    if (currentMood \== ANGRY) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL \+ eyeW, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR, finalY \+ mH, BGCOLOR);  
    }  
    else if (currentMood \== TIRED) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR \+ eyeW, finalY \+ mH, BGCOLOR);  
    }  
    else if (currentMood \== HAPPY) {  
      spr.fillRoundRect(xL \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
      spr.fillRoundRect(xR \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
    }  
  }  
}

void updateEyesLogic() {  
  unsigned long now \= millis();  
   
  // Cycle des émotions  
  if (now \> moodTimer) {  
    if (currentMood \== NORMAL) currentMood \= TIRED;  
    else if (currentMood \== TIRED) currentMood \= ANGRY;  
    else if (currentMood \== ANGRY) currentMood \= HAPPY;  
    else currentMood \= NORMAL;  
    moodTimer \= now \+ 4000;  
  }

  // Clignement  
  if (now \> blinkTimer) {  
    targetLh \= 0;  
    blinkTimer \= now \+ random(2000, 5000);  
  }

  // Mouvements  
  if (now \> idleTimer) {  
    targetLx \= random(40, 90);  
    targetLy \= random(70, 105);  
    idleTimer \= now \+ random(2000, 6000);  
  }

  // Interpolation  
  float speed \= (targetLh \== 0 || curLh \< 10) ? 0.40 : 0.20;  
  curLx \+= (targetLx \- curLx) \* 0.20;  
  curLy \+= (targetLy \- curLy) \* 0.20;  
  curLh \+= (targetLh \- curLh) \* speed;  
   
  if (curLh \< 1.0) targetLh \= eyeH\_Max;  
}

// \--- DASHBOARD OBD \---  
uint16\_t getSpeedColor(float spd) {  
  if (spd \< 40) return TFT\_BLUE;  
  if (spd \< 80) return TFT\_YELLOW;  
  return TFT\_GREEN;  
}

uint16\_t getRPMColor(float rpm) {  
  if (rpm \< 2000) return TFT\_YELLOW;  
  if (rpm \< 4000) return 0xFA20; // Orange  
  return TFT\_RED;  
}

void drawDashboard(float rpm, float spd, float tmp) {  
  spr.fillSprite(BG\_DARK\_BLUE);  
   
  // Rails de fond (drawArc standard)  
  // x, y, r\_out, r\_in, start, end, color, bg  
  spr.drawArc(120, 120, 115, 100, 150, 390, V\_DARK, BG\_DARK\_BLUE);  
  spr.drawArc(120, 120, 95, 85, 150, 390, V\_DARK, BG\_DARK\_BLUE);

  // Arcs dynamiques  
  int endAngleSpd \= 150 \+ (int)((spd / 120.0) \* 240.0);  
  int endAngleRpm \= 150 \+ (int)((rpm / 5000.0) \* 240.0);  
   
  if(spd \> 0.1) spr.drawArc(120, 120, 115, 100, 150, endAngleSpd, getSpeedColor(spd), BG\_DARK\_BLUE);  
  if(rpm \> 0.1) spr.drawArc(120, 120, 95, 85, 150, endAngleRpm, getRPMColor(rpm), BG\_DARK\_BLUE);

  // Textes  
  spr.setTextColor(TFT\_CYAN); spr.setTextSize(3);  
  spr.setCursor(85, 50); spr.print((int)tmp); spr.setTextSize(1); spr.print(" C");  
   
  spr.setTextColor(TFT\_WHITE); spr.setTextSize(5);  
  int xRpm \= (rpm \< 1000) ? 95 : 75;  
  spr.setCursor(xRpm, 100); spr.print((int)rpm);  
   
  spr.setTextColor(TFT\_YELLOW); spr.setTextSize(4);  
  int xSpd \= (spd \< 100) ? 95 : 75;  
  spr.setCursor(xSpd, 160); spr.print((int)spd);  
  spr.setTextSize(1); spr.print(" km/h");

  spr.pushSprite(0, 0);  
}

// \--- SETUP PRINCIPAL \---  
void setup() {  
  // Reset Ecran  
  pinMode(1, OUTPUT);  
  digitalWrite(1, LOW); delay(100);  
  digitalWrite(1, HIGH); delay(500);

  Serial.begin(115200); // Pour OBD  
  tft.init();  
  tft.setRotation(0);  
  spr.createSprite(SCREEN\_W, SCREEN\_H);  
  spr.setSwapBytes(true);

  // \--- PHASE 1 : YEUX (5 SECONDES) \---  
  targetLh \= eyeH\_Max;  
  targetLx \= (SCREEN\_W \- (eyeW \* 2 \+ spaceBetween)) / 2;  
  targetLy \= (SCREEN\_H \- eyeH\_Max) / 2;  
  curLx \= targetLx; curLy \= targetLy; curLh \= eyeH\_Max;  
   
  blinkTimer \= millis() \+ 2000;  
  idleTimer \= millis() \+ 4000;  
  moodTimer \= millis() \+ 4000;

  unsigned long startEyes \= millis();  
  while (millis() \- startEyes \< 5000) {  
    updateEyesLogic();  
    drawEyesToSprite();  
    spr.pushSprite(0, 0);  
    delay(16);  
  }

  // \--- PHASE 2 : INITIALISATION OBD (AVEC ATTENTE RÉELLE) \---  
  // AT Z affiche la version, donc true pour showVersion  
  if (\!initOBDCommand("AT Z", "RESET", true)) { showError(); }  
   
  // Les autres commandes, on ne s'attend pas forcément à une version spécifique, mais on attend le OK  
  if (\!initOBDCommand("AT E0", "ECHO OFF", false)) { showError(); }  
  if (\!initOBDCommand("AT SP 0", "AUTO PROTOCOL", false)) { showError(); }  
  // Optionnel: AT SH 81 10 F1 pour VW si besoin, décommentez ci-dessous  
  // if (\!initOBDCommand("AT SH 81 10 F1", "VW HEADER", false)) { showError(); }

  // \--- PHASE 3 : ANIMATION LANCEMENT DASHBOARD \---  
  for (int i \= 0; i \<= 100; i \+= 2) {  
    drawDashboard(i \* 50, i \* 1.2, i);  
    delay(20);  
  }  
}

// \--- LOOP PRINCIPAL \---  
void loop() {  
  // SIMULATION DES DONNÉES RÉELLES  
  // Remplacez cette partie par votre logique de lecture Serial.parseOBD()  
   
  realRPM \= random(800, 4800);  
  realSPD \= map(realRPM, 800, 4800, 0, 120);  
  realTMP \= 92;

  drawDashboard(realRPM, realSPD, realTMP);  
  delay(50);  
}

# eyes\_emotions

/\*\*  
 \* RoboEyes\_CleanBlink\_S3Zero.ino  
 \* Animation de clignement fluide sans déformation bizarre.  
 \* Pour GC9A01 \+ Waveshare ESP32-S3-Zero.  
 \*/

\#include \<TFT\_eSPI.h\>

TFT\_eSPI tft \= TFT\_eSPI();  
TFT\_eSprite spr \= TFT\_eSprite(\&tft);

\#define BGCOLOR    TFT\_BLACK  
\#define EYECOLOR   TFT\_WHITE

const int SCREEN\_W \= 240;  
const int SCREEN\_H \= 240;

// \--- Paramètres Géométriques \---  
int eyeW \= 56;              
int eyeH\_Max \= 56;          
int borderRadius \= 16;      
int spaceBetween \= 24;

// \--- Paramètres du Clignement \---  
int blinkThickness \= 6;      
int blinkWidth \= 64;        
float blinkThreshold \= 12.0; // Seuil de transition

// Variables d'animation  
float curLx, curLy, curLh;  
float targetLx, targetLy, targetLh;

enum Mood { NORMAL, TIRED, ANGRY, HAPPY };  
Mood currentMood \= NORMAL;

unsigned long lastUpdate \= 0;  
const int frameInterval \= 16;  
unsigned long blinkTimer \= 0;  
unsigned long idleTimer \= 0;  
unsigned long moodTimer \= 0;

void setup() {  
  pinMode(1, OUTPUT);  
  digitalWrite(1, LOW);  
  delay(100);  
  digitalWrite(1, HIGH);  
  delay(500);

  tft.init();  
  tft.setRotation(0);  
  spr.createSprite(SCREEN\_W, SCREEN\_H);  
  spr.setSwapBytes(true);

  targetLh \= eyeH\_Max;  
  targetLx \= (SCREEN\_W \- (eyeW \* 2 \+ spaceBetween)) / 2;  
  targetLy \= (SCREEN\_H \- eyeH\_Max) / 2;

  curLx \= targetLx; curLy \= targetLy; curLh \= eyeH\_Max;  
   
  blinkTimer \= millis() \+ 2000;  
  idleTimer \= millis() \+ 4000;  
  moodTimer \= millis() \+ 4000;  
}

void loop() {  
  unsigned long now \= millis();

  if (now \- lastUpdate \>= frameInterval) {  
    lastUpdate \= now;

    // 1\. Cycle des émotions  
    if (now \> moodTimer) {  
      if (currentMood \== NORMAL) currentMood \= TIRED;  
      else if (currentMood \== TIRED) currentMood \= ANGRY;  
      else if (currentMood \== ANGRY) currentMood \= HAPPY;  
      else currentMood \= NORMAL;  
      moodTimer \= now \+ 4000;  
    }

    // 2\. Clignement (On cible 0 pour une fermeture complète)  
    if (now \> blinkTimer) {  
      targetLh \= 0;  
      blinkTimer \= now \+ random(2000, 5000);  
    }

    // 3\. Mouvements  
    if (now \> idleTimer) {  
      targetLx \= random(40, 90);  
      targetLy \= random(70, 105);  
      idleTimer \= now \+ random(2000, 6000);  
    }

    // 4\. Interpolation NERVEUSE (Plus rapide pour le clignement)  
    float speed \= (targetLh \== 0 || curLh \< 10) ? 0.40 : 0.20;  
    curLx \+= (targetLx \- curLx) \* 0.20;  
    curLy \+= (targetLy \- curLy) \* 0.20;  
    curLh \+= (targetLh \- curLh) \* speed;  
     
    // Réouverture immédiate dès que c'est fermé  
    if (curLh \< 1.0) targetLh \= eyeH\_Max;

    // 5\. DESSIN LOGIQUE  
    spr.fillSprite(BGCOLOR);

    int drawW, drawH, drawRadius, finalY;  
    int xL, xR;

    if (curLh \< blinkThreshold) {  
      // PHASE TRAIT : On affiche le trait fixe  
      drawW \= blinkWidth;  
      drawH \= blinkThickness;  
      drawRadius \= 3;  
      finalY \= (int)curLy \+ (eyeH\_Max / 2) \- (blinkThickness / 2);  
      xL \= (int)curLx \- (blinkWidth \- eyeW) / 2;  
      xR \= xL \+ blinkWidth \+ (spaceBetween \- (blinkWidth \- eyeW));  
    } else {  
      // PHASE NORMALE : L'oeil scale en hauteur normalement  
      drawW \= eyeW;  
      drawH \= (int)curLh;  
      drawRadius \= borderRadius;  
      finalY \= (int)curLy \+ (eyeH\_Max \- drawH) / 2; // Centrage vertical pendant le scale  
      xL \= (int)curLx;  
      xR \= xL \+ eyeW \+ spaceBetween;  
    }

    // Dessin  
    spr.fillRoundRect(xL, finalY, drawW, drawH, drawRadius, EYECOLOR);  
    spr.fillRoundRect(xR, finalY, drawW, drawH, drawRadius, EYECOLOR);

    // 6\. ÉMOTIONS (Seulement en mode normal)  
    if (curLh \>= blinkThreshold) {  
      int mH \= drawH / 2;  
      if (currentMood \== ANGRY) {  
        spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL \+ eyeW, finalY \+ mH, BGCOLOR);  
        spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR, finalY \+ mH, BGCOLOR);  
      }  
      else if (currentMood \== TIRED) {  
        spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL, finalY \+ mH, BGCOLOR);  
        spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR \+ eyeW, finalY \+ mH, BGCOLOR);  
      }  
      else if (currentMood \== HAPPY) {  
        spr.fillRoundRect(xL \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
        spr.fillRoundRect(xR \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
      }  
    }

    spr.pushSprite(0, 0);  
  }  
}

# command test obd

\#include \<TFT\_eSPI.h\>  
\#include \<SPI.h\>

TFT\_eSPI tft \= TFT\_eSPI();

// \--- CONFIGURATION TROUVÉE \---  
\#define TX\_PIN 8    // GPIO8 \- TX vers ELM327  
\#define RX\_PIN 9    // GPIO9 \- RX depuis ELM327  
\#define OBD\_BAUD 38400  
\#define OBD\_CONFIG SERIAL\_8N1

// \--- COULEURS \---  
\#define BACKGROUND 0x0000  
\#define WHITE 0xFFFF  
\#define RED 0xF800  
\#define GREEN 0x07E0  
\#define YELLOW 0xFFE0  
\#define BLUE 0x001F  
\#define CYAN 0x07FF

void setup() {  
  Serial.begin(115200);  
  delay(1000);  
    
  tft.init();  
  tft.setRotation(0);  
  tft.fillScreen(BACKGROUND);  
    
  Serial.println("\\n=== CONFIGURATION OBD TROUVÉE \===");  
  Serial.println("Baud: 38400 8N1");  
  Serial.println("TX: GPIO8, RX: GPIO9");  
    
  // Afficher configuration  
  tft.setTextColor(GREEN);  
  tft.setTextSize(3);  
  tft.setCursor(20, 20);  
  tft.print("OBD OK\!");  
    
  tft.setTextSize(2);  
  tft.setCursor(20, 60);  
  tft.print("38400 8N1");  
    
  tft.setTextSize(1);  
  tft.setTextColor(WHITE);  
  tft.setCursor(20, 90);  
  tft.print("TX: GPIO8 \-\> OBD RX");  
  tft.setCursor(20, 110);  
  tft.print("RX: GPIO9 \<- OBD TX");  
    
  delay(2000);  
    
  // Initialiser Serial1  
  Serial1.begin(OBD\_BAUD, OBD\_CONFIG, RX\_PIN, TX\_PIN);  
  delay(100);  
    
  // Nettoyer buffer  
  while(Serial1.available()) {  
    Serial1.read();  
  }  
    
  // Tester la connexion  
  testConnection();  
}

void testConnection() {  
  tft.fillRect(0, 130, 240, 110, BACKGROUND);  
    
  // Test 1: ATZ (Reset)  
  tft.setTextColor(CYAN);  
  tft.setTextSize(2);  
  tft.setCursor(20, 130);  
  tft.print("1. ATZ...");  
    
  Serial1.println("ATZ");  
  delay(800);  
    
  String response \= readOBDResponse(2000);  
    
  tft.setTextSize(1);  
  tft.setCursor(20, 155);  
    
  if(response.length() \> 0\) {  
    tft.setTextColor(YELLOW);  
    tft.print("Reponse: ");  
      
    // Nettoyer l'affichage  
    String display \= cleanResponse(response);  
    if(display.length() \> 20\) {  
      tft.print(display.substring(0, 20));  
    } else {  
      tft.print(display);  
    }  
      
    Serial.print("ATZ \-\> ");  
    Serial.println(response);  
  } else {  
    tft.setTextColor(RED);  
    tft.print("Pas de reponse");  
    Serial.println("ATZ \-\> No response");  
  }  
    
  delay(1500);  
    
  // Test 2: ATI (Information)  
  tft.fillRect(20, 170, 200, 40, BACKGROUND);  
  tft.setTextColor(CYAN);  
  tft.setTextSize(2);  
  tft.setCursor(20, 170);  
  tft.print("2. ATI...");  
    
  Serial1.println("ATI");  
  delay(500);  
    
  response \= readOBDResponse(1500);  
    
  tft.setTextSize(1);  
  tft.setCursor(20, 195);  
    
  if(response.length() \> 0\) {  
    tft.setTextColor(YELLOW);  
    tft.print("Reponse: ");  
      
    String display \= cleanResponse(response);  
    if(display.length() \> 20\) {  
      tft.print(display.substring(0, 20));  
    } else {  
      tft.print(display);  
    }  
      
    Serial.print("ATI \-\> ");  
    Serial.println(response);  
  } else {  
    tft.setTextColor(RED);  
    tft.print("Pas de reponse");  
    Serial.println("ATI \-\> No response");  
  }  
    
  delay(1500);  
    
  // Test 3: ATRV (Tension batterie)  
  tft.fillRect(20, 210, 200, 40, BACKGROUND);  
  tft.setTextColor(CYAN);  
  tft.setTextSize(2);  
  tft.setCursor(20, 210);  
  tft.print("3. ATRV...");  
    
  Serial1.println("ATRV");  
  delay(500);  
    
  response \= readOBDResponse(1500);  
    
  tft.setTextSize(1);  
  tft.setCursor(20, 235);  
    
  if(response.length() \> 0\) {  
    tft.setTextColor(YELLOW);  
    tft.print("Reponse: ");  
      
    String display \= cleanResponse(response);  
    if(display.length() \> 20\) {  
      tft.print(display.substring(0, 20));  
    } else {  
      tft.print(display);  
    }  
      
    Serial.print("ATRV \-\> ");  
    Serial.println(response);  
  } else {  
    tft.setTextColor(RED);  
    tft.print("Pas de reponse");  
    Serial.println("ATRV \-\> No response");  
  }  
    
  // Mode prêt  
  delay(2000);  
  tft.fillScreen(BACKGROUND);  
  tft.setTextColor(GREEN);  
  tft.setTextSize(3);  
  tft.setCursor(30, 50);  
  tft.print("PRET\!");  
    
  tft.setTextSize(2);  
  tft.setTextColor(WHITE);  
  tft.setCursor(30, 100);  
  tft.print("Envoyez commandes");  
  tft.setCursor(50, 130);  
  tft.print("via Serial");  
    
  Serial.println("\\n=== MODE OBD ACTIF \===");  
  Serial.println("Envoyez commandes OBD:");  
  Serial.println("- ATZ (reset)");  
  Serial.println("- ATI (info)");  
  Serial.println("- ATRV (tension)");  
  Serial.println("- ATSP0 (auto protocol)");  
  Serial.println("- 0100 (test PID)");  
}

String readOBDResponse(unsigned long timeout) {  
  String response \= "";  
  unsigned long start \= millis();  
    
  while(millis() \- start \< timeout) {  
    if(Serial1.available()) {  
      char c \= Serial1.read();  
      response \+= c;  
      if(c \== '\>') {  
        break;  
      }  
    }  
  }  
    
  return response;  
}

String cleanResponse(String input) {  
  String output \= "";  
  for(int i \= 0; i \< input.length(); i++) {  
    char c \= input\[i\];  
    if(c \== '\\r') {  
      output \+= "\\\\r";  
    } else if(c \== '\\n') {  
      output \+= "\\\\n";  
    } else if(c \== '\>') {  
      output \+= "\>";  
    } else if(c \>= 32 && c \<= 126\) {  
      output \+= c;  
    } else {  
      output \+= "?";  
    }  
  }  
  return output;  
}

void loop() {  
  // Mode commande manuelle  
  if(Serial.available()) {  
    String cmd \= Serial.readStringUntil('\\n');  
    cmd.trim();  
      
    if(cmd.length() \> 0\) {  
      Serial.print("\>\>\> ");  
      Serial.println(cmd);  
        
      // Envoyer à l'ELM327  
      Serial1.println(cmd);  
      delay(200);  
        
      // Lire réponse  
      String response \= readOBDResponse(3000);  
      Serial.print("\<\<\< ");  
      Serial.println(response);  
        
      // Afficher sur écran  
      tft.fillScreen(BACKGROUND);  
      tft.setTextColor(CYAN);  
      tft.setTextSize(2);  
      tft.setCursor(10, 20);  
      tft.print("Commande:");  
        
      tft.setTextColor(WHITE);  
      tft.setTextSize(3);  
      tft.setCursor(10, 50);  
      if(cmd.length() \> 15\) {  
        tft.print(cmd.substring(0, 15));  
      } else {  
        tft.print(cmd);  
      }  
        
      tft.setTextColor(YELLOW);  
      tft.setTextSize(2);  
      tft.setCursor(10, 90);  
      tft.print("Reponse:");  
        
      tft.setTextSize(1);  
      tft.setCursor(10, 120);  
        
      String display \= cleanResponse(response);  
      // Afficher en plusieurs lignes si nécessaire  
      int line \= 0;  
      for(int i \= 0; i \< display.length(); i \+= 30\) {  
        int endPos \= (i \+ 30 \< display.length()) ? i \+ 30 : display.length();  
        tft.setCursor(10, 140 \+ line \* 15);  
        tft.print(display.substring(i, endPos));  
        line++;  
        if(line \>= 5\) break;  
      }  
    }  
  }  
    
  delay(10);  
}

# version0 code eye+command+cadran

/\*\*  
 \* RoboOBD\_Final\_S3Zero.ino  
 \* \- Délai affichage : 1s minimum pour commande et réponse.  
 \* \- Alignement : Textes centrés (2, 3, 4 chiffres).  
 \* \- Arcs : Début à 60° (Position 40min).  
 \* \- OBD : Vraies valeurs 10x/sec \+ Header VW.  
 \*/

\#include \<TFT\_eSPI.h\>  
\#include \<SPI.h\>

TFT\_eSPI tft \= TFT\_eSPI();  
TFT\_eSprite spr \= TFT\_eSprite(\&tft);

// \--- CONFIGURATION OBD \---  
\#define OBD\_TX\_PIN 8  
\#define OBD\_RX\_PIN 9  
\#define OBD\_BAUD 38400

// \--- CONFIGURATION ÉCRAN & COULEURS \---  
\#define BGCOLOR       TFT\_BLACK  
\#define EYECOLOR      TFT\_WHITE  
\#define BG\_DARK\_BLUE  0x0005  
\#define V\_DARK        0x0110  
\#define TFT\_YELLOW    0xFFE0  
\#define START\_ANGLE   60 // Position 40 min (Bas-Gauche)

// \--- PARAMÈTRES YEUX \---  
const int SCREEN\_W \= 240;  
const int SCREEN\_H \= 240;  
int eyeW \= 56;              
int eyeH\_Max \= 56;          
int borderRadius \= 16;      
int spaceBetween \= 24;  
int blinkThickness \= 6;      
int blinkWidth \= 64;        
float blinkThreshold \= 12.0;

float curLx, curLy, curLh;  
float targetLx, targetLy, targetLh;

enum Mood { NORMAL, TIRED, ANGRY, HAPPY };  
Mood currentMood \= NORMAL;

unsigned long blinkTimer \= 0;  
unsigned long idleTimer \= 0;  
unsigned long moodTimer \= 0;

// \--- VARIABLES OBD & DASHBOARD \---  
float realRPM \= 0, realSPD \= 0, realTMP \= 0;

// \--- FONCTIONS UTILITAIRES \---  
void printCentered(String s, int y, int size, uint16\_t color) {  
  spr.setTextSize(size);  
  spr.setTextColor(color);  
  // Calcul du centrage horizontal (X \= 120\) basé sur la longueur du texte  
  int x \= 120 \- (s.length() \* 6 \* size / 2);  
  spr.setCursor(x, y);  
  spr.print(s);  
}

// \--- GESTION OBD \---  
String readOBDResponse(unsigned long timeout) {  
  String response \= "";  
  unsigned long start \= millis();  
  while(millis() \- start \< timeout) {  
    if(Serial1.available()) {  
      char c \= Serial1.read();  
      response \+= c;  
      if(c \== '\>') break;  
    }  
  }  
  return response;  
}

String cleanResponse(String input) {  
  String output \= "";  
  for(int i \= 0; i \< input.length(); i++) {  
    char c \= input\[i\];  
    if(c \== '\\r' || c \== '\\n' || c \== ' ' || c \== '\>') continue;  
    if(c \>= 32 && c \<= 126) output \+= c;  
  }  
  return output;  
}

// Lecture des vraies valeurs  
int readSpeed() {  
  Serial1.println("010D");  
  delay(30);  
  String r \= readOBDResponse(150);  
  String clean \= cleanResponse(r);  
  int idx \= clean.indexOf("410D");  
  if (idx \!= \-1 && clean.length() \>= idx \+ 6) {  
    return (int)strtol(clean.substring(idx \+ 4, idx \+ 6).c\_str(), NULL, 16);  
  }  
  return \-1;  
}

int readRPM() {  
  Serial1.println("010C");  
  delay(30);  
  String r \= readOBDResponse(150);  
  String clean \= cleanResponse(r);  
  int idx \= clean.indexOf("410C");  
  if (idx \!= \-1 && clean.length() \>= idx \+ 8) {  
    int a \= (int)strtol(clean.substring(idx \+ 4, idx \+ 6).c\_str(), NULL, 16);  
    int b \= (int)strtol(clean.substring(idx \+ 6, idx \+ 8).c\_str(), NULL, 16);  
    return ((a \* 256) \+ b) / 4;  
  }  
  return \-1;  
}

int readTemp() {  
  Serial1.println("0105");  
  delay(30);  
  String r \= readOBDResponse(150);  
  String clean \= cleanResponse(r);  
  int idx \= clean.indexOf("4105");  
  if (idx \!= \-1 && clean.length() \>= idx \+ 6) {  
    return (int)strtol(clean.substring(idx \+ 4, idx \+ 6).c\_str(), NULL, 16) \- 40;  
  }  
  return \-1;  
}

// Initialisation OBD avec timing 1s  
bool initOBDDiagnostic(const char\* cmd, const char\* title, bool showData) {  
  String response \= "";  
   
  for (int attempt \= 1; attempt \<= 2; attempt++) {  
    // 1\. Affichage ENVOI (1s min)  
    spr.fillSprite(0x9009); // Purple  
    printCentered("DIAGNOSTIC", 30, 2, TFT\_WHITE);  
    if (attempt \> 1) { String txt \= "TENTATIVE " \+ String(attempt) \+ "/2"; printCentered(txt, 55, 1, TFT\_YELLOW); }  
    printCentered(String(cmd), 90, 3, TFT\_CYAN);  
    printCentered(String(title), 125, 2, TFT\_WHITE);  
    printCentered("ENVOI...", 160, 2, TFT\_WHITE);  
    spr.pushSprite(0, 0);  
     
    delay(1000); // Pause 1s pour voir la commande

    // 2\. Envoi  
    Serial1.println(cmd);  
    delay(150);  
    response \= readOBDResponse(1000);  
     
    if (response.length() \> 0 && response.indexOf("\>") \!= \-1) {  
      String cleanResp \= cleanResponse(response);  
       
      // 3\. Affichage RÉPONSE (1s min)  
      spr.fillSprite(0x04A9); // Green  
      printCentered("OK", 40, 4, TFT\_WHITE);  
      printCentered(String(cmd), 80, 2, TFT\_WHITE);  
       
      if (showData && cleanResp.length() \> 0) {  
        String displayData \= cleanResp;  
        if(displayData.length() \> 15) displayData \= displayData.substring(0, 15);  
        printCentered(displayData, 130, 3, TFT\_YELLOW);  
        printCentered(title, 170, 2, TFT\_WHITE);  
      } else {  
        printCentered("DONE", 130, 3, TFT\_WHITE);  
      }  
      spr.pushSprite(0, 0);  
       
      delay(1000); // Pause 1s pour voir la réponse  
      return true;  
    }  
  }  
  return false;  
}

void showError() {  
  spr.fillSprite(TFT\_RED);  
  printCentered("NOT CONNECTED", 110, 2, TFT\_WHITE);  
  printCentered("CHECK OBD PINS", 140, 1, TFT\_WHITE);  
  spr.pushSprite(0, 0);  
  while(1);  
}

// \--- LOGIQUE YEUX \---  
void drawEyesToSprite() {  
  spr.fillSprite(BGCOLOR);  
  int drawW, drawH, drawRadius, finalY;  
  int xL, xR;

  if (curLh \< blinkThreshold) {  
    drawW \= blinkWidth; drawH \= blinkThickness; drawRadius \= 3;  
    finalY \= (int)curLy \+ (eyeH\_Max / 2) \- (blinkThickness / 2);  
    xL \= (int)curLx \- (blinkWidth \- eyeW) / 2;  
    xR \= xL \+ blinkWidth \+ (spaceBetween \- (blinkWidth \- eyeW));  
  } else {  
    drawW \= eyeW; drawH \= (int)curLh; drawRadius \= borderRadius;  
    finalY \= (int)curLy \+ (eyeH\_Max \- drawH) / 2;  
    xL \= (int)curLx; xR \= xL \+ eyeW \+ spaceBetween;  
  }

  spr.fillRoundRect(xL, finalY, drawW, drawH, drawRadius, EYECOLOR);  
  spr.fillRoundRect(xR, finalY, drawW, drawH, drawRadius, EYECOLOR);

  if (curLh \>= blinkThreshold) {  
    int mH \= drawH / 2;  
    if (currentMood \== ANGRY) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL \+ eyeW, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR, finalY \+ mH, BGCOLOR);  
    } else if (currentMood \== TIRED) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR \+ eyeW, finalY \+ mH, BGCOLOR);  
    } else if (currentMood \== HAPPY) {  
      spr.fillRoundRect(xL \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
      spr.fillRoundRect(xR \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
    }  
  }  
}

void updateEyesLogic() {  
  unsigned long now \= millis();  
  if (now \> moodTimer) {  
    if (currentMood \== NORMAL) currentMood \= TIRED;  
    else if (currentMood \== TIRED) currentMood \= ANGRY;  
    else if (currentMood \== ANGRY) currentMood \= HAPPY;  
    else currentMood \= NORMAL;  
    moodTimer \= now \+ 4000;  
  }  
  if (now \> blinkTimer) { targetLh \= 0; blinkTimer \= now \+ random(2000, 5000); }  
  if (now \> idleTimer) { targetLx \= random(40, 90); targetLy \= random(70, 105); idleTimer \= now \+ random(2000, 6000); }  
  float speed \= (targetLh \== 0 || curLh \< 10) ? 0.40 : 0.20;  
  curLx \+= (targetLx \- curLx) \* 0.20; curLy \+= (targetLy \- curLy) \* 0.20; curLh \+= (targetLh \- curLh) \* speed;  
  if (curLh \< 1.0) targetLh \= eyeH\_Max;  
}

// \--- DASHBOARD \---  
uint16\_t getSpeedColor(float spd) {  
  if (spd \< 40) return TFT\_BLUE;  
  if (spd \< 80) return TFT\_YELLOW;  
  return TFT\_GREEN;  
}

uint16\_t getRPMColor(float rpm) {  
  if (rpm \< 2000) return TFT\_YELLOW;  
  if (rpm \< 4000) return 0xFA20;  
  return TFT\_RED;  
}

void drawDashboard(float rpm, float spd, float tmp) {  
  spr.fillSprite(BG\_DARK\_BLUE);  
   
  // Arcs: Commencent à 60 (Position 40min)  
  // Sweep de 240° va de 60 à 375 (15°) \-\> Couvre le bas à gauche jusqu'en haut à droite  
   
  // Rail Vitesse (Extérieur)  
  spr.drawArc(120, 120, 115, 100, START\_ANGLE, START\_ANGLE \+ 240, V\_DARK, BG\_DARK\_BLUE);  
  // Rail RPM (Intérieur)  
  spr.drawArc(120, 120, 95, 85, START\_ANGLE, START\_ANGLE \+ 240, V\_DARK, BG\_DARK\_BLUE);

  // Calcul angle fin  
  int endAngleSpd \= START\_ANGLE \+ (int)((spd / 120.0) \* 240.0);  
  int endAngleRpm \= START\_ANGLE \+ (int)((rpm / 5000.0) \* 240.0);  
   
  if(spd \> 0.1) spr.drawArc(120, 120, 115, 100, START\_ANGLE, endAngleSpd, getSpeedColor(spd), BG\_DARK\_BLUE);  
  if(rpm \> 0.1) spr.drawArc(120, 120, 95, 85, START\_ANGLE, endAngleRpm, getRPMColor(rpm), BG\_DARK\_BLUE);

  // Textes centrés dynamiquement (2,3,4 chiffres)  
  // Température  
  spr.setTextColor(TFT\_CYAN); spr.setTextSize(3);  
  printCentered(String((int)tmp), 60, 3, TFT\_CYAN);  
  spr.setTextSize(1); spr.setCursor(120 \+ 20, 60\+10); spr.print("C"); // Unité à droite  
   
  // RPM  
  spr.setTextColor(TFT\_WHITE); spr.setTextSize(5);  
  printCentered(String((int)rpm), 105, 5, TFT\_WHITE);  
   
  // Vitesse  
  spr.setTextColor(TFT\_YELLOW); spr.setTextSize(4);  
  printCentered(String((int)spd), 165, 4, TFT\_YELLOW);  
  spr.setTextSize(1); spr.setCursor(120 \+ 25, 165\+10); spr.print("km/h"); // Unité à droite

  spr.pushSprite(0, 0);  
}

// \--- SETUP \---  
void setup() {  
  pinMode(1, OUTPUT);  
  digitalWrite(1, LOW); delay(100);  
  digitalWrite(1, HIGH); delay(500);

  Serial.begin(115200);  
  tft.init();  
  tft.setRotation(0);  
  spr.createSprite(SCREEN\_W, SCREEN\_H);  
  spr.setSwapBytes(true);

  Serial1.begin(OBD\_BAUD, SERIAL\_8N1, OBD\_RX\_PIN, OBD\_TX\_PIN);  
  delay(100);  
  while(Serial1.available()) { Serial1.read(); }

  // \--- PHASE 1 : YEUX (5s) \---  
  targetLh \= eyeH\_Max;  
  targetLx \= (SCREEN\_W \- (eyeW \* 2 \+ spaceBetween)) / 2;  
  targetLy \= (SCREEN\_H \- eyeH\_Max) / 2;  
  curLx \= targetLx; curLy \= targetLy; curLh \= eyeH\_Max;  
  blinkTimer \= millis() \+ 2000; idleTimer \= millis() \+ 4000; moodTimer \= millis() \+ 4000;

  unsigned long startEyes \= millis();  
  while (millis() \- startEyes \< 5000) {  
    updateEyesLogic();  
    drawEyesToSprite();  
    spr.pushSprite(0, 0);  
    delay(16);  
  }

  // \--- PHASE 2 : INITIALISATION OBD (Timing 1s) \---  
  if (\!initOBDDiagnostic("ATZ", "RESET", true)) { showError(); }  
  if (\!initOBDDiagnostic("ATI", "VERSION", true)) { showError(); }  
  if (\!initOBDDiagnostic("ATRV", "VOLTAGE", true)) { showError(); }  
  if (\!initOBDDiagnostic("ATSP0", "AUTO PROTO", false)) { showError(); }  
  if (\!initOBDDiagnostic("AT SH 81 10 F1", "VW HEADER", false)) { showError(); }  
  if (\!initOBDDiagnostic("AT E0", "ECHO OFF", false)) { showError(); }

  // \--- PHASE 3 : ANIMATION LANCEMENT \---  
  for (int i \= 0; i \<= 100; i \+= 2) {  
    drawDashboard(i \* 50, i \* 1.2, i);  
    delay(20);  
  }  
}

// \--- LOOP \---  
void loop() {  
  int tmpRPM \= readRPM();  
  int tmpSPD \= readSpeed();  
  int tmpTMP \= readTemp();

  if (tmpRPM \!= \-1) realRPM \= tmpRPM;  
  if (tmpSPD \!= \-1) realSPD \= tmpSPD;  
  if (tmpTMP \!= \-1) realTMP \= tmpTMP;

  drawDashboard(realRPM, realSPD, realTMP);  
}

# version 0.1 eye+command+cadran

/\*\*  
 \* RoboOBD\_Final\_FuelCons\_S3Zero.ino  
 \* \- Commande AT E0 supprimée.  
 \* \- Arcs à bouts plats (plus de caps ronds).  
 \* \- Affichage Fuel et Consommation (01 5E) sur même ligne.  
 \* \- Fuel rapproché du centre.  
 \*/

\#include \<TFT\_eSPI.h\>  
\#include \<SPI.h\>

TFT\_eSPI tft \= TFT\_eSPI();  
TFT\_eSprite spr \= TFT\_eSprite(\&tft);

// \--- CONFIGURATION OBD \---  
\#define OBD\_TX\_PIN 8  
\#define OBD\_RX\_PIN 9  
\#define OBD\_BAUD 38400

// \--- CONFIGURATION ÉCRAN & COULEURS \---  
\#define BGCOLOR       TFT\_BLACK  
\#define EYECOLOR      TFT\_WHITE  
\#define BG\_DARK\_BLUE  0x0005  
\#define V\_DARK        0x0821  
\#define TFT\_YELLOW    0xFFE0  
\#define START\_ANGLE   60

// \--- PARAMÈTRES YEUX \---  
const int SCREEN\_W \= 240;  
const int SCREEN\_H \= 240;  
int eyeW \= 56;              
int eyeH\_Max \= 56;          
int borderRadius \= 16;      
int spaceBetween \= 24;  
int blinkThickness \= 6;      
int blinkWidth \= 64;        
float blinkThreshold \= 12.0;

float curLx, curLy, curLh;  
float targetLx, targetLy, targetLh;

enum Mood { NORMAL, TIRED, ANGRY, HAPPY };  
Mood currentMood \= NORMAL;

unsigned long blinkTimer \= 0;  
unsigned long idleTimer \= 0;  
unsigned long moodTimer \= 0;

// \--- VARIABLES OBD & DASHBOARD \---  
float realRPM \= 0, realSPD \= 0, realTMP \= 0;  
float fuelLevel \= 0;   // %  
float estRange \= 0;    // km  
float fuelCons \= 0;     // L/100km (Valeur brute)

// \--- FONCTIONS UTILITAIRES \---  
void printCentered(String s, int y, int size, uint16\_t color) {  
  spr.setTextSize(size);  
  spr.setTextColor(color);  
  int x \= 120 \- (s.length() \* 6 \* size / 2);  
  spr.setCursor(x, y);  
  spr.print(s);  
}

// \--- GESTION OBD \---  
String readOBDResponse(unsigned long timeout) {  
  String response \= "";  
  unsigned long start \= millis();  
  while(millis() \- start \< timeout) {  
    if(Serial1.available()) {  
      char c \= Serial1.read();  
      response \+= c;  
      if(c \== '\>') break;  
    }  
  }  
  return response;  
}

String cleanResponse(String input) {  
  String output \= "";  
  for(int i \= 0; i \< input.length(); i++) {  
    char c \= input\[i\];  
    if(c \== '\\r' || c \== '\\n' || c \== ' ' || c \== '\>') continue;  
    if(c \>= 32 && c \<= 126) output \+= c;  
  }  
  return output;  
}

// Lecture des PIDs  
int readSpeed() {  
  Serial1.println("010D"); delay(30);  
  String r \= readOBDResponse(150); String clean \= cleanResponse(r);  
  int idx \= clean.indexOf("410D");  
  if (idx \!= \-1 && clean.length() \>= idx \+ 6) return (int)strtol(clean.substring(idx \+ 4, idx \+ 6).c\_str(), NULL, 16);  
  return \-1;  
}

int readRPM() {  
  Serial1.println("010C"); delay(30);  
  String r \= readOBDResponse(150); String clean \= cleanResponse(r);  
  int idx \= clean.indexOf("410C");  
  if (idx \!= \-1 && clean.length() \>= idx \+ 8) {  
    int a \= (int)strtol(clean.substring(idx \+ 4, idx \+ 6).c\_str(), NULL, 16);  
    int b \= (int)strtol(clean.substring(idx \+ 6, idx \+ 8).c\_str(), NULL, 16);  
    return ((a \* 256) \+ b) / 4;  
  }  
  return \-1;  
}

int readTemp() {  
  Serial1.println("0105"); delay(30);  
  String r \= readOBDResponse(150); String clean \= cleanResponse(r);  
  int idx \= clean.indexOf("4105");  
  if (idx \!= \-1 && clean.length() \>= idx \+ 6) return (int)strtol(clean.substring(idx \+ 4, idx \+ 6).c\_str(), NULL, 16) \- 40;  
  return \-1;  
}

int readFuel() {  
  Serial1.println("012F"); delay(30);  
  String r \= readOBDResponse(150); String clean \= cleanResponse(r);  
  int idx \= clean.indexOf("412F");  
  if (idx \!= \-1 && clean.length() \>= idx \+ 6) return (int)strtol(clean.substring(idx \+ 4, idx \+ 6).c\_str(), NULL, 16);  
  return \-1;  
}

int readRange() {  
  Serial1.println("015F"); delay(30);  
  String r \= readOBDResponse(150); String clean \= cleanResponse(r);  
  int idx \= clean.indexOf("415F");  
  if (idx \!= \-1 && clean.length() \>= idx \+ 8) {  
    int a \= (int)strtol(clean.substring(idx \+ 4, idx \+ 6).c\_str(), NULL, 16);  
    int b \= (int)strtol(clean.substring(idx \+ 6, idx \+ 8).c\_str(), NULL, 16);  
    return (a \* 256) \+ b;  
  }  
  return \-1;  
}

float readFuelConsumption() {  
  // 01 5E \-\> 41 5E XX (Formule standard : (A \* 100\) / 255\)  
  // Note: Selon la voiture, cela peut être variable.  
  Serial1.println("015E"); delay(30);  
  String r \= readOBDResponse(150); String clean \= cleanResponse(r);  
  int idx \= clean.indexOf("415E");  
  if (idx \!= \-1 && clean.length() \>= idx \+ 6) {  
    int val \= (int)strtol(clean.substring(idx \+ 4, idx \+ 6).c\_str(), NULL, 16);  
    return (val \* 100.0) / 255.0;  
  }  
  return \-1;  
}

bool initOBDDiagnostic(const char\* cmd, const char\* title, bool showData) {  
  String response \= "";  
  for (int attempt \= 1; attempt \<= 2; attempt++) {  
    spr.fillSprite(0x9009);  
    printCentered("DIAGNOSTIC", 30, 2, TFT\_WHITE);  
    if (attempt \> 1) { String txt \= "TENTATIVE " \+ String(attempt) \+ "/2"; printCentered(txt, 55, 1, TFT\_YELLOW); }  
    printCentered(String(cmd), 90, 3, TFT\_CYAN);  
    printCentered(String(title), 125, 2, TFT\_WHITE);  
    printCentered("ENVOI...", 160, 2, TFT\_WHITE);  
    spr.pushSprite(0, 0); delay(1000);

    Serial1.println(cmd); delay(150);  
    response \= readOBDResponse(1000);  
     
    if (response.length() \> 0 && response.indexOf("\>") \!= \-1) {  
      String cleanResp \= cleanResponse(response);  
      spr.fillSprite(0x04A9);  
      printCentered("OK", 40, 4, TFT\_WHITE);  
      printCentered(String(cmd), 80, 2, TFT\_WHITE);  
      if (showData && cleanResp.length() \> 0) {  
        String displayData \= cleanResp;  
        if(displayData.length() \> 15) displayData \= displayData.substring(0, 15);  
        printCentered(displayData, 130, 3, TFT\_YELLOW);  
        printCentered(title, 170, 2, TFT\_WHITE);  
      } else {  
        printCentered("DONE", 130, 3, TFT\_WHITE);  
      }  
      spr.pushSprite(0, 0); delay(1000);  
      return true;  
    }  
  }  
  return false;  
}

void showError() {  
  spr.fillSprite(TFT\_RED);  
  printCentered("NOT CONNECTED", 110, 2, TFT\_WHITE);  
  printCentered("CHECK OBD PINS", 140, 1, TFT\_WHITE);  
  spr.pushSprite(0, 0);  
  while(1);  
}

// \--- LOGIQUE YEUX \---  
void drawEyesToSprite() {  
  spr.fillSprite(BGCOLOR);  
  int drawW, drawH, drawRadius, finalY;  
  int xL, xR;

  if (curLh \< blinkThreshold) {  
    drawW \= blinkWidth; drawH \= blinkThickness; drawRadius \= 3;  
    finalY \= (int)curLy \+ (eyeH\_Max / 2) \- (blinkThickness / 2);  
    xL \= (int)curLx \- (blinkWidth \- eyeW) / 2;  
    xR \= xL \+ blinkWidth \+ (spaceBetween \- (blinkWidth \- eyeW));  
  } else {  
    drawW \= eyeW; drawH \= (int)curLh; drawRadius \= borderRadius;  
    finalY \= (int)curLy \+ (eyeH\_Max \- drawH) / 2;  
    xL \= (int)curLx; xR \= xL \+ eyeW \+ spaceBetween;  
  }

  spr.fillRoundRect(xL, finalY, drawW, drawH, drawRadius, EYECOLOR);  
  spr.fillRoundRect(xR, finalY, drawW, drawH, drawRadius, EYECOLOR);

  if (curLh \>= blinkThreshold) {  
    int mH \= drawH / 2;  
    if (currentMood \== ANGRY) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL \+ eyeW, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR, finalY \+ mH, BGCOLOR);  
    } else if (currentMood \== TIRED) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR \+ eyeW, finalY \+ mH, BGCOLOR);  
    } else if (currentMood \== HAPPY) {  
      spr.fillRoundRect(xL \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
      spr.fillRoundRect(xR \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
    }  
  }  
}

void updateEyesLogic() {  
  unsigned long now \= millis();  
  if (now \> moodTimer) {  
    if (currentMood \== NORMAL) currentMood \= TIRED;  
    else if (currentMood \== TIRED) currentMood \= ANGRY;  
    else if (currentMood \== ANGRY) currentMood \= HAPPY;  
    else currentMood \= NORMAL;  
    moodTimer \= now \+ 4000;  
  }  
  if (now \> blinkTimer) { targetLh \= 0; blinkTimer \= now \+ random(2000, 5000); }  
  if (now \> idleTimer) { targetLx \= random(40, 90); targetLy \= random(70, 105); idleTimer \= now \+ random(2000, 6000); }  
  float speed \= (targetLh \== 0 || curLh \< 10) ? 0.40 : 0.20;  
  curLx \+= (targetLx \- curLx) \* 0.20; curLy \+= (targetLy \- curLy) \* 0.20; curLh \+= (targetLh \- curLh) \* speed;  
  if (curLh \< 1.0) targetLh \= eyeH\_Max;  
}

// \--- DASHBOARD \---  
uint16\_t getSpeedColor(float spd) {  
  if (spd \< 40) return TFT\_BLUE;  
  if (spd \< 80) return TFT\_YELLOW;  
  return TFT\_GREEN;  
}

uint16\_t getRPMColor(float rpm) {  
  if (rpm \< 2000) return TFT\_YELLOW;  
  if (rpm \< 4000) return 0xFA20;  
  return TFT\_RED;  
}

void drawDashboard(float rpm, float spd, float tmp) {  
  spr.fillSprite(BG\_DARK\_BLUE);  
   
  // Rails (Sans bouts ronds)  
  spr.drawArc(120, 120, 115, 100, START\_ANGLE, START\_ANGLE \+ 240, V\_DARK, BG\_DARK\_BLUE);  
  spr.drawArc(120, 120, 95, 85, START\_ANGLE, START\_ANGLE \+ 240, V\_DARK, BG\_DARK\_BLUE);

  // Arcs Dynamiques  
  int endAngleSpd \= START\_ANGLE \+ (int)((spd / 120.0) \* 240.0);  
  int endAngleRpm \= START\_ANGLE \+ (int)((rpm / 5000.0) \* 240.0);  
   
  if(spd \> 0.1) spr.drawArc(120, 120, 115, 100, START\_ANGLE, endAngleSpd, getSpeedColor(spd), BG\_DARK\_BLUE);  
  if(rpm \> 0.1) spr.drawArc(120, 120, 95, 85, START\_ANGLE, endAngleRpm, getRPMColor(rpm), BG\_DARK\_BLUE);

  // Textes Principaux  
  spr.setTextColor(TFT\_CYAN); spr.setTextSize(3);  
  printCentered(String((int)tmp), 60, 3, TFT\_CYAN);  
  spr.setTextSize(1); spr.setCursor(120 \+ 20, 60\+10); spr.print("C");  
   
  spr.setTextColor(TFT\_WHITE); spr.setTextSize(5);  
  printCentered(String((int)rpm), 105, 5, TFT\_WHITE);  
   
  spr.setTextColor(TFT\_YELLOW); spr.setTextSize(4);  
  printCentered(String((int)spd), 165, 4, TFT\_YELLOW);  
  spr.setTextSize(1); spr.setCursor(120 \+ 25, 165\+10); spr.print("km/h");

  // \--- FUEL & CONSOMMATION (Ligne 195\) \---  
  spr.setTextSize(1);  
  spr.setTextColor(0xAAAA); // Label couleur  
   
  // Fuel (Gauche, rapproché du centre)  
  String fuelTxt \= String((int)fuelLevel) \+ "%";  
  spr.setCursor(50, 195);  
  spr.print("FUEL:");  
  spr.setTextColor(TFT\_WHITE);  
  spr.print(fuelTxt);

  // Consommation (Droite, sur même ligne)  
  String consTxt \= String(fuelCons, 1) \+ "L/100";  
  spr.setTextColor(0xAAAA);  
  spr.setCursor(140, 195); // Position ajustée pour serrer  
  spr.print("CONS:");  
  spr.setTextColor(TFT\_WHITE);  
  spr.print(consTxt);

  // \--- AUTONOMIE (Ligne 215\) \---  
  String rangeTxt \= String((int)estRange) \+ "km";  
  // Centré exactement sur l'axe vertical  
  printCentered("RNG: " \+ rangeTxt, 215, 1, TFT\_WHITE);

  spr.pushSprite(0, 0);  
}

// \--- SETUP \---  
void setup() {  
  pinMode(1, OUTPUT);  
  digitalWrite(1, LOW); delay(100);  
  digitalWrite(1, HIGH); delay(500);

  Serial.begin(115200);  
  tft.init();  
  tft.setRotation(0);  
  spr.createSprite(SCREEN\_W, SCREEN\_H);  
  spr.setSwapBytes(true);

  Serial1.begin(OBD\_BAUD, SERIAL\_8N1, OBD\_RX\_PIN, OBD\_TX\_PIN);  
  delay(100);  
  while(Serial1.available()) { Serial1.read(); }

  // \--- PHASE 1 : YEUX (5s) \---  
  targetLh \= eyeH\_Max;  
  targetLx \= (SCREEN\_W \- (eyeW \* 2 \+ spaceBetween)) / 2;  
  targetLy \= (SCREEN\_H \- eyeH\_Max) / 2;  
  curLx \= targetLx; curLy \= targetLy; curLh \= eyeH\_Max;  
  blinkTimer \= millis() \+ 2000; idleTimer \= millis() \+ 4000; moodTimer \= millis() \+ 4000;

  unsigned long startEyes \= millis();  
  while (millis() \- startEyes \< 5000) {  
    updateEyesLogic();  
    drawEyesToSprite();  
    spr.pushSprite(0, 0);  
    delay(16);  
  }

  // \--- PHASE 2 : INITIALISATION OBD \---  
  if (\!initOBDDiagnostic("ATZ", "RESET", true)) { showError(); }  
  if (\!initOBDDiagnostic("ATI", "VERSION", true)) { showError(); }  
  if (\!initOBDDiagnostic("ATRV", "VOLTAGE", true)) { showError(); }  
  if (\!initOBDDiagnostic("ATSP0", "AUTO PROTO", false)) { showError(); }  
  if (\!initOBDDiagnostic("AT SH 81 10 F1", "VW HEADER", false)) { showError(); }  
  // AT E0 supprimé comme demandé

  // \--- PHASE 3 : ANIMATION LANCEMENT \---  
  for (int i \= 0; i \<= 100; i \+= 2) {  
    drawDashboard(i \* 50, i \* 1.2, i);  
    delay(20);  
  }  
}

// \--- LOOP \---  
void loop() {  
  int tmpRPM \= readRPM();  
  int tmpSPD \= readSpeed();  
  int tmpTMP \= readTemp();  
  int tmpFuel \= readFuel();  
  int tmpRange \= readRange();  
  float tmpCons \= readFuelConsumption();

  if (tmpRPM \!= \-1) realRPM \= tmpRPM;  
  if (tmpSPD \!= \-1) realSPD \= tmpSPD;  
  if (tmpTMP \!= \-1) realTMP \= tmpTMP;  
  if (tmpFuel \!= \-1) fuelLevel \= tmpFuel;  
  if (tmpRange \!= \-1) estRange \= tmpRange;  
  if (tmpCons \!= \-1) fuelCons \= tmpCons;

  drawDashboard(realRPM, realSPD, realTMP);  
}

# 40 ommandes test filamre

/\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*  
   TEST COMPLET OBD \- 40 COMMANDES  
   ESP32 \+ ELM327 \- Header VW 81 10 F1  
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*/

\#define RX\_PIN     9  
\#define TX\_PIN     8  
\#define OBD\_BAUD   38400

void setup() {  
  Serial.begin(115200);  
  delay(800);  
  Serial.println("\\n=== TEST COMPLET OBD \- 40 COMMANDES \===\\n");

  Serial1.begin(OBD\_BAUD, SERIAL\_8N1, RX\_PIN, TX\_PIN);  
  delay(300);

  // Initialisation ELM327  
  sendCommand("ATZ",   "Reset adaptateur");  
  sendCommand("ATE0",  "Désactiver echo");  
  sendCommand("AT SH 81 10 F1", "Header VW");  
  delay(1000);

  Serial.println("\\n=== DÉBUT DES TESTS \===\\n");

  // 1\. Commandes AT de base  
  runSection("=== COMMANDES AT DE BASE \===");  
  testPID("ATI",    "Version ELM327");          // ← changé ici  
  testPID("ATRV",   "Tension batterie");        // ← changé ici  
  testPID("ATDP",   "Protocole actuel");        // ← changé ici  
  testPID("ATDPN",  "Numéro protocole");        // ← changé ici

  // 2\. PIDs Standard Mode 01  
  runSection("=== PIDS STANDARD OBD-II (Mode 01\) \===");  
  testPID("0100", "PIDs supportés \[01-20\]");  
  testPID("0101", "Statut MIL \+ moniteurs");  
  testPID("0104", "Charge moteur calculée");  
  testPID("0105", "Température liquide refroidissement");  
  testPID("0106", "Correction carburant court terme B1");  
  testPID("010B", "Pression admission");  
  testPID("010C", "Régime moteur (RPM)");  
  testPID("010D", "Vitesse véhicule");  
  testPID("010E", "Avance allumage");  
  testPID("010F", "Température air admission");  
  testPID("0110", "Débit air MAF");  
  testPID("0111", "Position papillon");  
  testPID("0114", "Tension sonde O2 B1S1");  
  testPID("011F", "Temps depuis démarrage");  
  testPID("0121", "Distance avec MIL allumé");

  // 3\. Diagnostics & Info véhicule  
  runSection("=== DIAGNOSTICS & INFO VEHICULE \===");  
  testMode("03",   "Codes d'erreur (DTCs)");  
  testMode("09",   "Mode 09 \- VIN");  
  testMode("0902", "Numéro VIN");  
  testMode("0904", "Numéro calibration ECU");

  // 4\. PIDs Spécifiques VW (Mode 22\)  
  runSection("=== PIDS SPECIFIQUES VW (Mode 22\) \===");  
  testVW("22114F", "Masse suie DPF (g)");  
  testVW("22114E", "Masse suie mesurée");  
  testVW("22178C", "Cendres huile DPF");  
  testVW("221156", "Distance depuis dernière régénération DPF");  
  testVW("22115E", "Temps depuis dernière régénération");  
  testVW("2211B2", "Température entrée DPF");  
  testVW("2210F9", "Température sortie DPF");  
  testVW("221001", "Pression différentielle DPF");  
  testVW("220101", "Position vanne EGR (%)");  
  testVW("220104", "Pression turbo");

  Serial.println("\\n=== TEST TERMINÉ \===\\n");  
  Serial.println("Regarde les réponses qui commencent par '41' → données valides");  
  Serial.println("7F 01 12 → PID non supporté par ta voiture");  
}

void loop() {  
  // Rien ici \- le test se fait une seule fois au démarrage  
}

// \==============================================================  
// Fonctions utilitaires (inchangées)  
// \==============================================================

void runSection(String title) {  
  Serial.println("═══════════════════════════════════════════════");  
  Serial.println(title);  
  Serial.println("═══════════════════════════════════════════════");  
  delay(800);  
}

void sendCommand(String cmd, String desc) {  
  Serial.print("→ ");  
  Serial.print(desc);  
  Serial.print(" (");  
  Serial.print(cmd);  
  Serial.print(") → ");

  Serial1.println(cmd);  
  delay(120);

  String resp \= readResponse(1500);  
  Serial.println(resp);  
}

void testPID(String pid, String desc) {  
  Serial.print(pid);  
  Serial.print(" → ");  
  Serial.print(desc);  
  Serial.print(" : ");

  Serial1.println(pid);  
  delay(80);

  String resp \= readResponse(1400);  
  Serial.println(resp);

  if (resp.indexOf("41") \>= 0) {  
    Serial.println("   → OK (réponse valide)");  
  } else if (resp.indexOf("7F") \>= 0) {  
    Serial.println("   → Non supporté (7F)");  
  } else if (resp.indexOf("OK") \>= 0) {  
    Serial.println("   → OK (commande AT réussie)");  
  }  
  Serial.println();  
}

void testVW(String pid, String desc) {  
  testPID(pid, desc \+ " (VW)");  
}

void testMode(String mode, String desc) {  
  testPID(mode, desc);  
}

String readResponse(unsigned long timeout) {  
  String response \= "";  
  unsigned long start \= millis();  
  while (millis() \- start \< timeout) {  
    if (Serial1.available()) {  
      char c \= Serial1.read();  
      response \+= c;  
      if (c \== '\>') break;  
    }  
  }  
  // Nettoyage minimal  
  response.replace("\\r", "");  
  response.replace("\\n", " ");  
  return response;  
}

RESULTS 👍

\=== TEST COMPLET OBD \- 40 COMMANDES \===

→ Reset adaptateur (ATZ) → �ELM327 v1.5\>  
→ Désactiver echo (ATE0) → ATE0OK\>  
→ Header VW (AT SH 81 10 F1) → OK\>

\=== DÉBUT DES TESTS \===

═══════════════════════════════════════════════  
\=== COMMANDES AT DE BASE \===  
═══════════════════════════════════════════════  
ATI → Version ELM327 : ELM327 v1.5\>

ATRV → Tension batterie : 13.9V\>

ATDP → Protocole actuel : ISO 14230-4 (KWP FAST)\>

ATDPN → Numéro protocole : 5\>

═══════════════════════════════════════════════  
\=== PIDS STANDARD OBD-II (Mode 01\) \===  
═══════════════════════════════════════════════  
0100 → PIDs supportés \[01-20\] : BUS INIT: OK41 00 BE 3E F8 11 \>  
   → OK (réponse valide)

0101 → Statut MIL \+ moniteurs : 41 01 00 07 65 65 \>  
   → OK (réponse valide)

0104 → Charge moteur calculée : 41 04 26 \>  
   → OK (réponse valide)

0105 → Température liquide refroidissement : 41 05 6A \>  
   → OK (réponse valide)

0106 → Correction carburant court terme B1 : 41 06 80 \>  
   → OK (réponse valide)

010B → Pression admission : 41 0B 1F \>  
   → OK (réponse valide)

010C → Régime moteur (RPM) : 41 0C 0D 64 \>  
   → OK (réponse valide)

010D → Vitesse véhicule : 41 0D 00 \>  
   → OK (réponse valide)

010E → Avance allumage : 41 0E 87 \>  
   → OK (réponse valide)

010F → Température air admission : 41 0F 49 \>  
   → OK (réponse valide)

0110 → Débit air MAF : 7F 01 12 \>  
   → Non supporté (7F)

0111 → Position papillon : 41 11 0E \>  
   → OK (réponse valide)

0114 → Tension sonde O2 B1S1 : 41 14 1D 80 \>  
   → OK (réponse valide)

011F → Temps depuis démarrage : 7F 01 12 \>  
   → Non supporté (7F)

0121 → Distance avec MIL allumé : 41 21 00 00 \>  
   → OK (réponse valide)

═══════════════════════════════════════════════  
\=== DIAGNOSTICS & INFO VEHICULE \===  
═══════════════════════════════════════════════  
03 → Codes d'erreur (DTCs) : 43 00 00 00 00 00 00 \>

09 → Mode 09 \- VIN : 49 \>

0902 → Numéro VIN : 49 02 \>

0904 → Numéro calibration ECU : 49 04 01 30 33 45 39 49 04 03 33 47 20 20 49 04 04 39 35 31 31 \>

═══════════════════════════════════════════════  
\=== PIDS SPECIFIQUES VW (Mode 22\) \===  
═══════════════════════════════════════════════  
22114F → Masse suie DPF (g) (VW) : 7F 22 11 \>  
   → Non supporté (7F)

22114E → Masse suie mesurée (VW) : 7F 22 11 \>  
   → Non supporté (7F)

22178C → Cendres huile DPF (VW) : 7F 22 11 \>  
   → Non supporté (7F)

221156 → Distance depuis dernière régénération DPF (VW) : 7F 22 11 \>  
   → Non supporté (7F)

22115E → Temps depuis dernière régénération (VW) : 7F 22 11 \>  
   → Non supporté (7F)

2211B2 → Température entrée DPF (VW) : 7F 22 11 \>  
   → Non supporté (7F)

2210F9 → Température sortie DPF (VW) : 7F 22 11 \>  
   → Non supporté (7F)

221001 → Pression différentielle DPF (VW) : 7F 22 11 \>  
   → Non supporté (7F)

220101 → Position vanne EGR (%) (VW) : 7F 22 11 \>  
   → Non supporté (7F)

220104 → Pression turbo (VW) : 7F 22 11 \>  
   → Non supporté (7F)

\=== TEST TERMINÉ \===

Regarde les réponses qui commencent par '41' → données valides  
7F 01 12 → PID non supporté par ta voiture

GROK commente les resultat 👍

\*\*Super résultat \!\*\*    
Ton test est très réussi : la communication est stable, la voiture répond bien à la majorité des PIDs standards, et on a maintenant une bonne vue d’ensemble de ce qui est disponible sur ton véhicule.

Voici une synthèse claire de ce que tu peux réellement utiliser, et ce qui n’est pas disponible.

\#\#\# Résumé des données disponibles (ce qui marche)

| PID     | Description                                      | Réponse exemple      | Valeur décodée (approximative)              | Statut     |  
|---------|--------------------------------------------------|----------------------|----------------------------------------------|------------|  
| 0100    | PIDs supportés \[01-20\]                           | 41 00 BE 3E F8 11   | (support partiel des PIDs 01–20)            | OK         |  
| 0101    | Statut MIL \+ moniteurs                           | 41 01 00 07 65 65   | MIL éteint, certains moniteurs prêts        | OK         |  
| 0104    | Charge moteur calculée                           | 41 04 26            | \~15 %                                        | OK         |  
| 0105    | Température liquide refroidissement              | 41 05 6A            | 106 °C                                       | OK         |  
| 0106    | Correction carburant court terme B1              | 41 06 80            | 0 % (parfait)                                | OK         |  
| 010B    | Pression absolue collecteur admission            | 41 0B 1F            | 31 kPa                                       | OK         |  
| 010C    | Régime moteur                                    | 41 0C 0D 64         | \~860 tr/min                                  | OK         |  
| 010D    | Vitesse véhicule                                 | 41 0D 00            | 0 km/h                                       | OK         |  
| 010E    | Avance allumage                                  | 41 0E 87            | \~9.5° avant PMH                              | OK         |  
| 010F    | Température air admission                        | 41 0F 49            | 73 °C                                        | OK         |  
| 0111    | Position papillon                                | 41 11 0E            | \~5.5 %                                       | OK         |  
| 0114    | Tension sonde O2 B1S1                            | 41 14 1D 80         | \~0.15 V \+ trim 0 %                           | OK         |  
| 0121    | Distance avec MIL allumé                         | 41 21 00 00         | 0 km (pas de voyant moteur allumé)           | OK         |

\#\#\# Ce qui \*\*ne fonctionne pas\*\* (non supporté sur ta voiture)

| PID     | Description                                      | Réponse              | Signification                                |  
|---------|--------------------------------------------------|----------------------|----------------------------------------------|  
| 0110    | Débit air MAF                                    | 7F 01 12             | Non supporté                                 |  
| 011F    | Temps depuis démarrage                           | 7F 01 12             | Non supporté                                 |  
| Tous les Mode 22 (VW spécifiques) | Masse suie DPF, pression DPF, EGR, turbo, etc.   | 7F 22 11             | Non supporté (ta voiture n’a pas ces PIDs)   |

\#\#\# Observations importantes

1\. \*\*Ta voiture ne semble pas avoir de DPF actif ou de PIDs spécifiques VW en Mode 22\*\*    
   → Cela arrive souvent sur des moteurs essence, des petits moteurs diesel sans DPF, ou des modèles plus anciens.    
   → Les PIDs DPF/EGR/turbo en Mode 22 ne sont pas universels ; ils dépendent du calculateur et du moteur.

2\. \*\*Ton véhicule supporte bien les PIDs classiques moteur/émissions\*\*    
   → Tu as tout ce qu’il faut pour un tableau de bord temps réel intéressant :    
     \- Vitesse    
     \- Régime moteur    
     \- Température moteur    
     \- Température air    
     \- Position papillon    
     \- Charge moteur    
     \- Correction carburant    
     \- Avance allumage    
     \- Tension sonde lambda

3\. \*\*Protocole détecté\*\* : \*\*ISO 14230-4 (KWP FAST)\*\*    
   → C’est un protocole relativement ancien (souvent sur VW/Audi/Skoda jusqu’aux années \~2010–2015).    
   → Cela explique pourquoi certains PIDs modernes (MAF, temps depuis démarrage, DPF) ne sont pas présents.

\#\#\# Suggestions pour la suite

Tu peux maintenant construire un \*\*affichage temps réel\*\* intéressant avec les PIDs qui marchent.

Voici les \*\*meilleurs candidats\*\* pour un dashboard :

| Priorité | PID   | Valeur à afficher                | Formule de calcul                          | Affichage conseillé          |  
|----------|-------|----------------------------------|--------------------------------------------|-------------------------------|  
| ★★★★★    | 010D  | Vitesse                          | A                                          | km/h (gros chiffre jaune)     |  
| ★★★★★    | 010C  | Régime moteur                    | ((A\*256)+B)/4                              | tr/min (orange/rouge)         |  
| ★★★★     | 0105  | Température moteur               | A \- 40                                     | °C (cyan)                     |  
| ★★★★     | 0104  | Charge moteur                    | A / 2.55                                   | % (barre ou chiffre)          |  
| ★★★      | 0111  | Position accélérateur            | A / 2.55                                   | %                             |  
| ★★★      | 010E  | Avance allumage                  | A/2 \- 64                                   | ° avant PMH                   |  
| ★★★      | 010F  | Température air admission        | A \- 40                                     | °C                            |  
| ★★       | 0106  | Correction court terme           | (A \- 128\) / 1.28                           | % (vert si proche de 0\)       |

\#\#\# Veux-tu que je te prépare :

1\. Un \*\*code avec affichage temps réel\*\* sur le moniteur série (vitesse, régime, température, charge) ?  
2\. Un \*\*code avec affichage TFT\*\* (comme ton projet initial avec les yeux \+ dashboard) ?  
3\. Ajouter un \*\*rafraîchissement rapide\*\* (vitesse \+ régime à 10 Hz, autres à 1–2 Hz) ?  
4\. Ajouter une \*\*détection de ralenti / accélération\*\* pour changer l’animation des yeux ?

Dis-moi exactement ce que tu veux faire maintenant et je te prépare le code correspondant. 😄

# code affichage temp rpm en filaire

/\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*  
   DASHBOARD TEMPS REEL \- RPM & TEMPÉRATURE  
   Conversion Hex \-\> Décimal basée sur tes résultats  
   Initialisation : AT SH 81 10 F1 (Indispensable VW)  
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*/

\#include \<TFT\_eSPI.h\>  
\#include \<SPI.h\>  
\#include \<HardwareSerial.h\>

TFT\_eSPI tft \= TFT\_eSPI();

// \--- CONFIGURATION \---  
\#define RX\_PIN     9  
\#define TX\_PIN     8  
\#define OBD\_BAUD   38400

// Variables globales  
int currentRPM \= 0;  
int currentTemp \= 0;

// \--- FONCTIONS OBD (ADAPTÉES DE TON CODE) \---

String readResponse(unsigned long timeout) {  
  String response \= "";  
  unsigned long start \= millis();  
  while (millis() \- start \< timeout) {  
    if (Serial1.available()) {  
      char c \= Serial1.read();  
      response \+= c;  
      if (c \== '\>') break;  
    }  
  }  
  // Nettoyage : on enlève les espaces pour faciliter le parsing  
  response.replace(" ", "");  
  response.replace("\\r", "");  
  response.replace("\\n", "");  
  return response;  
}

void sendCommand(String cmd) {  
  while(Serial1.available()) Serial1.read(); // Flush  
  Serial1.println(cmd);  
  delay(50); // Délai court pour aller plus vite  
}

// \--- FONCTIONS DE CONVERSION (PARSING) \---

int getRPM(String response) {  
  // On cherche "410C" dans la réponse  
  int idx \= response.indexOf("410C");  
  if (idx \!= \-1 && response.length() \>= idx \+ 8\) {  
    // Ex: "410C0D40"  
    // Byte A \= 0D, Byte B \= 40  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    String hexB \= response.substring(idx \+ 6, idx \+ 8);  
      
    int a \= (int)strtol(hexA.c\_str(), NULL, 16);  
    int b \= (int)strtol(hexB.c\_str(), NULL, 16);  
      
    // Formule RPM : ((A\*256) \+ B) / 4  
    return ((a \* 256\) \+ b) / 4;  
  }  
  return \-1;  
}

int getTemp(String response) {  
  // On cherche "4105" dans la réponse  
  int idx \= response.indexOf("4105");  
  if (idx \!= \-1 && response.length() \>= idx \+ 6\) {  
    // Ex: "41056D"  
    // Byte A \= 6D  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    int a \= (int)strtol(hexA.c\_str(), NULL, 16);  
      
    // Formule Temp : A \- 40  
    return a \- 40;  
  }  
  return \-1;  
}

// \--- AFFICHAGE \---

void displayDashboard() {  
  // 1\. Afficher RPM  
  // On efface l'ancien chiffre uniquement (zone estimée : x=100, y=50, largeur=140, hauteur=40)  
  tft.fillRect(100, 50, 140, 40, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(4);  
  tft.setCursor(10, 50);  
  tft.print("RPM: ");  
  tft.setTextColor(TFT\_YELLOW);  
  if(currentRPM \> 0\) tft.print(currentRPM);  
  else tft.print("---");

  // 2\. Afficher Température  
  // On efface l'ancien chiffre  
  tft.fillRect(100, 100, 140, 40, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(4);  
  tft.setCursor(10, 100);  
  tft.print("TMP: ");  
  tft.setTextColor(TFT\_CYAN);  
  if(currentTemp \> \-50) tft.print(currentTemp);  
  else tft.print("---");  
    
  // 3\. Petit indicateur de "Vivant" (clignote si ça tourne)  
  tft.fillCircle(220, 20, 5, (millis() % 1000 \< 500\) ? TFT\_RED : TFT\_BLACK);  
}

void setup() {  
  Serial.begin(115200);  
  delay(500);  
    
  tft.init();  
  tft.setRotation(0);  
  tft.fillScreen(TFT\_BLACK);  
    
  // Titre  
  tft.setTextColor(TFT\_GREEN);  
  tft.setTextSize(2);  
  tft.setCursor(10, 10);  
  tft.print("VW DASHBOARD");

  // \--- INITIALISATION OBD (TON CODE FONCTIONNEL) \---  
  Serial1.begin(OBD\_BAUD, SERIAL\_8N1, RX\_PIN, TX\_PIN);  
  delay(300);

  Serial1.println("ATZ");  
  delay(500);  
  readResponse(2000);  
    
  Serial1.println("ATE0");   
  delay(200);  
  readResponse(1000);  
    
  // LE HEADER MAGIC  
  Serial1.println("AT SH 81 10 F1");  
  delay(200);  
  readResponse(1000);  
    
  delay(1000);  
}

void loop() {  
  // 1\. Lire Température  
  sendCommand("0105");  
  String respTemp \= readResponse(500);  
  currentTemp \= getTemp(respTemp);

  // 2\. Lire RPM  
  sendCommand("010C");  
  String respRPM \= readResponse(500);  
  currentRPM \= getRPM(respRPM);

  // 3\. Afficher  
  displayDashboard();  
    
  // Debug sur PC (optionnel)  
  // Serial.print("RPM: "); Serial.print(currentRPM);  
  // Serial.print(" Temp: "); Serial.println(currentTemp);  
}

# affichage vitesse rpm temp en filaire

/\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*  
   DASHBOARD TEMPS REEL \- RPM & TEMPÉRATURE & VITESSE  
   Conversion Hex \-\> Décimal basée sur tes résultats  
   Initialisation : AT SH 81 10 F1 (Indispensable VW)  
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*/  
\#include \<TFT\_eSPI.h\>  
\#include \<SPI.h\>  
\#include \<HardwareSerial.h\>  
TFT\_eSPI tft \= TFT\_eSPI();

// \--- CONFIGURATION \---  
\#define RX\_PIN 9  
\#define TX\_PIN 8  
\#define OBD\_BAUD 38400

// Variables globales  
int currentRPM \= 0;  
int currentTemp \= 0;  
int currentSpeed \= 0;   // ← NOUVEAU : vitesse en km/h

// \--- FONCTIONS OBD (ADAPTÉES DE TON CODE) \---  
String readResponse(unsigned long timeout) {  
  String response \= "";  
  unsigned long start \= millis();  
  while (millis() \- start \< timeout) {  
    if (Serial1.available()) {  
      char c \= Serial1.read();  
      response \+= c;  
      if (c \== '\>') break;  
    }  
  }  
  // Nettoyage : on enlève les espaces pour faciliter le parsing  
  response.replace(" ", "");  
  response.replace("\\r", "");  
  response.replace("\\n", "");  
  return response;  
}

void sendCommand(String cmd) {  
  while(Serial1.available()) Serial1.read(); // Flush  
  Serial1.println(cmd);  
  delay(50); // Délai court pour aller plus vite  
}

// \--- FONCTIONS DE CONVERSION (PARSING) \---  
int getRPM(String response) {  
  int idx \= response.indexOf("410C");  
  if (idx \!= \-1 && response.length() \>= idx \+ 8\) {  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    String hexB \= response.substring(idx \+ 6, idx \+ 8);  
     
    int a \= (int)strtol(hexA.c\_str(), NULL, 16);  
    int b \= (int)strtol(hexB.c\_str(), NULL, 16);  
     
    return ((a \* 256\) \+ b) / 4;  
  }  
  return \-1;  
}

int getTemp(String response) {  
  int idx \= response.indexOf("4105");  
  if (idx \!= \-1 && response.length() \>= idx \+ 6\) {  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    int a \= (int)strtol(hexA.c\_str(), NULL, 16);  
     
    return a \- 40;  
  }  
  return \-1;  
}

// NOUVEAU : parsing de la vitesse (PID 010D)  
int getSpeed(String response) {  
  int idx \= response.indexOf("410D");  
  if (idx \!= \-1 && response.length() \>= idx \+ 6\) {  
    // Réponse type : 410D XX  
    // XX \= vitesse en km/h (décimal direct)  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    int speed \= (int)strtol(hexA.c\_str(), NULL, 16);  
    return speed;  
  }  
  return \-1;  
}

// \--- AFFICHAGE \---  
void displayDashboard() {  
  // 1\. RPM  
  tft.fillRect(100, 50, 140, 40, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(4);  
  tft.setCursor(10, 50);  
  tft.print("RPM: ");  
  tft.setTextColor(TFT\_YELLOW);  
  if(currentRPM \> 0\) tft.print(currentRPM);  
  else tft.print("---");

  // 2\. Température  
  tft.fillRect(100, 100, 140, 40, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(4);  
  tft.setCursor(10, 100);  
  tft.print("TMP: ");  
  tft.setTextColor(TFT\_CYAN);  
  if(currentTemp \> \-50) tft.print(currentTemp);  
  else tft.print("---");

  // 3\. VITESSE (nouveau)  
  tft.fillRect(100, 150, 140, 40, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(4);  
  tft.setCursor(10, 150);  
  tft.print("VIT: ");  
  tft.setTextColor(TFT\_GREEN);  
  if(currentSpeed \>= 0\) tft.print(currentSpeed);  
  else tft.print("---");

  // Petit indicateur de vie  
  tft.fillCircle(220, 20, 5, (millis() % 1000 \< 500\) ? TFT\_RED : TFT\_BLACK);  
}

void setup() {  
  Serial.begin(115200);  
  delay(500);  
   
  tft.init();  
  tft.setRotation(0);  
  tft.fillScreen(TFT\_BLACK);  
   
  // Titre  
  tft.setTextColor(TFT\_GREEN);  
  tft.setTextSize(2);  
  tft.setCursor(10, 10);  
  tft.print("VW DASHBOARD");  
   
  Serial1.begin(OBD\_BAUD, SERIAL\_8N1, RX\_PIN, TX\_PIN);  
  delay(300);  
  Serial1.println("ATZ");  
  delay(500);  
  readResponse(2000);  
   
  Serial1.println("ATE0");  
  delay(200);  
  readResponse(1000);  
   
  Serial1.println("AT SH 81 10 F1");  
  delay(200);  
  readResponse(1000);  
   
  delay(1000);  
}

void loop() {  
  // 1\. Température  
  sendCommand("0105");  
  String respTemp \= readResponse(500);  
  currentTemp \= getTemp(respTemp);

  // 2\. RPM  
  sendCommand("010C");  
  String respRPM \= readResponse(500);  
  currentRPM \= getRPM(respRPM);

  // 3\. Vitesse (NOUVEAU)  
  sendCommand("010D");  
  String respSpeed \= readResponse(500);  
  currentSpeed \= getSpeed(respSpeed);

  // 4\. Affichage  
  displayDashboard();

  // Debug sur PC (optionnel \- décommente si besoin)  
  // Serial.print("RPM: "); Serial.print(currentRPM);  
  // Serial.print("  Temp: "); Serial.print(currentTemp);  
  // Serial.print("  Speed: "); Serial.println(currentSpeed);  
}

# vitesse+rpm+tx rx

/\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*  
   DASHBOARD TEMPS REEL \- RPM & TEMPÉRATURE & VITESSE \+ TX/RX Voltage  
   Conversion Hex \-\> Décimal basée sur tes résultats  
   Initialisation : AT SH 81 10 F1 (Indispensable VW)  
     
   CORRECTION AFFICHAGE : Taille police réduite (Dezoom 30%)  
   et coordonnées ajustées pour rentrer dans l'écran 240x240.  
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*/  
\#include \<TFT\_eSPI.h\>  
\#include \<SPI.h\>  
\#include \<HardwareSerial.h\>

TFT\_eSPI tft \= TFT\_eSPI();

// \--- CONFIGURATION \---  
\#define RX\_PIN 9  
\#define TX\_PIN 8  
\#define OBD\_BAUD 38400

// Variables globales  
int currentRPM \= 0;  
int currentTemp \= 0;  
int currentSpeed \= 0;

// NOUVEAU : tensions mesurées en volts (valeurs brutes lues sur les pins)  
float txVoltage \= 0.0;  
float rxVoltage \= 0.0;

// \--- FONCTIONS OBD (ADAPTÉES DE TON CODE) \---  
String readResponse(unsigned long timeout) {  
  String response \= "";  
  unsigned long start \= millis();  
  while (millis() \- start \< timeout) {  
    if (Serial1.available()) {  
      char c \= Serial1.read();  
      response \+= c;  
      if (c \== '\>') break;  
    }  
  }  
  // Nettoyage : on enlève les espaces pour faciliter le parsing  
  response.replace(" ", "");  
  response.replace("\\r", "");  
  response.replace("\\n", "");  
  return response;  
}

void sendCommand(String cmd) {  
  while(Serial1.available()) Serial1.read(); // Flush  
  Serial1.println(cmd);  
  delay(50); // Délai court pour aller plus vite  
}

// \--- FONCTIONS DE CONVERSION (PARSING) \---  
int getRPM(String response) {  
  int idx \= response.indexOf("410C");  
  if (idx \!= \-1 && response.length() \>= idx \+ 8) {  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    String hexB \= response.substring(idx \+ 6, idx \+ 8);  
     
    int a \= (int)strtol(hexA.c\_str(), NULL, 16);  
    int b \= (int)strtol(hexB.c\_str(), NULL, 16);  
     
    return ((a \* 256) \+ b) / 4;  
  }  
  return \-1;  
}

int getTemp(String response) {  
  int idx \= response.indexOf("4105");  
  if (idx \!= \-1 && response.length() \>= idx \+ 6) {  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    int a \= (int)strtol(hexA.c\_str(), NULL, 16);  
     
    return a \- 40;  
  }  
  return \-1;  
}

int getSpeed(String response) {  
  int idx \= response.indexOf("410D");  
  if (idx \!= \-1 && response.length() \>= idx \+ 6) {  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    int speed \= (int)strtol(hexA.c\_str(), NULL, 16);  
    return speed;  
  }  
  return \-1;  
}

// \--- NOUVEAU : Lecture des tensions TX et RX (en volts) \---  
void readVoltages() {  
  // Lecture analogique brute (0 à 4095 sur ESP32)  
  int txRaw \= analogRead(TX\_PIN);  
  int rxRaw \= analogRead(RX\_PIN);

  // Conversion en tension (ESP32 3.3V ref, 12 bits \= 4096 niveaux)  
  txVoltage \= (txRaw / 4095.0) \* 3.3;  
  rxVoltage \= (rxRaw / 4095.0) \* 3.3;  
}

// \--- AFFICHAGE \---  
void displayDashboard() {  
  // 1\. RPM (Y=30, Taille 3\)  
  tft.fillRect(90, 30, 150, 30, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(3);  
  tft.setCursor(10, 30);  
  tft.print("RPM: ");  
  tft.setTextColor(TFT\_YELLOW);  
  if(currentRPM \> 0) tft.print(currentRPM);  
  else tft.print("---");

  // 2\. Température (Y=75, Taille 3\)  
  tft.fillRect(90, 75, 150, 30, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(3);  
  tft.setCursor(10, 75);  
  tft.print("TMP: ");  
  tft.setTextColor(TFT\_CYAN);  
  if(currentTemp \> \-50) tft.print(currentTemp);  
  else tft.print("---");

  // 3\. VITESSE (Y=120, Taille 3\)  
  tft.fillRect(90, 120, 150, 30, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(3);  
  tft.setCursor(10, 120);  
  tft.print("VIT: ");  
  tft.setTextColor(TFT\_GREEN);  
  if(currentSpeed \>= 0) tft.print(currentSpeed);  
  else tft.print("---");

  // 4\. TX Voltage (Y=165, Taille 2\)  
  tft.fillRect(70, 165, 170, 25, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(2);    
  tft.setCursor(10, 165);  
  tft.print("TX: ");  
  tft.setTextColor(TFT\_MAGENTA);  
  tft.print(txVoltage, 2);    
  tft.print("V");

  // 5\. RX Voltage (Y=210, Taille 2\) \- REMONTÉ ICI POUR ÊTRE VISIBLE  
  tft.fillRect(70, 210, 170, 25, TFT\_BLACK);  
  tft.setTextColor(TFT\_WHITE, TFT\_BLACK);  
  tft.setTextSize(2);  
  tft.setCursor(10, 210);  
  tft.print("RX: ");  
  tft.setTextColor(TFT\_ORANGE);  
  tft.print(rxVoltage, 2);  
  tft.print("V");

  // Petit indicateur de vie  
  tft.fillCircle(220, 10, 4, (millis() % 1000 \< 500) ? TFT\_RED : TFT\_BLACK);  
}

void setup() {  
  Serial.begin(115200);  
  delay(500);  
   
  tft.init();  
  tft.setRotation(0);  
  tft.fillScreen(TFT\_BLACK);  
   
  // Titre  
  tft.setTextColor(TFT\_GREEN);  
  tft.setTextSize(2);  
  tft.setCursor(10, 5);  
  tft.print("VW DASHBOARD");

  // Configuration des pins analogiques en entrée  
  pinMode(TX\_PIN, INPUT);  
  pinMode(RX\_PIN, INPUT);

  Serial1.begin(OBD\_BAUD, SERIAL\_8N1, RX\_PIN, TX\_PIN);  
  delay(300);  
  Serial1.println("ATZ");  
  delay(500);  
  readResponse(2000);  
   
  Serial1.println("ATE0");  
  delay(200);  
  readResponse(1000);  
   
  Serial1.println("AT SH 81 10 F1");  
  delay(200);  
  readResponse(1000);  
   
  delay(1000);  
}

void loop() {  
  // 1\. Température  
  sendCommand("0105");  
  String respTemp \= readResponse(500);  
  currentTemp \= getTemp(respTemp);

  // 2\. RPM  
  sendCommand("010C");  
  String respRPM \= readResponse(500);  
  currentRPM \= getRPM(respRPM);

  // 3\. Vitesse  
  sendCommand("010D");  
  String respSpeed \= readResponse(500);  
  currentSpeed \= getSpeed(respSpeed);

  // NOUVEAU : lecture des tensions TX et RX  
  readVoltages();

  // 4\. Affichage  
  displayDashboard();

  // Debug sur PC (optionnel \- décommente si besoin)  
  // Serial.print("RPM: "); Serial.print(currentRPM);  
  // Serial.print("  Temp: "); Serial.print(currentTemp);  
  // Serial.print("  Speed: "); Serial.print(currentSpeed);  
  // Serial.print("  TX: "); Serial.print(txVoltage, 2);  
  // Serial.print("V  RX: "); Serial.print(rxVoltage, 2);  
  // Serial.println("V");

  delay(200);  
}

# cadran \+eyes+rpm+temp+speed filaire

/\*\*  
 \* RoboOBD\_Final\_S3Zero\_CORRIGE.ino  
 \* \- Utilise le parsing et l'initialisation du code qui fonctionne  
 \* \- Arcs synchronisés avec vraies valeurs OBD  
 \* \- Délai affichage raisonnable  
 \*/  
\#include \<TFT\_eSPI.h\>  
\#include \<SPI.h\>  
\#include \<HardwareSerial.h\>

TFT\_eSPI tft \= TFT\_eSPI();  
TFT\_eSprite spr \= TFT\_eSprite(\&tft);

// \--- CONFIGURATION OBD \---  
\#define OBD\_TX\_PIN 8  
\#define OBD\_RX\_PIN 9  
\#define OBD\_BAUD   38400

// \--- CONFIGURATION ÉCRAN & COULEURS \---  
\#define BGCOLOR       TFT\_BLACK  
\#define EYECOLOR      TFT\_WHITE  
\#define BG\_DARK\_BLUE  0x0005  
\#define V\_DARK        0x0110  
\#define START\_ANGLE   60   // Position 40 min (bas-gauche)

// \--- PARAMÈTRES YEUX \---  
const int SCREEN\_W \= 240;  
const int SCREEN\_H \= 240;  
int eyeW \= 56;  
int eyeH\_Max \= 56;  
int borderRadius \= 16;  
int spaceBetween \= 24;  
int blinkThickness \= 6;  
int blinkWidth \= 64;  
float blinkThreshold \= 12.0;

float curLx, curLy, curLh;  
float targetLx, targetLy, targetLh;

enum Mood { NORMAL, TIRED, ANGRY, HAPPY };  
Mood currentMood \= NORMAL;

unsigned long blinkTimer \= 0;  
unsigned long idleTimer \= 0;  
unsigned long moodTimer \= 0;

// \--- VARIABLES OBD & DASHBOARD \---  
int realRPM  \= 0;  
int realSPD  \= 0;  
int realTMP  \= 0;

// \--- FONCTIONS OBD (COPIÉES DU CODE QUI FONCTIONNE) \---  
String readResponse(unsigned long timeout) {  
  String response \= "";  
  unsigned long start \= millis();  
  while (millis() \- start \< timeout) {  
    if (Serial1.available()) {  
      char c \= Serial1.read();  
      response \+= c;  
      if (c \== '\>') break;  
    }  
  }  
  response.replace(" ", "");  
  response.replace("\\r", "");  
  response.replace("\\n", "");  
  return response;  
}

void sendCommand(String cmd) {  
  while (Serial1.available()) Serial1.read(); // Flush  
  Serial1.println(cmd);  
  delay(50);  
}

// \--- PARSING (exactement comme dans le code qui fonctionne) \---  
int getRPM(String response) {  
  int idx \= response.indexOf("410C");  
  if (idx \!= \-1 && response.length() \>= idx \+ 8) {  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    String hexB \= response.substring(idx \+ 6, idx \+ 8);  
    int a \= (int)strtol(hexA.c\_str(), NULL, 16);  
    int b \= (int)strtol(hexB.c\_str(), NULL, 16);  
    return ((a \* 256) \+ b) / 4;  
  }  
  return \-1;  
}

int getTemp(String response) {  
  int idx \= response.indexOf("4105");  
  if (idx \!= \-1 && response.length() \>= idx \+ 6) {  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    int a \= (int)strtol(hexA.c\_str(), NULL, 16);  
    return a \- 40;  
  }  
  return \-1;  
}

int getSpeed(String response) {  
  int idx \= response.indexOf("410D");  
  if (idx \!= \-1 && response.length() \>= idx \+ 6) {  
    String hexA \= response.substring(idx \+ 4, idx \+ 6);  
    int speed \= (int)strtol(hexA.c\_str(), NULL, 16);  
    return speed;  
  }  
  return \-1;  
}

// \--- AFFICHAGE CADRAN \---  
uint16\_t getSpeedColor(float spd) {  
  if (spd \< 40) return TFT\_BLUE;  
  if (spd \< 80) return TFT\_YELLOW;  
  return TFT\_GREEN;  
}

uint16\_t getRPMColor(float rpm) {  
  if (rpm \< 2000) return TFT\_YELLOW;  
  if (rpm \< 4000) return 0xFA20; // orange  
  return TFT\_RED;  
}

void drawDashboard(int rpm, int spd, int tmp) {  
  spr.fillSprite(BG\_DARK\_BLUE);

  // Rails fixes  
  spr.drawArc(120, 120, 115, 100, START\_ANGLE, START\_ANGLE \+ 240, V\_DARK, BG\_DARK\_BLUE);  
  spr.drawArc(120, 120, 95,  85,  START\_ANGLE, START\_ANGLE \+ 240, V\_DARK, BG\_DARK\_BLUE);

  // Arc vitesse (max 240 km/h → 240°)  
  if (spd \>= 0) {  
    int endAngleSpd \= START\_ANGLE \+ (int)((spd / 240.0) \* 240.0);  
    if (endAngleSpd \> START\_ANGLE \+ 240) endAngleSpd \= START\_ANGLE \+ 240;  
    spr.drawArc(120, 120, 115, 100, START\_ANGLE, endAngleSpd, getSpeedColor(spd), BG\_DARK\_BLUE);  
  }

  // Arc RPM (max 8000 tr/min → 240°)  
  if (rpm \>= 0) {  
    int endAngleRpm \= START\_ANGLE \+ (int)((rpm / 8000.0) \* 240.0);  
    if (endAngleRpm \> START\_ANGLE \+ 240) endAngleRpm \= START\_ANGLE \+ 240;  
    spr.drawArc(120, 120, 95, 85, START\_ANGLE, endAngleRpm, getRPMColor(rpm), BG\_DARK\_BLUE);  
  }

  // Température (centrée en haut)  
  spr.setTextColor(TFT\_CYAN);  
  spr.setTextSize(3);  
  String tmpStr \= (tmp \> \-50) ? String(tmp) : "---";  
  int xTmp \= 120 \- (tmpStr.length() \* 9); // centrage approximatif  
  spr.setCursor(xTmp, 55);  
  spr.print(tmpStr);  
  spr.setTextSize(1);  
  spr.setCursor(120 \+ 35, 65);  
  spr.print("C");

  // RPM (gros au centre)  
  spr.setTextColor(TFT\_WHITE);  
  spr.setTextSize(5);  
  String rpmStr \= (rpm \> 0) ? String(rpm) : "---";  
  int xRpm \= 120 \- (rpmStr.length() \* 15);  
  spr.setCursor(xRpm, 100);  
  spr.print(rpmStr);

  // Vitesse (en bas)  
  spr.setTextColor(TFT\_YELLOW);  
  spr.setTextSize(4);  
  String spdStr \= (spd \>= 0) ? String(spd) : "---";  
  int xSpd \= 120 \- (spdStr.length() \* 12);  
  spr.setCursor(xSpd, 165);  
  spr.print(spdStr);  
  spr.setTextSize(1);  
  spr.setCursor(120 \+ 40, 175);  
  spr.print("km/h");

  spr.pushSprite(0, 0);  
}

// \--- YEUX (inchangé) \---  
void drawEyesToSprite() {  
  spr.fillSprite(BGCOLOR);  
  int drawW, drawH, drawRadius, finalY;  
  int xL, xR;

  if (curLh \< blinkThreshold) {  
    drawW \= blinkWidth; drawH \= blinkThickness; drawRadius \= 3;  
    finalY \= (int)curLy \+ (eyeH\_Max / 2) \- (blinkThickness / 2);  
    xL \= (int)curLx \- (blinkWidth \- eyeW) / 2;  
    xR \= xL \+ blinkWidth \+ (spaceBetween \- (blinkWidth \- eyeW));  
  } else {  
    drawW \= eyeW; drawH \= (int)curLh; drawRadius \= borderRadius;  
    finalY \= (int)curLy \+ (eyeH\_Max \- drawH) / 2;  
    xL \= (int)curLx; xR \= xL \+ eyeW \+ spaceBetween;  
  }

  spr.fillRoundRect(xL, finalY, drawW, drawH, drawRadius, EYECOLOR);  
  spr.fillRoundRect(xR, finalY, drawW, drawH, drawRadius, EYECOLOR);

  if (curLh \>= blinkThreshold) {  
    int mH \= drawH / 2;  
    if (currentMood \== ANGRY) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL \+ eyeW, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR, finalY \+ mH, BGCOLOR);  
    } else if (currentMood \== TIRED) {  
      spr.fillTriangle(xL, finalY, xL \+ eyeW, finalY, xL, finalY \+ mH, BGCOLOR);  
      spr.fillTriangle(xR, finalY, xR \+ eyeW, finalY, xR \+ eyeW, finalY \+ mH, BGCOLOR);  
    } else if (currentMood \== HAPPY) {  
      spr.fillRoundRect(xL \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
      spr.fillRoundRect(xR \- 2, finalY \+ mH, eyeW \+ 4, eyeH\_Max, borderRadius, BGCOLOR);  
    }  
  }  
}

void updateEyesLogic() {  
  unsigned long now \= millis();  
  if (now \> moodTimer) {  
    if (currentMood \== NORMAL) currentMood \= TIRED;  
    else if (currentMood \== TIRED) currentMood \= ANGRY;  
    else if (currentMood \== ANGRY) currentMood \= HAPPY;  
    else currentMood \= NORMAL;  
    moodTimer \= now \+ 4000;  
  }  
  if (now \> blinkTimer) { targetLh \= 0; blinkTimer \= now \+ random(2000, 5000); }  
  if (now \> idleTimer) { targetLx \= random(40, 90); targetLy \= random(70, 105); idleTimer \= now \+ random(2000, 6000); }  
  float speed \= (targetLh \== 0 || curLh \< 10) ? 0.40 : 0.20;  
  curLx \+= (targetLx \- curLx) \* 0.20;  
  curLy \+= (targetLy \- curLy) \* 0.20;  
  curLh \+= (targetLh \- curLh) \* speed;  
  if (curLh \< 1.0) targetLh \= eyeH\_Max;  
}

// \--- INIT OBD (simplifiée mais fiable comme ton premier code) \---  
void initOBD() {  
  Serial1.begin(OBD\_BAUD, SERIAL\_8N1, OBD\_RX\_PIN, OBD\_TX\_PIN);  
  delay(300);  
  sendCommand("ATZ");   readResponse(2000);  
  sendCommand("ATE0");  readResponse(1000);  
  sendCommand("AT SH 81 10 F1"); readResponse(1000);  
  delay(500);  
}

// \--- SETUP \---  
void setup() {  
  Serial.begin(115200);  
  delay(500);

  tft.init();  
  tft.setRotation(0);  
  spr.createSprite(SCREEN\_W, SCREEN\_H);  
  spr.setSwapBytes(true);

  // Initialisation OBD  
  initOBD();

  // Position initiale yeux  
  targetLh \= eyeH\_Max;  
  targetLx \= (SCREEN\_W \- (eyeW \* 2 \+ spaceBetween)) / 2;  
  targetLy \= (SCREEN\_H \- eyeH\_Max) / 2;  
  curLx \= targetLx; curLy \= targetLy; curLh \= eyeH\_Max;  
  blinkTimer \= millis() \+ 2000;  
  idleTimer \= millis() \+ 4000;  
  moodTimer \= millis() \+ 4000;

  // Animation yeux 5s  
  unsigned long startEyes \= millis();  
  while (millis() \- startEyes \< 5000) {  
    updateEyesLogic();  
    drawEyesToSprite();  
    spr.pushSprite(0, 0);  
    delay(16);  
  }  
}

// \--- LOOP \---  
void loop() {  
  // Lecture OBD (comme dans ton code qui fonctionne)  
  sendCommand("0105");  
  String rTemp \= readResponse(500);  
  int tmp \= getTemp(rTemp);  
  if (tmp \> \-50 && tmp \< 150) realTMP \= tmp;

  sendCommand("010C");  
  String rRPM \= readResponse(500);  
  int rpm \= getRPM(rRPM);  
  if (rpm \>= 0) realRPM \= rpm;

  sendCommand("010D");  
  String rSPD \= readResponse(500);  
  int spd \= getSpeed(rSPD);  
  if (spd \>= 0) realSPD \= spd;

  // Animation yeux  
  updateEyesLogic();  
  drawEyesToSprite();

  // Dessin dashboard par-dessus  
  drawDashboard(realRPM, realSPD, realTMP);

  delay(150);  // rafraîchissement rapide mais pas trop agressif  
}  
