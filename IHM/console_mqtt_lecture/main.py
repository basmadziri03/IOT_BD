# Projet IOT - IHM du capteur TempHumiX
# Partie 3.3 - Client MQTT en lecture

from client_mqtt import ClientMQTT

id_pc = 48566

# parametres du broker dans config_mqtt.py (pas sur GitHub)
from config_mqtt import broker, port, username, password

# memes topics que l'Arduino (MQTT respecte les majuscules)
topics = [
    f"/iot/VERSION_{id_pc}",
    f"/iot/TEMPERATURE_{id_pc}",
    f"/iot/HUMIDITY_{id_pc}",
    f"/iot/GET_DATE_{id_pc}",
]

if __name__ == '__main__':
    print("Démarrage de l'application")
    print("==========================")
    print("Instanciation de votre ClientMQTT")
    mqtt = ClientMQTT(broker, port, f"Client_{id_pc}", username, password)
    if mqtt.connect_mqtt() is None:
        raise SystemExit("Fin de l'application")

    print(f"Souscription aux données du capteur : 'Client_{id_pc}'")
    for topic in topics:
        mqtt.subscribe_mqtt(topic)

    # attente des messages (Ctrl+C pour arreter)
    try:
        mqtt.wait_mqtt(timeout=0)
    except KeyboardInterrupt:
        mqtt.disconnect_mqtt()

    print("Fin de l'application")
