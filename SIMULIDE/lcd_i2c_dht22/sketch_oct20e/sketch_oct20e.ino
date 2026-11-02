#include <Wire.h>
#include <LiquidCrystal_AIP31068_I2C.h>
#include <DHT.h>

#define DHTPIN A0     // Broche connectée au DHT22
#define DHTTYPE DHT22   // Définition du type de capteur

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2); // Adresse I2C (0x3E) et format 16x2

void setup() {
  Wire.begin();       // Initialisation du bus I2C
  dht.begin();        // Démarrage du capteur DHT
  lcd.begin(16, 2);   // Initialisation de l'afficheur LCD
  
  // En-tête fixe
  lcd.setCursor(0, 0);
  lcd.print("IMT NE - PPI");
}

void loop() {
  delay(2000); // Attente entre chaque mesure

  // Lecture de l'humidité et de la température
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Vérification si les données sont valides
  if (isnan(h) || isnan(t)) {
    return;
  }

  // Affichage sur la deuxième ligne : T 22.5oC/H 68.5%
  lcd.setCursor(0, 1);
  lcd.print("T ");
  lcd.print(t, 1);
  lcd.print("oC/H ");
  lcd.print(h, 1);
  lcd.print("%");
}