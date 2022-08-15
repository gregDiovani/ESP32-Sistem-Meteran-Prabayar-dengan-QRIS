#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ArduinoJson.h>



const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

//Your Domain name with URL path or IP address with path
const char* host = "https://gregorio.neojt.com";


unsigned long timer = 0;
int interval = 15000;
const int httpPort = 80;
WiFiClient client;




/////---------------------------------

/* Hardware Serial2 is only available on certain boards.
 * For example the Arduino MEGA 2560
*/
#if defined(ESP32)
PZEM004Tv30 pzem(Serial2, 16, 17);
#else
PZEM004Tv30 pzem(Serial2);
#endif



const int relay = 26; ///  Relay

/// LCD I2C 
int lcdColumns = 16;
int lcdRows = 2;

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  


void setup() {

    Serial.begin(115200);
    Serial2.begin(9600,SERIAL_8N1, 16, 17);


/////--------------- Deklarasi PIN-----------------------/////
    pinMode(relay, OUTPUT);
/////-------------------------------------------------/////




/////--------------- LCD Init-----------------------/////
    lcd.init();
    lcd.backlight();
/////-------------------------------------------------/////



    WiFi.begin(ssid, password);
    Serial.println("Connecting...");

    while (WiFi.status() != WL_CONNECTED) {
    bacaPzem();
    delay(500);
    Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    
   


}

void loop() {

  
   digitalWrite(relay, LOW);
        
    
}


void bacaPzem() {

   // Read the data from the sensor
    int voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

   if(isnan(voltage)){
        Serial.println("Error reading voltage");
    } else if (isnan(current)) {
        Serial.println("Error reading current");
    } else if (isnan(power)) {
        Serial.println("Error reading power");
    } else if (isnan(energy)) {
        Serial.println("Error reading energy");
    } else if (isnan(frequency)) {
        Serial.println("Error reading frequency");
    } else if (isnan(pf)) {
        Serial.println("Error reading power factor");
    } else {

        // Print the values to the Serial console

        lcd.setCursor(0, 0);
        lcd.print(String(voltage) + String("V")); 

        lcd.setCursor(5, 0);
        lcd.print(String(current) + String("A"));

        lcd.setCursor(11, 0);
        lcd.print(String(power,1) + String("W")); 
 

        lcd.setCursor(0, 1);
        lcd.print(String(energy,3) + String("kWh")); 

         lcd.setCursor(0, 1);
        lcd.print(String(energy,3) + String("kWh")); 

    
    }
}

void readJson(){

  if ((millis() - timer) >= interval || timer == 0) {
    timer = millis();
    client.stop();
    if (client.connect(host, httpPort)) {
      String url = "/qris_cek_statis.php?external_id=K01";
      Serial.println("connection is successful, Trying to load the JSON data... ");
      client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: NodeMCU\r\n" + "Connection: close\r\n\r\n");
      while (client.connected()) {
        String line = client.readStringUntil('\n'); //HTTP HEADER
        //Serial.println(line);
        if (line == "\r") {
          break;
        }
      }
      DynamicJsonDocument doc(512);
      String line = client.readString(); //PAYLOAD
      Serial.println(line);
      deserializeJson(doc, line);
      JsonObject obj = doc.as<JsonObject>();



      float hystereza = obj[String("Hysteresis")];
      float cielova_teplota = obj[String("Target_Temperature")];
      float actual_temperature = obj[String("Actual_Temperature")];


      
      Serial.print("Hystereza: ");
      Serial.println(hystereza);
      Serial.print("Cielova teplota: ");
      Serial.println(cielova_teplota);
      Serial.print("Namerana (aktualna) teplota: ");
      Serial.println(actual_temperature);
    } else if (!client.connect(host, httpPort)) {
      Serial.println("Nepodarilo sa pripojenie k termostatu, ani nacitanie JSON data");
    }
  }

}