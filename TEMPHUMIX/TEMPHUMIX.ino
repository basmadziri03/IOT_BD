// Projet IOT - Boitier TempHumiX (Arduino UNO R4 WiFi)
// - mesure la temperature et l'humidite (DHT22) et les affiche sur le LCD
// - se connecte au WiFi et met l'heure a jour avec le serveur NTP
// - publie les mesures sur le broker MQTT (seulement quand elles changent)
// - recoit les commandes : COMMAND, BARGRAPH et BLINK

#include <Wire.h>
#include <rgb_lcd.h>
#include <DHT.h>
#include "RTC.h"
#include <WiFiS3.h>
#include <NTPClient.h>
#include <ArduinoMqttClient.h>
#include "arduino_secrets.h"   // identifiants WiFi et MQTT (pas sur GitHub)

// ----- Parametres -----

#define VERSION_NUM "1.0"            // format X.Y
#define VERSION "V" VERSION_NUM      // pour l'ecran d'accueil
#define LED_VIE 2
#define PIN_DHT A0

// bargraphe sur D3 a D12
#define BAR_PREMIERE 3
#define BAR_DERNIERE 12

// numero du PC (etiquette au-dessus du poste)
#define PC_ID "48566"

// la loop tourne toutes les 50 ms pour gerer des periodes multiples de 100 ms
#define TICK 50
#define PERIODE_NORMALE 500   // 2 Hz
#define PERIODE_LENTE 1000    // 1 Hz
#define PERIODE_RAPIDE 200    // 5 Hz

rgb_lcd lcd;
DHT dht(PIN_DHT, DHT22);

// ----- Objets -----

// serveur NTP de l'ecole, +3600 s pour l'heure francaise
WiFiUDP udp;
NTPClient ntp(udp, IPAddress(192, 168, 1, 254), 3600);

// broker Mosquitto
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "192.168.1.1";
const int port = 1883;

// ----- Variables -----

int etat = 0;
int tick = 0;           // 10 ticks = 500 ms
bool ledAllumee = false;
bool wifiOk = false;
bool heureOk = false;   // true quand le NTP a repondu une fois
int compteurNtp = 0;
int compteurMqtt = 0;

// LED de vie
int periodeLed = PERIODE_NORMALE;
int periodeBlink = PERIODE_NORMALE;   // periode recue sur BLINK
bool modePerso = false;               // commande 60
int compteurLed = 0;

bool mesureActive = true;     // commandes 20 et 30
bool demandeSynchro = false;  // commande 10

// derniere mesure lue
float derniereTemp = 0;
float derniereHum = 0;
bool mesureOk = false;

// dernieres valeurs publiees en dixiemes (23.4 -> 234)
// -9999 au depart pour que la 1ere mesure soit toujours publiee
int tempPubliee = -9999;
int humPubliee = -9999;

// ----- WiFi, MQTT et NTP -----

// retourne true si on a une adresse IP
bool setup_wifi() {
  Serial.print("Connexion a ");
  Serial.println(SECRET_SSID);

  WiFi.begin(SECRET_SSID, SECRET_PASS);

  // 10 s maximum
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

// donne le topic complet, ex : /iot/TEMPERATURE_48566
void construireTopic(char* topic, int taille, const char* nom) {
  snprintf(topic, taille, "/iot/%s_%s", nom, PC_ID);
}

void souscrire(const char* nom) {
  char topic[40];
  construireTopic(topic, sizeof(topic), nom);
  mqttClient.subscribe(topic);

  Serial.print("Abonne a ");
  Serial.println(topic);
}

// retourne 0 si OK, sinon le code d'erreur MQTT
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

  // on se reabonne a chaque connexion
  mqttClient.onMessage(onMqttMessage);
  souscrire("COMMAND");
  souscrire("BARGRAPH");
  souscrire("BLINK");
  return 0;
}

// met le RTC a l'heure du serveur NTP
bool synchroHeure() {
  if (!wifiOk) return false;

  if (!ntp.forceUpdate()) {
    Serial.println("NTP : pas de reponse");
    return false;
  }

  RTCTime heureNtp((time_t)ntp.getEpochTime());
  RTC.setTime(heureNtp);

  Serial.print("Heure NTP : ");
  Serial.println(ntp.getFormattedTime());
  return true;
}

