/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load libraries
#include <ESP8266WiFi.h>
#include <SPI.h>

// Username and Password for the WiFi

const char* ssid     = "PedoPlanet";
const char* password = "hootfoot1";

//const char* ssid     = "Tenda_161730";
//const char* password = "00GodIsDead00!";

// Set web server port number to 80
WiFiServer server(80);

// Global Variables
String header;
bool ACState = false;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
// Lights Pins 
const int output5 = 0; // Light 1
const int output4 = 1; // Light 2


void setup() {
  Serial.begin(9600);
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output5, HIGH);
  digitalWrite(output4, HIGH);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  
  // Start SPI Esp to arduino
  SPI.begin();
}


void loop(){
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output5State = "on";
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            } else if (header.indexOf("GET /both/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /both/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
              Serial.println("GPIO 5 on");
              output5State = "on";
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /ac/manual") >= 0) {
              // send AC stat to Arduino
              ACState = true;
              SPI.transfer(ACState);
            } else if (header.indexOf("GET /ac/automatic") >= 0) {
              // send AC stat to Arduino
              ACState = false;
              SPI.transfer(ACState);
            }
            

            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            // CSS to style the on/off buttons 
            client.println("<style>html { font-family: courier new; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>GOD IS DEAD</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>Light 1</p>");
            
            // If the output5State is off, it displays the ON button       
            if (output5State=="off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">Turn Off</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">Turn On</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>Light 2</p>");
            
            // If the output4State is off, it displays the ON button       
            if (output4State=="off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">Turn Off</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">Turn On</button></a></p>");
            }
            
            // If both are on or off display Both On/Off button
            if (output4State=="off" && output5State=="off") {
              client.println("<br><br><p><a href=\"/both/on\"><button class=\"button\">Turn both Off</button></a></p>");
            } else if (output4State=="on" && output5State=="on") {
              client.println("<br><br><p><a href=\"/both/off\"><button class=\"button button2\">Turn both On</button></a></p>");
            }
            
            client.println("<p>AC</p>");
            
            // If the AC manual is on
            if (ACState) {
              client.println("<p><a href=\"/ac/automatic\"><button class=\"button\">Go Automatic</button></a></p>");
            }
            // If the AC manual is off
            else {
              client.println("<p><a href=\"/ac/manual\"><button class=\"button button2\">Go Manual</button></a></p>");
            }
            

            client.println("<p>Temperature in the room is: </p>");
            client.println("<input id=\"input\" type=\"text\">");
            client.println("<script>");
                client.println("var input = document.getElementById(\"input\");");
                client.println("input.addEventListener(\"keyup\", function(event) {");
                    client.println("event.preventDefault();");
                    client.println("if (event.keyCode === 13) {");
                        client.println("window.location.href = '/write/' + input.value;");
                    client.println("}");
                client.println("});");
            client.println("</script>");
              
            client.println("</body></html>");

            
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
