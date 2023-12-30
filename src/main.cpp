/*
 (c) by BlackLeakz | 12/29/23

 Name: blackleakz-nodemcuv3-wifi
 Version: 0.1
 Github: https://github.com/blackzspace-de/blackleakz-nodemcuv3-wifi
 Homepage: https://blackzspace.de/
 Details: This program is made by learning by doing.
 We use an Asynchrone WebServer, LittleFS, Adafruits OLED SSD1306 Display Libarys and other essential WiFi-Modules you need.


1x OLED SSD1306 128x64
1x Breadboard
*x Some Wires
2x LED
1x resistor

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AsyncElegantOTA.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

// Adafruit Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi Details
const char *ssid = "BLACKLEAKZ-AP";
const char *password = "123456789";

// mDNS Credentials
const char *dns_name = "blackleakz"

// Define RED-LED
#define LED D1

    // Values for WiFi-Scanner
    int numberOfNetworks;
unsigned long lastAusgabe;
const unsigned long intervall = 5000;

// Starts Asynchrone WebServer
AsyncWebServer server(80);

String content;

// Initialize Messages on Screen
void oled_start()
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Console >");
  display.display();
  delay(2000);
  display.clearDisplay();

  display.setCursor(0, 0);
  display.println("Started---");
  display.display();
  delay(2000);
  display.clearDisplay();
}

// Starting WiFi Network scan
void wifi_oledscan()
{
  Serial.println("Console > Starting Network Scan !! :::..::.");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Console > ");
  display.display();
  display.setCursor(2, 10);
  display.println("Scan for: ");
  display.display();
  display.setCursor(1, 0);
  display.println("Networkz !!");
  display.display();

  delay(900);

  if (numberOfNetworks <= 0)
  {
    numberOfNetworks = WiFi.scanNetworks();
    Serial.print(numberOfNetworks);
    Serial.println(F(" gefundene Netzwerke"));
  }
  else if (millis() - lastAusgabe > intervall)
  {
    numberOfNetworks--;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Network-name: ");
    display.setCursor(0, 10);
    display.print(WiFi.SSID(numberOfNetworks));
    display.setCursor(0, 20);
    display.print("Signal strength: ");
    display.setCursor(0, 30);
    display.print(WiFi.RSSI(numberOfNetworks));
    display.display();
    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(numberOfNetworks));
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(numberOfNetworks));
    Serial.println("-----------------------");
    lastAusgabe = millis();
  }
}

// WebServer's 404 Page
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

// The Main-Part of this program
void setup()
{

  // Starts Serial-Communication || BAUDRATE: 115200
  Serial.begin(115200);
  Serial.println("Console > Blackz NodeMCUv3 ESP8266 App started...");

  // Declare BUILTIN LED
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED, OUTPUT);

  // Mounting FileSystem
  if (!LittleFS.begin())
  {
    Serial.println("Console > An error has occured while mounting LittleFS");
    return;
  }

  // Starting Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("Console > SSD1306 allocation failed!!! :::..:.:..---__---_-"));
    for (;;)
      ;
  }

  // Starting Display-Text function
  display.clearDisplay();
  oled_start();

  // Starting mDNS
  if (MDNS.begin(dns_name))
  {
    Serial.println("DNS gestartet, erreichbar unter: ");
    Serial.println("http://" + String(dns_name) + ".local/");
  }

  // Setting up WiFi softAP with given variables
  Serial.print("Console > Setting AP (Access Point)…");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Setting AP");
  display.display();
  display.setCursor(0, 10);
  display.println(ssid);
  display.display();
  display.setCursor(0, 20);
  display.println(password);
  display.display();
  delay(555);

  WiFi.softAP(ssid, password); // Starts WiFi

  IPAddress IP = WiFi.softAPIP(); // output variable from function to string
  Serial.print("Console > AP IP address: ");
  Serial.println(IP);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AP IP >");
  display.display();
  display.setCursor(0, 10);
  display.println(IP);
  display.display();

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
  display.setCursor(0, 20);
  display.println(WiFi.localIP());
  display.display();

  // Starts WiFi-Network Scan
  wifi_networkscan();

  //        // AsyncWebServer's     ||   Route-Handler    || AsyncWebServer's      ||   Route-Handler // //
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  server.on("/", []()
            {
  String header;
  if (!is_authentified()){
    server.sendHeader("Location","/login");
    server.sendHeader("Cache-Control","no-cache");
    server.send(301);
    return;
  }
  String content;
  content = "<!DOCTYPE html><html lang='en'><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>ul { list-style-type: none; margin: 0; padding: 0; overflow: hidden; background-color: #333; } li { float: left; border-right:1px solid #bbb; } li:last-child { border-right: none; } li a { display: block; color: white; text-align: center; padding: 14px 16px; text-decoration: none; } li a:hover:not(.active) { background-color: #111; } .active { background-color: #04AA6D; } head { background-color: #8F8F8F; color: #FF0000; } body{ background-color: #8F8F8F; color: #00ECFF; } h2 { color: green; }</style><head><center><h2>You successfully connected to blackleakz.local</h2></center>";
  if (server.hasHeader("User-Agent")){
    content += "<center><p1>the user agent used is : </p1></center><center>" + server.header("User-Agent") + "</center><br><br>";
  }
  content += "<center><p1>You can access this page until you</p1></center><center><a href=\"/login?DISCONNECT=YES\">disconnect</a></center><br></head>";
  content += "<body><ul><li><a class='active' href='/'>Home</a></li><li><a href='/repeater'>Repeater</a></li><li><a href='/otaupdate'>OTA-Update</a></li><li style='float:right'><a href='/reboot'>Reboot</a></li><li><a href='/wifiscan'>WiFi-Scan</a></li></ul>";
  server.send(200, "text/html", content); });

  server.on("/login", []()
            {
    String msgd;
  if (server.hasHeader("Cookie")){
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")){
    Serial.println("Console<>> Disconnection");
    server.sendHeader("Location","/login");
    server.sendHeader("Cache-Control","no-cache");
    server.sendHeader("Set-Cookie","ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("REPEATER")){
    Serial.println("Console<>> REPEATER.");
    server.sendHeader("Location","/repeater");
    server.sendHeader("Cache-Control","no-cache");
    server.sendHeader("Set-Cookie","ESPSESSIONID=2");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")){
    if (server.arg("USERNAME") == "root" &&  server.arg("PASSWORD") == "toor" ){
      server.sendHeader("Location","/");
      server.sendHeader("Cache-Control","no-cache");
      server.sendHeader("Set-Cookie","ESPSESSIONID=1");
      server.send(301);
      Serial.println("Console<>> Log in Successful");
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Console<>> ");
      display.display();
      display.setCursor(0,10);
      display.println("Login.");
      display.display();
      display.setCursor(0,20);
      display.println("successfull");
      display.display();
      display.display();
      return;
    }
  msgd = "<style>p1 { color: red; }</style><center><p1>Wrong username/password! try again.</p1></center>";
  Serial.println("Console<>> Log in Failed");
  }
  String content = "<html><body><style>p1 { color: red; } body { background-color: grey; color: green; } head { background-color: grey; color: green; }</style><center><form action='/login' method='POST'><center><p1>To log in, please use : root/toor</p1></center><br>";
  content += "<center><p1>User:</p1></center><center><input type='text' name='USERNAME' placeholder='user name'></center><br>";
  content += "<center><p1>Password:</p1></center><input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<center><input type='submit' name='SUBMIT' value='Submit'></center></form></center>" + msgd + "<br>";
  content += "<center><p1>You also can go</p1></center><center><a href='/inline'><p1>here</p1></a></center></body></html>";
  server.send(200, "text/html", content); });

  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/wifi_scan.html", "text/html"); });

  server.onNotFound(notFound);

  //      // END AsyncWebServer's     ||   Route-Handler    ||  END AsyncWebServer's      ||   Route-Handler

  // Start AsyncElegantOTA
  AsyncElegantOTA.begin(&server);

  // Start server
  server.begin();
}

void loop()
{

  server.handleClient();

  // mDNS update
  MDNS.update();

  // WiFi-Scan Loop
  scan_wifi();

  // LED BLINK LOOP
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);

  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
}