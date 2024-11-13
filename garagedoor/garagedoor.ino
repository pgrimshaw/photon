// Include Particle Device OS APIs
#include "Particle.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

int UPDATE_PERIOD   = 1000;

int doorSensorPin   = D0;
int doorOperatorPin = D4;
int doorLightPin    = D5;

unsigned long updateTime;

String doorState = "UNNOWN";

boolean initializing = true;
boolean operating = false;

// setup() runs once, when the device is first turned on
void setup() {
  // Put initialization like pinMode and begin functions here
  pinMode(doorSensorPin, INPUT_PULLUP);
  pinMode(doorOperatorPin, OUTPUT);
  pinMode(doorLightPin, OUTPUT);
  digitalWrite(doorOperatorPin, LOW);
  digitalWrite(doorLightPin, LOW);
  
  Particle.variable("doorState", &doorState, STRING);
  Particle.function("doorOperator", doorControl);
  
  // Subscribe to the integration response event
  //Particle.subscribe("hook-response/doorSensor", hookResponseHandler, MY_DEVICES);
  
  initializing = false;
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.

  // Example: Publish event to cloud every 10 seconds. Uncomment the next 3 lines to try it!
  // Log.info("Sending Hello World to the cloud!");
  // Particle.publish("Hello world!");
  // delay( 10 * 1000 ); // milliseconds and blocking - see docs for more info!
  
  bool doorIsClosed = digitalRead(doorSensorPin) == 0;
  doorState = doorIsClosed ? "CLOSED" : "NOT CLOSED";
  
  //Particle.publish("doorSensor", doorState, PRIVATE);
  
  delay(UPDATE_PERIOD);
}

int doorControl(String command) {
    
    if(operating) {
        return -1;
    }
    
    if (command == "1") {
        operating = true;
        digitalWrite(doorOperatorPin, 1);
        delay(200);
        digitalWrite(doorOperatorPin, 0);
        operating = false;
        return 1;
    }
    else if (command == "2") {
        digitalWrite(doorLightPin, 1);
        delay(200);
        digitalWrite(doorLightPin, 0);
        return 1;
    }
    else {
        return -1;
    }
}

void hookResponseHandler(const char *event, const char *data) {
  // Handle the integration response
}

