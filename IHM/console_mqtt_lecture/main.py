# Projet IOT - Construction de l'IHM
# Partie 3.3 : client MQTT en lecture (abonnement aux donnees du capteur)

from client_mqtt import ClientMQTT

id_pc = 48566

# adresse et identifiants du broker dans config_mqtt.py (non envoye sur GitHub)
from config_mqtt import broker, port, username, password

# memes noms de topics que le programme Arduino (en majuscules, MQTT respecte la casse)
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

    # Attente des valeurs souscrites avec timeout=0 (Ctrl+C ou bouton Stop pour arreter)
    try:
        mqtt.wait_mqtt(timeout=0)
    except KeyboardInterrupt:
        mqtt.disconnect_mqtt()

    print("Fin de l'application")
