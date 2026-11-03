# Projet IOT - Construction de l'IHM
# Partie 3.6.1 : thread qui ecoute le broker MQTT en arriere-plan

from threading import Thread

id_pc = 48566

# memes noms de topics que le programme Arduino (en majuscules, MQTT respecte la casse)
topic_version = f"/iot/VERSION_{id_pc}"
topic_temperature = f"/iot/TEMPERATURE_{id_pc}"
topic_humidite = f"/iot/HUMIDITY_{id_pc}"
topic_datetime = f"/iot/GET_DATE_{id_pc}"


class ThreadMQTT(Thread):
    def __init__(self, client_mqtt):
        Thread.__init__(self)
        # daemon : le thread ne bloque pas la fermeture du programme
        self.daemon = True
        self.mqtt = client_mqtt
        self.stop = False

    def run(self):
        for topic in (topic_version, topic_temperature, topic_humidite, topic_datetime):
            self.mqtt.subscribe_mqtt(topic)

        # on traite le reseau par tranches d'1 s pour pouvoir verifier self.stop regulierement
        while not self.stop:
            self.mqtt.wait_mqtt(timeout=1.0)

    def stopthread(self):
        self.stop = True
