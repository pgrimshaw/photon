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

int sendPin = A0;
int recvPin = D1;

int tempSensorPin = D6;
int ledPin = D0;

unsigned long pubTime;

boolean blinking = true;
boolean initializing = true;

OneWire oneWire(tempSensorPin);
DallasTemperature sensors(&oneWire);

HC_SR04 rangeFinder = HC_SR04(sendPin, recvPin, MIN_CM, MAX_CM);

void setup() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    
    sensors.begin();
    Spark.variable("tempc", &tempC, DOUBLE);
    Spark.variable("tempf", &tempF, DOUBLE);
    Spark.variable("cm", &cm, DOUBLE);

    Particle.function("led", ledControl);

    // Subscribe to the integration response event
    Particle.subscribe("hook-response/temperature", hookResponseHandler, MY_DEVICES);
    Particle.subscribe("hook-response/distance", hookResponseHandler, MY_DEVICES);
    
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
    
    double rfDist = rangeFinder.getDistanceCM();
    double dist =  rfDist * tempCorrection(tempC);
    bool distValid = ((dist >= MIN_CM) && (dist <= MAX_CM));
    if(distValid) {
        cm = initializing ? dist : smooth(dist, cm);
    }

    initializing = false;
    digitalWrite(ledPin, LOW);

    if(millis() >= pubTime) {
        if(tempValid) {
            Spark.publish("temperature", String(tempF), PRIVATE);
        }
        if(distValid) {
            Spark.publish("distance", String(cm), PRIVATE);
        }
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
