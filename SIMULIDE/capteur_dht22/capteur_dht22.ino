

#include <DHT.h>

// Définition des broches et du type de capteur
#define DHTPIN A0     // Le capteur DHT22 est branché sur la broche A0[cite: 4]
#define DHTTYPE DHT22   // Type de capteur : DHT22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

// Définition des broches des LEDs
const int ledVerte = 3;   // LED verte sur la broche 3[cite: 4]
const int ledRouge = 12;  // LED rouge sur la broche 12[cite: 4]

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Configuration des broches des LEDs en sortie
  pinMode(ledVerte, OUTPUT);
  pinMode(ledRouge, OUTPUT);
}

void loop() {
  // Attendre quelques secondes entre les mesures
  delay(2000);

  // Lecture de la température en Celsius
  float t = dht.readTemperature();

  // Vérifier si la lecture a échoué
  if (isnan(t)) {
    Serial.println(F("Erreur de lecture du capteur DHT !"));
    return;
  }

  // Affichage de la température dans le moniteur série
  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.println(F("°C"));

  // Condition : si température < 20°C, allumer la LED verte et éteindre la rouge
  if (t < 20.0) {
    digitalWrite(ledVerte, HIGH);
    digitalWrite(ledRouge, LOW);
  } 
  // Sinon (température >= 20°C), allumer la LED rouge et éteindre la verte
  else {
    digitalWrite(ledVerte, LOW);
    digitalWrite(ledRouge, HIGH);
  }
}