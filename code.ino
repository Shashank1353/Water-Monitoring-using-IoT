#include <WiFi.h>
#include <Wire.h>
#include "ThingSpeak.h"

#define UPDATE_INTERVAL 60000 // Interval for posting to ThingSpeak in milliseconds

const char * myWriteAPIKey = "XXXXX";
String apiKey = "XXXXX"; // Enter your Write API key from ThingSpeak
const char *ssid = "XXXXX";     // replace with your wifi ssid and wpa2 key
const char *pass = "XXXXX";
const char* server = "api.thingspeak.com";

#define LED_BUILTIN 2 // Use GPIO 2 for LED on ESP32
#define SENSOR  13

long currentMillis = 0;
long previousMillis = 0;
long previousPostMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 2.25;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;

float cost_pmL= 0.007;              // assumed cost of water per litre

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

WiFiClient client;
unsigned long myChannelNumber = 1;
void setup() {
  Serial.begin(115200);

  delay(10);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  digitalWrite(26, HIGH);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
}

void loop() {
  currentMillis = millis();

  // Update flow rate and total volume every second
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);

    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;

    float sensor_val= totalMilliLitres;
    float total_val= totalLitres;
    float bill_cost= total_val*cost_pmL;

    int x = ThingSpeak.writeField(myChannelNumber,1,flowRate,myWriteAPIKey);          // to send the data to the thingspeak platform
    int y = ThingSpeak.writeField(myChannelNumber,2,total_val,myWriteAPIKey);
    int z = ThingSpeak.writeField(myChannelNumber,3,bill_cost,myWriteAPIKey);

// To display the flow rate, total amount, bill cost in Serial Monitor

    Serial.print("Flow rate: ");                      // show flow rate
    Serial.print(float(flowRate));
    Serial.print("L/min\t");

    Serial.print("Output Liquid Quantity: ");         // show total amount
    Serial.print(totalMilliLitres);
    Serial.print("mL / \t");
    Serial.print(totalLitres);

    Serial.print("bill_cost rate: ");                 // show the bill amount
    Serial.print(float(bill_cost));
    Serial.print("Rs/L \t");

    Serial.print("\n");

}
}
