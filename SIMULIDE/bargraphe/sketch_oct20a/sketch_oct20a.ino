const int premiereLED = 3;
const int derniereLED = 11;
const int vitesse = 100; // Vitesse en millisecondes (100 ms)

void setup() {
  // Configuration de toutes les broches de la séquence en sortie (OUTPUT)
  for (int thisPin = premiereLED; thisPin <= derniereLED; thisPin++) {
    pinMode(thisPin, OUTPUT);
  }
}

void loop() {
  // Balayage de la première vers la dernière LED
  for (int thisPin = premiereLED; thisPin <= derniereLED; thisPin++) {
    digitalWrite(thisPin, HIGH);   // Allume la LED
    delay(vitesse);                // Attend 100 ms
    digitalWrite(thisPin, LOW);    // Éteint la LED
  }

  // Balayage retour de la dernière vers la première LED
  for (int thisPin = derniereLED; thisPin >= premiereLED; thisPin--) {
    digitalWrite(thisPin, HIGH);   // Allume la LED
    delay(vitesse);                // Attend 100 ms
    digitalWrite(thisPin, LOW);    // Éteint la LED
  }
}
