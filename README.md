# 💡 Project: Clignotement de LED avec Arduino & SimulIDE

Ce projet consiste en la conception, le câblage et la simulation du clignotement d'une LED à l'aide d'une carte **Arduino Uno** dans l'environnement **SimulIDE**.

---

## 📌 Description

L'objectif de ce TP est d'implémenter un programme de base permettant de faire clignoter une LED toutes les 500 millisecondes. Une mesure au voltmètre est intégrée dans la simulation pour observer l'alternance de tension ($0\text{ V} \leftrightarrow 5\text{ V}$) sur le port de sortie.

---

## 📁 Structure du Dépôt

```text
.
├── README.md               # Documentation du projet
├── Compte rendu.docx       # Rapport détaillé du TP
└── SIMULIDE/               # Dossier des fichiers de travail
    └── clignotement_led/
        ├── clignotement_led.ino   # Code source Arduino
        └── clignotement_led.sim1  # Schéma du circuit sous SimulIDE
