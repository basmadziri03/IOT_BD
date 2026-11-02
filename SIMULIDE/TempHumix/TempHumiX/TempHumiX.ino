
 // TempHumiX : Wi-Fi, horloge, DHT22, LCD RGB et bargraphe
 // Carte : Arduino Uno R4 WiFi

#include <Wire.h>
#include <rgb_lcd.h>
#include <DHT.h>
#include <WiFiS3.h>

// Broches
#define PIN_LED_VIE 2
#define PIN_BARGRAPH 3
#define NB_LEDS 10
#define PIN_DHT A0

// Objets
rgb_lcd lcd;
DHT dht(PIN_DHT, DHT22);

// Identifiants Wi-Fi
char ssid[] = "IoT IMT Nord Europe";
char pass[] = "<mot de passe WiFi>";

char adresseIP[20] = "IP indisponible";

// Date et heure initiales
int jour = 10, mois = 2, annee = 25;
int heure = 13, minute = 42, seconde = 29;

// Mesures
float temperature = 0;
float humidite = 0;

// Cadences temporelles
unsigned long dernierTempsLed = 0;
unsigned long dernierTempsSeconde = 0;
unsigned long dernierTempsCapteur = 0;

bool etatLed = false;

// --------------------------------------------------
// Avancer la date et l'heure
// --------------------------------------------------
void avancerDate()
{
  seconde++;

  if (seconde >= 60)
  {
    seconde = 0;
    minute++;
  }

  if (minute >= 60)
  {
    minute = 0;
    heure++;
  }

  if (heure >= 24)
  {
    heure = 0;
    jour++;
  }

  // Calendrier simplifie : 30 jours par mois
  if (jour > 30)
  {
    jour = 1;
    mois++;
  }

  if (mois > 12)
  {
    mois = 1;
    annee++;
  }
}

// --------------------------------------------------
// Afficher l'humidite sur le bargraphe
// --------------------------------------------------
void afficherBargraphe()
{
  int nb = (int)(humidite / 10.0);
  nb = constrain(nb, 0, NB_LEDS);

  for (int i = 0; i < NB_LEDS; i++)
  {
    digitalWrite(PIN_BARGRAPH + i,
                 i < nb ? HIGH : LOW);
  }
}

// --------------------------------------------------
// Initialisation
// --------------------------------------------------
void setup()
{
  // Configuration des sorties
  pinMode(PIN_LED_VIE, OUTPUT);

  for (int i = 0; i < NB_LEDS; i++)
  {
    pinMode(PIN_BARGRAPH + i, OUTPUT);
    digitalWrite(PIN_BARGRAPH + i, LOW);
  }

  // Port serie
  Serial.begin(9600);

  // Initialisation du LCD RGB
  lcd.begin(16, 2);
  lcd.setRGB(0, 128, 128);
  lcd.setCursor(0, 0);
  lcd.print("Connexion WiFi");

  Serial.println("Connexion au Wi-Fi...");

  // Demarrer la connexion Wi-Fi
  WiFi.begin(ssid, pass);

  // Attendre jusqu'a 30 secondes une IP valide
  unsigned long debutConnexion = millis();
  const unsigned long delaiMax = 30000UL;
  bool ipValide = false;

  while (millis() - debutConnexion < delaiMax)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      IPAddress ip = WiFi.localIP();

      if (ip != IPAddress(0, 0, 0, 0))
      {
        snprintf(adresseIP, sizeof(adresseIP),
                 "%d.%d.%d.%d",
                 ip[0], ip[1], ip[2], ip[3]);

        ipValide = true;
        break;
      }
    }

    lcd.setCursor(0, 1);
    lcd.print("Attente IP...   ");

    Serial.println("En attente de l'adresse IP...");
    delay(500);
  }

  // Afficher le resultat de la connexion
  lcd.clear();

  if (ipValide)
  {
    Serial.print("Wi-Fi connecte. IP : ");
    Serial.println(adresseIP);

    lcd.setCursor(0, 0);
    lcd.print("WiFi connecte !");
    lcd.setCursor(0, 1);
    lcd.print(adresseIP);

    delay(5000);
  }
  else
  {
    strcpy(adresseIP, "IP indisponible");

    Serial.println("Echec : aucune IP obtenue.");

    lcd.setCursor(0, 0);
    lcd.print("IP indisponible");
    lcd.setCursor(0, 1);
    lcd.print("Verifier WiFi");

    delay(3000);
  }

  // Initialisation du capteur DHT22
  dht.begin();

  lcd.clear();

  // Initialiser les compteurs temporels
  unsigned long maintenant = millis();
  dernierTempsLed = maintenant;
  dernierTempsSeconde = maintenant;
  dernierTempsCapteur = maintenant;
}

// --------------------------------------------------
// Boucle principale
// --------------------------------------------------
void loop()
{
  unsigned long tempsActuel = millis();

  char date[17];
  char ligne[17];
  char txtT[8];
  char txtH[8];

  // 1. LED de vie : alternance toutes les 250 ms
  if (tempsActuel - dernierTempsLed >= 250)
  {
    dernierTempsLed = tempsActuel;
    etatLed = !etatLed;

    digitalWrite(PIN_LED_VIE,
                 etatLed ? HIGH : LOW);
  }

  // 2. Horloge : mise a jour chaque seconde
  if (tempsActuel - dernierTempsSeconde >= 1000)
  {
    dernierTempsSeconde += 1000;
    avancerDate();

    snprintf(date, sizeof(date),
             "%02d/%02d/%02d %02d:%02d",
             jour, mois, annee, heure, minute);

    lcd.setCursor(0, 0);
    lcd.print(date);
    lcd.print(" ");
  }

  // 3. Lire le DHT22 toutes les 2 secondes
  if (tempsActuel - dernierTempsCapteur >= 2000)
  {
    dernierTempsCapteur = tempsActuel;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h))
    {
      temperature = t;
      humidite = h;

      // Mettre a jour le bargraphe
      afficherBargraphe();

      // Convertir les valeurs en texte
      snprintf(txtT, sizeof(txtT), "%.1f", temperature);
      snprintf(txtH, sizeof(txtH), "%.1f", humidite);

      // Affichage sur le port serie
      Serial.print("IP : ");
      Serial.println(adresseIP);

      Serial.print("Temperature : ");
      Serial.print(txtT);
      Serial.println(" C");

      Serial.print("Humidite : ");
      Serial.print(txtH);
      Serial.println(" %");

      Serial.println();

      // Affichage LCD : temperature et humidite
      snprintf(ligne, sizeof(ligne),
               "T:%sC H:%s%%", txtT, txtH);

      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(ligne);
    }
    else
    {
      Serial.println("Erreur de lecture du DHT22");
    }
  }
}
