# IT-BOBD — Robot Compagnon Voiture OBD2
**Documentation Complète du Projet**
_Dernière mise à jour : 2026-03-15 — par FreBot (BotFleet)_

---

## Résumé

**IT-BOBD** est un robot compagnon embarqué dans la voiture. Il se connecte au port OBD2 du véhicule via un câble 5 fils et un module ELM327, lit les données moteur en temps réel, et les exprime visuellement via des **animations de yeux** sur un écran TFT rond. Objectif : prototype fonctionnel → produit DIY maker → vente en ligne V1 décembre 2026 → V2 avec Intelligence Artificielle.

---

## Matériel (Hardware)

| Composant | Référence | Rôle |
|---|---|---|
| Microcontrôleur | ESP32-S3 Zero | Cerveau du robot |
| Écran | GC9A01 Round TFT 1.28" (SPI) | Affichage des yeux / data |
| Interface OBD2 | ELM327 PIC18F25K80 | Lecture des données voiture |
| Câble | 5 fils (RX, TX, VCC, GND, +) | Connexion ELM327 ↔ ESP32 |
| Véhicule test | VW Polo 9N | Voiture de développement |
| Capteur tactile | TTP223 | Interaction utilisateur |

### Connexion OBD2

Le module ELM327 (puce PIC18F25K80) est connecté à l'ESP32-S3 via un **câble 5 fils** en communication série (AT commands). Protocole utilisé : **ISO 14230-4 KWP** avec le handshake VW spécifique.

**Handshake Volkswagen (découvert le 15 Jan 2026) :**
```
AT Z        → Reset ELM327
AT SP 5     → Protocole ISO 14230
AT SH 81 10 F1 → Header VW spécifique
```

**PIDs OBD2 testés et fonctionnels :**
```
010C → RPM moteur
010D → Vitesse (km/h)
0105 → Température liquide refroidissement
0104 → Charge moteur (%)
0111 → Position papillon
ATRV → Tension batterie
AT LP → Low Power (zero drain)
```

---

## Timeline du Projet

### Janvier 2026 — Phase 1 : Fondations
- 06-12 Jan : Échecs connexion OBD standards (Bluetooth ELM327)
- **15 Jan** : **Breakthrough** — Handshake VW réussi (`AT SH 81 10 F1`)
- 16 Jan : Première lecture RPM stable
- 20-22 Jan : Dashboard LCD 240×240 (Python PIL, Raspberry Pi)
- 24-25 Jan : Coach Trip IA (analyse historique LOAD, note 0-10)
- 26-27 Jan : Tactile TTP223 ultra-réactif (0.15s)
- 28 Jan : Analyse hardware ELM327 — puce PIC18F25K80 confirmée
- 28 Jan : Gestion énergie AT LP (zero drain batterie)
- 29 Jan : Migration vers ESP32-C3 (câble CAT6, 0 Bluetooth)

### Février 2026 — Phase 2 : Intégration Visuelle
- 18 Fév : Animations yeux (NORMAL, TIRED, ANGRY, HAPPY, clignement)
- 20 Fév : Interface OBD + Yeux fusionnée (arcs RPM/vitesse/temp)
- 20 Fév : Interface test commandes OBD via Serial
- 23 Fév : Bootstrap OpenClaw + Assistant Maxi intégré
- 23-24 Fév : **Prototype breadboard complet** — ESP32-S3 Zero + GC9A01 SPI + ELM327 câble 5 fils
- 24 Fév : Identification câbles RX/TX sur l'ELM327
- **Fév** : **Test réussi sur OBD2 VW Polo 9N** — RPM, vitesse, température eau
- 26 Fév : Unification code Wokwi (ESP32-S3 Zero + GC9A01 Round LCD)
- 26 Fév : Rapport HTML publié sur GitHub Pages
- Fév : Documentation de tous les codes fonctionnels sur GitHub

### 10 Février → 15 Mars 2026 — Pause Stratégique
**Arrêt des travaux hardware** pour se concentrer sur :
- Mise en place de la flotte IA (BotFleet / OpenClaw)
- Configuration de **FreBot** comme assistant principal du projet
- Planification et structuration du pipeline de développement
- Préparation de la documentation et du contenu pour les réseaux

### Mars → Mai 2026 — Phase 3 : Expressions & OBD Avancé (EN COURS)
_Reprise le 15 mars 2026_

