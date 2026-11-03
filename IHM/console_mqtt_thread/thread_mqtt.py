# Projet IOT - IHM du capteur TempHumiX
# Thread qui ecoute le broker MQTT en arriere-plan

from threading import Thread

id_pc = 48566

# memes topics que l'Arduino (MQTT respecte les majuscules)
topic_version = f"/iot/VERSION_{id_pc}"
topic_temperature = f"/iot/TEMPERATURE_{id_pc}"
topic_humidite = f"/iot/HUMIDITY_{id_pc}"
topic_datetime = f"/iot/GET_DATE_{id_pc}"


class ThreadMQTT(Thread):
    def __init__(self, client_mqtt):
        Thread.__init__(self)
        # le thread s'arrete avec le programme
        self.daemon = True
        self.mqtt = client_mqtt
        self.stop = False

    def run(self):
        for topic in (topic_version, topic_temperature, topic_humidite, topic_datetime):
            self.mqtt.subscribe_mqtt(topic)

        # boucle par tranches d'1 s pour pouvoir s'arreter
        while not self.stop:
            self.mqtt.wait_mqtt(timeout=1.0)

    def stopthread(self):
        self.stop = True
