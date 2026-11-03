# Projet IOT - Construction de l'IHM
# Partie 3.2 : classe ClientMQTT basee sur la librairie paho-mqtt
# Partie 3.6.2 : les valeurs recues sont memorisees dans la classe
# Partie 5 : archivage des mesures dans une base SQLite

import os
import sqlite3
import time
from datetime import datetime

from paho.mqtt import client as mqtt_client

# la base est creee a cote de ce fichier, quel que soit le dossier de lancement
DOSSIER = os.path.dirname(os.path.abspath(__file__))


def date_capteur_vers_iso(date_capteur):
    # "03/11/2026 15:46:21" -> "2026-11-03 15:46:21"
    # au format annee-mois-jour, l'ordre alphabetique est aussi l'ordre chronologique :
    # c'est indispensable pour les requetes SQLite BETWEEN et ORDER BY sur la date
    return datetime.strptime(date_capteur, "%d/%m/%Y %H:%M:%S").strftime("%Y-%m-%d %H:%M:%S")


class ClientMQTT:
    def __init__(self, ip_address, port, client_id, username, password):
        super().__init__()
        self.ip_address = ip_address
        self.port = port
        self.client_id = client_id
        self.username = username
        self.password = password
        self.client = None

        # topics souscrits, pour se reabonner si la connexion est perdue puis retablie
        self.topics = []

        # 3.6.2 : dernieres valeurs recues du capteur (None tant que rien n'est arrive)
        self.version = None
        self.temperature = None
        self.humidite = None
        self.date_IdO = None
        self.debug = True   # affiche aussi chaque reception sur la console

        # 5 : base SQLite d'archivage
        self.conn = None
        self.tablename = None
        self.nb_enregistrements = 0

    # 5.2 a) : connexion a la base SQLite (creee si elle n'existe pas) et creation de la table
    def connect_sqlite(self, database_name="IOT.db", tablename="IOT_DATA"):
        self.tablename = tablename
        self.conn = sqlite3.connect(os.path.join(DOSSIER, database_name))
        db = self.conn.cursor()
        db.execute(f"CREATE TABLE IF NOT EXISTS {tablename} "
                   "(Id_capteur integer, Date text, Version text, Temperature real, Humidite real)")
        self.conn.commit()
        print(f"Base SQLite ouverte : {database_name}, table {tablename}")

    # 5.2 b) : deconnexion de la base SQLite
    def disconnect_sqlite(self):
        if self.conn is not None:
            self.conn.close()
            self.conn = None
            print("Base SQLite fermée")

    # 5.2 d) : sauvegarde d'une mesure complete dans la base
    def enregistrer_mesure(self, id_capteur):
        if self.conn is None or self.temperature is None or self.humidite is None:
            return
        try:
            date_iso = date_capteur_vers_iso(self.date_IdO)
        except ValueError:
            print(f"Date du capteur illisible, mesure non archivée : {self.date_IdO}")
            return
        self.conn.cursor().execute(f"INSERT INTO {self.tablename} VALUES (?,?,?,?,?)",
                                   (id_capteur, date_iso, self.version,
                                    float(self.temperature), float(self.humidite)))
        self.conn.commit()
        self.nb_enregistrements += 1

    # 3.2.1 : connexion au broker
    # boucle_fond = True lance la boucle reseau de paho dans un thread a part
    # (utile pour le programme en ecriture qui attend les saisies clavier)
    def connect_mqtt(self, boucle_fond=False) -> mqtt_client:
        def on_connect(client, userdata, flags, rc, properties):
            if rc == 0:
                print(f"Connecté au broker MQTT : {self.ip_address} {self.port}")
                # le broker oublie les abonnements a chaque nouvelle connexion
                for topic, qos in self.topics:
                    client.subscribe(topic, qos)
            else:
                print(f"Echec de la connexion au broker, code : {rc}")

        self.client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2, client_id=self.client_id)
        self.client.username_pw_set(self.username, self.password)
        self.client.on_connect = on_connect
        try:
            self.client.connect(self.ip_address, self.port)
        except OSError as erreur:
            # broker injoignable : en general le PC n'est pas sur le WiFi IoT
            print(f"Impossible de joindre le broker {self.ip_address}:{self.port} ({erreur})")
            print("Vérifier que le PC est connecté au WiFi 'IoT IMT Nord Europe'")
            self.client = None
            return None

        if boucle_fond:
            self.client.loop_start()
            # on attend la reponse du broker (CONNACK), 5 s max, avant de rendre la main
            for _ in range(50):
                if self.client.is_connected():
                    break
                time.sleep(0.1)
        return self.client

    # 3.2.2 : deconnexion du broker
    def disconnect_mqtt(self) -> None:
        if self.client is None:
            return
        self.client.disconnect()
        self.client.loop_stop()
        print("Déconnecté du broker MQTT")

    # 3.2.3 : publication d'une valeur sur un topic
    def publish_mqtt(self, topic, message, qos=0):
        resultat = self.client.publish(topic, str(message), qos)
        if resultat.rc == mqtt_client.MQTT_ERR_SUCCESS:
            print(f"Publication : `{message}` sur `{topic}`")
        else:
            print(f"Echec de la publication sur `{topic}`, code : {resultat.rc}")

    # 3.2.4 : souscription a un topic
    # 3.6.2 : on memorise la valeur recue selon le topic
    # (topics en majuscules comme dans le programme Arduino)
    def subscribe_mqtt(self, topic, qos=0):
        def on_message(client, userdata, msg):
            valeur = msg.payload.decode()
            if "VERSION_" in msg.topic:
                self.version = valeur
            elif "TEMPERATURE_" in msg.topic:
                self.temperature = valeur
            elif "HUMIDITY_" in msg.topic:
                self.humidite = valeur
            elif "GET_DATE_" in msg.topic:
                self.date_IdO = valeur

            if self.debug:
                print(f"Réception : `{valeur}` depuis '{msg.topic}'")

            # 5.2 d) : le capteur publie TEMPERATURE, HUMIDITY puis GET_DATE (6.1.6) ;
            # GET_DATE est donc le dernier topic d'une mesure : on archive a sa reception
            if "GET_DATE_" in msg.topic:
                self.enregistrer_mesure(int(msg.topic[-5:]))

        self.topics.append((topic, qos))
        self.client.subscribe(topic, qos)
        self.client.on_message = on_message
        print(f"Attente du topic : `{topic}`")

    # 3.2.5 : attente des donnees (timeout = 0 : on attend indefiniment)
    def wait_mqtt(self, timeout=0):
        if timeout == 0:
            self.client.loop_forever()
        else:
            self.client.loop(timeout=timeout)
