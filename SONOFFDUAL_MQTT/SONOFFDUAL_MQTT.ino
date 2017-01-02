#include <Arduino.h>

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <EEPROM.h>
#include "password.inc"

#define INDICATOR 13
#define RELAY0 0x01
#define RELAY1 0x02

int ledState = LOW;

byte relay0State = 0;
byte relay1State = 0;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);
Adafruit_MQTT_Publish devicestatus = Adafruit_MQTT_Publish(&mqtt, "/UUID2/status");
Adafruit_MQTT_Subscribe devicecommand = Adafruit_MQTT_Subscribe(&mqtt, "/UUID2/command");

void setup() {
  // TODO Recover last relay state from EEPROM
  //EEPROM.begin(10);
  //relay0State = EEPROM.read(0);
  //relay1State = EEPROM.read(1);
  //updateRelay();

  pinMode(INDICATOR, OUTPUT);

  digitalWrite(INDICATOR, ledState);

  Serial.begin(19200);
  delay(10);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ledState = !ledState;
    digitalWrite(INDICATOR, ledState);
  }

  digitalWrite(INDICATOR, LOW);

  mqtt.subscribe(&devicecommand);

}

void loop()
{
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(0))) {
    if (subscription == &devicecommand) {

      char * commandval = (char *)devicecommand.lastread;
      if (strcmp(commandval, "RELAY0ON") == 0){
        relay0State = B00000001;
      }else
      if (strcmp(commandval, "RELAY0OFF") == 0){
        relay0State = 0;
      }else
      if (strcmp(commandval, "RELAY1ON") == 0){
        relay1State = B00000010;
      }else
      if (strcmp(commandval, "RELAY1OFF") ==0){
        relay1State = 0;
      }
      updateRelay();
      //EEPROM.write(0, relay0State);
      //EEPROM.write(1, relay1State);
      //EEPROM.commit();

    }
  }
}
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    digitalWrite(INDICATOR, LOW);
    return;
  }

  //Serial.print("Connecting to MQTT... ");
  digitalWrite(INDICATOR, HIGH);

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    //Serial.println(mqtt.connectErrorString(ret));
    //Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  //Serial.println("MQTT Connected!");
}

void updateRelay() {
  // Send command
  Serial.write(0xA0); //Start transmission
  Serial.write(0x04); //Start transmission
  Serial.write((relay0State & RELAY0) | (relay1State & RELAY1)); //first two bits represent the relays
  Serial.write(0xA1); //End transmission
  Serial.flush();
}
