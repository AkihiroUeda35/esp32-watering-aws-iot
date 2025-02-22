#include <WiFi.h>
#include <DHT.h>
#include <DHT_U.h>

#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#include "secrets.h"

struct tm timeinfo;

const float VOLT = 3.3; // 3.3V
const int WATER_MOTOR = 13; 
const int MOISTURE = 34;

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

const bool AWS_CONNECT = true;
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);
RTC_DATA_ATTR int bootCounter = 0;

//Temperature
constexpr uint8_t DHTPIN = 27;   
constexpr uint8_t DHTTYPE = DHT22;
DHT dht{DHTPIN, DHTTYPE};

char buff[100];

void get_moisture(double *moisture)
{
  *moisture = VOLT * analogRead(MOISTURE)*1000.0/4096;
  Serial.print("MOISTURE LEVEL:");
  Serial.println(*moisture);
}
void get_temperature_humidity(double *temperature, double *humidity)
{
  *temperature = dht.readTemperature();
  *humidity = dht.readHumidity();
  Serial.println(" Temperature:" + String(*temperature ) + 
  " Humidity:" + String(*humidity));
}

void setup_connection(){
  WiFi.mode(WIFI_STA);
  if (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_DISCONNECTED) {
    ESP.restart();
  }
  int cnt =0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    cnt+=1;
    if(cnt==100){
      ESP.restart();
    }
  }
  if(AWS_CONNECT == false){
    return;
  }
  //Connect to AWS IoT Core
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  client.begin(AWS_IOT_ENDPOINT, 8883, net);  
  Serial.print("Connecting to AWS IOT");
  cnt=0;
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
    if(cnt==100){
      break;
    }
    cnt++;
  }
  Serial.println(".");

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    ESP.restart();
  }
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  //set hardware
  dht.begin();
  pinMode(WATER_MOTOR, OUTPUT);
  pinMode(MOISTURE, INPUT);
  digitalWrite(WATER_MOTOR, HIGH);
  setup_connection();
  //ntp
  configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");//NTP
  getLocalTime(&timeinfo);
  sprintf(buff, "Current Time: %04d/%02d/%02d %02d:%02d:%02d",
          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  Serial.println(buff);
  
}

void loop() {
  double moisture = 0.0;
  double temperature = 0.0;
  double humidity = 0.0;
  StaticJsonDocument<200> doc;
  char timestamp[100];
  sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02dZ",
          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  get_moisture(&moisture);
  get_temperature_humidity(&temperature, &humidity);
  doc["deviceid"] = THINGNAME;
  doc["timestamp"] = timestamp;
  doc["moisture"] = moisture;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  char jsonBuffer[512];
  
  if(AWS_CONNECT){
    serializeJson(doc, jsonBuffer);
    Serial.println(jsonBuffer);
    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
    
  }
  unsigned long long watering_time_sec = 0; // unit:sec
  unsigned long long sleep_time_sec = 0; // unit:sec
  int current_hour = timeinfo.tm_hour;
  if(timeinfo.tm_min > 30){
    current_hour += 1;
  }
  sprintf(buff, "bootCounter=%d", bootCounter);
  Serial.println(buff);
  if(bootCounter==0){
    watering_time_sec = 10;
  }
  bootCounter++;
  if(current_hour == 6 || current_hour == 7
    // || current_hour==18 || current_hour== 19
  ){
    watering_time_sec = 10;
  }
  if(watering_time_sec > 0){
    sprintf(buff, "Motor ON %d seconds", watering_time_sec);
    Serial.println(buff);
    digitalWrite(WATER_MOTOR, LOW);
    delay(watering_time_sec*1000);
    Serial.println("Motor OFF");
    digitalWrite(WATER_MOTOR, HIGH);  
  }
  sleep_time_sec = 3600 * (current_hour + 1 - timeinfo.tm_hour) - timeinfo.tm_min*60 - timeinfo.tm_sec - watering_time_sec;
  if(sleep_time_sec < 600){
    sleep_time_sec = 600;
  }
  //Deep Sleep
  delay(10000);
  sprintf(buff, "Sleep %d seconds", sleep_time_sec);
  Serial.println(buff);  
  esp_sleep_enable_timer_wakeup(sleep_time_sec*1000000ULL);
  esp_deep_sleep_start();
}
