# Projet IOT - Construction de l'IHM
# Partie 4 : IHM simple capteur sous PySide6
# Partie 5 : les mesures recues sont archivees dans IOT.db (voir client_mqtt.py)
#  - connexion / deconnexion au broker MQTT avec les parametres saisis
#  - affichage de la date, de la temperature et de l'humidite toutes les secondes
#  - envoi des commandes au capteur

import sys

from PySide6.QtCore import QTimer, QRegularExpression
from PySide6.QtGui import QRegularExpressionValidator, QIntValidator
from PySide6.QtWidgets import QApplication, QMainWindow, QMessageBox

from client_mqtt import ClientMQTT
from thread_mqtt import ThreadMQTT
from mon_ihm import Ui_MainWindow

# valeurs proposees par defaut dans l'interface (config_mqtt.py n'est pas envoye sur GitHub)
from config_mqtt import broker, port, username, password

id_pc = 48566

topic_commande = f"/iot/COMMAND_{id_pc}"
topic_bargraphe = f"/iot/BARGRAPH_{id_pc}"
topic_periode = f"/iot/BLINK_{id_pc}"

# codes commande du paragraphe 2.2
SYNCHRO_NTP = 10
DEMARRAGE_MESURE = 20
ARRET_MESURE = 30
CLIGNOTEMENT_LENT = 40
CLIGNOTEMENT_RAPIDE = 50
CLIGNOTEMENT_PERSO = 60


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi(self)

        self.mqtt = None
        self.thread_mqtt = None
        self.debug = True

        # parametres du broker pre-remplis
        self.lneIpAddress.setText(broker)
        self.lnePort.setText(str(port))
        self.lneUser.setText(username)
        self.lnePassword.setText(password)

        # on n'accepte que des saisies valides
        octet = r"(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)"
        self.lneIpAddress.setValidator(QRegularExpressionValidator(
            QRegularExpression(rf"^{octet}(\.{octet}){{3}}$"), self))
        self.lnePort.setValidator(QIntValidator(1, 65535, self))
        self.lnePeriod.setValidator(QIntValidator(100, 10000, self))
        self.lneBargraph.setValidator(QIntValidator(0, 1023, self))

        self.timer = QTimer()

        self.timer.timeout.connect(self.display)
        self.btnConnect.clicked.connect(self.connectMQTT)
        self.btnDisconnect.clicked.connect(self.disconnectMQTT)
        self.btnSynchroDateTime.clicked.connect(self.synchroDateTime)
        self.btnStartMeasure.clicked.connect(self.startMeasure)
        self.btnStopMeasure.clicked.connect(self.stopMeasure)
        self.btnFastBlink.clicked.connect(self.fastBlink)
        self.btnSlowBlink.clicked.connect(self.slowBlink)
        self.btnBlink.clicked.connect(self.customBlink)
        self.lneBargraph.returnPressed.connect(self.setBargraph)
        self.btnBargraph.clicked.connect(self.setBargraph)
        self.actionQuit.triggered.connect(self.quit)
        self.actionAbout.triggered.connect(self.about)

        self.etatConnexion(False)
        self.statusbar.showMessage("Non connecté au broker MQTT")

    # active ou grise les boutons selon l'etat de la connexion
    def etatConnexion(self, connecte):
        self.btnConnect.setEnabled(not connecte)
        self.btnDisconnect.setEnabled(connecte)
        self.grpCommands.setEnabled(connecte)
        for champ in (self.lneIpAddress, self.lnePort, self.lneUser, self.lnePassword):
            champ.setEnabled(not connecte)

    # affiche les dernieres valeurs recues par le thread (appelee toutes les 1000 ms)
    def display(self):
        if self.mqtt is None:
            return
        self.lblDateTime.setText(self.mqtt.date_IdO or "00/00/0000 00:00:00")
        self.lcdTemperature.display(self.mqtt.temperature or "--")
        self.lcdHumidity.display(self.mqtt.humidite or "--")

        if self.mqtt.client is not None and self.mqtt.client.is_connected():
            version = f" - capteur V{self.mqtt.version}" if self.mqtt.version else ""
            self.statusbar.showMessage(f"Connexion au broker MQTT OK{version}"
                                       f" - {self.mqtt.nb_enregistrements} mesure(s) archivée(s)")
        else:
            self.statusbar.showMessage("Connexion au broker MQTT en cours ou perdue...")

    # lit les parametres, se connecte, lance le thread de lecture et le timer d'affichage
    def connectMQTT(self):
        if not self.lneIpAddress.hasAcceptableInput() or not self.lnePort.hasAcceptableInput():
            self.statusbar.showMessage("Adresse IP ou port invalide")
            return

        self.statusbar.showMessage(f"Connexion à {self.lneIpAddress.text()}...")
        QApplication.processEvents()   # affiche le message avant l'attente de connexion

        self.mqtt = ClientMQTT(self.lneIpAddress.text(), int(self.lnePort.text()),
                               f"Client_{id_pc}", self.lneUser.text(), self.lnePassword.text())
        self.mqtt.debug = self.debug
        if self.mqtt.connect_mqtt() is None:
            self.mqtt = None
            self.statusbar.showMessage("Broker injoignable : vérifier le WiFi IoT et l'adresse IP")
            return

        # un thread ne peut etre lance qu'une fois : on en recree un a chaque connexion
        self.thread_mqtt = ThreadMQTT(self.mqtt)
        self.thread_mqtt.start()
        self.timer.start(1000)
        self.etatConnexion(True)

    # arrete le timer d'affichage et le thread de lecture, puis se deconnecte
    def disconnectMQTT(self):
        self.timer.stop()
        if self.thread_mqtt is not None:
            self.thread_mqtt.stopthread()
            self.thread_mqtt.join()
            self.thread_mqtt = None
        if self.mqtt is not None:
            self.mqtt.disconnect_mqtt()
            self.mqtt = None
        self.etatConnexion(False)
        self.statusbar.showMessage("Déconnecté du broker MQTT")

    def envoyer(self, topic, valeur, message):
        if self.mqtt is None:
            return
        self.mqtt.publish_mqtt(topic, valeur)
        self.statusbar.showMessage(message)

    # envoi de la commande de mise a l'heure du capteur (NTP)
    def synchroDateTime(self):
        self.envoyer(topic_commande, SYNCHRO_NTP, "Synchronisation NTP demandée")

    # demarrage de la mesure, signale par un clignotement rapide
    def startMeasure(self):
        self.envoyer(topic_commande, DEMARRAGE_MESURE, "Démarrage des mesures")
        self.envoyer(topic_commande, CLIGNOTEMENT_RAPIDE, "Mesures démarrées (clignotement rapide)")

    # arret de la mesure, signale par un clignotement lent
    def stopMeasure(self):
        self.envoyer(topic_commande, ARRET_MESURE, "Arrêt des mesures")
        self.envoyer(topic_commande, CLIGNOTEMENT_LENT, "Mesures arrêtées (clignotement lent)")

    def fastBlink(self):
        self.envoyer(topic_commande, CLIGNOTEMENT_RAPIDE, "Clignotement rapide")

    def slowBlink(self):
        self.envoyer(topic_commande, CLIGNOTEMENT_LENT, "Clignotement lent")

    # lit la periode saisie, l'envoie sur BLINK puis demande le clignotement personnalise
    def customBlink(self):
        if not self.lnePeriod.hasAcceptableInput():
            self.statusbar.showMessage("Période invalide (100 à 10000 ms)")
            return
        periode = int(self.lnePeriod.text())
        if periode % 100 != 0:
            # le programme Arduino n'accepte que des multiples de 100 ms
            periode = round(periode / 100) * 100
            self.lnePeriod.setText(str(periode))
        self.envoyer(topic_periode, periode, f"Période de clignotement : {periode} ms")
        self.envoyer(topic_commande, CLIGNOTEMENT_PERSO, f"Clignotement personnalisé : {periode} ms")

    # lit la valeur saisie (0 a 1023) et la publie sur le bargraphe
    def setBargraph(self):
        if not self.lneBargraph.hasAcceptableInput():
            self.statusbar.showMessage("Valeur du bargraphe invalide (0 à 1023)")
            return
        valeur = int(self.lneBargraph.text())
        self.envoyer(topic_bargraphe, valeur, f"Bargraphe : {valeur}")

    def about(self):
        QMessageBox.information(self, "A propos",
                                f"Capteur IdO {id_pc}\nIHM MQTT - Projet IOT\nIMT Nord Europe - CERI SN - PPI")

    # quitte l'application (menu Fichier > Quitter)
    def quit(self):
        self.close()

    # fermeture de la fenetre : on se deconnecte proprement
    def closeEvent(self, event):
        self.disconnectMQTT()
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
