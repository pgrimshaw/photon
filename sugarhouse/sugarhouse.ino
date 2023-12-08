// This #include statement was automatically added by the Particle IDE.
#include "OneWire.h"

// This #include statement was automatically added by the Particle IDE.
#include "spark-dallas-temperature.h"

// This #include statement was automatically added by the Particle IDE.
#include <HC_SR04.h>

int WAIT_PERIOD = 1000;
int PUB_PERIOD  = 300000;

double MIN_TEMPC = -30.0;
double MAX_TEMPC = 50.0;

double tempC = 0.0;
double tempF = 32.0;

double MIN_CM = 5.0;
double MAX_CM = 200.0;

double cm = 0.0;
double cm2 = 0.0;

int sendPin = A0;
int recvPin = D1;

int sendPin2 = A1;
int recvPin2 = D2;

int tempSensorPin = D6;
int ledPin = D0;

unsigned long pubTime;

boolean blinking = true;
boolean initializing = true;

OneWire oneWire(tempSensorPin);
DallasTemperature sensors(&oneWire);

HC_SR04 rangeFinder = HC_SR04(sendPin, recvPin, MIN_CM, MAX_CM);
HC_SR04 rangeFinder2 = HC_SR04(sendPin2, recvPin2, MIN_CM, MAX_CM);

void setup() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    sensors.begin();
    Particle.variable("tempc", &tempC, DOUBLE);
    Particle.variable("tempf", &tempF, DOUBLE);
    Particle.variable("cm", &cm, DOUBLE);
    Particle.variable("cm2", &cm2, DOUBLE);

    Particle.function("led", ledControl);

    // Subscribe to the integration response event
    Particle.subscribe("hook-response/sugardata", hookResponseHandler, MY_DEVICES);
    //Particle.subscribe("hook-response/temperature", hookResponseHandler, MY_DEVICES);
    //Particle.subscribe("hook-response/distance", hookResponseHandler, MY_DEVICES);
    //Particle.subscribe("hook-response/dist2", hookResponseHandler, MY_DEVICES);

    pubTime = millis();
}

void loop() {
    if (blinking) {
        digitalWrite(ledPin, HIGH);
    }
    sensors.requestTemperatures();
    double temp = sensors.getTempCByIndex(0);
    bool tempValid = ((temp >= MIN_TEMPC) && (temp <= MAX_TEMPC));
    if(tempValid) {
        tempC = initializing ? temp : smooth(temp, tempC);
        tempF = tempC * 9.0 / 5.0 + 32.0;
    }

    double gallon = 0;
    double rfDist = rangeFinder.getDistanceCM();
    double dist =  rfDist * tempCorrection(tempC);
    bool distValid = ((dist >= MIN_CM) && (dist <= MAX_CM));
    if(distValid) {
        cm = initializing ? dist : smooth(dist, cm);
        gallon = (89 - cm) * 5.0 / 8.0;
    }
    double gallon2 = 0;
    double rfDist2 = rangeFinder2.getDistanceCM();
    double dist2 =  rfDist2 * tempCorrection(tempC);
    bool dist2Valid = ((dist2 >= MIN_CM) && (dist2 <= MAX_CM));
    if(dist2Valid) {
        cm2 = initializing ? dist2 : smooth(dist2, cm2);
        gallon2 = (89 - cm2) * 5.0 / 8.0;
    }

    initializing = false;
    digitalWrite(ledPin, LOW);

    if(millis() >= pubTime) {
        // Publish sensor data in JSON format
        String key = "I83WBSJA8GE7QWN0";
        String sdata = "{ \"k\": \"" + key + "\"";
        if(tempValid) {
            sdata += ",\"1\": \"" + String(tempF) + "\"";
            //Particle.publish("temperature", String(tempF), PRIVATE);
        }
        if(distValid) {
            sdata += ",\"2\": \"" + String(cm) + "\"";
            sdata += ",\"4\": \"" + String(gallon) + "\"";
            //Particle.publish("distance", String(cm), PRIVATE);
        }
        if(dist2Valid) {
            sdata += ",\"3\": \"" + String(cm2) + "\"";
            sdata += ",\"5\": \"" + String(gallon2) + "\"";
            //Particle.publish("dist2", String(cm2), PRIVATE);
        }
        sdata += " }";
        Particle.publish("sugardata", sdata, PRIVATE);
        pubTime += PUB_PERIOD;
    }
    delay(WAIT_PERIOD);
}

double tempCorrection(double degC) {
    // temperature correction factor for speed of sound in air, relative to 20 C
    double correction = 1 + (0.606 * (degC - 20) / 343.42);
    return correction;
}

double smooth(double newValue, double prevValue) {
    return (0.2 * newValue) + (0.8 * prevValue);
}

int ledControl(String command) {
    if (command =="on") {
        blinking = true;
        return 1;
    }
    else if (command=="off") {
        blinking = false;
        return 0;
    }
    else {
        return -1;
    }
}

void hookResponseHandler(const char *event, const char *data) {
  // Handle the integration response
}

// https://api.spark.io/v1/devices/54ff72066672524860351167/tempc?access_token=cb8b348000e9d0ea9e354990bbd39ccbfb57b30e
