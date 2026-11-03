// Partie 1 - Chenillard sur le bargraphe
// Les LEDs s'allument une par une, aller puis retour

const int premiereLED = 3;   // LEDs branchees de D3 a D12
const int derniereLED = 12;
const int vitesse = 100;     // temps d'allumage de chaque LED en ms

void setup() {
  // toutes les broches du bargraphe en sortie
  for (int thisPin = premiereLED; thisPin <= derniereLED; thisPin++) {
    pinMode(thisPin, OUTPUT);
  }
}

void loop() {
  // aller : de la premiere a la derniere LED
  for (int thisPin = premiereLED; thisPin <= derniereLED; thisPin++) {
    digitalWrite(thisPin, HIGH);
    delay(vitesse);
    digitalWrite(thisPin, LOW);
  }

  // retour : de la derniere a la premiere LED
  for (int thisPin = derniereLED; thisPin >= premiereLED; thisPin--) {
    digitalWrite(thisPin, HIGH);
    delay(vitesse);
    digitalWrite(thisPin, LOW);
  }
}
