// Partie 1 - Clignotement de la LED de vie
// La LED change d'etat toutes les 500 ms (periode de 1 s)

#define PIN_LED 2

void setup()
{
   pinMode(PIN_LED, OUTPUT);
}

void loop()
{
   digitalWrite(PIN_LED, HIGH);   // LED allumee
   delay(500);
   digitalWrite(PIN_LED, LOW);    // LED eteinte
   delay(500);
}
