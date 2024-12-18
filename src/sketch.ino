#include <WiFi.h>
#include <ArduinoHA.h> // https://github.com/dawidchyrzynski/arduino-home-assistant
#include "DHTesp.h"

#define PUBLISH_PERIOD    10000  // Publish sensor data every 20000 msec
#define DHT_PIN           13
#define LED_BLUE_PIN      18
#define LED_GREEN_PIN     19
#define LED_RED_PIN       21
#define BROKER_ADDR       IPAddress(10,7,16,202)
#define BROKER_USERNAME   "aluno" 
#define BROKER_PASSWORD   "4luno#imd"
#define WIFI_SSID         "Wokwi-GUEST"
#define WIFI_PASSWORD     ""

unsigned long lastTime = 0;
DHTesp dhtSensor;
WiFiClient client;
HADevice device("AluizioRocha_de34ad24be15effeed"); // Each device must have its unique Id
HAMqtt mqtt(client, device);

// "led" is unique ID of the switch. You should define your own ID.
HASwitch led_red("aluiziorocha_led_red");
HASwitch led_green("aluiziorocha_led_green");
HASwitch led_blue("aluiziorocha_led_blue");
HASensor dhtSensorTemp("aluiziorocha_temperature");
HASensor dhtSensorHumi("aluiziorocha_humidity");

void onRedSwitchCommand(bool state, HASwitch* sender)
{
  Serial.print("LED red turning ");
  Serial.println((state ? "ON" : "OFF"));
  digitalWrite(LED_RED_PIN, (state ? HIGH : LOW));
  sender->setState(state); // report state back to the Home Assistant
}

void onGreenSwitchCommand(bool state, HASwitch* sender)
{
  Serial.print("LED green turning ");
  Serial.println((state ? "ON" : "OFF"));
  digitalWrite(LED_GREEN_PIN, (state ? HIGH : LOW));
  sender->setState(state); // report state back to the Home Assistant
}

void onBlueSwitchCommand(bool state, HASwitch* sender)
{
  Serial.print("LED blue turning ");
  Serial.println((state ? "ON" : "OFF"));
  digitalWrite(LED_BLUE_PIN, (state ? HIGH : LOW));
  sender->setState(state); // report state back to the Home Assistant
}

void setup() {
  Serial.begin(115200);
  Serial.print("Starting ");
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  pinMode(LED_RED_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, LOW);
  pinMode(LED_GREEN_PIN, OUTPUT);
  digitalWrite(LED_GREEN_PIN, LOW);
  pinMode(LED_BLUE_PIN, OUTPUT);
  digitalWrite(LED_BLUE_PIN, LOW);

  // connect to wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500); // waiting for the connection
  }
  Serial.println();
  Serial.println("Connected to the network");
  Serial.print("Device ID: "); 
  Serial.println(device.getUniqueId());
  Serial.println("---");

  // set device's details
  device.setName("AluizioRocha");
  device.setManufacturer("Aluizio Rocha");
  device.setModel("Wokwi-ESP32");
  device.setSoftwareVersion("1.0.0");
  // set device's availability when connected to MQTT broker
  device.enableSharedAvailability();
  device.setAvailability(true); 
  device.enableLastWill();

  // handle switch state
  led_red.onCommand(onRedSwitchCommand);
  led_red.setName("Sala Vermelha");
  led_green.onCommand(onGreenSwitchCommand);
  led_green.setName("Sala Verde");
  led_blue.onCommand(onBlueSwitchCommand);
  led_blue.setName("Sala Azul");

  // set temperature sensor details
  dhtSensorTemp.setName("Temperatura");
  dhtSensorTemp.setDeviceClass("temperature");
  dhtSensorTemp.setUnitOfMeasurement("°C");
  dhtSensorTemp.setIcon("mdi:temperature-celsius");
  
  // set humidity sensor details
  dhtSensorHumi.setName("Umidade");
  dhtSensorHumi.setDeviceClass("humidity");
  dhtSensorHumi.setUnitOfMeasurement("%");
  dhtSensorHumi.setIcon("mdi:water-percent");


  mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
}

void loop() {
  mqtt.loop();

  if(millis() - lastTime > PUBLISH_PERIOD)  // Every PUBLISH_PERIOD [msec]
  {
    lastTime = millis();

    TempAndHumidity  data = dhtSensor.getTempAndHumidity();
    String temp = String(data.temperature, 1);
    String humi = String(data.humidity, 1);
    Serial.println("Publishing DHT22 sensor data:\nTemperature: " + temp + "°C");
    Serial.println("Humidity: " + humi + "%");
    Serial.println("---");
    dhtSensorTemp.setValue(temp.c_str());
    dhtSensorHumi.setValue(humi.c_str());
  }
}
