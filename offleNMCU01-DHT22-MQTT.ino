#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "DHT.h"
#include <MQTTClient.h>

#define DHTTYPE DHT22
#define DHTPIN 4

const char* host = "mqtthost";
const char* ssid = "ssid";
const char* password = "password";

WiFiClient net;
MQTTClient mqtt;

DHT dht(DHTPIN, DHTTYPE);
ADC_MODE(ADC_VCC);

void connect();


void setup() {
  
  dht.begin();

  Serial.begin(9600);
  Serial.println();
  Serial.println("Booting...");

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  mqtt.begin(host, net);
  
  connect();
  mqtt.publish("RW8/ESP/offleNMCU01/DHT22/Status", "Boot");
  Serial.println("Setup completed...");
}



void loop() {

  mqtt.loop();
  if (!mqtt.connected()) {
    connect();
  }
  
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int vcc = ESP.getVcc();
  


  if (!isnan(humidity) || !isnan(temp)) {
    Serial.println("Sending Data to MQTT... ");
    Serial.print("IP Address: ");
    Serial.println(String(WiFi.localIP()));
    Serial.print("Voltage: ");
    Serial.println(vcc);
    Serial.print("Temp: ");
    Serial.print(String(temp));
    Serial.print(" Humidity: ");
    Serial.println(String(humidity));

    
    mqtt.publish("RW8/ESP/offleNMCU01/DHT22/Status", "DHT22 OK");
    mqtt.publish("RW8/ESP/offleNMCU01/DHT22/IPAddress", WiFi.localIP().toString());
    mqtt.publish("RW8/ESP/offleNMCU01/DHT22/Voltage", String(vcc));
    mqtt.publish("RW8/ESP/offleNMCU01/DHT22/Temperature", String(temp));
    mqtt.publish("RW8/ESP/offleNMCU01/DHT22/Humidity", String(humidity));
    
  }
  else
  {
    Serial.println("Error reading Data from DHT22... ");
    mqtt.publish("RW8/ESP/offleNMCU01/DHT22/Status", "DHT22 Read Error");
  }

  delay(15000); 

  Serial.println("Going into deep sleep for 600 seconds");
  ESP.deepSleep(600e6); // 20e6 is 20 microseconds
  delay(500);
    
}

void connect() {
  while(WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("WiFi connection failed. Retry.");
  }

  WiFi.hostname("offleNMCU01");
  Serial.print("Wifi connection successful - IP-Address: ");
  Serial.println(WiFi.localIP());

  while (!mqtt.connect(host)) {
    Serial.print(".");
  }

  Serial.println("MQTT connected!");
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
