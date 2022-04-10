/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive data with the MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/

#include <MKRWAN.h>
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

LoRaModem modem;

#include "arduino_secrets.h"
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

#define RainPin 4  

Adafruit_BME280 bme280;

bool bucketPositionA = false;    
const double bucketAmount = 0.364;
double hourlyRain = 0.0;  

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(RainPin, INPUT);  
  attachInterrupt(digitalPinToInterrupt(RainPin), addRainCounter, RISING);
   
  if (!bme280.begin()) {
    Serial.println("Invalid BME280, control sensor connection!");
    while (1);
  }

  while (!Serial);
  if (!modem.begin(EU868)) {
    Serial.println("ERROR. Failed to start LoRa module");
    while (1) {}
  };

  Serial.print("Module version: ");
  Serial.println(modem.version());
  Serial.print("DevEUI: ");
  Serial.println(modem.deviceEUI());

  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("ERROR. Failed to connect to gateway");
    while (1) {}
  }

  // Set poll interval to 60 secs.
  modem.minPollInterval(60);
  // NOTE: independent of this setting, the modem will
  // not allow sending more than one message every 2 minutes,
  // this is enforced by firmware and can not be changed.

}

void sendSensorValuesToGateway() {
  float temp = bme280.readTemperature();
  float humidity = bme280.readHumidity();
  int pressure = bme280.readPressure() / 100.0F;

  String payload = "{\"t\":" + String(temp) + ",\"h\":" + String(humidity) + ",\"p\":" + String(pressure) +",\"r\":" + String(hourlyRain) + "}" ;

  int err;
  modem.beginPacket();
  modem.print(payload);
  err = modem.endPacket(true);

  if (err > 0) {
    Serial.println("Message sent successfully!");
  } else {
    Serial.println("ERROR sending message");
  }
}

void loop(){
  sendSensorValuesToGateway();
  delay(20000);
}


void addRainCounter(){

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
    {
      hourlyRain+=bucketAmount; 
    }
  last_interrupt_time = interrupt_time;
         
}
