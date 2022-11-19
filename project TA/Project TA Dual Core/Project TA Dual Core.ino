#include <WiFiManager.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Preferences.h>
#include <math.h>







/// Wifi manager object

WiFiManager wc;

/* 
 * Deklrasi PIN
 */
const int ledBiru = 15;
const int ledMerah = 18;
const int ledHijau = 19;
const int buzzer = 25;
const int relay = 26;







// LCD address and geometry and library initialization
const byte lcdAddr = 0x27;  // Address of I2C backpack
const byte lcdCols = 16;    // Number of character in a row
const byte lcdRows = 2;     // Number of lines



//// Preferences
Preferences preferences;


/* 
 * inisiasi LCD
 */
LiquidCrystal_I2C lcd(lcdAddr, lcdCols, lcdRows);


/* 
 * inisiasi PZEM
 */
#if defined(ESP32)
PZEM004Tv30 pzem(Serial2, 16, 17);
#else
PZEM004Tv30 pzem(Serial2);
#endif



unsigned long lastTime = 0;
unsigned long timerDelay = 5000;


/* 
 * Keperluan LED Alarm Blink
 */
int ledState = HIGH;
const long onDurationLED = 200;
const long offDurationLED = 500;
long rememberTimeLED = 0;


/* 
 * --> Global
 */

String getBalance;  /// Payload nominal dari web server
double nominalPenggunaan;
float pemakaianDaya;  /// Pemakian daya untuk daya


float voltage, current, power, energy, frequency, pf, sisakWh; /// Variable untuk pzem004t





void readSensorPZEM() {

  voltage = pzem.voltage();
  current = pzem.current();
  power = pzem.power();
  energy = pzem.energy();
  frequency = pzem.frequency();
  pf = pzem.pf();

  // Check if the data is valid
  if (isnan(voltage)) {
    voltage = 0.0;
  } else if (isnan(current)) {
    current = 0.0;
  } else if (isnan(power)) {
    power = 0.0;
    } else if (isnan(energy)) {
        Serial.println("Error reading energy");
    } else if (isnan(frequency)) {
        Serial.println("Error reading frequency");
    } else if (isnan(pf)) {
        Serial.println("Error reading power factor");
  } else {


    if (WiFi.status() == WL_CONNECTED) {

    decrementkwh();
      
    sisakWh =  ConvertTbalancetoKwh(getBalance);

    digitalWrite(ledBiru, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String("ONLINE ")  + String(power) + String(" W") );
    lcd.setCursor(0, 1);
    lcd.print("Pulsa:" + String(sisakWh, 3) + " kWh");



    }

      sisakWh =  ConvertTbalancetoKwh(getBalance) - energy;      
      lcd.clear();      
      digitalWrite(ledBiru, LOW);
      lcd.setCursor(0, 0);
      lcd.print(String("OFFLINE ") + String(power)+ String(" W") );
      lcd.setCursor(0, 1);
      lcd.print("Pulsa:" + String(sisakWh, 3) + " kWh");

    }

  
  ///// Serial  PZEM

    Serial.println("===============================PZEM-004T==================================");
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.println("V");
    Serial.print("Current: ");
    Serial.print(current);
    Serial.println("A");
    Serial.print("Power: ");
    Serial.print(power);
    Serial.println("W");
    Serial.print("Frequency: ");
    Serial.print(frequency, 1);
    Serial.println("Hz");
    Serial.print("PF: ");
    Serial.println(pf);
    Serial.print("Energy: ");
    Serial.print(energy, 3);
    Serial.println("kWh");
    Serial.println("=======================================================================");
    Serial.println();  

   
  }
}




void reconnectWifi() {

const unsigned long CONNECT_TIMEOUT = 120; // Wait 3 minutes to connect to the real AP before trying to boot the local AP
const unsigned long AP_TIMEOUT = 120; // Wait 3 minutes in the config portal before trying again the original WiFi creds


if (  WiFi.status() != WL_CONNECTED)  {

  wc.setConnectTimeout(CONNECT_TIMEOUT);
  wc.setTimeout(AP_TIMEOUT);
  wc.autoConnect("ESP32");
   
  }
}


