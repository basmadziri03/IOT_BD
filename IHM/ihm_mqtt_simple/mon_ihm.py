# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'mon_ihm.ui'
##
## Created by: Qt User Interface Compiler version 6.12.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QAction, QBrush, QColor, QConicalGradient,
    QCursor, QFont, QFontDatabase, QGradient,
    QIcon, QImage, QKeySequence, QLinearGradient,
    QPainter, QPalette, QPixmap, QRadialGradient,
    QTransform)
from PySide6.QtWidgets import (QApplication, QGridLayout, QGroupBox, QHBoxLayout,
    QLCDNumber, QLabel, QLineEdit, QMainWindow,
    QMenu, QMenuBar, QPushButton, QSizePolicy,
    QStatusBar, QVBoxLayout, QWidget)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(440, 600)
        self.actionQuit = QAction(MainWindow)
        self.actionQuit.setObjectName(u"actionQuit")
        self.actionAbout = QAction(MainWindow)
        self.actionAbout.setObjectName(u"actionAbout")
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.layoutPrincipal = QVBoxLayout(self.centralwidget)
        self.layoutPrincipal.setObjectName(u"layoutPrincipal")
        self.grpBroker = QGroupBox(self.centralwidget)
        self.grpBroker.setObjectName(u"grpBroker")
        self.layoutBroker = QGridLayout(self.grpBroker)
        self.layoutBroker.setObjectName(u"layoutBroker")
        self.lblLogo = QLabel(self.grpBroker)
        self.lblLogo.setObjectName(u"lblLogo")
        self.lblLogo.setMinimumSize(QSize(110, 80))
        self.lblLogo.setStyleSheet(u"background-color: white; color: #00a3c4; font-weight: bold;")
        self.lblLogo.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.layoutBroker.addWidget(self.lblLogo, 0, 0, 4, 1)

        self.lblIpAddress = QLabel(self.grpBroker)
        self.lblIpAddress.setObjectName(u"lblIpAddress")
        self.lblIpAddress.setAlignment(Qt.AlignmentFlag.AlignRight|Qt.AlignmentFlag.AlignVCenter)

        self.layoutBroker.addWidget(self.lblIpAddress, 0, 1, 1, 1)

        self.lneIpAddress = QLineEdit(self.grpBroker)
        self.lneIpAddress.setObjectName(u"lneIpAddress")

        self.layoutBroker.addWidget(self.lneIpAddress, 0, 2, 1, 1)

        self.lblPort = QLabel(self.grpBroker)
        self.lblPort.setObjectName(u"lblPort")
        self.lblPort.setAlignment(Qt.AlignmentFlag.AlignRight|Qt.AlignmentFlag.AlignVCenter)

        self.layoutBroker.addWidget(self.lblPort, 1, 1, 1, 1)

        self.lnePort = QLineEdit(self.grpBroker)
        self.lnePort.setObjectName(u"lnePort")

        self.layoutBroker.addWidget(self.lnePort, 1, 2, 1, 1)

        self.lblUser = QLabel(self.grpBroker)
        self.lblUser.setObjectName(u"lblUser")
        self.lblUser.setAlignment(Qt.AlignmentFlag.AlignRight|Qt.AlignmentFlag.AlignVCenter)

        self.layoutBroker.addWidget(self.lblUser, 2, 1, 1, 1)

        self.lneUser = QLineEdit(self.grpBroker)
        self.lneUser.setObjectName(u"lneUser")

        self.layoutBroker.addWidget(self.lneUser, 2, 2, 1, 1)

        self.lblPassword = QLabel(self.grpBroker)
        self.lblPassword.setObjectName(u"lblPassword")
        self.lblPassword.setAlignment(Qt.AlignmentFlag.AlignRight|Qt.AlignmentFlag.AlignVCenter)

        self.layoutBroker.addWidget(self.lblPassword, 3, 1, 1, 1)

        self.lnePassword = QLineEdit(self.grpBroker)
        self.lnePassword.setObjectName(u"lnePassword")
        self.lnePassword.setEchoMode(QLineEdit.EchoMode.Password)

        self.layoutBroker.addWidget(self.lnePassword, 3, 2, 1, 1)

        self.btnConnect = QPushButton(self.grpBroker)
        self.btnConnect.setObjectName(u"btnConnect")
        self.btnConnect.setMinimumSize(QSize(0, 34))

        self.layoutBroker.addWidget(self.btnConnect, 4, 1, 1, 1)

        self.btnDisconnect = QPushButton(self.grpBroker)
        self.btnDisconnect.setObjectName(u"btnDisconnect")
        self.btnDisconnect.setMinimumSize(QSize(0, 34))

        self.layoutBroker.addWidget(self.btnDisconnect, 4, 2, 1, 1)


        self.layoutPrincipal.addWidget(self.grpBroker)

        self.grpData = QGroupBox(self.centralwidget)
        self.grpData.setObjectName(u"grpData")
        self.layoutData = QGridLayout(self.grpData)
        self.layoutData.setObjectName(u"layoutData")
        self.lblDateTime = QLabel(self.grpData)
        self.lblDateTime.setObjectName(u"lblDateTime")
        font = QFont()
        font.setFamilies([u"Consolas"])
        font.setPointSize(14)
        self.lblDateTime.setFont(font)
        self.lblDateTime.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.layoutData.addWidget(self.lblDateTime, 0, 0, 1, 4)

        self.lblTemperature = QLabel(self.grpData)
        self.lblTemperature.setObjectName(u"lblTemperature")

        self.layoutData.addWidget(self.lblTemperature, 1, 0, 1, 1)

        self.lcdTemperature = QLCDNumber(self.grpData)
        self.lcdTemperature.setObjectName(u"lcdTemperature")
        self.lcdTemperature.setMinimumSize(QSize(70, 28))
        self.lcdTemperature.setDigitCount(5)
        self.lcdTemperature.setSegmentStyle(QLCDNumber.SegmentStyle.Flat)

        self.layoutData.addWidget(self.lcdTemperature, 1, 1, 1, 1)

        self.lblHumidity = QLabel(self.grpData)
        self.lblHumidity.setObjectName(u"lblHumidity")

        self.layoutData.addWidget(self.lblHumidity, 1, 2, 1, 1)

        self.lcdHumidity = QLCDNumber(self.grpData)
        self.lcdHumidity.setObjectName(u"lcdHumidity")
        self.lcdHumidity.setMinimumSize(QSize(70, 28))
        self.lcdHumidity.setDigitCount(5)
        self.lcdHumidity.setSegmentStyle(QLCDNumber.SegmentStyle.Flat)

        self.layoutData.addWidget(self.lcdHumidity, 1, 3, 1, 1)


        self.layoutPrincipal.addWidget(self.grpData)

        self.grpCommands = QGroupBox(self.centralwidget)
        self.grpCommands.setObjectName(u"grpCommands")
        self.layoutCommands = QGridLayout(self.grpCommands)
        self.layoutCommands.setObjectName(u"layoutCommands")
        self.btnSynchroDateTime = QPushButton(self.grpCommands)
        self.btnSynchroDateTime.setObjectName(u"btnSynchroDateTime")
        self.btnSynchroDateTime.setMinimumSize(QSize(0, 36))

        self.layoutCommands.addWidget(self.btnSynchroDateTime, 0, 0, 1, 2)

        self.btnStopMeasure = QPushButton(self.grpCommands)
        self.btnStopMeasure.setObjectName(u"btnStopMeasure")
        self.btnStopMeasure.setMinimumSize(QSize(0, 36))

        self.layoutCommands.addWidget(self.btnStopMeasure, 1, 0, 1, 1)

        self.btnStartMeasure = QPushButton(self.grpCommands)
        self.btnStartMeasure.setObjectName(u"btnStartMeasure")
        self.btnStartMeasure.setMinimumSize(QSize(0, 36))

        self.layoutCommands.addWidget(self.btnStartMeasure, 1, 1, 1, 1)

        self.btnSlowBlink = QPushButton(self.grpCommands)
        self.btnSlowBlink.setObjectName(u"btnSlowBlink")
        self.btnSlowBlink.setMinimumSize(QSize(0, 36))

        self.layoutCommands.addWidget(self.btnSlowBlink, 2, 0, 1, 1)

        self.btnFastBlink = QPushButton(self.grpCommands)
        self.btnFastBlink.setObjectName(u"btnFastBlink")
        self.btnFastBlink.setMinimumSize(QSize(0, 36))

        self.layoutCommands.addWidget(self.btnFastBlink, 2, 1, 1, 1)

        self.btnBlink = QPushButton(self.grpCommands)
        self.btnBlink.setObjectName(u"btnBlink")
        self.btnBlink.setMinimumSize(QSize(0, 36))

        self.layoutCommands.addWidget(self.btnBlink, 3, 0, 1, 1)

        self.layoutPeriod = QHBoxLayout()
        self.layoutPeriod.setObjectName(u"layoutPeriod")
        self.lnePeriod = QLineEdit(self.grpCommands)
        self.lnePeriod.setObjectName(u"lnePeriod")
        self.lnePeriod.setAlignment(Qt.AlignmentFlag.AlignRight|Qt.AlignmentFlag.AlignVCenter)

        self.layoutPeriod.addWidget(self.lnePeriod)

        self.lblMs = QLabel(self.grpCommands)
        self.lblMs.setObjectName(u"lblMs")

        self.layoutPeriod.addWidget(self.lblMs)


        self.layoutCommands.addLayout(self.layoutPeriod, 3, 1, 1, 1)

        self.btnBargraph = QPushButton(self.grpCommands)
        self.btnBargraph.setObjectName(u"btnBargraph")
        self.btnBargraph.setMinimumSize(QSize(0, 36))

        self.layoutCommands.addWidget(self.btnBargraph, 4, 0, 1, 1)

        self.lneBargraph = QLineEdit(self.grpCommands)
        self.lneBargraph.setObjectName(u"lneBargraph")

        self.layoutCommands.addWidget(self.lneBargraph, 4, 1, 1, 1)


        self.layoutPrincipal.addWidget(self.grpCommands)

        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(MainWindow)
        self.menubar.setObjectName(u"menubar")
        self.menuFichier = QMenu(self.menubar)
        self.menuFichier.setObjectName(u"menuFichier")
        self.menuAide = QMenu(self.menubar)
        self.menuAide.setObjectName(u"menuAide")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(MainWindow)
        self.statusbar.setObjectName(u"statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.menubar.addAction(self.menuFichier.menuAction())
        self.menubar.addAction(self.menuAide.menuAction())
        self.menuFichier.addAction(self.actionQuit)
        self.menuAide.addAction(self.actionAbout)

        self.retranslateUi(MainWindow)

        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"Mon capteur IDO via MQTT (c)2026 CERI SN - PPI", None))
        self.actionQuit.setText(QCoreApplication.translate("MainWindow", u"Quitter", None))
