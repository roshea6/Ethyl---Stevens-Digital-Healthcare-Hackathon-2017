// Libraries
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <SPI.h>
#include <Wire.h>
#include "Timer.h"

// WiFi credentials
const char* ssid     = "EthylHotspot";
const char* password = "ethylpass";

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "happyface"
#define AIO_KEY         "bfa49b338dd74eac8067e97e065faff4"

// Input Definitions
#define ALCOHOL_IN    A0
#define REMOVED_IN    D1
#define USER_IN       D2

// Output Definitions
#define LED_R   2
#define LED_G   D6
#define LED_B   D7

enum colors {
  nothing,
  red,
  green,
  blue,
  yellow,
  cyan,
  purple,
  white
};

Timer t;

// Functions
void connect();
void check_connection();
int publish_gas();
int publish_user();
int publish_removed();
int publish_breath();
void publish_stuff();
void set_red();
void set_LED(colors color);

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT,AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'test' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish alcohol_sensor = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/alcohol-sensor");
Adafruit_MQTT_Publish removed_sensor = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/removed-sensor");
Adafruit_MQTT_Publish user_sensor = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/user-sensor");
Adafruit_MQTT_Publish breath_sensor = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/breath-sensor");

void setup()   {

  Serial.begin(115200);

  // Set up LEDS
  pinMode(LED_R, OUTPUT);
  delay(500);
  pinMode(LED_G, OUTPUT);
  delay(500);
  pinMode(LED_B, OUTPUT);
  delay(500);

  // Set LED to RED to indicate setup start
  set_LED(red);

  pinMode(REMOVED_IN, INPUT);
  delay(500);
  pinMode(USER_IN, INPUT);
  delay(500);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to Adafruit IO
  connect();

  // Set LED to GREEN to indicate setup complete
  set_LED(nothing);

  // every 4 seconds, publish stuff
  t.every(4*1000, publish_stuff);
}

void loop() {
  check_connection();

  // every minute set LED to RED
  // every 10 second update values
  t.update();

  // if the user didn't remove and is in contact
  if (digitalRead(USER_IN)) {
    publish_breath();
    delay(500);
  }

  if (digitalRead(REMOVED_IN)) {
    set_LED(red);
  } else {
    set_LED(nothing);
  }
}

void check_connection() {
  // ping adafruit io a few times to make sure we remain connected
  if (!mqtt.ping(3)) {
    // reconnect to adafruit io
    if (!mqtt.connected())
      connect();
  }
}

// connect to adafruit io via MQTT
void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);
  }

  Serial.println(F("Adafruit IO Connected!"));
}

int publish_gas() {
  int reading = analogRead(ALCOHOL_IN);
  if(!alcohol_sensor.publish(reading)) {
    Serial.println(F("Failed to publish alcohol"));
  } else {
    Serial.println(F("Published alcohol"));
  }
  Serial.println(reading);
  return reading;
}

int publish_removed() {
  int reading = digitalRead(REMOVED_IN);
  if (!removed_sensor.publish(reading)) {
    Serial.println(F("Failed to publish removal data"));
  } else {
    Serial.println(F("Published removal data"));
  }
  Serial.println(reading);
  return reading;
}

int publish_user() {
  int reading = digitalRead(USER_IN);
  if (!user_sensor.publish(reading)) {
    Serial.println(F("Failed to publish user contact"));
  } else {
    Serial.println(F("Published user contact"));
  }
  Serial.println(reading);
  return reading;
}

int publish_breath() {
  int reading = analogRead(ALCOHOL_IN);
  if (!breath_sensor.publish(reading)) {
    Serial.println(F("Failed to publish breath"));
  } else {
    Serial.println(F("Published breath"));
  }
  Serial.println(reading);
  return reading;
}

void set_red() {
  set_LED(red);
}

void publish_stuff() {
  publish_gas();
  publish_removed();
  publish_user();
}

void set_LED(colors color) {
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);

  switch(color) {
    case red:
      digitalWrite(LED_R, HIGH);
      break;
    case green:
      digitalWrite(LED_G, HIGH);
      break;
    case blue:
      digitalWrite(LED_B, HIGH);
      break;
    case yellow:
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_G, HIGH);
      break;
    case cyan:
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, HIGH);
      break;
    case purple:
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_B, HIGH);
      break;
    case white:
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, HIGH);
      break;
  }
}
