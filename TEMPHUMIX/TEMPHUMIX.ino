// Projet IOT - TempHumiX
// Partie 6.1.6 : on ne publie plus que quand c'est utile
//  - VERSION seulement apres une synchro NTP demandee par la commande 10
//  - TEMPERATURE, HUMIDITY et GET_DATE seulement si la temperature ou l'humidite a change
// (WiFi, NTP, publication et souscription MQTT des parties 6.1.3 a 6.1.5 conserves)

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

// LEDs du bargraphe (meme cablage que le bargraphe de la partie 1)
#define BAR_PREMIERE 3
#define BAR_DERNIERE 12

// numero du PC (etiquette code-barre 44xxx au-dessus du poste)
#define PC_ID "48566"

// la boucle tourne toutes les 50 ms pour pouvoir faire clignoter la LED
// avec une periode multiple de 100 ms (demi-periode multiple de 50 ms)
#define TICK 50
#define PERIODE_NORMALE 500   // 2 Hz
#define PERIODE_LENTE 1000    // 1 Hz
#define PERIODE_RAPIDE 200    // 5 Hz

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
int tick = 0;           // une etape de la machine d'etat tous les 10 ticks = 500 ms
bool ledAllumee = false;
bool wifiOk = false;
bool heureOk = false;   // passe a true des que le NTP a repondu au moins une fois
int compteurNtp = 0;    // pour retenter la synchro regulierement
int compteurMqtt = 0;   // pour retenter la connexion au broker regulierement

// clignotement de la LED de vie, modifiable par COMMAND et BLINK
int periodeLed = PERIODE_NORMALE;
int periodeBlink = PERIODE_NORMALE;   // derniere periode recue sur BLINK
bool modePerso = false;               // true apres la commande 60
int compteurLed = 0;

bool mesureActive = true;     // commandes 20 / 30
bool demandeSynchro = false;  // commande 10, traitee dans la loop

// derniere mesure valide, publiee sur MQTT
float derniereTemp = 0;
float derniereHum = 0;
bool mesureOk = false;

// dernieres valeurs publiees, en dixiemes (ex : 23.4 C -> 234)
// on compare des entiers car on publie au dixieme : 23.41 et 23.44 donnent le meme "23.4"
// -9999 = rien publie encore, donc la 1ere mesure est toujours envoyee
int tempPubliee = -9999;
int humPubliee = -9999;

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

// construit le topic /iot/<nom>_<PC_ID>, ex : /iot/TEMPERATURE_48566
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

  // les abonnements sont perdus a chaque deconnexion, on les refait ici
  mqttClient.onMessage(onMqttMessage);
  souscrire("COMMAND");
  souscrire("BARGRAPH");
  souscrire("BLINK");
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
  for (int pin = BAR_PREMIERE; pin <= BAR_DERNIERE; pin++) {
    pinMode(pin, OUTPUT);
  }
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
  // a appeler souvent : garde la connexion en vie et declenche onMqttMessage
  mqttClient.poll();

  // LED de vie : on l'inverse toutes les demi-periodes
  compteurLed += TICK;
  if (compteurLed >= periodeLed / 2) {
    compteurLed = 0;
    ledAllumee = !ledAllumee;
    digitalWrite(LED_VIE, ledAllumee);
  }

  // commande 10 : on fait la synchro ici et pas dans le callback MQTT
  if (demandeSynchro) {
    demandeSynchro = false;
    if (synchroHeure()) {
      heureOk = true;
      // 6.1.6 a) : la version n'est publiee que lors de cette synchro
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

// appelee toutes les 500 ms
void machineEtat() {
  switch (etat) {
    case 0:   // t = 0 s : date + capteur + publication MQTT (toutes les 2 s)
      afficherDate();
      if (mesureActive) {
        lireCapteur();
      } else {
        lcd.setCursor(0, 1);
        lcd.print("Mesure arretee  ");
      }
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

// appelee toutes les 2 s : publie TEMPERATURE, HUMIDITY et GET_DATE
// seulement si la temperature ou l'humidite a change (6.1.6 b)
void publierDonnees() {
  if (!mqttClient.connected()) {
    // on retente la connexion toutes les 10 s (5 appels x 2 s) pour ne pas bloquer
    compteurMqtt++;
    if (compteurMqtt < 5) return;
    compteurMqtt = 0;
    if (setup_mqtt() != 0) return;
  }

  // rien a publier si la mesure est arretee (commande 30) ou si le capteur ne repond pas
  if (!mesureActive || !mesureOk) return;

  // passage en dixiemes, arrondi comme le "%.1f" de la publication
  int temp10 = lround(derniereTemp * 10);
  int hum10 = lround(derniereHum * 10);

  if (temp10 == tempPubliee && hum10 == humPubliee) return;   // rien n'a change

  tempPubliee = temp10;
  humPubliee = hum10;

  char valeur[24];

  // au dixieme de degre / de pourcent
  snprintf(valeur, sizeof(valeur), "%.1f", derniereTemp);
  publier("TEMPERATURE", valeur);

  snprintf(valeur, sizeof(valeur), "%.1f", derniereHum);
  publier("HUMIDITY", valeur);

  // date de la mesure, format DD/MM/YYYY HH:MM:SS
  RTCTime t;
  RTC.getTime(t);
  snprintf(valeur, sizeof(valeur), "%02d/%02d/%04d %02d:%02d:%02d",
           t.getDayOfMonth(), Month2int(t.getMonth()), t.getYear(),
           t.getHour(), t.getMinutes(), t.getSeconds());
  publier("GET_DATE", valeur);
}

// appelee par mqttClient.poll() quand un message arrive sur un topic souscrit
void onMqttMessage(int tailleMessage) {
  // acquisition du topic
  String topic = mqttClient.messageTopic();

  // acquisition du message (15 caracteres max, ca suffit pour un nombre)
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
    // la periode doit etre un multiple de 100 ms
    if (valeur <= 0 || valeur % 100 != 0) {
      Serial.println("BLINK refuse : periode multiple de 100 ms attendue");
      return;
    }
    periodeBlink = valeur;
    if (modePerso) periodeLed = periodeBlink;   // deja en mode 60 : on applique tout de suite

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

  // pour verifier sur le moniteur serie que la commande a bien ete appliquee
  Serial.print("Commande ");
  Serial.print(code);
  Serial.print(" appliquee -> LED : periode = ");
  Serial.print(periodeLed);
  Serial.print(" ms, mesure ");
  Serial.println(mesureActive ? "active" : "arretee");
}

// 0 -> aucune LED, 1023 -> toutes les LEDs du bargraphe allumees
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
