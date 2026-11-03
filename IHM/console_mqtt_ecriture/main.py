# Projet IOT - IHM du capteur TempHumiX
# Partie 3.4 - Client MQTT en ecriture (menu des commandes)

from client_mqtt import ClientMQTT

id_pc = 48566

# parametres du broker dans config_mqtt.py (pas sur GitHub)
from config_mqtt import broker, port, username, password

topic_commande = f"/iot/COMMAND_{id_pc}"
topic_bargraphe = f"/iot/BARGRAPH_{id_pc}"
topic_periode = f"/iot/BLINK_{id_pc}"

# codes commande
SYNCHRO_NTP = 10
DEMARRAGE_MESURE = 20
ARRET_MESURE = 30
CLIGNOTEMENT_LENT = 40
CLIGNOTEMENT_RAPIDE = 50
CLIGNOTEMENT_PERSO = 60


def lire_entier(question, mini, maxi):
    # redemande tant que la saisie n'est pas valide
    while True:
        saisie = input(question)
        if saisie.isdigit() and mini <= int(saisie) <= maxi:
            return int(saisie)
        print(f"Valeur invalide, entrer un entier entre {mini} et {maxi}")


def synchroniser(mqtt):
    mqtt.publish_mqtt(topic_commande, SYNCHRO_NTP)
    print(f"Synchronisation du capteur : {id_pc}")


def demarrer_mesures(mqtt):
    # mesure en marche = clignotement rapide
    mqtt.publish_mqtt(topic_commande, DEMARRAGE_MESURE)
    print(f"Démarrage des mesures sur le capteur : {id_pc}")
    mqtt.publish_mqtt(topic_commande, CLIGNOTEMENT_RAPIDE)


def arreter_mesures(mqtt):
    # mesure arretee = clignotement lent
    mqtt.publish_mqtt(topic_commande, ARRET_MESURE)
    print(f"Arrêt des mesures sur le capteur : {id_pc}")
    mqtt.publish_mqtt(topic_commande, CLIGNOTEMENT_LENT)


def clignoter_lent(mqtt):
    mqtt.publish_mqtt(topic_commande, CLIGNOTEMENT_LENT)
    print(f"Clignotement lent de la led de vie du capteur : {id_pc}")


def clignoter_rapide(mqtt):
    mqtt.publish_mqtt(topic_commande, CLIGNOTEMENT_RAPIDE)
    print(f"Clignotement rapide de la led de vie du capteur : {id_pc}")


def clignoter_perso(mqtt):
    # l'Arduino veut un multiple de 100 ms
    periode = lire_entier("Période du clignotement en ms (multiple de 100) : ", 100, 10000)
    if periode % 100 != 0:
        periode = round(periode / 100) * 100
        print(f"Période arrondie à {periode} ms")
    mqtt.publish_mqtt(topic_periode, periode)
    mqtt.publish_mqtt(topic_commande, CLIGNOTEMENT_PERSO)
    print(f"Clignotement personnalisé ({periode} ms) de la led de vie du capteur : {id_pc}")


def ecrire_bargraphe(mqtt):
    valeur = lire_entier("Choisir la valeur à écrire sur le bargraphe (0 à 1023) : ", 0, 0x03FF)
    mqtt.publish_mqtt(topic_bargraphe, valeur)
    print(f"Ecriture de la valeur {valeur} sur le bargraphe du capteur : {id_pc}")


if __name__ == '__main__':
    print("Démarrage de l'application")
    print("==========================")
    print("Instanciation de votre ClientMQTT")
    # Id different du programme en lecture (3.5)
    mqtt = ClientMQTT(broker, port, f"Client_E{id_pc}", username, password)
    # paho en arriere-plan pendant les input()
    if mqtt.connect_mqtt(boucle_fond=True) is None:
        raise SystemExit("Fin de l'application")

    car = ' '
    while car.upper() != 'Q':
        print("Console capteur IdO")
        print("-------------------")
        print("1 - Mise à l'heure du capteur")
        print("2 - Démarrage des mesures")
        print("3 - Arrêt des mesures")
        print("4 - Clignotement lent")
        print("5 - Clignotement rapide")
        print("6 - Clignotement personnalisé")
        print("7 - Ecriture du bargraphe")
        print("Q - Quitter")
        car = input("Votre choix : ")
        match car.upper():
            case '1':
                synchroniser(mqtt)
            case '2':
                demarrer_mesures(mqtt)
            case '3':
                arreter_mesures(mqtt)
            case '4':
                clignoter_lent(mqtt)
            case '5':
                clignoter_rapide(mqtt)
            case '6':
                clignoter_perso(mqtt)
            case '7':
                ecrire_bargraphe(mqtt)
            case 'Q':
                print("Arrêt du client MQTT")
            case _:
                print("Choix inconnu")

    mqtt.disconnect_mqtt()
    print("Fin de l'application")
