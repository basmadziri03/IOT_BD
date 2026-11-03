// Partie 1 - Premier test de l'ecran LCD en I2C
// Affiche HELLO sur la ligne 1 et WORLD sur la ligne 2

#include <LiquidCrystal_AIP31068_I2C.h>

// ecran 16 colonnes x 2 lignes, adresse I2C 0x3E
LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2);

void setup() {
  lcd.init();

  lcd.setCursor(1, 0);   // colonne 1, ligne 0
  lcd.print("HELLO");

  lcd.setCursor(8, 1);   // colonne 8, ligne 1
  lcd.print("WORLD");
}

void loop() {
  // rien a faire, le texte reste affiche
}
