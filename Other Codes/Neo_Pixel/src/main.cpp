#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_COUNT 5
#define LED_PIN 15
#define Delay_Time 10 * 1000

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void red(void)
{
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.setPixelColor(1, strip.Color(0, 0, 0));
  strip.setPixelColor(2, strip.Color(0, 0, 0));
  strip.setPixelColor(3, strip.Color(0, 0, 0));
  strip.show();
}

void single_yellow(void)
{
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.setPixelColor(1, strip.Color(255, 75, 0));
  strip.setPixelColor(2, strip.Color(0, 0, 0));
  strip.setPixelColor(3, strip.Color(0, 0, 0));
  strip.show();
}

void double_yellow(void)
{
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.setPixelColor(1, strip.Color(255, 75, 0));
  strip.setPixelColor(2, strip.Color(0, 0, 0));
  strip.setPixelColor(3, strip.Color(255, 75, 0));
  strip.show();
}

void green(void)
{
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.setPixelColor(1, strip.Color(0, 0, 0));
  strip.setPixelColor(2, strip.Color(0, 255, 0));
  strip.setPixelColor(3, strip.Color(0, 0, 0));
  strip.show();
}

void setup()
{
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.print("Setup is running on core ");
  Serial.println(xPortGetCoreID());
  Serial.print(" With Priority ");
  Serial.print(uxTaskPriorityGet(NULL));
  Serial.println("Welcome to NeoPixel!");
  strip.begin();
  Serial.println("Setting Brightness!");
  strip.setBrightness(255);
  Serial.println("Starting NeoPixel!");
}

void loop()
{
  red();
  delay(Delay_Time);
  single_yellow();
  delay(Delay_Time);
  double_yellow();
  delay(Delay_Time);
  green();
  delay(Delay_Time);
}
