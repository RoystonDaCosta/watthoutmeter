#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1015.h>

const char* ssid     = "xxxxx";
const char* password = "xxxxx";

#define OLED_RESET 12
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

Adafruit_ADS1115 ads;

unsigned long lastMicros = 0, lastMillis = 0;
double wattHourCumulative = 0, wattHourIncremental = 0;
float v, i, multiplier;

const int numReadings = 10;
double readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
double total = 0;                  // the running total
double average = 0;                // the average

void getCurrentAmps();
void getVoltageVolts();
double getWattHourIncremental();
unsigned long elapsedTimeMicros();

void setup() {
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  Serial.begin(115200);
  Wire.begin(D1, D2);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.dim(false);
  display.clearDisplay();
  display.display();

  ads.begin();
  ads.setGain(GAIN_SIXTEEN); // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  multiplier = ads.voltsPerBit() * 1000.0F;

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Connecting to ");
  display.print(ssid);

  display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    display.display();
  }

  display.println("");
  display.println("WiFi connected");
  display.println("IP address: ");
  display.println(WiFi.localIP());
  display.display();
  delay(1000);
}

void loop() {

  double wattHourIncremental = getWattHourIncremental();
  wattHourCumulative += wattHourIncremental;

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  getCurrentAmps();
  getVoltageVolts();
  int mV = v * 1000;
  display.print("V  ");
  if (mV < 10000)
  {
    display.print(" ");
  }
  display.println((float)mV / 1000, 3);
  display.print("A   ");
  display.println(i, 3);
  display.print("W  ");
  if ((v * i) < 10.0)
  {
    display.print(" ");
  }
  display.println(v * i, 3);
  display.print("mWh ");
  display.println(wattHourCumulative, 0);
  display.display();
  //delay(50);
}

void getCurrentAmps()
{
  i = (double)ads.readADC_Differential_2_3();
  //Calibraiton formulae i=;
  total = total - readings[readIndex];
  readings[readIndex] = i;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  average = total / numReadings;
  i = average;
}

void getVoltageVolts()
{
  v = (double)ads.readADC_SingleEnded(0);
  //Calibraiton formulae v=;
}

double getWattHourIncremental()
{
  return v * i * elapsedTimeMicros() / 3600000.00;
}

unsigned long elapsedTimeMicros()
{
  unsigned long tempmicros = lastMicros;
  lastMicros = micros();
  return lastMicros - tempmicros;
}


