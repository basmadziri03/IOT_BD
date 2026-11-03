// Partie 1 - Lecture du capteur DHT22
// On lit la temperature toutes les 2 s :
//   - en dessous de 20 C : LED verte
//   - a partir de 20 C   : LED rouge

#include <DHT.h>

#define DHTPIN A0       // DHT22 branche sur A0
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

const int ledVerte = 3;
const int ledRouge = 12;

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(ledVerte, OUTPUT);
  pinMode(ledRouge, OUTPUT);
}

void loop() {
  delay(2000);   // le DHT22 a besoin de 2 s entre deux mesures

  float t = dht.readTemperature();

  // si la lecture rate, on recommence au tour suivant
  if (isnan(t)) {
    Serial.println(F("Erreur de lecture du capteur DHT !"));
    return;
  }

  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.println(F("°C"));

  if (t < 20.0) {
    digitalWrite(ledVerte, HIGH);
    digitalWrite(ledRouge, LOW);
  }
  else {
    digitalWrite(ledVerte, LOW);
    digitalWrite(ledRouge, HIGH);
  }
}