// ----- Programme principal -----

void setup() {
  Serial.begin(9600);

  pinMode(LED_VIE, OUTPUT);
  for (int pin = BAR_PREMIERE; pin <= BAR_DERNIERE; pin++) {
    pinMode(pin, OUTPUT);
  }
  dht.begin();

  lcd.begin(16, 2);
  lcd.setRGB(0, 128, 128);
  lcd.print("Connexion WiFi..");

  // heure par defaut si le NTP ne repond pas
  RTC.begin();
  RTCTime depart(1, Month::JANUARY, 2026, 0, 0, 0, DayOfWeek::THURSDAY, SaveLight::SAVING_TIME_INACTIVE);
  RTC.setTime(depart);

  wifiOk = setup_wifi();

  if (wifiOk) {
    ntp.begin();
    heureOk = synchroHeure();
    setup_mqtt();
  }

  // ecran d'accueil pendant 3 s
  ecranAccueil();
  delay(3000);
  lcd.clear();
}

void loop() {
  // lit les messages MQTT recus
  mqttClient.poll();

  // LED de vie
  compteurLed += TICK;
  if (compteurLed >= periodeLed / 2) {
    compteurLed = 0;
    ledAllumee = !ledAllumee;
    digitalWrite(LED_VIE, ledAllumee);
  }

  // commande 10 : synchro NTP puis publication de la version
  if (demandeSynchro) {
    demandeSynchro = false;
    if (synchroHeure()) {
      heureOk = true;
      if (mqttClient.connected()) publier("VERSION", VERSION_NUM);
    }
  }

  tick++;
  if (tick >= 500 / TICK) {
    tick = 0;
    machineEtat();
  }

  delay(TICK);
}

// machine d'etat appelee toutes les 500 ms
void machineEtat() {
  switch (etat) {
    case 0:   // toutes les 2 s : date, mesure et publication
      afficherDate();
      if (mesureActive) {
        lireCapteur();
      } else {
        lcd.setCursor(0, 1);
        lcd.print("Mesure arretee  ");
      }
      publierDonnees();
      break;

    case 2:   // 1 s plus tard : la date
      afficherDate();
      break;
  }

  // synchro NTP toutes les 10 s si ca n'a jamais marche, sinon toutes les 60 s
  compteurNtp++;
  int limite = heureOk ? 120 : 20;
  if (compteurNtp >= limite) {
    compteurNtp = 0;
    if (synchroHeure()) heureOk = true;
  }

  etat = (etat + 1) % 4;
}

// ----- Affichage -----

void ecranAccueil() {
  RTCTime t;
  RTC.getTime(t);

  // ligne 1 : version et date, ligne 2 : adresse IP
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

  // pas d'heure fiable tant que le NTP n'a pas repondu
  if (!heureOk) {
    lcd.print("Synchro NTP KO  ");
    return;
  }

  RTCTime t;
  RTC.getTime(t);

  // pas la place pour l'annee sur 16 caracteres
  char ligne[17];
  snprintf(ligne, sizeof(ligne), "%02d/%02d %02d:%02d:%02d",
           t.getDayOfMonth(), Month2int(t.getMonth()),
           t.getHour(), t.getMinutes(), t.getSeconds());

  lcd.print(ligne);
}

void lireCapteur() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // on efface la ligne 2
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

  // 223 = symbole degre
  char ligne[17];
  snprintf(ligne, sizeof(ligne), "T %.1f%cC/H %.1f%%", temp, 223, hum);
  lcd.print(ligne);

  Serial.print("T = ");
  Serial.print(temp, 1);
  Serial.print(" C   H = ");
  Serial.print(hum, 1);
  Serial.println(" %");
}

// ----- Publication MQTT -----

void publier(const char* nom, const char* valeur) {
  char topic[40];
  construireTopic(topic, sizeof(topic), nom);

  mqttClient.beginMessage(topic);
  mqttClient.print(valeur);
  mqttClient.endMessage();

  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(valeur);
}

