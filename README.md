# Projet IOT – Capteur TempHumiX

Projet réalisé à IMT Nord Europe dans le cadre du module IOT.
Le boîtier « capteur intelligent » mesure la température et l'humidité
et les affiche sur un écran LCD.

## Auteur
- <Prénom NOM> – IMT Nord Europe

## Matériel
- Arduino UNO R3 (partie 1), puis Arduino UNO R4 WiFi (partie 2)
- Capteur de température et d'humidité DHT22
- Écran LCD Grove RGB Backlight (I2C)
- LED de vie

## Logiciels
- Arduino IDE
- Git / GitHub

## Structure du dépôt
IOT_BD/
├── <dossier partie 1>/ # Code de la partie 1 (Arduino UNO R3)
├── TEMPHUMIX/ # Code de la partie 2 (MQTT, Arduino UNO R4)
└── README.md

## Partie 1 – Acquisition de la température (Arduino UNO R3)
Fonctionnalités réalisées :
- Clignotement de la LED de vie à 2 Hz
- Lecture de la température et de l'humidité (DHT22) toutes les 2000 ms
- Affichage de la température et de l'humidité sur le LCD
- <ajoute ici les autres fonctions de ta partie 1>

## Partie 2 – Communication WiFi via MQTT (à venir)
- Migration vers Arduino UNO R4 WiFi
- Connexion WiFi, synchronisation NTP
- Publication et souscription MQTT via le broker Mosquitto

## Utilisation
1. Ouvrir le sketch dans Arduino IDE
2. Sélectionner la carte et le port
3. Téléverser le programme
