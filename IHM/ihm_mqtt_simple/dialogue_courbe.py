# Projet IOT - IHM du capteur TempHumiX
# Fenetre des courbes (donnees de IOT.db)

import os
import sqlite3
from datetime import datetime

import matplotlib
matplotlib.use("Agg")   # dessin en memoire puis affichage dans Qt
import matplotlib.dates as mdates
from matplotlib import pyplot as plt
from matplotlib.backends.backend_agg import FigureCanvasAgg

from PySide6.QtCore import QDateTime
from PySide6.QtGui import QImage, QPixmap
from PySide6.QtWidgets import QGraphicsScene, QDialog, QGraphicsPixmapItem

from graphe import Ui_GraphWindow
from client_mqtt import DOSSIER

id_pc = 48566

database_name = os.path.join(DOSSIER, "IOT.db")
table_name = "IOT_DATA"

# format des dates dans la base
FORMAT_BASE = "yyyy-MM-dd HH:mm:ss"


class CourbeDialog(QDialog, Ui_GraphWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)

        # par defaut : la derniere heure
        dte = QDateTime.currentDateTime()
        dts = dte.addSecs(-3600)

        self.dteStartTime.setDateTime(dts)
        self.dteEndTime.setDateTime(dte)

        self.scene = QGraphicsScene()
        self.graphicsView.setScene(self.scene)

        self.btnLoadData.clicked.connect(self.afficher_courbe)
        self.btnClearData.clicked.connect(self.effacer_courbe)

    def figure_to_pixmap(self, fig):
        canvas = FigureCanvasAgg(fig)
        canvas.draw()
        width, height = fig.get_size_inches() * fig.get_dpi()
        width, height = int(width), int(height)
        buf = canvas.buffer_rgba()
        image = QImage(buf, width, height, QImage.Format.Format_RGBA8888)
        # copie car la figure est fermee ensuite
        return QPixmap.fromImage(image.copy())

    def afficher_courbe(self):
        # dates au format de la base pour le BETWEEN
        debut = self.dteStartTime.dateTime().toString(FORMAT_BASE)
        fin = self.dteEndTime.dateTime().toString(FORMAT_BASE)

        conn = sqlite3.connect(database_name)
        cur = conn.cursor()
        cur.execute(f"SELECT Date, Temperature, Humidite FROM {table_name} "
                    "WHERE Id_capteur = ? AND Date BETWEEN ? AND ? ORDER BY Date ASC",
                    (id_pc, debut, fin))
        rows = cur.fetchall()
        conn.close()

        self.scene.clear()
        if not rows:
            self.scene.addText(f"Aucune mesure archivée entre le {debut} et le {fin}")
            return

        date_x = [datetime.strptime(r[0], "%Y-%m-%d %H:%M:%S") for r in rows]
        temp_y = [r[1] for r in rows]
        humi_y = [r[2] for r in rows]

        # figure de la taille de la zone d'affichage
        taille = self.graphicsView.viewport().size()
        fig, ax = plt.subplots(figsize=(taille.width() / 100, taille.height() / 100), dpi=100)

        # un axe pour la temperature, un pour l'humidite
        ax.plot(date_x, temp_y, marker="o", markersize=3, color="tab:red", label="Température (°C)")
        ax.set_ylabel("Température (°C)", color="tab:red")
        ax.tick_params(axis="y", labelcolor="tab:red")

        ax2 = ax.twinx()
        ax2.plot(date_x, humi_y, marker="+", color="tab:blue", label="Humidité (%)")
        ax2.set_ylabel("Humidité (%)", color="tab:blue")
        ax2.tick_params(axis="y", labelcolor="tab:blue")

        ax.set_title(f"Données capteur IdO {id_pc} ({len(rows)} mesures)")
        ax.set_xlabel("Date")
        # graduations adaptees a la duree affichee
        localisateur = mdates.AutoDateLocator()
        ax.xaxis.set_major_locator(localisateur)
        ax.xaxis.set_major_formatter(mdates.ConciseDateFormatter(localisateur))
        ax.grid(True)
        fig.tight_layout()

        pixmap = self.figure_to_pixmap(fig)
        plt.close(fig)   # libere la memoire

        item = QGraphicsPixmapItem(pixmap)
        self.scene.addItem(item)
        self.scene.setSceneRect(item.boundingRect())

    def effacer_courbe(self):
        self.scene.clear()
