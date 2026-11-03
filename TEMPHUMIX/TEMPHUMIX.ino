// Projet IOT - TempHumiX
// Partie 6.1.4 : publication des donnees du capteur sur le broker MQTT
// (WiFi + NTP de la partie 6.1.3 conserves)

#include <Wire.h>
#include <rgb_lcd.h>
#include <DHT.h>
#include "RTC.h"
#include <WiFiS3.h>
#include <NTPClient.h>
#include <ArduinoMqttClient.h>
#include "arduino_secrets.h"   // SSID, mot de passe WiFi et identifiants MQTT, pas envoye sur GitHub

#define VERSION_NUM "1.0"            // format X.Y demande pour le topic VERSION
#define VERSION "V" VERSION_NUM      // affiche sur l'ecran d'accueil
#define LED_VIE 2
#define PIN_DHT A0

// numero du PC (etiquette code-barre 44xxx au-dessus du poste) : A MODIFIER
#define PC_ID "48566"

rgb_lcd lcd;
DHT dht(PIN_DHT, DHT22);

// serveur NTP de l'ecole, +3600 s pour etre a l'heure francaise (heure d'hiver)
WiFiUDP udp;
NTPClient ntp(udp, IPAddress(192, 168, 1, 254), 3600);

// broker Mosquitto (paragraphe 5.1.2)
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "192.168.1.1";
const int port = 1883;

int etat = 0;
bool ledAllumee = false;
bool wifiOk = false;
bool heureOk = false;   // passe a true des que le NTP a repondu au moins une fois
int compteurNtp = 0;    // pour retenter la synchro regulierement
int compteurMqtt = 0;   // pour retenter la connexion au broker regulierement

// derniere mesure valide, publiee sur MQTT
float derniereTemp = 0;
float derniereHum = 0;
bool mesureOk = false;

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

// retourne 0 si OK, sinon le code erreur MQTT
// (-1 = timeout, -2 = connexion refusee, 4 = mauvais login, 5 = non autorise...)
int setup_mqtt() {
  if (!wifiOk) return MQTT_CONNECTION_REFUSED;

  mqttClient.setId("IdO_Client_" PC_ID);
  mqttClient.setUsernamePassword(SECRET_MQTT_USER, SECRET_MQTT_PASS);

  Serial.print("Connexion au broker ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    int erreur = mqttClient.connectError();
    Serial.print("MQTT : echec, code ");
    Serial.println(erreur);
    return erreur;
  }

  Serial.println("MQTT : connecte");
  return 0;
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
    setup_mqtt();
  }

  // ecran d'accueil pendant 3 s avant l'affichage classique
  ecranAccueil();
  delay(3000);
  lcd.clear();
}

void loop() {
  // a appeler souvent : garde la connexion au broker en vie (keep alive)
  mqttClient.poll();

  ledAllumee = !ledAllumee;
  digitalWrite(LED_VIE, ledAllumee);

  switch (etat) {
    case 0:   // t = 0 s : date + capteur + publication MQTT (toutes les 2 s)
      afficherDate();
      lireCapteur();
      publierDonnees();
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
    mesureOk = false;
    return;
  }

  derniereTemp = temp;
  derniereHum = hum;
  mesureOk = true;

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

// publie une valeur sur le topic /iot/<nom>_<PC_ID>, ex : /iot/TEMPERATURE_44370
void publier(const char* nom, const char* valeur) {
  char topic[40];
  snprintf(topic, sizeof(topic), "/iot/%s_%s", nom, PC_ID);

  mqttClient.beginMessage(topic);
  mqttClient.print(valeur);
  mqttClient.endMessage();

  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(valeur);
}

// publie les 4 topics du paragraphe 3.2
void publierDonnees() {
  if (!mqttClient.connected()) {
    // on retente la connexion toutes les 10 s (5 appels x 2 s) pour ne pas bloquer
    compteurMqtt++;
    if (compteurMqtt < 5) return;
    compteurMqtt = 0;
    if (setup_mqtt() != 0) return;
  }

  char valeur[24];

  publier("VERSION", VERSION_NUM);

  // au dixieme de degre / de pourcent
  if (mesureOk) {
    snprintf(valeur, sizeof(valeur), "%.1f", derniereTemp);
    publier("TEMPERATURE", valeur);

    snprintf(valeur, sizeof(valeur), "%.1f", derniereHum);
    publier("HUMIDITY", valeur);
  }

  // format DD/MM/YYYY HH:MM:SS
  RTCTime t;
  RTC.getTime(t);
  snprintf(valeur, sizeof(valeur), "%02d/%02d/%04d %02d:%02d:%02d",
           t.getDayOfMonth(), Month2int(t.getMonth()), t.getYear(),
           t.getHour(), t.getMinutes(), t.getSeconds());
  publier("GET_DATE", valeur);
}
