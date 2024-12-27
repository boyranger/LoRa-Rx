#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include "ThingSpeak.h"
const char* ssid = "Redmi 13C";   // your network SSID (name) 
const char* password = "054$#09!!";   // your network password

WiFiClient  client;

unsigned long myChannelNumber = 1;
const char * myWriteAPIKey = "K6NNZ4HOPIOO87Q8";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

#define ss 15
#define rst 16
#define dio0 2

// Initialize variables to get and save LoRa data
int rssi;
String loRaMessage;
String rhSoil;
String lvlRain;
String aZ;
String gZ;
String readingID;

//Initialize LoRa module
void startLoRA(){
  int counter;

  while (!Serial);

  Serial.println("LoRa Receiver");
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }

  if (counter == 10) {
    // Increment readingID on every new reading
    Serial.println("Starting LoRa failed!"); 
  }
}
void connectWiFi(){
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
  // WiFi.mode(WIFI_STA);   
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak

}

// Read LoRa packet and get the sensor readings
void getLoRaData() {
   Serial.print("Lora packet received: ");
  // Read packet
  while (LoRa.available()) {
    String LoRaData = LoRa.readString();
    // LoRaData format: readingID/temperature&soilMoisture#batterylevel
    // String example: 1/27.43&654#95.34
    Serial.print(LoRaData); 
    
    // Get readingID, temperature and soil moisture
    int pos1 = LoRaData.indexOf('/');
    int pos2 = LoRaData.indexOf('&');
    int pos3 = LoRaData.indexOf('#');
    int pos4 = LoRaData.indexOf('$');
    readingID = LoRaData.substring(0, pos1);
    rhSoil = LoRaData.substring(pos1 +1, pos2);
    lvlRain = LoRaData.substring(pos2+1, pos3);
    aZ=LoRaData.substring(pos3+1, pos4);
    gZ = LoRaData.substring(pos4+1, LoRaData.length());

     // set the fields with the values
    ThingSpeak.setField(1, rhSoil);
    //ThingSpeak.setField(1, temperatureF);
    ThingSpeak.setField(2, lvlRain);
    ThingSpeak.setField(3, aZ);
    ThingSpeak.setField(6, gZ);

    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  


  }
  // Get RSSI
  rssi = LoRa.packetRssi();
  Serial.print(" with RSSI ");    
  Serial.println(rssi);
}

void setup() {
  Serial.begin(9600);
  startLoRA();
  connectWiFi();
  
}

void loop() {
    // Check if there are LoRa packets available
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    getLoRaData();
  }
}