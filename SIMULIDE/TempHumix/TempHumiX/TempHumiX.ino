// Projet IOT - TempHumiX
// Partie 6.1.3 : connexion au WiFi IoT + mise a l'heure du RTC par NTP

#include <Wire.h>
#include <rgb_lcd.h>
#include <DHT.h>
#include "RTC.h"
#include <WiFiS3.h>
#include <NTPClient.h>
#include "arduino_secrets.h"   // SSID et mot de passe, pas envoye sur GitHub

#define VERSION "V1.0"
#define LED_VIE 2
#define PIN_DHT A0

rgb_lcd lcd;
DHT dht(PIN_DHT, DHT22);

// serveur NTP de l'ecole, +3600 s pour etre a l'heure francaise (heure d'hiver)
WiFiUDP udp;
NTPClient ntp(udp, IPAddress(192, 168, 1, 254), 3600);

int etat = 0;
bool ledAllumee = false;
bool wifiOk = false;
bool heureOk = false;   // passe a true des que le NTP a repondu au moins une fois
int compteurNtp = 0;    // pour retenter la synchro regulierement

bool setup_wifi() {
  Serial.print("Connexion a ");
  Serial.println(SECRET_SSID);

  WiFi.begin(SECRET_SSID, SECRET_PASS);

  // on laisse 10 s max pour avoir une IP, sinon on abandonne
  for (int i = 0; i < 20; i++) {
    if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
      Serial.print("Connecte, IP : ");
      Serial.println(WiFi.localIP());
      return true;
    }
    delay(500);
  }

  Serial.println("Pas de connexion WiFi");
  return false;
}

bool synchroHeure() {
  if (!wifiOk) return false;

  // forceUpdate attend la reponse 1 s max, donc ca ne bloque pas longtemps
  if (!ntp.forceUpdate()) {
    Serial.println("NTP : pas de reponse");
    return false;
  }

  // on recopie l'heure recue dans le RTC
  RTCTime heureNtp((time_t)ntp.getEpochTime());
  RTC.setTime(heureNtp);

  Serial.print("Heure NTP : ");
  Serial.println(ntp.getFormattedTime());
  return true;
}

void setup() {
  Serial.begin(9600);

  pinMode(LED_VIE, OUTPUT);
  dht.begin();

  lcd.begin(16, 2);
  lcd.setRGB(0, 128, 128);
  lcd.print("Connexion WiFi..");

  // heure par defaut, au cas ou le NTP ne repond pas
  RTC.begin();
  RTCTime depart(1, Month::JANUARY, 2026, 0, 0, 0, DayOfWeek::THURSDAY, SaveLight::SAVING_TIME_INACTIVE);
  RTC.setTime(depart);

  wifiOk = setup_wifi();

  if (wifiOk) {
    ntp.begin();
    heureOk = synchroHeure();
  }

  // ecran d'accueil pendant 3 s avant l'affichage classique
  ecranAccueil();
  delay(3000);
  lcd.clear();
}

void loop() {
  ledAllumee = !ledAllumee;
  digitalWrite(LED_VIE, ledAllumee);

  switch (etat) {
    case 0:   // t = 0 s : date + capteur
      afficherDate();
      lireCapteur();
      break;

    case 2:   // t = 1 s : juste la date
      afficherDate();
      break;
  }

  // resynchro NTP : toutes les 10 s tant que ca n'a jamais marche,
  // puis toutes les 60 s une fois qu'on a eu l'heure
  compteurNtp++;
  int limite = heureOk ? 120 : 20;
  if (compteurNtp >= limite) {
    compteurNtp = 0;
    if (synchroHeure()) heureOk = true;
  }

  etat = (etat + 1) % 4;
  delay(500);
}

void ecranAccueil() {
  RTCTime t;
  RTC.getTime(t);

  // ligne 1 : version + date, ligne 2 : adresse IP
  char ligne[17];
  snprintf(ligne, sizeof(ligne), "%s  %02d/%02d/%d", VERSION,
           t.getDayOfMonth(), Month2int(t.getMonth()), t.getYear());

  lcd.clear();
  lcd.print(ligne);
  lcd.setCursor(0, 1);

  if (wifiOk) lcd.print(WiFi.localIP());
  else lcd.print("Pas de WiFi");

  Serial.println(ligne);
}

void afficherDate() {
  lcd.setCursor(0, 0);

  // tant que le NTP n'a jamais repondu, l'heure du RTC est fausse, on le signale
  if (!heureOk) {
    lcd.print("Synchro NTP KO  ");
    return;
  }

  RTCTime t;
  RTC.getTime(t);

  // pas assez de place pour l'annee ET les secondes sur 16 caracteres
  char ligne[17];
  snprintf(ligne, sizeof(ligne), "%02d/%02d %02d:%02d:%02d",
           t.getDayOfMonth(), Month2int(t.getMonth()),
           t.getHour(), t.getMinutes(), t.getSeconds());

  lcd.print(ligne);
}

void lireCapteur() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // on efface la 2e ligne avant d'ecrire dessus
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);

  if (isnan(temp) || isnan(hum)) {
    lcd.print("Erreur capteur");
    Serial.println("Erreur lecture DHT22");
    return;
  }

  // 223 = symbole degre sur ce LCD
  char ligne[17];
  snprintf(ligne, sizeof(ligne), "T %.1f%cC/H %.1f%%", temp, 223, hum);
  lcd.print(ligne);

  Serial.print("T = ");
  Serial.print(temp, 1);
  Serial.print(" C   H = ");
  Serial.print(hum, 1);
  Serial.println(" %");
}