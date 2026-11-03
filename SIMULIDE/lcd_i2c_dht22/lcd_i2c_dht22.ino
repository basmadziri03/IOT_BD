// Partie 1 - Temperature et humidite sur l'ecran LCD
// Ligne 1 : IMT NE - PPI
// Ligne 2 : T 22.5oC/H 68.5%   (mise a jour toutes les 2 s)

#include <Wire.h>
#include <LiquidCrystal_AIP31068_I2C.h>
#include <DHT.h>

#define DHTPIN A0
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2);

void setup() {
  Wire.begin();   // bus I2C pour l'ecran
  dht.begin();
  lcd.begin(16, 2);

  lcd.setCursor(0, 0);
  lcd.print("IMT NE - PPI");
}

void loop() {
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // mesure ratee : on garde l'ancien affichage
  if (isnan(h) || isnan(t)) {
    return;
  }

  lcd.setCursor(0, 1);
  lcd.print("T ");
  lcd.print(t, 1);
  lcd.print("oC/H ");
  lcd.print(h, 1);
  lcd.print("%");
}
