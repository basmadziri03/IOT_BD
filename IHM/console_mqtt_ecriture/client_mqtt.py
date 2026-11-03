# Projet IOT - Construction de l'IHM
# Partie 3.2 : classe ClientMQTT basee sur la librairie paho-mqtt

import time

from paho.mqtt import client as mqtt_client


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
    def subscribe_mqtt(self, topic, qos=0):
        def on_message(client, userdata, msg):
            print(f"Réception : `{msg.payload.decode()}` depuis '{msg.topic}'")

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