#if QT_CONFIG(shortcut)
        self.actionQuit.setShortcut(QCoreApplication.translate("MainWindow", u"Ctrl+Q", None))
#endif // QT_CONFIG(shortcut)
        self.actionAbout.setText(QCoreApplication.translate("MainWindow", u"A propos", None))
        self.grpBroker.setTitle(QCoreApplication.translate("MainWindow", u"Param\u00e8tres de communication du Broker MQTT", None))
        self.lblLogo.setText(QCoreApplication.translate("MainWindow", u"IMT\n"
"Nord Europe", None))
        self.lblIpAddress.setText(QCoreApplication.translate("MainWindow", u"<b>Adresse IP :</b>", None))
        self.lblPort.setText(QCoreApplication.translate("MainWindow", u"<b>Port :</b>", None))
        self.lblUser.setText(QCoreApplication.translate("MainWindow", u"<b>Utilisateur :</b>", None))
        self.lblPassword.setText(QCoreApplication.translate("MainWindow", u"<b>Mot de passe :</b>", None))
        self.btnConnect.setText(QCoreApplication.translate("MainWindow", u"Connecter", None))
        self.btnDisconnect.setText(QCoreApplication.translate("MainWindow", u"D\u00e9connecter", None))
        self.grpData.setTitle(QCoreApplication.translate("MainWindow", u"Affichage des donn\u00e9es du capteur", None))
        self.lblDateTime.setText(QCoreApplication.translate("MainWindow", u"00/00/0000 00:00:00", None))
        self.lblTemperature.setText(QCoreApplication.translate("MainWindow", u"<b>Temp\u00e9rature (en \u00b0C) :</b>", None))
        self.lblHumidity.setText(QCoreApplication.translate("MainWindow", u"<b>Humidit\u00e9 (en %) :</b>", None))
        self.grpCommands.setTitle(QCoreApplication.translate("MainWindow", u"Commandes", None))
        self.btnSynchroDateTime.setText(QCoreApplication.translate("MainWindow", u"Synchronisation de la date et heure du capteur (NTP)", None))
        self.btnStopMeasure.setText(QCoreApplication.translate("MainWindow", u"Arr\u00eater les mesures", None))
        self.btnStartMeasure.setText(QCoreApplication.translate("MainWindow", u"D\u00e9marrer les mesures", None))
        self.btnSlowBlink.setText(QCoreApplication.translate("MainWindow", u"Clignotement lent", None))
        self.btnFastBlink.setText(QCoreApplication.translate("MainWindow", u"Clignotement rapide", None))
        self.btnBlink.setText(QCoreApplication.translate("MainWindow", u"Clignotement personnalis\u00e9", None))
        self.lnePeriod.setText(QCoreApplication.translate("MainWindow", u"500", None))
        self.lblMs.setText(QCoreApplication.translate("MainWindow", u"<b>ms</b>", None))
        self.btnBargraph.setText(QCoreApplication.translate("MainWindow", u"Valeur du bargraphe :", None))
        self.lneBargraph.setPlaceholderText(QCoreApplication.translate("MainWindow", u"0 \u00e0 1023, puis Entr\u00e9e", None))
        self.menuFichier.setTitle(QCoreApplication.translate("MainWindow", u"Fichier", None))
        self.menuAide.setTitle(QCoreApplication.translate("MainWindow", u"Aide", None))
    # retranslateUi

