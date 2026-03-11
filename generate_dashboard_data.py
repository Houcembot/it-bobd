#!/usr/bin/env python3
"""
generate_dashboard_data.py — Lit les fichiers .md de ~/.botfleet et génère data.json
Usage : python3 generate_dashboard_data.py
Rocbot exécute ce script et pousse le résultat sur GitHub Pages.
"""

import json
import re
import os
from pathlib import Path
from datetime import datetime

BOTFLEET_DIR = Path.home() / ".botfleet"
SHARED_DIR = BOTFLEET_DIR / "shared"
PROJETS_DIR = SHARED_DIR / "projets"
OUTPUT_DIR = PROJETS_DIR / "it-bobd"  # Push vers le repo GitHub


def parse_plan_projet(filepath):
    """Parse un PLAN_PROJET.md et extrait les tâches."""
    tasks = []
    if not filepath.exists():
        return tasks, "À définir", ""

    content = filepath.read_text(encoding="utf-8", errors="ignore")

    # Extraire le titre du projet
    title_match = re.search(r"^# PLAN_PROJET\.md — (.+)$", content, re.MULTILINE)
    title = title_match.group(1).strip() if title_match else filepath.parent.name

    # Extraire la description
    desc_match = re.search(r"^_([^_\n]+)_\s*$", content, re.MULTILINE)
    desc = desc_match.group(1).strip() if desc_match else ""

    # Extraire les tâches
    current_phase = ""
    for line in content.split("\n"):
        # Détecter les phases
        phase_match = re.match(r"^## (Phase \d+.*)$", line)
        if phase_match:
            current_phase = phase_match.group(1).strip()

        # Tâches cochées [x]
        done_match = re.match(r"^- \[x\]\s+\*\*(\d+\.\d+)\*\*\s+(.+)$", line)
        if done_match:
            tasks.append({
                "id": done_match.group(1),
                "title": done_match.group(2).strip(),
                "status": "done",
                "phase": current_phase,
                "bot": None,
            })
            continue

        # Tâches non cochées [ ]
        todo_match = re.match(r"^- \[ \]\s+\*\*(\d+\.\d+)\*\*\s+(.+)$", line)
        if todo_match:
            tasks.append({
                "id": todo_match.group(1),
                "title": todo_match.group(2).strip(),
                "status": "todo",
                "phase": current_phase,
                "bot": None,
            })
            continue

        # Tâches simples cochées (sans numéro gras)
        simple_done = re.match(r"^- \[x\]\s+(.+)$", line)
        if simple_done and not done_match:
            text = simple_done.group(1).strip()
            if not text.startswith("**"):
                tasks.append({
                    "id": f"P{len(tasks)+1}",
                    "title": text,
                    "status": "done",
                    "phase": current_phase,
                    "bot": None,
                })

    # Détecter la phase active
    active_phase = "À définir"
    for t in tasks:
        if t["status"] == "todo":
            active_phase = t["phase"]
            break
    if not tasks:
        active_phase = "En attente"

    return tasks, active_phase, desc


def parse_agent_status(filepath):
    """Parse la section Statut des Agents dans un PLAN_PROJET.md."""
    statuses = {}
    if not filepath.exists():
        return statuses

    content = filepath.read_text(encoding="utf-8", errors="ignore")
    in_status = False

    for line in content.split("\n"):
        if "Statut des Agents" in line:
            in_status = True
            continue
        if in_status:
            if line.startswith("## ") or line.startswith("---"):
                break
            agent_match = re.match(r"^\*\s+\*\*(\w+)\s*:\*\*\s*(.+)$", line)
            if agent_match:
                name = agent_match.group(1).strip().lower()
                status_text = agent_match.group(2).strip()
                statuses[name] = status_text

    return statuses


def parse_fleet_status(filepath):
    """Parse FLEET_STATUS.md pour l'état des bots."""
    bots = {}
    if not filepath.exists():
        return bots

    content = filepath.read_text(encoding="utf-8", errors="ignore")
    for line in content.split("\n"):
        if "|" in line and ("Maxbot" in line or "Rocbot" in line or "Clobot" in line):
            cols = [c.strip() for c in line.split("|") if c.strip()]
            if len(cols) >= 4:
                name = cols[0].lower()
                if "maxbot" in name:
                    bots["maxbot"] = {"status": cols[2], "last_activity": cols[3]}
                elif "rocbot" in name:
                    bots["rocbot"] = {"status": cols[2], "last_activity": cols[3]}
                elif "clobot" in name:
                    bots["clobot"] = {"status": cols[2], "last_activity": cols[3]}

    return bots


def detect_doing_tasks(agent_statuses, tasks):
    """Marque les tâches 'en cours' basé sur le statut des agents."""
    for agent_name, status_text in agent_statuses.items():
        status_lower = status_text.lower()
        if "en cours" in status_lower or "recherche" in status_lower or "analyse" in status_lower:
            # Essayer de matcher avec un numéro de tâche
            task_match = re.search(r"(\d+\.\d+)", status_text)
            if task_match:
                task_id = task_match.group(1)
                for t in tasks:
                    if t["id"] == task_id and t["status"] == "todo":
                        t["status"] = "doing"
                        t["bot"] = agent_name
            else:
                # Créer une tâche dynamique pour l'activité en cours
                tasks.append({
                    "id": f"R-{agent_name[:3]}",
                    "title": status_text[:80],
                    "status": "doing",
                    "phase": "",
                    "bot": agent_name,
                })

    return tasks


def main():
    now = datetime.now().strftime("%Y-%m-%d %H:%M")

    # Scanner tous les projets
    projects = []
    projet_dirs = sorted(PROJETS_DIR.iterdir()) if PROJETS_DIR.exists() else []

    for pdir in projet_dirs:
        if not pdir.is_dir():
            continue
        plan_file = pdir / "PLAN_PROJET.md"
        tasks, active_phase, desc = parse_plan_projet(plan_file)
        agent_statuses = parse_agent_status(plan_file)
        tasks = detect_doing_tasks(agent_statuses, tasks)

        done_count = len([t for t in tasks if t["status"] == "done"])
        total_count = len(tasks)

        projects.append({
            "id": pdir.name,
            "name": pdir.name.upper().replace("-", " ").replace("_", " "),
            "desc": desc,
            "phase": active_phase,
            "tasks": tasks,
            "progress": f"{done_count}/{total_count}",
            "agent_statuses": agent_statuses,
        })

    # Parser le fleet status
    fleet_bots = parse_fleet_status(SHARED_DIR / "FLEET_STATUS.md")

    # Construire le JSON
    data = {
        "generated_at": now,
        "bots": {
            "maxbot": {
                "name": "Maxbot",
                "role": "Orchestrateur",
                "model": "Llama 3.3 70B",
                "fleet_status": fleet_bots.get("maxbot", {}),
            },
            "rocbot": {
                "name": "Rocbot",
                "role": "Exécuteur Local",
                "model": "Qwen (Ollama)",
                "fleet_status": fleet_bots.get("rocbot", {}),
            },
            "clobot": {
                "name": "Clobot",
                "role": "Admin On-Call",
                "model": "Claude",
                "fleet_status": fleet_bots.get("clobot", {}),
            },
        },
        "projects": projects,
    }

    # Écrire le JSON
    output_path = OUTPUT_DIR / "dashboard_data.json"
    output_path.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"✅ dashboard_data.json généré : {output_path}")
    print(f"   {len(projects)} projets, {sum(len(p['tasks']) for p in projects)} tâches")

    return data


if __name__ == "__main__":
    main()
