#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>


const char* ssid = "KAKA";
const char* password = "kokomong66";

//Your Domain name with URL path or IP address with path

 String serverName = "https://gregorio.neojt.com/qris_cek_statis.php?external_id=K01";


unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;


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


     pzem.resetEnergy();

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

  readJson();    
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

  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      String serverPath = serverName;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        
        
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

}