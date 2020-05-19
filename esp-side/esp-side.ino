#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

//#include "index.h"

ESP8266WebServer server;
//char* ssid = "NETorare";
//char* password = "00GodIsDead00!";
char* ssid = "PedoPlanet";
char* password = "hootfoot1";

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

const int room_light_one = D0;
const int room_light_two = D1;
String room_light_one_state = "on";
String room_light_two_state = "on";

double room_humid = 0, room_temp = 0;

const int room_ac = D4;
String room_ac_state = "on";

  
void setup()
{
  WiFi.begin(ssid,password);
  Serial.begin(9600);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleIndex);
  server.on("/room/light_1", handleRoomLightOne);
  server.on("/room/light_2", handleRoomLightTwo);
  server.on("/room/light_all", handleRoomLightAll);
  server.on("/room/update_temp", handleRoomUpdateTemp);
  server.on("/room/ac", handleRoomAC);
  server.begin();

  pinMode(room_light_one, OUTPUT);
  pinMode(room_light_two, OUTPUT);
  pinMode(room_ac, OUTPUT);
}

void loop()
{
  server.handleClient();
}

void handleIndex() {
  // Prepare the data for serving it over HTTP
  String output = "<!DOCTYPE html><html>" + 
    String("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">") +
      "<link rel=\"icon\" href=\"data:,\">" +
      "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}" +
      ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;" +
      "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}" +
      ".button2 {background-color: #77878A;}</style></head>" +
    "<body><h1>ESP8266 Web Server</h1>" +
      "<h3>Network: " + ssid + "</h3>" + 
      "<p>Humidity: " + String(room_humid) + " temperature: " + String(room_temp) + "</p>";

      output += "<p><a href=\"/room/update_temp\"><button class=\"button\">UPDATE</button></a></p>";

      output += "<p>Both Lights</p>";
      if (room_light_one_state == "off" && room_light_two_state == "off") 
      output += "<p><a href=\"/room/light_all?state=on\"><button class=\"button\">ON</button></a></p>";
      else
      output += "<p><a href=\"/room/light_all?state=off\"><button class=\"button button2\">OFF</button></a></p>";
      
      output += "<p>Light 1 - " + String(room_light_one_state) + "</p>";
      if (room_light_one_state == "off") 
      output += "<p><a href=\"/room/light_1?state=on\"><button class=\"button\">ON</button></a></p>";
      else
      output += "<p><a href=\"/room/light_1?state=off\"><button class=\"button button2\">OFF</button></a></p>";

      output += "<p>Light 2 - " + String(room_light_two_state) + "</p>";
      if (room_light_two_state == "off") 
      output += "<p><a href=\"/room/light_2?state=on\"><button class=\"button\">ON</button></a></p>";
      else
      output += "<p><a href=\"/room/light_2?state=off\"><button class=\"button button2\">OFF</button></a></p>";

      output += "<p>AC - " + String(room_ac_state) + "</p>";
      if (room_ac_state == "on") 
      output += "<p><a href=\"/room/ac?state=off\"><button class=\"button button2\">OFF</button></a></p>";
      else
      output += "<p><a href=\"/room/ac?state=on\"><button class=\"button\">ON</button></a></p>";
      
    output += "</body></html>";
      
//  String output = MAIN_page;
    
  // Serve the data as plain text, for example
  server.send(200, "text/html", output);
}

void handleRoomLightOne() {
  String state=server.arg("state");
  room_light_one_state = state;
  if (state == "on") digitalWrite(room_light_one, LOW);
  else if (state == "off") digitalWrite(room_light_one, HIGH);
  handleIndex();
}

void handleRoomLightTwo() {
  String state=server.arg("state");
  room_light_two_state = state;
  if (state == "on") digitalWrite(room_light_two, LOW);
  else if (state == "off") digitalWrite(room_light_two, HIGH);
  handleIndex();
}

void handleRoomLightAll() {
  String state=server.arg("state");
  room_light_one_state = state;
  room_light_two_state = state;
  if (state == "on") {
    digitalWrite(room_light_one, LOW);
    digitalWrite(room_light_two, LOW);
  }
  else if (state == "off") {
    digitalWrite(room_light_one, HIGH);
    digitalWrite(room_light_two, HIGH);
  }
  handleIndex();
}

void handleRoomAC() {
  String state=server.arg("state");
  room_ac_state = state;
  if (state == "on") digitalWrite(room_ac, LOW);
  else if (state == "off") digitalWrite(room_ac, HIGH);
  handleIndex();
}

void handleRoomUpdateTemp() {
  // Send a JSON-formatted request with key "type" and value "request"
  // then parse the JSON-formatted response with keys "gas" and "distance"
  DynamicJsonDocument doc(1024);
  
  // Sending the request
  doc["type"] = "request";
  serializeJson(doc, Serial);
  
  // Reading the response
  boolean messageReady = false;
  String message = "";
  while(messageReady == false) { // blocking but that's ok
    if(Serial.available()) {
      message = Serial.readString();
      messageReady = true;
    }
  }
  
  // Attempt to deserialize the JSON-formatted message
  DeserializationError error = deserializeJson(doc, message);
  Serial.println(message);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    String output = "Error: " + String(error.c_str());
    server.send(200, "text/plain", output);
  }
  room_humid = doc["humid"];
  room_temp = doc["temp"];

  handleIndex();
}
