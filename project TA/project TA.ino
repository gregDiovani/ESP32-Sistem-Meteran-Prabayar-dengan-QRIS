#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>


double previousNominal;
double nominalNow;

const char* ssid = "KAKA";
const char* password = "kokomong66";

// Server ke Xendit
String serverName = "https://gregorio.neojt.com/qris_cek_statis.php?external_id=K01";



/* 
 * keperluan Request ke server setial interval tertentu
*/
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;  ////// Untuk Request ke server setiap 5 menit
unsigned long previousMillis = 0;
const long interval = 1000;
WiFiClient client;



/* 
 * Keperluan Buzzer 
*/
int buzzerState = HIGH;
const long onDuration = 500;
const long offDuration = 10000;
long rememberTime = 0;  //



/* 
 * Keperluan LED Alarm Blink
*/
int ledState = HIGH;
const long onDurationLED = 200;
const long offDurationLED = 500;
long rememberTimeLED = 0;




/* Hardware Serial2 is only available on certain boards.
 * For example the Arduino MEGA 2560
*/
#if defined(ESP32)
PZEM004Tv30 pzem(Serial2, 16, 17);
#else
PZEM004Tv30 pzem(Serial2);
#endif

const int ledHijau = 19;
const int ledMerah = 18;
const int buzzer = 25;
const int relay = 26;  

/// LCD I2C
int lcdColumns = 16;
int lcdRows = 2;

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);


String getNominal;
String idKamar = "K01";
float pemakaianDaya;
double rupiah;

void setup() {

  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);



  pemakaianDaya = pzem.energy();
  previousNominal = ambilNominal();

  /* 
 *  kondisi awal output
*/
  pinMode(relay, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(ledMerah, OUTPUT);
  pinMode(ledHijau, OUTPUT);


  digitalWrite(ledMerah, ledState);
  digitalWrite(ledHijau, LOW);
  digitalWrite(buzzer, HIGH);
  digitalWrite(relay, HIGH);



  /* 
 * LCD INIT
*/
  lcd.init();
  lcd.backlight();



  /* 
 * iNIT WIFI
*/
  WiFi.begin(ssid, password);
}

void loop() {
  readJson();
  bacaPzem();

  if (getNominal.toDouble() <= 1000) {
    digitalWrite(ledHijau, HIGH);
    alarm();
    ledAlarm();



  } else if (getNominal.toDouble() <= 0) {

    digitalWrite(ledHijau, HIGH);
    pzem.resetEnergy();
    digitalWrite(relay, HIGH);

  } else

  {
    digitalWrite(relay, LOW);
    digitalWrite(ledHijau, LOW);

    /* 
 * Check jika ada saldo masuk
*/
    if (ambilNominal() > previousNominal) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tranksasi berhasil");
      delay(400);
      lcd.clear();

      previousNominal = ambilNominal();
    }
    // ---------------------------------


    /* 
 * Check jika ada perubahan pemakaian
*/
    if (pzem.energy() > pemakaianDaya) {

      // rupiah = (pzem.power() - pemakaianDaya) * 3660 / 1000 / (millis() - lastTime)  * 1400 ;
      rupiah = (pzem.energy() - pemakaianDaya) * 1400;
      Serial.print("Nominal penggunaan: Rp ");
      Serial.println(rupiah);

      //Check WiFi connection status
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // String postData = (String)"id_kamar=" + idKamar  + "&ammount_rp=" + rupiah;
        http.begin("http://gregorio.neojt.com/t_pakai.php?id_kamar=" + idKamar + "&amount_rp=" + rupiah);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        auto httpCode = http.GET();

      } else {
        Serial.println("WiFi Disconnected");
      }


      pemakaianDaya = pzem.energy();
    }
  }
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


  if (isnan(voltage)) {
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
    lcd.print("Sisa:" + String(sisakWh) + "kWh");
  }
}

float Transaksi(String nominal) {

  float total = +nominal.toFloat() / 1400;
  return total;
}



String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void readJson() {

  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String serverPath = serverName;

      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());

      // Send HTTP GET request
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        String payload = http.getString();
        getNominal = getValue(payload, ',', 0);

        //previousNominal = getNominal.toDouble()
        //double nominalNow;
        /*
        if(previousNominal < nominalNow){
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tranksasi berhasil");
        delay(400);
        lcd.clear();
                              
        }

        //nominalNow = previousNominal
        previousNominal = nominalNow;  
        */

        Serial.println("Nominal dari server:" + getNominal);


      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

float ambilNominal() {
  nominalNow = 0;
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String serverPath = serverName;

      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());

      // Send HTTP GET request
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        String payload = http.getString();
        getNominal = getValue(payload, ',', 0);

        nominalNow = getNominal.toDouble();
        //previousNominal = getNominal.toDouble()
        //double nominalNow;
        /*
        if(previousNominal < nominalNow){
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tranksasi berhasil");
        delay(400);
        lcd.clear();
                              
        }

        //nominalNow = previousNominal
        previousNominal = nominalNow;  
        */

        Serial.println("Nominal dari server:" + getNominal);


      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

void alarm() {
  if (buzzerState == HIGH) {
    if ((millis() - rememberTime) >= onDuration) {
      buzzerState = LOW;        
      rememberTime = millis();  
    }
  } else {
    if ((millis() - rememberTime) >= offDuration) {
      buzzerState = HIGH;       
      rememberTime = millis();  
    }
  }

 
  digitalWrite(buzzer, buzzerState);  
                                      
}

void ledAlarm() {


  if (ledState == HIGH) {
    if ((millis() - rememberTimeLED) >= onDurationLED) {
      ledState = LOW;              
      rememberTimeLED = millis();  
    }
  } else {
    if ((millis() - rememberTimeLED) >= offDurationLED) {
      ledState = HIGH;             
      rememberTimeLED = millis();  
    }
  }

  
  digitalWrite(ledMerah, ledState);  
                                     
}