void decrementkwh() {

  String idKamar = "K01";

  if (pzem.energy() > pemakaianDaya) {

    nominalPenggunaan = (pzem.energy() - pemakaianDaya) * 1500;
    Serial.print("Nominal penggunaan: Rp ");
    Serial.println(nominalPenggunaan);
  
      HTTPClient http;

      http.begin("http://gregorio.neojt.com/t_pakai.php?id_kamar=" + idKamar + "&amount_rp=" + nominalPenggunaan);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

       int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
       
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }

      
     // Free resources
      http.end();
      
     

     

      
      digitalWrite(ledBiru, LOW);
      delay(200);
      digitalWrite(ledBiru, HIGH);


      pemakaianDaya = energy;

  }

  
  
}


void runOutput() {

  
  /* 
 *  Pembatasan daya sebesar 350 W
 */

  ///if (power > 350) digitalWrite(relay, HIGH);  

  /* 
 *  Notifikasi saat rupiah di bawah 1 kwh
 */

   if ( sisakWh <= 1)ledMerahBlink();


    
  if ( sisakWh < 0 ||  sisakWh == 0) {
    digitalWrite(relay, HIGH);
    digitalWrite(ledHijau, LOW);
    ledMerahBlink();    
    pzem.resetEnergy();


  } else {

    digitalWrite(ledHijau, HIGH); /// Led Hijau Menyala
    digitalWrite(ledMerah, LOW);  /// Led Merah Mati
    digitalWrite(relay, LOW);
    digitalWrite(buzzer, LOW);
  }



}


void setup() {

  
/* 
 * Program Saat Awal dijalankan
 */

  Serial.begin(115200);

  
/* 
 * preferences
 */
  preferences.begin("simpan-sisa", false);
  getBalance = preferences.getString("nominal", "");
  Serial.println("Payload Tersimpan: " + getBalance);
  


/* 
 * Mode pin
 */

  pinMode(relay, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(ledMerah, OUTPUT);
  pinMode(ledHijau, OUTPUT);
  pinMode(ledBiru, OUTPUT);


/* 
 * Kondisi Awal
 */  

  digitalWrite(relay, LOW); ///  Relay Menyala
  digitalWrite(ledHijau, HIGH); /// LED Hijau Power menyala
  digitalWrite(ledMerah, LOW);  /// LED Merah Mati
  digitalWrite(ledBiru, LOW); /// LED Biru WIFI mati
  digitalWrite(buzzer, LOW); // Buzzer Mati


  
/* 
 * RTOS
 */  
  
  xTaskCreatePinnedToCore(Task_1, "Task_Count", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(Task_2, "Task_GPIO_1", 10000, NULL, 1, NULL, 1);


  
}




void loop() {

  
}

void Task_1(void *parameter) {

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(String("INITIALIZING..."));


  wc.autoConnect("ESP32-WIFI");
  Serial.println("Connected");  

  
  while (1) {

    readSensorPZEM();
    runOutput();


    vTaskDelay(2000 / portTICK_PERIOD_MS);
  

  }
}



void Task_2(void *parameter) {
  while (1) {  // infinite loop


    getBalancefromServer();
    reconnectWifi();


  }
}


void getBalancefromServer() {

if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

    String serverPath = "http://gregorio.neojt.com/qris_cek_statis.php?external_id=K01";
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.println("===================Server========================");
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        getBalance = http.getString();
        Serial.println("Payload nominal dari server: " + getBalance);
         
   
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      preferences.putString("nominal", getBalance);
      preferences.end();
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}



float ConvertTbalancetoKwh(String getBalance) {

  float kwh = getBalance.toFloat() / 1500;
  return kwh;
}



void ledMerahBlink() {

      if (ledState == HIGH) {
        if ((millis() - rememberTimeLED) >= onDurationLED) {
          ledState = LOW;
          rememberTimeLED = millis();
          
        }
      } else {
        if ((millis() - rememberTimeLED) >= offDurationLED) {
          ledState = HIGH;
          tone(buzzer, 50, 500);
          rememberTimeLED = millis();
        }
      }

      digitalWrite(ledMerah, ledState);
    }




