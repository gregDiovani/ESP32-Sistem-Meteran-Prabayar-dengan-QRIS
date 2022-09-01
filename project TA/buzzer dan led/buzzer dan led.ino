const int led = 18;
const int buzzer = 25;
int buzzerState = HIGH;             // ledState used to set the LED
int ledState = HIGH;

const long onDuration = 500;// OFF time for LED
const long offDuration = 10000;// ON time for LED  
long rememberTime=0;// this is used by the code


const long onDurationLED = 200;// OFF time for LED
const long offDurationLED = 500;// ON time for LED  
long rememberTimeLED=0;// this is used by the code


void setup() {
  // initialize digital pin LEDPin as an output.
      pinMode(buzzer, OUTPUT);
      pinMode(led, OUTPUT);
      digitalWrite(led,ledState); 
      digitalWrite(buzzer,buzzerState); 
      

}

// the loop function runs over and over again forever
void loop() {
 
alarm();
ledAlarm();

}

void alarm(){
  if( buzzerState == HIGH )
 {
    if( (millis()- rememberTime) >= onDuration){   
    buzzerState = LOW;// change the state of LED
    rememberTime=millis();// remember Current millis() time
    }
 }
 else
 {   
    if( (millis()- rememberTime) >= offDuration){     
    buzzerState =HIGH;// change the state of LED
    rememberTime=millis();// remember Current millis() time
    }
 }

 // Robojax LED blink with millis()
 digitalWrite(buzzer,buzzerState);// turn the LED ON or OFF
                    // wait for a second

  
}

void ledAlarm(){


  if( ledState == HIGH )
 {
    if( (millis()- rememberTimeLED) >= onDurationLED){   
    ledState = LOW;// change the state of LED
    rememberTimeLED=millis();// remember Current millis() time
    }
 }
 else
 {   
    if( (millis()- rememberTimeLED) >= offDurationLED){     
    ledState =HIGH;// change the state of LED
    rememberTimeLED=millis();// remember Current millis() time
    }
 }

 // Robojax LED blink with millis()
 digitalWrite(led,ledState);// turn the LED ON or OFF
                    // wait for a second
}