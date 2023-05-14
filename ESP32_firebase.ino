#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#include <FirebaseESP8266.h>
#endif

// Provide the token generation process info.
#include <addons/TokenHelper.h>
#include <DHT.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Flybox-AC07"
#define WIFI_PASSWORD "vc2CPi25PUzy"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyD4Fn09QkiGFte8rz8bhIsrmEVVVqhu3C4"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "razafimahatratramisa@gmail.com"
#define USER_PASSWORD "Asim?j0h8982"

/* 3. Define the RTDB URL */
#define DATABASE_URL "esp32-flutter-1f7d9-default-rtdb.firebaseio.com/"


#define DHTPIN 2
 
// Definit the sensor type being used
#define DHTTYPE DHT11
 
// Declare un objet de type DHT
// Il faut passer en parametre du constructeur 
// de l'objet la broche et le type de capteur
DHT dht(DHTPIN, DHTTYPE);


// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;

// Database child nodes
String tempPath = "/temperature";
String humPath = "/humidity";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Uncomment this if want to to format the time
/*

void formatEpochTimeToString(time_t epochTime, char* str) {
  struct tm timeinfo;
  localtime_r(&epochTime, &timeinfo);
  strftime(str, 20, "%d/%m/%Y:%H:%M", &timeinfo);
}

*/

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now + 10800;
}


void setup()
{

  Serial.begin(115200);

  initWiFi();

  configTime(0, 0, ntpServer);

  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";

  dht.begin();
}

void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);
    parentPath= databasePath + "/" + String(timestamp);

    json.set(tempPath.c_str(), String(String(dht.readTemperature())+" °C"));
    json.set(humPath.c_str(), String(String(dht.readHumidity())+" %"));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}
