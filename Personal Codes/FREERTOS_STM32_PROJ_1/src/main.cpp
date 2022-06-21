#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>

static xSemaphoreHandle my_mutex;

#define DHTPIN A0     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define PIN D7

DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, PIN, NEO_GRB + NEO_KHZ800);

float t;
float h;
float t_norm = 0.0;
float h_norm = 0.0;

void Send_Color_Neo(uint8_t _LED, uint8_t _red, uint8_t _green, uint8_t _blue)
{
  strip.setPixelColor(_LED, strip.Color(_red, _green, _blue));
  strip.show();
}

void Blink(void *pvParam)
{
  while (1)
  {
    digitalWrite(LED_GREEN, HIGH);
    Send_Color_Neo(0, uint8_t(255 * (t_norm)), uint8_t(255 * (1.0 - t_norm)), 0);
    Send_Color_Neo(1, uint8_t(255 * (1.0 - h_norm)), 0, uint8_t(255 * (h_norm)));
    vTaskDelay(800 / portTICK_PERIOD_MS);
    digitalWrite(LED_GREEN, LOW);
    // Send_Color_Neo(0, 0, 0, 0);
    // Send_Color_Neo(1, 0, 0, 0);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void Temp(void *pvParam)
{
  while (1)
  {
    while (my_mutex == NULL)
      ;
    if (xSemaphoreTake(my_mutex, (TickType_t)0) == pdTRUE)
    {
      t = dht.readTemperature();
      h = dht.readHumidity();
      t_norm = ((t / 50.0) >= 1.0f) ? (1.0) : (t / 50.0);
      h_norm = ((h / 100.0) >= 1.0f) ? (1.0) : (h / 100.0);
      if (isnan(h) || isnan(t))
      {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }
      xSemaphoreGive(my_mutex);
    }
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print("°C\tHumidity: ");
    Serial.print(h);
    Serial.print("%\n");
    // Serial.printf("Temperature: %0.2f\n", t);
    // Serial.printf("Temperature: %d\n", analogRead(A0));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void intial_task(void)
{
  Serial.begin(115200);
  pinMode(DHTPIN, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  dht.begin();
  strip.begin();
  strip.setBrightness(255);
  strip.show();
  Serial.println();
  Serial.println("STM32L070RB");
  Serial.printf("Chip Version: %d.%d.%d\n", STM32_CORE_VERSION_MAJOR, STM32_CORE_VERSION_MINOR, STM32_CORE_VERSION_PATCH);
}

void setup()
{
  // put your setup code here, to run once:
  intial_task();
  my_mutex = xSemaphoreCreateMutex();
  xTaskCreate(&Blink, "Blink", 1024, NULL, 1, NULL);
  xTaskCreate(&Temp, "Temp", 1024, NULL, 2, NULL);
  vTaskStartScheduler();
}

void loop()
{
  // put your main code here, to run repeatedly:
  // vTaskDelete(NULL);
}