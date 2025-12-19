// Include Particle Device OS APIs
#include "Particle.h"
#include "IoTClassroom_CNM.h"
#include <Adafruit_GFX_RK.h>
#include <neopixel.h>
#include <DFRobotDFPlayerMini.h>
#include <Adafruit_ILI9341_RK.h>
#include <Adafruit_MQTT_SPARK.h>
#include <Adafruit_MQTT.h>
#include <Credentials.h>

TCPClient TheClient;

Adafruit_MQTT_SPARK mqtt(&TheClient, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish reminders(&mqtt, AIO_USERNAME "/feeds/reminders");
Adafruit_MQTT_Publish colors(&mqtt, AIO_USERNAME "/feeds/colors");
void MQTT_connect();
bool MQTT_ping();
#define TFT_DC D5
#define TFT_CS D4
#define STMPE_CS D3
#define TFT_WIDTH 320
#define TFT_HEIGHT 240

SYSTEM_MODE(AUTOMATIC);

// VARIABLES
// GLOBAL
char buffer[32];
char timeString[9];
int i;
uint8_t x;
unsigned long currentTime = Particle.syncTime();
static unsigned long lastUpdateTime = 0;
bool buttonState, alarmOnOff, MP3OnOff;
unsigned long lastRemindToPeeTime = 0, lastRemindFeetUpTime = o, lastRemindMoveAroundTime = 0;
unsigned long lastCheckTime = 0, lastRemindUpdate = 0;
// Time variables
unsigned long remindToPeeInterval = 3600000, remindFeetUpInterval = 27000000, remindMoveAroundInterval = 108000000;
// Scheduled reminders
unsigned long morningBreakfastTime = 28800000UL, lunchBreakReminderTime = 46800000UL;
unsigned long dishesTime = 36000000UL, eveningWindDownTime = 64800000UL;

// CONSTANTS
const int PIXELCOUNT = 7, NUMBEROFTRACKS = 6, WEMO = 1, HUE = 1;
const unsigned long DEBOUNCE_DELAY = 1000; // 1 second debounce time

// OBJECTS
Adafruit_ILI9341 tft(TFT_CS, TFT_DC);
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
Button encoderSwitch(D15);
DFRobotDFPlayerMini MomsGrooves;

// FUNCTIONS
// DISPLAY FUNCTIONS
unsigned long testText();
unsigned long testFillScreen();
unsigned long testFilledRects(uint16_t color1, uint16_t color2);
unsigned long testRects(uint16_t color);
unsigned long testFastLines(uint16_t color1, uint16_t color2);
unsigned long testLines(uint16_t color);
unsigned long testRoundRects();
unsigned long testFilledTriangles();
unsigned long testTriangles();
unsigned long testFilledCircles(uint8_t radius, uint16_t color);
unsigned long testCircles(uint8_t radius, uint16_t color);
unsigned long testFilledRoundRects();

// LIGHT FUNCTIONS
void updatePixelState(uint32_t color);
void updateHueState(uint32_t color);
//  TIME FUNCTIONS
void updateTimeAndDate();
// REMINDERS FUNCTIONS
void printRemindToPee();
void printRemindFeetUp();
void printRemindMoveAround();
void printMorningBreakfast();
void printLunchBreakReminder();
void printDishes();
void printEveningWindDown();
// OBJECT FUNCTIONS
void onOff();

Timer timeToPee(remindToPeeInterval, printRemindToPee);
Timer feetUp(remindFeetUpInterval, printRemindFeetUp);
Timer moveIt(remindMoveAroundInterval, printRemindMoveAround);
Timer breakfast(morningBreakfastTime, printMorningBreakfast);
Timer lunch(lunchBreakReminderTime, printLunchBreakReminder);
Timer dishes(dishesTime, printDishes);
Timer windDown(eveningWindDownTime, printEveningWindDown);

// MAIN
void setup()
{
  Serial.begin(9600);
  Serial1.begin(115200);

  // initiate display
  tft.begin();
  // initialte wifi
  WiFi.on();

  waitFor(WiFi.ready, 1000);
  // initiate time and date
  Particle.syncTime();
  Time.zone(-7);

  // initiate neopixels
  pixel.begin();
  pixel.setBrightness(15);
  for (i = 0; i <= 7; i++)
  {
    pixel.setPixelColor(i, rainbow[i % 7]);
    pixel.show();
  }
  for ((currentTime - lastUpdateTime) < 5000)
  {
    pixel.clear();
  }

  x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x");
  Serial.println(x, HEX);

  Serial.println(F("Benchmark                Time (microseconds)"));
  delay(10);
  Serial.print(F("Screen fill              "));
  Serial.println(testFillScreen());
  delay(500);

  Serial.print(F("Text                     "));
  Serial.println(testText());
  delay(3000);

  Serial.print(F("Lines                    "));
  Serial.println(testLines(ILI9341_CYAN));
  delay(500);

  Serial.print(F("Horiz/Vert Lines         "));
  Serial.println(testFastLines(ILI9341_RED, ILI9341_BLUE));
  delay(500);

  Serial.print(F("Rectangles (outline)     "));
  Serial.println(testRects(ILI9341_GREEN));
  delay(500);

  Serial.print(F("Rectangles (filled)      "));
  Serial.println(testFilledRects(ILI9341_YELLOW, ILI9341_MAGENTA));
  delay(500);

  Serial.print(F("Circles (filled)         "));
  Serial.println(testFilledCircles(10, ILI9341_MAGENTA));

  Serial.print(F("Circles (outline)        "));
  Serial.println(testCircles(10, ILI9341_WHITE));
  delay(500);

  Serial.print(F("Triangles (outline)      "));
  Serial.println(testTriangles());
  delay(500);

  Serial.print(F("Triangles (filled)       "));
  Serial.println(testFilledTriangles());
  delay(500);

  Serial.print(F("Rounded rects (outline)  "));
  Serial.println(testRoundRects());
  delay(500);

  Serial.print(F("Rounded rects (filled)   "));
  Serial.println(testFilledRoundRects());
  delay(500);
  for ((currentTime - lastUpdateTime) < 5000)
  {
    updateTimeAndDate();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    printRemindToPee();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    updatePixelState(indigo);
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    updateTimeAndDate();
    printRemindFeetUp();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    updatePixelState(indigo);
    updateTimeAndDate();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    printRemindMoveAround();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    updatePixelState(indigo);
    updateTimeAndDate();
  }
  for ((currentTime - lastUpdateTime) < 5000)
  {
    printMorningBreakfast();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    updatePixelState(indigo);
    updateTimeAndDate();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    printLunchBreakReminder();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    updatePixelState(indigo);
    updateTimeAndDate();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    printDishes();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    updatePixelState(indigo);
    updateTimeAndDate();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    printEveningWindDown();
  }
  for ((currentTime - lastUpdateTime) < 10000)
  {
    updatePixelState(indigo);
    updateTimeAndDate();
  }
  //  initiate MP3 player
  MomsGrooves.begin(Serial1);
  MP3OnOff = MomsGrooves.begin(Serial1);
  if (!MP3OnOff)
  {
    Serial.printf("Unable to begin:\n");
    Serial.printf("1. Please recheck the connection\n");
    Serial.printf("2. Please insert the SD card!\n");
    while (true)
      ;
  }
  Serial.printf("DFPlayer Mini Online\n");
  MomsGrooves.volume(25);
}

void loop()
{
  MQTT_connect();
  MQTT_ping();
  // Update pixel state every second
  updatePixelState(tomato);

  // Demo code
  for ((currentTime - lastUpdateTime) < 3000)
  {
    timeToPee.start();
  }
  for ((currentTime - lastUpdateTime) < 3000)
  {
    feetUp.start();
  }
  for ((currentTime - lastUpdateTime) < 3000)
  {
    moveIt.start();
  }
  for ((currentTime - lastUpdateTime) < 3000)
  {
    breakfast.start();
  }
  for ((currentTime - lastUpdateTime) < 3000)
  {
    lunch.start();
  }
  for ((currentTime - lastUpdateTime) < 3000)
  {
    dishes.start();
  }
  for ((currentTime - lastUpdateTime) < 3000)
  {
    windDown.start();
  }

  // Update time every 30 seconds
  if (millis() - lastUpdateTime >= 30000)
  {
    updateTimeAndDate();
    lastUpdateTime = millis();
  }
  if (encoderSwitch.isClicked())
  {
    buttonState = !buttonState;
    Serial.print("on/off");
  }
  onOff();
  timeToPee.reset();
  delay(3000);
  feetUp.reset();
  delay(3000);
  moveIt.reset();
}

// FUNCTION DEFINITIONS
void MQTT_connect()
{
  int8_t ret;

  // Return if already connected.
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

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

// DISPLAY FUNCTIONS
unsigned long testFillScreen()
{
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.fillScreen(ILI9341_RED);
  yield();
  tft.fillScreen(ILI9341_GREEN);
  yield();
  tft.fillScreen(ILI9341_BLUE);
  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  return micros() - start;
}
unsigned long testText()
{
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setRotation(1),
      tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println("Hello Tammylee!");
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
  tft.println("How Are you?");
  tft.setTextSize(2);
  tft.println("Jamie loves you!");
  tft.setTextSize(1);
  tft.println("Time to meet your helping hands pal");
  tft.println("I'm a device Jamie made for you");
  tft.println("I'm here to help you remember!");
  return micros() - start;
}
unsigned long testLines(uint16_t color)
{
  unsigned long start, t;
  int x1, y1, x2, y2,
      w = tft.width(),
      h = tft.height();

  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1 = y1 = 0;
  y2 = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6)
    tft.drawLine(x1, y1, x2, y2, color);
  x2 = w - 1;
  for (y2 = 0; y2 < h; y2 += 6)
    tft.drawLine(x1, y1, x2, y2, color);
  t = micros() - start; // fillScreen doesn't count against timing

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1 = w - 1;
  y1 = 0;
  y2 = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6)
    tft.drawLine(x1, y1, x2, y2, color);
  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6)
    tft.drawLine(x1, y1, x2, y2, color);
  t += micros() - start;

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1 = 0;
  y1 = h - 1;
  y2 = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6)
    tft.drawLine(x1, y1, x2, y2, color);
  x2 = w - 1;
  for (y2 = 0; y2 < h; y2 += 6)
    tft.drawLine(x1, y1, x2, y2, color);
  t += micros() - start;

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1 = w - 1;
  y1 = h - 1;
  y2 = 0;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6)
    tft.drawLine(x1, y1, x2, y2, color);
  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6)
    tft.drawLine(x1, y1, x2, y2, color);

  yield();
  return micros() - start;
}
unsigned long testFastLines(uint16_t color1, uint16_t color2)
{
  unsigned long start;
  int x, y, w = tft.width(), h = tft.height();

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for (y = 0; y < h; y += 5)
    tft.drawFastHLine(0, y, w, color1);
  for (x = 0; x < w; x += 5)
    tft.drawFastVLine(x, 0, h, color2);

  return micros() - start;
}
unsigned long testRects(uint16_t color)
{
  unsigned long start;
  int n, i, i2,
      cx = tft.width() / 2,
      cy = tft.height() / 2;

  tft.fillScreen(ILI9341_BLACK);
  n = min(tft.width(), tft.height());
  start = micros();
  for (i = 2; i < n; i += 6)
  {
    i2 = i / 2;
    tft.drawRect(cx - i2, cy - i2, i, i, color);
  }

  return micros() - start;
}
unsigned long testFilledRects(uint16_t color1, uint16_t color2)
{
  unsigned long start, t = 0;
  int n, i, i2,
      cx = tft.width() / 2 - 1,
      cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  n = min(tft.width(), tft.height());
  for (i = n; i > 0; i -= 6)
  {
    i2 = i / 2;
    start = micros();
    tft.fillRect(cx - i2, cy - i2, i, i, color1);
    t += micros() - start;
    // Outlines are not included in timing results
    tft.drawRect(cx - i2, cy - i2, i, i, color2);
    yield();
  }

  return t;
}
unsigned long testFilledCircles(uint8_t radius, uint16_t color)
{
  unsigned long start;
  int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for (x = radius; x < w; x += r2)
  {
    for (y = radius; y < h; y += r2)
    {
      tft.fillCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}
unsigned long testCircles(uint8_t radius, uint16_t color)
{
  unsigned long start;
  int x, y, r2 = radius * 2,
            w = tft.width() + radius,
            h = tft.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for (x = 0; x < w; x += r2)
  {
    for (y = 0; y < h; y += r2)
    {
      tft.drawCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}
unsigned long testTriangles()
{
  unsigned long start;
  int n, i, cx = tft.width() / 2 - 1,
            cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  n = min(cx, cy);
  start = micros();
  for (i = 0; i < n; i += 5)
  {
    tft.drawTriangle(
        cx, cy - i,     // peak
        cx - i, cy + i, // bottom left
        cx + i, cy + i, // bottom right
        tft.color565(i, i, i));
  }

  return micros() - start;
}
unsigned long testFilledTriangles()
{
  unsigned long start, t = 0;
  int i, cx = tft.width() / 2 - 1,
         cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for (i = min(cx, cy); i > 10; i -= 5)
  {
    start = micros();
    tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.color565(0, i * 10, i * 10));
    t += micros() - start;
    tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.color565(i * 10, i * 10, 0));
    yield();
  }

  return t;
}
unsigned long testRoundRects()
{
  unsigned long start;
  int w, i, i2,
      cx = tft.width() / 2 - 1,
      cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  w = min(tft.width(), tft.height());
  start = micros();
  for (i = 0; i < w; i += 6)
  {
    i2 = i / 2;
    tft.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(i, 0, 0));
  }

  return micros() - start;
}
unsigned long testFilledRoundRects()
{
  unsigned long start;
  int i, i2,
      cx = tft.width() / 2 - 1,
      cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for (i = min(tft.width(), tft.height()); i > 20; i -= 6)
  {
    i2 = i / 2;
    tft.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(0, i, 0));
    yield();
  }

  return micros() - start;
}

