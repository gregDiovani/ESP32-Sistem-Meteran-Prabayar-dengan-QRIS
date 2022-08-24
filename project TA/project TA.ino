#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <EEPROM.h>



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


String getNominal;

    // String part02;
    // String part03;

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
    
}

void loop() {
  bacaPzem();
  readJson();    
    
}


void bacaPzem() {

   // Read the data from the sensor
     int voltage = pzem.voltage();
     float current = pzem.current();
     float power = pzem.power();
     float energy = pzem.energy();
     float frequency = pzem.frequency();
     float pf = pzem.pf();
     float sisakWh = Transaksi(getNominal) - energy;
     

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

       lcd.setCursor(0, 0);
        lcd.print(String(voltage) + String("V")); 

        lcd.setCursor(5, 0);
        lcd.print(String(current) + String("A"));

        lcd.setCursor(0, 1);
        lcd.print("Sisa:" + String(sisakWh) + " kWh"); 

        //// Logika kWh saat akan habis
        if(sisakWh < 3){          
        ///// Buzzer Berbunyi dan led Menyala
        int test;

        }else if(sisakWh == 0){
          ////  Buzzer berbunyi dan led mayala
          pzem.resetEnergy();
          digitalWrite(relay, HIGH);
          
        } else if(sisakWh < 0){

           sisakWh = 0;

        }
    
    }
}

float Transaksi(String nominal){

  float total =+  nominal.toFloat() / 1400;     
  return total;
  
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
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
        String payload = http.getString();
         getNominal = getValue(payload,',',0);
         Serial.println("Nominal:"+ getNominal);
        
        // part02 = getValue(payload,',',1);
        // part03 = getValue(payload,',',2);
                
      }
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

void kirim_data() {

  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;


    // String postData = (String)"tegangan=" + tegangan + "&arus=" + arus

    http.begin("http://192.168.43.140/postesp8266/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    auto httpCode = http.POST(postData);
    String payload = http.getString();

    Serial.println(postData);
    Serial.println(payload);
  

    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }






}



