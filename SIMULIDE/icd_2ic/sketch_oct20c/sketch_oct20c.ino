#include <LiquidCrystal_AIP31068_I2C.h> // Remplacement du nom d'en-tête

// Initialisation (Adresse I2C 0x3E par défaut, 16 colonnes, 2 lignes)
LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2); 

void setup() {
  lcd.init();
  
  lcd.setCursor(1, 0);
  lcd.print("HELLO");
  
  lcd.setCursor(8, 1);
  lcd.print("WORLD");
}

void loop() {
}