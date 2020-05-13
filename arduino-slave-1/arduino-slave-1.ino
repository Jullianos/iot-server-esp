#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
// Temperature sensor on pin 7
OneWire oneWire(7);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

float temperature = 0;
bool hot = false;
int delayTime = 200;
int desiredTemp = 23;
volatile bool receivedone;
bool ACState = false;
bool flag = true, flagTwo = true, firstStart = true;

void setup(void) {
  // start serial port
  Serial.begin(115200);
  Serial.println("Dallas Temperature IC Control Library Demo");
  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  // AC on pin 8
  pinMode(8, OUTPUT);
  // Turn AC Off
  digitalWrite(8, LOW);

  // ESP to Arduino
  SPCR |= bit(SPE);
  pinMode(MISO, OUTPUT);
  receivedone = false;
  SPI.attachInterrupt();
}

void loop(void) { 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.println("DONE");
  
  temperature = sensors.getTempCByIndex(0);

  // Print the Temperature in console
  Serial.print("Temperature is: ");
  Serial.print(temperature);
  Serial.print(" Desired temp is: ");
  Serial.print(desiredTemp);

  // If temperature is more than desired temperature
  if (temperature > desiredTemp) {
    
    flagTwo = true;
    // It is hot
    hot = true;

    if (flag) {
      desiredTemp -= 2;
      flag = false;
    }

    // if its first time reset desired temperature
    if (firstStart) {
      desiredTemp += 2;
      firstStart = false;
    }
    
  }
  // If temperature is less than desired temperature
  else {
    flag = true;
    // It is not hot
    hot = false;

    
    if (flagTwo) {
      desiredTemp += 2;
      flagTwo = false;
    }

    // if its first time reset desired temperature
    if (firstStart) {
      desiredTemp -= 2;
      firstStart = false;
    }
    
  }
  
  // If AC is On from Server
  if (ACState) {
    // Turn AC On
    
    Serial.println(" Manual: On");
    digitalWrite(8, HIGH);
    delay(200);
  }
  // If AC is Off from Server
  else {
    // If its hot
    if (hot) {
      // Turn AC On
      
      digitalWrite(8, HIGH);
      Serial.println(" Automatic: On");
      delay(200);
    }
    // If its not hot
    else {
      //Turn AC Off
      digitalWrite(8, LOW);
      Serial.println(" Automatic: Off");
      delay(200);
    }
  }
  
  // If recieved something set the ACState acordingly
  if (receivedone) {
    if (ACState == 1) {
      Serial.print("ACState got True!");
    }
    else {
      Serial.print("ACState got False!");
    }
    Serial.println(ACState);
    receivedone = false;
  }
  
  
}

ISR (SPI_STC_vect) {
  uint8_t oldsrg = SREG;
  cli();
  ACState = SPDR;
  
  receivedone = true;
  
  SREG = oldsrg;
}
