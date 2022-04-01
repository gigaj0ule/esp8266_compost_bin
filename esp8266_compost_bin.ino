#include <DHT_U.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// Wifi connection settings
const char* ssid = "Omni Commons";
const char* password = "";

// Pin definitions
const int StatusLed = 4;
const int PollingLED = 5;
const int dhtSensorPin = 2;
const int fanPin = 3;

// Fan rules
const int fan_target_humidity = 70;
const int fan_hysteresis = 5;

// Objects
ESP8266WebServer server(80);
DHT dht11(dhtSensorPin, DHT11);

// Helper functions
double Fahrenheit(double celsius) {
  return ((double)(9 / 5) * celsius) + 32;
}

double Kelvin(double celsius) {
  return celsius + 273.15;
}

// Global variables
float temp, humi;
long intervalcounter = 0;
bool fan_on = false;

// Setup routine
void setup(void)
{
  Serial.begin(115200);

  // Pin settings
  pinMode(StatusLed, OUTPUT);
  pinMode(PollingLED, OUTPUT);
  pinMode(fanPin, OUTPUT);
  digitalWrite(StatusLed,  0);
  digitalWrite(PollingLED, 1);
  digitalWrite(fanPin, fan_on);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  int ledtoggle = 0;
  while (WiFi.status() != WL_CONNECTED) {
    ledtoggle = !ledtoggle;
    Serial.print("Waiting for ");
    Serial.print(ssid);
    Serial.println("...");
    digitalWrite(StatusLed, ledtoggle);
    delay(500);
  }
   
  digitalWrite(StatusLed, 1);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
   
  server.on("/", handle_root);
  server.on("/readsensor", handle_sensor);
  
  server.begin();
  
  Serial.println("HTTP server started");
}
 
 
void loop(void)
{
  delay(1);
  intervalcounter = intervalcounter + 1;
  
  if((intervalcounter % (1000)) == 0)
  {
    temp = dht11.readTemperature();
    humi = dht11.readHumidity();

    if(humi < (70 - fan_hysteresis / 2))
    {
      fan_on = false;
      digitalWrite(fanPin, fan_on);
    }
    else
    if(humi > (70 + fan_hysteresis / 2))
    {
      fan_on = true;
      digitalWrite(fanPin, fan_on);
    } 
  }

  int ledtoggle = 1;
  digitalWrite(StatusLed, 1);
  server.handleClient();
  
  while (WiFi.status() != WL_CONNECTED) {
    ledtoggle = !ledtoggle;
    Serial.print("Waiting for ");
    Serial.print(ssid);
    Serial.println("...");
    digitalWrite(StatusLed, ledtoggle);
    delay(500);
  }
} 


void handle_root(){ 
  String html = "";
  String html_1 = "<h1>Thermostat sensor @ ";
  String html_2 = "</h1>Connected to ";
  String html_3 = "<br><br>";
  String html_4 = "<a href='./readsensor' target='iframe' style='text-decoration:none'>Poll sensors</a><br><br><iframe name='iframe' src='./readsensor'></iframe>";

  IPAddress ip = WiFi.localIP();
  char sprintf_buffer[128];
  sprintf(sprintf_buffer, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  html.concat(html_1);
  html.concat(sprintf_buffer);
  html.concat(html_2); 
  html.concat(ssid);
  html.concat(html_3); 
  html.concat(html_4);
  
  server.send(200, "text/html", html);
}


void handle_sensor(){ 
  // Turn led on
  digitalWrite(PollingLED, 0);

  // Buffers for human readable values
  char sprintf_buffer_1[20];
  char sprintf_buffer_2[20];

  // Print sensor values on serial port
  Serial.print("Temp: ");
  Serial.println(temp);
  Serial.print("Humi: ");
  Serial.println(humi);

  // String for html new line object 
  String newline = "<br>";

  // Take variable names and format as strings
  sprintf(sprintf_buffer_1, "Temperature: %d deg C", (int) temp);
  sprintf(sprintf_buffer_2, "Humidity: %d %rh", (int) humi);

  // Make string to hold html.
  String html = "";

  // Stitch all the strings together
  html.concat(sprintf_buffer_1);
  html.concat(newline);
  html.concat(sprintf_buffer_2);

  // Send strings to browser
  server.send(200, "text/html", html);

  // Turn led off
  digitalWrite(PollingLED, 1);
}
