/*
 * Project automatedWatering(refactored)
 * Author: Jamie Dowden-Duarte
 * Date: 11/03/25
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Air_quality_Sensor.h"
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"
#include "JsonParserGeneratorRK.h"
#include "Credentials.h"
#include <math.h>

////////////////////////////////////////////////////////////
// Let Device OS manage the connection to the Particle Cloud
////////////////////////////////////////////////////////////
SYSTEM_MODE(AUTOMATIC);
TCPClient TheClient;

////////////////////
// Adafruit.io feeds
///////////////////
Adafruit_MQTT_SPARK mqtt(&TheClient, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
///////////////
// subscription
///////////////
Adafruit_MQTT_Subscribe waterFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/waterFeed");
//////////
// publish
//////////
Adafruit_MQTT_Publish airQualFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/airqualfeed");
Adafruit_MQTT_Publish humFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humfeed");
Adafruit_MQTT_Publish moistureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisturefeed");
Adafruit_MQTT_Publish tempFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tempfeed");

///////////
// GLOBALS
///////////
/////////////
//  constants
/////////////
const int OLED_RESET = -1, PUMP = D16, HEXADDRESS = 0x76, DUSTS = D4, HOUR = 600000, QUAL = A2;
////////////
// Variables
///////////
unsigned int startTime1, airValue, duration, lowPulse, currentQual = -1;
int soilMoist = A1, tempC, presPA, buttonState, moistureReads;
float ratio = 0, concentration = 0;
bool status;
////////
// class
////////
String dateTime, timeOnly;

//////////
// objects
//////////
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BME280 bme;
AirQualitySensor airQualSens(QUAL);
struct Enviromental_Cond
{
  float TempF;
  float Hum;
  float PressHG;
};
Enviromental_Cond env_Cond;
///////////////
// functions
//////////////
void MQTT_connect();
bool MQTT_ping();
void bme(int timeFrame);
// void dustS(int timeFrame);
void air(int timeFrame);
void soil(int pin, int timeFrame);
void eventPayLoad(Enviromental_Cond env_Conditions);

void setup()
{
  WiFi.on();
  WiFi.connect();
  Serial.begin(9600);
  waitFor(Serial.isConnected, 1000);

  bme.begin(HEXADDRESS);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  Time.zone(-7);
  Particle.syncTime();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  Serial.printf("Ready to go\n");
  status = bme.begin(HEXADDRESS);
  if (status == false)
  {
    Serial.printf("BME280 at address 0x%02X failed to start", HEXADDRESS);
  }

  ///////////////
  // subscription
  ///////////////
  mqtt.subscribe(&waterFeed);

  //////////
  // pinMode
  //////////
  pinMode(soilMoist, INPUT);
  pinMode(PUMP, OUTPUT);
}

void loop()
{
  MQTT_connect();
  MQTT_ping();
  bme(10000);
  // dustS(HOUR);
  air(10000);
  soil(soilMoist, 10000);

  Adafruit_MQTT_Subscribe *subscription;
  while (subscription = mqtt.readSubscription(100))
  {
    if (subscription == &waterFeed)
    {
      buttonState = atoi((char *)waterfeed.lastread);
      if (buttonState == 0)
      {
        digitalWrite(PUMPPIN, LOW);
      }
      if (buttonState == 1)
      {
        digitalWrite(PUMPPIN, HIGH);
      }
    }
  }
}

void MQTT_connect()
{
  int8_t ret;

  // Return if already connected.
  if (mqtt.connected())
  {
    return;
  }
  Serial.printf("MQTT connected!\n");
  while ((ret = mqtt.connect()) != 0)
  { // connect will return 0 for connected
    Serial.printf("Error Code %s\n", mqtt.connectErrorString(ret));
    Serial.printf("Retrying MQTT connection in 5 seconds...\n");
    mqtt.disconnect();
    delay(5000); // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}
bool MQTT_ping()
{
  static unsigned int last;
  bool pingStatus;

  if ((millis() - last) > 120000)
  {
    Serial.printf("Pinging MQTT \n");
    pingStatus = mqtt.ping();
    if (!pingStatus)
    {
      Serial.printf("Disconnecting \n");
      mqtt.disconnect();
    }
    last = millis();
  }
  return pingStatus;
}
void eventPayLoad(Enviromental_Cond env_Conditions)
{
  JsonWriterStatic<256> jw;
  {
    JsonWriterAutoObject obj(&jw);
    jw.insertKeyValue("Humidity", env_Conditions.Hum);
    jw.insertKeyValue("Tempurate F", env_Conditions.TempF);
    jw.insertKeyValue("Pressure HG", env_Conditions.PressHG);
  }
  enviroFeed.publish(jw.getBuffer());
}
void bme(int timeFrame)
{
  static unsigned int currentTime, lastTime;
  currentTime = millis();

  if ((currentTime - lastTime) > timeFrame)
  {
    lastTime = millis();
    display.clearDisplay();
    tempC = bme.readTemperature();
    presPA = bme.readPressure();
    humRH = bme.readHumidity();

    tempF = map(tempC, 0, 38, 32, 100);
    inHG = map(presPA, 0, 135456, 0, 40);
    display.setRotation(3);
    display.setCursor(0, 0);
    display.printf("Temp F: %i", tempF);
    display.display();
    display.setCursor(0, 20);
    display.printf("Press: %i", inHG);
    display.display();
    display.setCursor(0, 40);
    display.printf("Hum: %i", humRH);
    display.display();
    humFeed.publish(humRH);
    tempFeed.publish(tempF);
  }
}

// void dustS(int timeFrame)
// {
//   static unsigned int currentTime, lastTime;
//   int startTime = millis();
//   currentTime = millis();
//   if ((currentTime - lastTime) > timeFrame)
//   {
//     lastTime = millis();
//     duration = pulseIn(DUSTS, LOW);
//     lowPulse = lowPulse + duration;
//   }
//   if ((millis() - startTime) > 30000)
//   {
//     ratio = lowPulse / (30000.0);
//     concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 3) + 520 * ratio + 0.62;
//     Serial.printf("Low pulse = %i, Ratio = %f\n", lowPulse, ratio);
//     lowPulse = 0;
//     startTime = millis();
//   }
//   return;
// }

void air(int timeFrame)
{
  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();

  if ((currentTime - lastTime) >= timeFrame)
  {
    lastTime = currentTime;

    airValue = airQualSens.getValue();
    currentQual = airQualSens.slope();

    char qualityStr[50];

    switch (currentQual)
    {
    case 0:
      strcpy(qualityStr, "{\"quality\":\"High Pollution\",\"message\":\"Caution! High pollution detected!\"}");
      break;
    case 1:
      strcpy(qualityStr, "{\"quality\":\"Rising\",\"message\":\"Pollution rising!\"}");
      break;
    case 2:
      strcpy(qualityStr, "{\"quality\":\"Low\",\"message\":\"Low pollution.\"}");
      break;
    case 3:
      strcpy(qualityStr, "{\"quality\":\"Good\",\"message\":\"Fresh air!\"}");
      break;
    default:
      strcpy(qualityStr, "{\"quality\":\"Unknown\",\"message\":\"Unknown quality\"}");
      break;
    }

    Serial.printf("Air Quality: %s\n", qualityStr);
    Serial.printf("Quant Value = %i\n", airValue);

    // Check MQTT connection before publishing
    if (!mqtt.connected())
    {
      MQTT_connect();
    }

    // Publish air quality
    if (!airQualFeed.publish(qualityStr))
    {
      Serial.println("Failed to publish air quality");
    }
    if (airQualFeed.publish(qualityStr))
    {
      Serial.println("Published air quality successfully");
    }

    delay(500); // Small delay after publishing
  }
}

void soil(int pin, int timeFrame)
{
  static unsigned long lastCheckTime = 0;
  static bool isWatering = false;
  const int MOISTURE_THRESHOLD = 1000; // Adjust this value based on your soil sensor readings
  const int WATERING_DURATION = 30000; // 30 seconds
  const int COOLDOWN_PERIOD = 60000;   // 60 seconds

  unsigned long currentTime = millis();

  if ((currentTime - lastCheckTime) >= timeFrame)
  {
    lastCheckTime = currentTime;

    int moisturereads = analogRead(pin);
    moistureFeed.publish(moisturereads);

    if (!isWatering && moisturereads <= MOISTURE_THRESHOLD)
    {
      Serial.printf("Soil is dry! Watering pump activated");
      digitalWrite(PUMPPIN, HIGH);
      isWatering = true;
      lastCheckTime += WATERING_DURATION; // Skip next checks during watering
    }
    else if (isWatering && currentTime - lastCheckTime >= WATERING_DURATION)
    {
      Serial.printf("Watering completed");
      digitalWrite(PUMPPIN, LOW);
      isWatering = false;
      lastCheckTime += COOLDOWN_PERIOD; // Skip next checks during cooldown
    }
    else
    {
      Serial.printf("Soil moisture is at the right levels");
      digitalWrite(PUMPPIN, LOW);
    }
  }
}