#include <Wire.h>

#include <ThreeWire.h>
#include <RtcDS1302.h>


#define PUMP_PIN 4
#define MOISTURE_SENSOR_PIN A0
#define WATER_SENSOR_PIN A1
#define BUZZER_PIN 8
//led vars
#define GREEN_LED 13
#define RED_LED 12

//time vars
#define DAT_PIN 5
#define CLK_PIN 6

//button var
#define BTN_PIN 2

ThreeWire myWire(6, 5, 7);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);


unsigned long previousMillis = 0;
unsigned long checkInterval = 3000;
unsigned long countdownStart = 0;

bool isPumping = false;
bool isInCountdown = false;
bool isWaiting = false;
bool btnState = false;


void setup() {
  Serial.begin(9600);
  pinMode(BTN_PIN, INPUT);  

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);



  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid()) {

    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");
  } else if (now == compiled) {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }


  Serial.println("System Ready...");
}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();

  // printDateTime(now);
  // Serial.println();

  if (!now.IsValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
  }

  unsigned long currentMillis = millis();
  int waterRawVal = analogRead(WATER_SENSOR_PIN);

  if (waterRawVal >= 250) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  } else if (waterRawVal <= 50) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    for (int i = 0; i < 3; i++) {  
      tone(BUZZER_PIN, 1000);      
      delay(300);                 
      noTone(BUZZER_PIN);         
      delay(300);                  
    }

  } else {
    noTone(BUZZER_PIN); 
  }

  if (!isPumping && !isInCountdown && !isWaiting && currentMillis - previousMillis >= checkInterval && waterRawVal >= 50 && isWateringTime(now)) {
    previousMillis = currentMillis;

    int value = analogRead(MOISTURE_SENSOR_PIN);
    int moisturePercentage = map(value, 0, 1023, 0, 100);

    Serial.print("Moisture Value: ");
    Serial.print(value);
    Serial.print(" | Moisture Percentage: ");
    Serial.print(moisturePercentage);
    Serial.println("%");

    int waterRaw = analogRead(WATER_SENSOR_PIN);
    int waterPercentage = map(waterRaw, 300, 700, 0, 100);
    waterPercentage = constrain(waterPercentage, 0, 100);

    Serial.print("Water Level: ");
    Serial.print(waterRaw);
    Serial.println("%");
    
    if (value > 800) {
      isPumping = true;
      Serial.println("Pump ON for 3 seconds...");
      digitalWrite(PUMP_PIN, HIGH);
      delay(10000);
      digitalWrite(PUMP_PIN, LOW);
      Serial.println("Pump OFF");

      for (int i = 0; i < 2; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(150);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
      }
      delay(300);
      digitalWrite(BUZZER_PIN, HIGH);
      delay(250);
      digitalWrite(BUZZER_PIN, LOW);

      isPumping = false;
      isInCountdown = true;
      countdownStart = millis();
    }
  }

  if(!isWateringTime(now)) {
    delay(5000);
    Serial.println("Not in the time, press the button to set it manually");
  }

  if (isInCountdown) {
    for (int i = 5; i > 0; i--) {
      Serial.print("Waiting ");
      Serial.print(i);
      Serial.println(" sec...");
      delay(1000);
    }
    isInCountdown = false;
    isWaiting = false;
    Serial.println("Resuming sensor check every 3 seconds...");
  }
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt) {
  char timestring[10]; 

  snprintf_P(timestring,
             countof(timestring),
             PSTR("%02u:%02u:%02u"), 
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(timestring);
}

bool isWateringTime(const RtcDateTime& dt) {
  int hour = dt.Hour();
  int minute = dt.Minute();
  
  // (6:00 AM - 8:59 AM)
  bool isMorning = (hour == 6 && minute >= 0) || 
                  (hour == 7) || 
                  (hour == 8 && minute <= 59);
  
  //  (5:00 PM - 7:59 PM)
  bool isEvening = (hour == 17 && minute >= 0) || 
                   (hour == 18) || 
                   (hour == 19 && minute <= 59);
  
  return isMorning || isEvening;
}
