# Projet IOT - IHM du capteur TempHumiX
# Partie 3.6 - Lecture et ecriture dans le meme programme
# le thread recoit les donnees, le menu envoie les commandes

import time

from client_mqtt import ClientMQTT
from thread_mqtt import ThreadMQTT

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


def afficher_valeurs(mqtt):
    # dernieres valeurs recues par le thread
    print(f"Capteur {id_pc} - version : {mqtt.version or '?'}")
    print(f"  Date et heure : {mqtt.date_IdO or 'pas encore recue'}")
    print(f"  Température   : {mqtt.temperature or '--'} °C")
    print(f"  Humidité      : {mqtt.humidite or '--'} %")


if __name__ == '__main__':
    print("Démarrage de l'application")
    print("==========================")
    print("Instanciation de votre ClientMQTT")
    mqtt = ClientMQTT(broker, port, f"Client_{id_pc}", username, password)
    if mqtt.connect_mqtt() is None:
        raise SystemExit("Fin de l'application")
    # on n'affiche plus chaque reception (sinon ca coupe le menu)
    mqtt.debug = False

    # creation et lancement du thread
    thread_mqtt = ThreadMQTT(mqtt)
    thread_mqtt.start()
    time.sleep(1)   # le temps que le thread se connecte

    car = ' '
    while car.upper() != 'Q':
        print("Console capteur IdO")
        print("-------------------")
        print("A - Afficher les dernières valeurs du capteur")
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
            case 'A':
                afficher_valeurs(mqtt)
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

    # arret du thread puis deconnexion
    thread_mqtt.stopthread()
    thread_mqtt.join()
    mqtt.disconnect_mqtt()
    print("Fin de l'application")