**À faire :**
- [ ] Classifier 40+ paramètres OBD2 (affichage brut + expressions visuelles)
- [ ] Créer les expressions yeux liées à chaque commande OBD
- [ ] Code Arduino modulaire + repo GitHub public (README, licence)
- [ ] Post "Jour 1" réseaux sociaux + pipeline contenu vidéo DIY

---

## Architecture Logicielle

```
ESP32-S3 Zero
├── UART Serial1 (TX=GPIO8, RX=GPIO9, 38400 8N1)
│   └── ELM327 PIC18F25K80
│       └── OBD2 port voiture (ISO 14230-4 KWP)
│           └── VW Polo 9N ECU
│
└── SPI
    └── GC9A01 Round TFT 240×240
        └── Animations yeux (TFT_eSPI / Arduino GFX)
```

**Stack technique :**
- Embarqué : C++/Arduino (.ino), TFT_eSPI / Arduino GFX Library
- Protocole : ELM327 AT commands, ISO 14230-4 KWP
- Simulation : Wokwi (ESP32-S3 Zero + GC9A01)
- CI/Docs : GitHub Pages (houcembot.github.io/it-bobd)
- Agents IA : OpenClaw / BotFleet (FreBot, Maxbot, Rocbot)

---

## Flotte IA — BotFleet

Depuis mars 2026, une **flotte d'agents IA** (OpenClaw) est déployée pour assister le projet :

| Agent | Rôle sur IT-BOBD |
|---|---|
| **FreBot** | Planification, documentation, recherche technique, marketing |
| **Maxbot** | Orchestration des tâches, coordination bots |
| **Rocbot** | Exécution des tâches, interface Discord |

**Architecture IA (V2 future) :**
- Mistral (local) → tâches mécaniques/OBD2 simples
- Gemini → orchestration complexe et décisions
- Claude (API) → architecture, debug, code review

---

## Roadmap Complète

### V1 — Décembre 2026 (objectif)
- Tutoriel DIY complet (vidéo + documentation)
- Vente en ligne du kit robot (version makers)
- Cible : amateurs d'électronique, IoT, mécanique, sports mécaniques
- Prix estimé : 100-200 €

### Phases intermédiaires :
| Phase | Période | Contenu |
|---|---|---|
| Phase 3 | Mars-Mai 2026 | Expressions ↔ OBD, 40+ params, code modulaire |
| Phase 4 | Juin-Août 2026 | Audio & Voix (INMP441 + MAX98357A) |
| Phase 5 | Sep-Oct 2026 | Miniaturisation PCB custom + boîtier 3D |
| Phase 6 | Nov-Déc 2026 | App mobile (Flutter/React Native) |
| Phase 7 | Déc 2026 | Tuto DIY + Vente V1 |

### V2 — 2027
- Intégration Intelligence Artificielle embarquée
- Assistant vocal évolué
- Connexion cloud / app mobile avancée
- Certification CE + distribution Europe

---

## Marchés & Positionnement

- **Cible principale** : Clara, 28-45 ans, conductrice urbaine, tech-savvy
- **Marchés** : France + Tunisie (phase 1), Europe + Maghreb (V2)
- **Concurrents** : Blazexel, Dasai Mochi, Niko Robot
- **Différenciation** : émotions expressives + lecture OBD2 réelle + prix maker
- **Positionnement** : outil de sécurité premium, pas un gadget

---

## Liens Utiles

- GitHub : https://github.com/Houcembot/it-bobd
- GitHub Pages : https://houcembot.github.io/it-bobd
- Roadmap : https://houcembot.github.io/it-bobd/it-bobd-product-roadmap.html
- Node Graph : https://houcembot.github.io/it-bobd/it-bobd-node-graph.html
- Codes OBD : https://github.com/Houcembot/it-bobd/blob/main/BOBD-%20Historique%20Codes..md

---

## Statistiques (au 15 mars 2026)

- Jours de développement : ~40 actifs (pause 10 Fév - 15 Mar incluse dans 73 jours calendaires)
- Documents archivés : 50+
- Tâches terminées : 19
- Heures estimées : 200h+
- PIDs OBD2 documentés : 40+
- Véhicule testé : VW Polo 9N (OBD2 ISO 14230-4 KWP)

---

_Projet IT-BOBD — Houcemedinne_
_Assisté par FreBot / BotFleet (OpenClaw)_
_© 2026 — Tous droits réservés_
