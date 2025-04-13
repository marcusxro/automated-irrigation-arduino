#define PUMP_PIN 4
#define MOISTURE_SENSOR_PIN A0
#define WATER_SENSOR_PIN A1
#define BUZZER_PIN 8

unsigned long previousMillis = 0;
unsigned long checkInterval = 3000;
unsigned long countdownStart = 0;

bool isPumping = false;
bool isInCountdown = false;
bool isWaiting = false;

void setup() {
  Serial.begin(9600);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  
  Serial.println("System Ready...");
}

void loop() {
  unsigned long currentMillis = millis();

  if (!isPumping && !isInCountdown && !isWaiting && currentMillis - previousMillis >= checkInterval) {
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

      // Gentle beep pattern 
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