// LIGHT FUNCTIONS
void updatePixelState(uint32_t color)
{
  if (currentTime - lastUpdateTime <= 3000)
  {
    for (i = 0; i <= 7; i++)
    {
      pixel.setPixelColor(0, color);

      pixel.show();
    }
  }
}
void updateHueState(uint32_t color)
{
  setHue(HUE, true, color, 30, 255);
}

// TIME FUNCTIONS
void updateTimeAndDate()
{
  bool synced = Particle.syncTime();
  if (!synced)
  {
    Serial.printf("Not synced with Particle Cloud. Attempting sync...\n");
    Particle.syncTime();
    delay(5000); // Wait for sync to complete
  }
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(3);

  snprintf(timeString, sizeof(timeString), "%02d:%02d", Time.hour(), Time.minute());
  snprintf(buffer, sizeof(buffer), " %02d-%02d-%04d", Time.month(), Time.day(), Time.year());

  Serial.printf("Current Time: %s\n", timeString);
  Serial.printf("Current Date: %s\n", buffer);
  tft.println(timeString);
  tft.println(buffer);
}

// REMINDERS FUNCTIONS
void printRemindToPee()
{
  updatePixelState(yellow);
  updateHueState(yellow);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(45, 20);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(3);
  tft.println("Time to try to use \nthe restroom!");
  tft.println(timeString);
  tft.println(buffer);

  reminders.publish("Reminder to Pee");
  colors.publish(yellow);
  Serial.printf("Publishing %s \n", "Reminder to Pee");
}
void printRemindFeetUp()
{
  updatePixelState(green);
  updateHueState(green);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(45, 20);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(3);
  tft.println("Are your feet up?");
  tft.println(timeString);
  tft.println(buffer);
  reminders.publish("Are your feet up?");
  colors.publish(green);
  Serial.printf("Publishing %s \n", "Are your feet up?");
}
void printRemindMoveAround()
{
  updatePixelState(orange);
  updateHueState(orange);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(45, 20);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setTextSize(3);
  tft.println("Lets get up \nand move around!");
  tft.println(timeString);
  tft.println(buffer);
  reminders.publish("Time to move around");
  colors.publish(orange);
  Serial.printf("Publishing %s \n", "Time to move around");
}
void printMorningBreakfast()
{
  updatePixelState(cyan);
  updateHueState(cyan);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(45, 20);
  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(3);
  tft.println("Good morning Mama\nTime eat some oatemeal!!");
  tft.println(timeString);
  tft.println(buffer);
  reminders.publish("breakfast time");
  colors.publish(cyan);
  Serial.printf("Publishing %s \n", "breakfast time");
}
void printLunchBreakReminder()
{
  updatePixelState(red);
  updateHueState(red);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(45, 20);
  tft.setTextColor(ILI9341_PINK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_PINK);
  tft.setTextSize(3);
  tft.println("Lunch time!");
  tft.println(timeString);
  tft.println(buffer);
  reminders.publish("Lunch!");
  colors.publish(red);
  Serial.printf("Publishing %s \n", "Lunch!");
}
void printDishes()
{
  updatePixelState(cyan);
  updateHueState(cyan);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(45, 20);
  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(3);
  tft.println("Lets help out\na little and\n load the diswasher\n keep cups paired with lids!");
  tft.println(timeString);
  tft.println(buffer);
  reminders.publish("Dishes");
  colors.publish(cyan);
  Serial.printf("Publishing %s \n", "Dishes");
}
void printEveningWindDown()
{
  updatePixelState(violet);
  updateHueState(violet);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(45, 20);
  tft.setTextColor(ILI9341_PURPLE);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_PURPLE);
  tft.setTextSize(3);
  tft.println("Time to WIND DOWN");
  tft.println(timeString);
  tft.println(buffer);
  reminders.publish("Wind down");
  colors.publish(violet);
  Serial.printf("Publishing %s \n", "Wind down");
}

// OBJECT FUNCTIONS
void onOff()
{
  if (buttonState == TRUE)
  {
    wemoWrite(WEMO, LOW);
    Serial.print("wemo off");
  }
  else
  {
    wemoWrite(WEMO, HIGH);
    Serial.print("wemo on");
  }
}