# Projet IOT - IHM du capteur TempHumiX
# Classe ClientMQTT (librairie paho-mqtt)
# + memorisation des valeurs recues
# + archivage des mesures dans la base SQLite

import os
import sqlite3
import time
from datetime import datetime

from paho.mqtt import client as mqtt_client

# la base est creee dans ce dossier
DOSSIER = os.path.dirname(os.path.abspath(__file__))


def date_capteur_vers_iso(date_capteur):
    # "03/11/2026 15:46:21" -> "2026-11-03 15:46:21"
    # en annee-mois-jour les dates se trient bien
    # (necessaire pour BETWEEN et ORDER BY dans SQLite)
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

        # topics souscrits (pour se reabonner apres une coupure)
        self.topics = []

        # dernieres valeurs recues (None au depart)
        self.version = None
        self.temperature = None
        self.humidite = None
        self.date_IdO = None
        self.debug = True   # affiche chaque reception

        # base SQLite
        self.conn = None
        self.tablename = None
        self.nb_enregistrements = 0

    # ouverture de la base et creation de la table
    def connect_sqlite(self, database_name="IOT.db", tablename="IOT_DATA"):
        self.tablename = tablename
        self.conn = sqlite3.connect(os.path.join(DOSSIER, database_name))
        db = self.conn.cursor()
        db.execute(f"CREATE TABLE IF NOT EXISTS {tablename} "
                   "(Id_capteur integer, Date text, Version text, Temperature real, Humidite real)")
        self.conn.commit()
        print(f"Base SQLite ouverte : {database_name}, table {tablename}")

    # fermeture de la base
    def disconnect_sqlite(self):
        if self.conn is not None:
            self.conn.close()
            self.conn = None
            print("Base SQLite fermée")

    # enregistrement d'une mesure
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

    # connexion au broker
    # boucle_fond = True : paho tourne en arriere-plan
    def connect_mqtt(self, boucle_fond=False) -> mqtt_client:
        def on_connect(client, userdata, flags, rc, properties):
            if rc == 0:
                print(f"Connecté au broker MQTT : {self.ip_address} {self.port}")
                # on se reabonne a chaque connexion
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
            # broker injoignable (souvent : pas sur le WiFi IoT)
            print(f"Impossible de joindre le broker {self.ip_address}:{self.port} ({erreur})")
            print("Vérifier que le PC est connecté au WiFi 'IoT IMT Nord Europe'")
            self.client = None
            return None

        if boucle_fond:
            self.client.loop_start()
            # on attend la reponse du broker (5 s max)
            for _ in range(50):
                if self.client.is_connected():
                    break
                time.sleep(0.1)
        return self.client

    # deconnexion du broker
    def disconnect_mqtt(self) -> None:
        if self.client is None:
            return
        self.client.disconnect()
        self.client.loop_stop()
        print("Déconnecté du broker MQTT")

    # publication d'une valeur
    def publish_mqtt(self, topic, message, qos=0):
        resultat = self.client.publish(topic, str(message), qos)
        if resultat.rc == mqtt_client.MQTT_ERR_SUCCESS:
            print(f"Publication : `{message}` sur `{topic}`")
        else:
            print(f"Echec de la publication sur `{topic}`, code : {resultat.rc}")

    # abonnement a un topic
    # la valeur recue est gardee selon le topic
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

            # GET_DATE arrive en dernier, on enregistre la mesure a ce moment
            if "GET_DATE_" in msg.topic:
                self.enregistrer_mesure(int(msg.topic[-5:]))

        self.topics.append((topic, qos))
        self.client.subscribe(topic, qos)
        self.client.on_message = on_message
        print(f"Attente du topic : `{topic}`")

    # attente des messages (timeout = 0 : sans fin)
    def wait_mqtt(self, timeout=0):
        if timeout == 0:
            self.client.loop_forever()
        else:
            self.client.loop(timeout=timeout)
