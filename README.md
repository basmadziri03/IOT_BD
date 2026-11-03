# Projet IOT – Capteur TempHumiX

Projet réalisé à **IMT Nord Europe** (CERI Systèmes Numériques – PPI) dans le cadre de l'UV IOT.

Le boîtier TempHumiX mesure la **température** et l'**humidité** avec un capteur DHT22, les affiche sur un écran LCD et les envoie en **WiFi** avec le protocole **MQTT**. Une application Python sur le PC affiche les mesures, envoie des commandes au boîtier, archive les données dans une base **SQLite** et trace les **courbes**.

**Auteur :** Basma Dziri – poste n° 48566

---

## Structure du dépôt

```
IOT_BD/
├── SIMULIDE/                  Partie 1 : premiers programmes (Arduino UNO R3, simulés dans SimulIDE)
│   ├── clignotement_led/      LED de vie qui clignote
│   ├── bargraphe/             chenillard sur le bargraphe (10 LEDs)
│   ├── capteur_dht22/         lecture de la température + LED verte / rouge
│   ├── lcd_i2c/               premier test de l'écran LCD en I2C
│   └── lcd_i2c_dht22/         température et humidité sur le LCD
│
├── TEMPHUMIX/                 Partie 2 : boîtier connecté (Arduino UNO R4 WiFi)
│   ├── TEMPHUMIX.ino
│   └── arduino_secrets.example.h
│
└── IHM/                       Partie 3 : application Python sur le PC
    ├── console_mqtt_lecture/  client MQTT qui lit les mesures (console)
    ├── console_mqtt_ecriture/ client MQTT qui envoie les commandes (menu console)
    ├── console_mqtt_thread/   les deux en même temps grâce à un thread
    └── ihm_mqtt_simple/       interface graphique PySide6 + SQLite + courbes
```

Chaque dossier de `SIMULIDE` contient le programme (`.ino`), la simulation (`.sim1`) et le programme compilé (`.hex`) chargé par SimulIDE.

---

## Matériel

- Arduino UNO R3 (partie 1), puis **Arduino UNO R4 WiFi** (parties 2 et 3)
- Capteur de température et d'humidité **DHT22** (broche A0)
- Écran **LCD Grove RGB Backlight** en I2C
- LED de vie (D2) et bargraphe de 10 LEDs (D3 à D12)

## Logiciels

- Arduino IDE, SimulIDE
- MQTTX (client MQTT de test) et Wireshark (analyse des trames)
- Python 3.13 avec `paho-mqtt`, `PySide6` et `matplotlib`
- VS Code, Git / GitHub

---

## Partie 1 – Premiers programmes (SimulIDE)

- LED de vie qui clignote
- Chenillard sur le bargraphe
- Lecture du DHT22 toutes les 2 s
- Affichage de la température et de l'humidité sur le LCD

## Partie 2 – Communication WiFi avec MQTT (`TEMPHUMIX`)

- Passage de l'Arduino UNO R3 à l'**UNO R4 WiFi** (nouvelle librairie LCD, RTC intégré)
- Connexion au WiFi, écran d'accueil avec la version et l'adresse IP
- Mise à l'heure du RTC avec le **serveur NTP** de l'école
- **Publication** sur le broker Mosquitto (`192.168.1.1:1883`) :

| Topic | Contenu |
|---|---|
| `/iot/VERSION_48566` | version du programme (`1.0`) |
| `/iot/TEMPERATURE_48566` | température au dixième de degré |
| `/iot/HUMIDITY_48566` | humidité au dixième de pourcent |
| `/iot/GET_DATE_48566` | date et heure `JJ/MM/AAAA HH:MM:SS` |

- **Abonnement** aux commandes :

| Topic | Contenu |
|---|---|
| `/iot/COMMAND_48566` | 10 = synchro NTP, 20 = démarrer la mesure, 30 = arrêter la mesure, 40 = clignotement lent, 50 = rapide, 60 = personnalisé |
| `/iot/BARGRAPH_48566` | valeur de 0 à 1023 affichée sur le bargraphe |
| `/iot/BLINK_48566` | période du clignotement personnalisé (multiple de 100 ms) |

- Amélioration : les mesures ne sont publiées **que si elles changent**, et la version seulement après une synchro NTP (commande 10)
- Analyse des trames MQTT avec **Wireshark** (CONNECT, SUBSCRIBE, PUBLISH, PING…) et tests des QoS, du Retain et des jokers `+` / `#`

## Partie 3 – Application Python (`IHM`)

- Classe `ClientMQTT` (paho-mqtt) : connexion, publication, abonnement
- Programmes console en lecture et en écriture, puis fusion avec un **thread**
- **Interface graphique PySide6** : connexion au broker, affichage de la date, de la température et de l'humidité, boutons de commande
- **Archivage** de chaque mesure dans la base SQLite `IOT.db` (table `IOT_DATA`)
- **Courbes** de température et d'humidité entre deux dates (matplotlib)

---

## Utilisation

### Boîtier (Arduino)
1. Copier `TEMPHUMIX/arduino_secrets.example.h` en `arduino_secrets.h` et mettre les identifiants WiFi et MQTT.
2. Ouvrir `TEMPHUMIX/TEMPHUMIX.ino` dans l'Arduino IDE.
3. Choisir la carte **Arduino UNO R4 WiFi** et le port, puis téléverser.
4. Librairies nécessaires : `Grove - LCD RGB Backlight`, `DHT sensor library`, `NTPClient`, `ArduinoMqttClient`.

### Application (Python)
1. Créer l'environnement et installer les librairies :
   ```
   cd IHM
   python -m venv .venv
   .venv\Scripts\pip install paho-mqtt PySide6 matplotlib
   ```
2. Dans chaque dossier, copier `config_mqtt.example.py` en `config_mqtt.py` et mettre les identifiants du broker.
3. Le PC doit être connecté au WiFi **IoT IMT Nord Europe**.
4. Lancer l'interface :
   ```
   .venv\Scripts\python ihm_mqtt_simple\main.py
   ```
   Dans VS Code, les configurations de lancement sont prêtes (`Ctrl+Shift+D`).

> Les identifiants (`arduino_secrets.h`, `config_mqtt.py`), la base `IOT.db` et les captures Wireshark ne sont pas envoyés sur GitHub (voir `.gitignore`).
