#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define DHTPIN D3
#define DHTTYPE DHT22

int dustPin = A0; // dust sensor
int ledPin = D5;
int d = 0;

float voltsMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

DHT dht(DHTPIN, DHTTYPE);

void setup()
{

    Serial.begin(115200); //Serial connection
    Serial.println("Device Started");
    Serial.println("-------------------------------------");
    Serial.println("Running DHT!");
    Serial.println("-------------------------------------");
    pinMode(ledPin, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
    WiFi.begin("xx", "xxd"); //WiFi connection

    while (WiFi.status() != WL_CONNECTED)
    { //Wait for the WiFI connection completion
        digitalWrite(D6, HIGH);
        digitalWrite(D7, LOW);
        delay(2000);
        Serial.println("Waiting for connection");
    }
    digitalWrite(D6, LOW);
    digitalWrite(D7, HIGH);
    Serial.println("Wifi Connected");
}

void loop()
{
    digitalWrite(ledPin, LOW); // power on the LED
    delayMicroseconds(280);

    voltsMeasured = analogRead(dustPin); // read the dust value

    delayMicroseconds(40);
    digitalWrite(ledPin, HIGH); // turn the LED off
    delayMicroseconds(9680);
    calcVoltage = voltsMeasured * (3.3 / 1024.0);
    dustDensity = 0.17 * calcVoltage - 0.1;
    Serial.println("GP2Y1010AU0F readings");
    Serial.print("Raw Signal Value = ");
    Serial.println(voltsMeasured);
    Serial.print("Voltage = ");
    Serial.println(calcVoltage);
    Serial.print("Dust Density = ");
    Serial.println(dustDensity); // mg/m3
    Serial.println("");
    if (voltsMeasured < 400)
    {
        d = 1;
    }
    else
    {
        d = 0;
    }
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
        Serial.println("Failed to read from DHT sensor!");
        h = 0; //in a future update, we'll get this from yahoo api
        t = 0;
        return;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        const size_t bufferSize = JSON_OBJECT_SIZE(4);
        DynamicJsonBuffer jsonBuffer(bufferSize);

        JsonObject &root = jsonBuffer.createObject();
        root["kit"] = 1;
        root["tmprtr"] = t;
        root["hmdt"] = h;
        root["dst"] = d;

        //root.printTo(Serial);
        char JSONmessageBuffer[200];
        root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        Serial.println(JSONmessageBuffer);

        HTTPClient http;

        http.begin("xxx"); //Specify request destination
        http.addHeader("Content-Type", "application/json");     //Specify content-type header
        http.addHeader("Accept", "application/json");           //Specify Accept header, from postman fuck

        int httpCode = http.POST(JSONmessageBuffer); //Send the request
        String payload = http.getString();           //Get the response payload

        Serial.println(httpCode); //Print HTTP return code
        Serial.println(payload);  //Print request response payload

        http.end();
    }
    else
    {

        Serial.println("Error in WiFi connection");
    }

    delay(15000);
}
