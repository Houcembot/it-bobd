# PLAN_PROJET.md — IT-BOBD (Robot Compagnon Voiture OBD2)

_Dernière mise à jour : 2026-03-10_
_Géré par : Maxbot (orchestration) + Rocbot (exécution)_

---

## Résumé

Robot compagnon voiture basé sur ESP32-S3 Zero, lecture OBD2 via CAN bus (MCP2515), écran rond TFT GC9A01 avec expressions de yeux animées. Objectif : prototype → produit commercialisable fin 2026.

## Phase actuelle : PHASE 1 — Hardware + Écran (Mars - Mai 2026)

### Tâches en cours
- [ ] Stabiliser lecture CAN bus (ESP32-S3 Zero + MCP2515/SN65HVD230)
- [ ] Définir PIDs OBD2 prioritaires (RPM, température, vitesse, tension batterie)
- [ ] Structurer le code Arduino par modules
- [ ] Créer repo GitHub public (README, licence, CLAUDE.md)
- [ ] Premier post "Jour 1" sur les réseaux sociaux

### Prochaine étape (Avril 2026)
- Intégrer écran TFT GC9A01 (SPI) avec expressions de yeux
- Lier les expressions aux données OBD2

## Fichiers clés du projet
- `PROJECT_HUB.md` — vue d'ensemble
- `CHRONOLOGIE.md` — historique des phases
- `bobd_main.py` — code principal (ancienne version Python/Raspberry Pi)
- `it-bobd/` — dossier code Arduino actuel
- `expressions.html` — démos visuelles expressions yeux

## Matériel
- ESP32-S3 Zero
- MCP2515 + SN65HVD230 (CAN bus)
- TFT GC9A01 1.28" (écran rond)
- Connecteur OBD2 mâle

## Budget Phase 1 : 28-56 EUR

## Roadmap complète
Voir `~/.botfleet/maxbot/../../../Documents/IT-BOBD/CLAUDE.md`