// publie TEMPERATURE, HUMIDITY et GET_DATE si une valeur a change
void publierDonnees() {
  if (!mqttClient.connected()) {
    // nouvel essai de connexion toutes les 10 s
    compteurMqtt++;
    if (compteurMqtt < 5) return;
    compteurMqtt = 0;
    if (setup_mqtt() != 0) return;
  }

  // rien a publier si la mesure est arretee ou en erreur
  if (!mesureActive || !mesureOk) return;

  int temp10 = lround(derniereTemp * 10);
  int hum10 = lround(derniereHum * 10);

  if (temp10 == tempPubliee && hum10 == humPubliee) return;   // pas de changement

  tempPubliee = temp10;
  humPubliee = hum10;

  char valeur[24];

  snprintf(valeur, sizeof(valeur), "%.1f", derniereTemp);
  publier("TEMPERATURE", valeur);

  snprintf(valeur, sizeof(valeur), "%.1f", derniereHum);
  publier("HUMIDITY", valeur);

  // date au format DD/MM/YYYY HH:MM:SS
  RTCTime t;
  RTC.getTime(t);
  snprintf(valeur, sizeof(valeur), "%02d/%02d/%04d %02d:%02d:%02d",
           t.getDayOfMonth(), Month2int(t.getMonth()), t.getYear(),
           t.getHour(), t.getMinutes(), t.getSeconds());
  publier("GET_DATE", valeur);
}

// ----- Reception MQTT -----

// appelee par mqttClient.poll() quand un message arrive
void onMqttMessage(int tailleMessage) {
  // acquisition du topic
  String topic = mqttClient.messageTopic();

  // acquisition du message
  char message[16];
  int i = 0;
  while (mqttClient.available()) {
    char c = mqttClient.read();
    if (i < 15) message[i++] = c;
  }
  message[i] = 0;

  // conversion du message en entier
  int valeur = atoi(message);

  Serial.print("Recu ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(valeur);

  if (topic.indexOf("COMMAND") >= 0) {
    traiterCommande(valeur);
  } else if (topic.indexOf("BARGRAPH") >= 0) {
    afficherBargraphe(valeur);
  } else if (topic.indexOf("BLINK") >= 0) {
    // periode multiple de 100 ms obligatoire
    if (valeur <= 0 || valeur % 100 != 0) {
      Serial.println("BLINK refuse : periode multiple de 100 ms attendue");
      return;
    }
    periodeBlink = valeur;
    if (modePerso) periodeLed = periodeBlink;

    Serial.print("BLINK : periode enregistree = ");
    Serial.print(periodeBlink);
    Serial.println(" ms");
  }
}

void traiterCommande(int code) {
  switch (code) {
    case 10:   // forcer la mise a la date et l'heure
      demandeSynchro = true;
      break;
    case 20:   // demarrage mesure
      mesureActive = true;
      break;
    case 30:   // arret mesure
      mesureActive = false;
      break;
    case 40:   // clignotement lent
      modePerso = false;
      periodeLed = PERIODE_LENTE;
      break;
    case 50:   // clignotement rapide
      modePerso = false;
      periodeLed = PERIODE_RAPIDE;
      break;
    case 60:   // clignotement personnalise (periode recue sur BLINK)
      modePerso = true;
      periodeLed = periodeBlink;
      break;
    default:
      Serial.println("Code commande inconnu");
      return;
  }

  // affichage pour verifier la commande
  Serial.print("Commande ");
  Serial.print(code);
  Serial.print(" appliquee -> LED : periode = ");
  Serial.print(periodeLed);
  Serial.print(" ms, mesure ");
  Serial.println(mesureActive ? "active" : "arretee");
}

// 0 = aucune LED, 1023 = toutes les LEDs
void afficherBargraphe(int valeur) {
  valeur = constrain(valeur, 0, 1023);

  int nbLeds = BAR_DERNIERE - BAR_PREMIERE + 1;
  int allumees = map(valeur, 0, 1023, 0, nbLeds);

  for (int i = 0; i < nbLeds; i++) {
    digitalWrite(BAR_PREMIERE + i, i < allumees ? HIGH : LOW);
  }

  Serial.print("BARGRAPH : ");
  Serial.print(allumees);
  Serial.println(" LED(s) allumee(s)");
}
