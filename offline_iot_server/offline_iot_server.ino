#include "DHT.h"

#define DHTPIN 2     // DHT sensor pin
#define DHTPIN_OUT 7     // DHT sensor pin
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE); // Init DHT temp and humid sensor
DHT dht_out(DHTPIN_OUT, DHTTYPE); // Init DHT temp and humid sensor

// Digital pins
const int light_one_pin = 3;
const int light_two_pin = 4;
const int led_strip_pin = 5;
const int ac_pin = 6;

// Vars
float target = 24.0;
float humid = 0;
float temp = 0;
float humid_out = 0;
float temp_out = 0;
int incomingByte = 0;
bool lights_on = false;
bool led_on = false;
bool ac_on = false;
String ac = "off";

long ac_active_cooldown = 0;
long ac_cooldown = 0;
bool ac_active = false;

// 36000 ~ 1min
long ac_active_time = 180000;  // ~ 5min
long ac_sleep_time = 180000;   // ~ 5min


void setup() {
  Serial.begin(9600);
  dht.begin();      // Start DHT sensor
  dht_out.begin();  // Start DHT sensor

  pinMode(light_one_pin, OUTPUT);
  pinMode(light_two_pin, OUTPUT);
  pinMode(led_strip_pin, OUTPUT);
  pinMode(ac_pin, OUTPUT);


  // Set the default values to the pins
  if (lights_on) {
    digitalWrite(light_one_pin, LOW);
    digitalWrite(light_two_pin, LOW);
  } else {
    digitalWrite(light_one_pin, HIGH);
    digitalWrite(light_two_pin, HIGH);
  }
  if (led_on) digitalWrite(led_strip_pin, HIGH);
  else digitalWrite(led_strip_pin, LOW);
  if (ac_on) digitalWrite(ac_pin, HIGH);
  else digitalWrite(ac_pin, LOW);
}

void loop() {
  humid = dht.readHumidity();
  temp = dht.readTemperature();

  humid_out = dht_out.readHumidity();
  temp_out = dht_out.readTemperature();

  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    if (incomingByte == 49) {                         // #1 for Lights
      lights_on = !lights_on;
      if (lights_on) {
        digitalWrite(light_one_pin, LOW);
        digitalWrite(light_two_pin, LOW);
      } else {
        digitalWrite(light_one_pin, HIGH);
        digitalWrite(light_two_pin, HIGH);
      }
    } else if (incomingByte == 50) {                  // #2 for Led
      led_on = !led_on;
      if (led_on) digitalWrite(led_strip_pin, LOW);
      else digitalWrite(led_strip_pin, HIGH);
    } else if (incomingByte == 51) {                  // #3 for AC
      ac_on = !ac_on;
    } else if (incomingByte == 54) {                  // #6 for decreasing target temp
      target -= 0.5;
    } else if (incomingByte == 57) {                  // #9 for increasing target temp
      target += 0.5;
    }
  }

  if (ac_on) {
    if (ac_active) {
      ac_active_cooldown = ac_active_cooldown - 100;
      if (ac_active_cooldown <= 0) {
        ac_active_cooldown = 0;
        ac_active = false;
        ac_cooldown = ac_sleep_time;
        digitalWrite(ac_pin, LOW);
      }
    } else if (ac_cooldown > 0) {
      ac_cooldown -= 100;
    } else if (target < temp && !ac_active) {
      digitalWrite(ac_pin, HIGH);
      ac_active_cooldown = ac_active_time;
      ac_active = true;
    }
  } else {
    digitalWrite(ac_pin, LOW);
    ac_active = false;
    ac_cooldown = 0;
    ac_active_cooldown = 0;
  }
  
  if (ac_on) ac = "on";
  else ac = "off";
  Serial.print("T: " + String(target) + " | ");
  Serial.print(String(temp) + "C | " + String(humid) + "% | Out: " + String(temp_out) + "C | " + String(humid_out) + "% | AC: " + ac);
  Serial.println(" | CD: " + String(ac_cooldown) + " ACD: " + String(ac_active_cooldown));
  delay(100);
}
