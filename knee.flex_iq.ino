#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

#define FLEX_PIN A0
#define BUTTON_PIN 2
#define BUZZER_PIN 9
#define SD_CS_PIN 10
#define BT_RX 11
#define BT_TX 12
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define BATTERY_PIN A1
#define MAX_REPS 20
#define WINDOW_SIZE 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MPU6050 mpu;
SoftwareSerial bluetooth(BT_RX, BT_TX);

int flexValue;
float angle;
float maxAngle = 0;
float avgAngle = 0;
int repCount = 0;
bool sessionActive = false;
File dataFile;
float flexMin = 400;
float flexMax = 600;
float angleBuffer[WINDOW_SIZE];
int bufferIndex = 0;
float batteryVoltage;
bool calibrated = false;

void initializeSensors();
void readSensors();
float calculateAngle();
void smoothData(float newAngle);
String classifyExercise(float maxAngle, float avgAngle);
void displayFeedback(String feedback);
void logData(float angle, String classification);
void sendBluetoothData(float angle, String classification);
void toggleSession();
void calibrateFlexSensor();
void resetSession();
float readBattery();
void checkBatteryLevel();
void updateRepCount(float currentAngle, float lastAngle);
void drawWelcomeScreen();
void drawCalibrationScreen();
void drawSessionScreen(float angle, String feedback);
void drawBatteryWarning();
void beepBuzzer(int frequency, int duration);
float getAverageAngle();

void setup() {
  Serial.begin(9600);
  Wire.begin();
  SPI.begin();
  bluetooth.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FLEX_PIN, INPUT);
  pinMode(BATTERY_PIN, INPUT);

  initializeSensors();
  drawWelcomeScreen();
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), toggleSession, FALLING);

  for (int i = 0; i < WINDOW_SIZE; i++) {
    angleBuffer[i] = 0;
  }
}

void loop() {
  if (!calibrated) {
    calibrateFlexSensor();
  } else if (sessionActive) {
    float lastAngle = angle;
    readSensors();
    smoothData(angle);
    angle = getAverageAngle();
    updateRepCount(angle, lastAngle);
    if (angle > maxAngle) maxAngle = angle;
    avgAngle = (avgAngle * (repCount - 1) + angle) / repCount;
    String classification = classifyExercise(maxAngle, avgAngle);
    displayFeedback(classification);
    logData(angle, classification);
    sendBluetoothData(angle, classification);
    checkBatteryLevel();
    delay(100);
  }
}

void initializeSensors() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (1);
  }
  display.clearDisplay();
  display.display();

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 failed");
    while (1);
  }

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD failed");
    while (1);
  }
  dataFile = SD.open("rehab.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println("File failed");
    while (1);
  }
}

void readSensors() {
  flexValue = analogRead(FLEX_PIN);
  angle = map(flexValue, flexMin, flexMax, 0, 90);
  angle = constrain(angle, 0, 90);

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getAcceleration(&ax, &ay, &az);
  mpu.getRotation(&gx, &gy, &gz);
  float pitch = atan2(ax, sqrt(ay * ay + az * az)) * 180 / PI;
  float roll = atan2(ay, sqrt(ax * ax + az * az)) * 180 / PI;
  angle = (angle + pitch + roll) / 3;
}

float calculateAngle() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  float pitch = atan2(ax, sqrt(ay * ay + az * az)) * 180 / PI;
  return (angle + pitch) / 2;
}

void smoothData(float newAngle) {
  angleBuffer[bufferIndex] = newAngle;
  bufferIndex = (bufferIndex + 1) % WINDOW_SIZE;
}

float getAverageAngle() {
  float sum = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sum += angleBuffer[i];
  }
  return sum / WINDOW_SIZE;
}

String classifyExercise(float maxAngle, float avgAngle) {
  if (maxAngle >= 50 && maxAngle <= 70 && avgAngle >= 40 && avgAngle <= 60) {
    return "correct";
  } else if (maxAngle > 70 || avgAngle > 60) {
    return "too_much";
  } else if (maxAngle < 50 || avgAngle < 40) {
    return "too_little";
  } else {
    return "unknown";
  }
}

void displayFeedback(String feedback) {
  drawSessionScreen(angle, feedback);
  if (feedback == "too_much") {
    beepBuzzer(1000, 200);
  } else if (feedback == "too_little") {
    beepBuzzer(500, 200);
  }
}

void logData(float angle, String classification) {
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.print(angle);
    dataFile.print(",");
    dataFile.print(maxAngle);
    dataFile.print(",");
    dataFile.print(avgAngle);
    dataFile.print(",");
    dataFile.print(repCount);
    dataFile.print(",");
    dataFile.println(classification);
    dataFile.flush();
  }
}

void sendBluetoothData(float angle, String classification) {
  bluetooth.print("Time: ");
  bluetooth.print(millis());
  bluetooth.print(" | Angle: ");
  bluetooth.print(angle);
  bluetooth.print(" | Max: ");
  bluetooth.print(maxAngle);
  bluetooth.print(" | Avg: ");
  bluetooth.print(avgAngle);
  bluetooth.print(" | Reps: ");
  bluetooth.print(repCount);
  bluetooth.print(" | Status: ");
  bluetooth.println(classification);
}

void toggleSession() {
  sessionActive = !sessionActive;
  if (sessionActive) {
    resetSession();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Session Started");
    display.display();
    delay(1000);
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Session Stopped");
    display.display();
    delay(1000);
  }
}

void calibrateFlexSensor() {
  display.clearDisplay();
  drawCalibrationScreen();
  delay(2000);

  flexMin = analogRead(FLEX_PIN);
  display.setCursor(0, 20);
  display.println("Bend knee fully...");
  display.display();
  delay(3000);

  flexMax = analogRead(FLEX_PIN);
  calibrated = true;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Calibration Done");
  display.display();
  delay(1000);
}

void resetSession() {
  maxAngle = 0;
  avgAngle = 0;
  repCount = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    angleBuffer[i] = 0;
  }
  bufferIndex = 0;
}

float readBattery() {
  int batteryRaw = analogRead(BATTERY_PIN);
  batteryVoltage = (batteryRaw / 1023.0) * 5.0;
  return batteryVoltage;
}

void checkBatteryLevel() {
  float voltage = readBattery();
  if (voltage < 3.5) {
    drawBatteryWarning();
    beepBuzzer(2000, 500);
    delay(1000);
  }
}

void updateRepCount(float currentAngle, float lastAngle) {
  if (lastAngle < 30 && currentAngle >= 30 && repCount < MAX_REPS) {
    repCount++;
  }
}

void drawWelcomeScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("Knee Rehab");
  display.setTextSize(1);
  display.setCursor(10, 40);
  display.println("Press button to start");
  display.display();
  delay(2000);
}

void drawCalibrationScreen() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Calibration Mode");
  display.setCursor(0, 10);
  display.println("Keep knee straight...");
  display.display();
}

void drawSessionScreen(float angle, String feedback) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Angle: ");
  display.print(angle);
  display.println(" deg");
  display.setCursor(0, 10);
  display.print("Max: ");
  display.print(maxAngle);
  display.println(" deg");
  display.setCursor(0, 20);
  display.print("Avg: ");
  display.print(avgAngle);
  display.println(" deg");
  display.setCursor(0, 30);
  display.print("Reps: ");
  display.println(repCount);
  display.setCursor(0, 40);
  display.print("Status: ");
  display.println(feedback);
  display.setCursor(0, 50);
  display.print("Battery: ");
  display.print(readBattery());
  display.println("V");
  display.display();
}

void drawBatteryWarning() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Low Battery Warning!");
  display.setCursor(0, 10);
  display.print("Voltage: ");
  display.print(readBattery());
  display.println("V");
  display.display();
}

void beepBuzzer(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}

void drawProgressBar(int progress, int total) {
  int barWidth = SCREEN_WIDTH - 20;
  int filledWidth = (progress * barWidth) / total;
  display.drawRect(10, 50, barWidth, 10, SSD1306_WHITE);
  display.fillRect(10, 50, filledWidth, 10, SSD1306_WHITE);
}

void updateProgress() {
  if (sessionActive) {
    drawProgressBar(repCount, MAX_REPS);
    display.display();
  }
}

void checkSensorHealth() {
  if (flexValue < 100 || flexValue > 900) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Flex Sensor Error");
    display.display();
    beepBuzzer(1500, 300);
    delay(1000);
  }
}

void saveSessionSummary() {
  if (dataFile) {
    dataFile.println("--- Session Summary ---");
    dataFile.print("Max Angle: ");
    dataFile.println(maxAngle);
    dataFile.print("Avg Angle: ");
    dataFile.println(avgAngle);
    dataFile.print("Repetitions: ");
    dataFile.println(repCount);
    dataFile.println("-----------------------");
    dataFile.flush();
  }
}

void displaySummary() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Session Summary");
  display.setCursor(0, 10);
  display.print("Max: ");
  display.print(maxAngle);
  display.println(" deg");
  display.setCursor(0, 20);
  display.print("Avg: ");
  display.print(avgAngle);
  display.println(" deg");
  display.setCursor(0, 30);
  display.print("Reps: ");
  display.println(repCount);
  display.display();
  delay(3000);
}

void handleBluetoothCommands() {
  if (bluetooth.available()) {
    char command = bluetooth.read();
    if (command == 'S') {
      sessionActive = true;
      resetSession();
    } else if (command == 'E') {
      sessionActive = false;
      saveSessionSummary();
      displaySummary();
    }
  }
}

void updateDisplayBrightness() {
  int lightLevel = analogRead(A2);
  int brightness = map(lightLevel, 0, 1023, 0, 255);
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(brightness);
}

void monitorTemperature() {
  int tempRaw = analogRead(A3);
  float tempVoltage = (tempRaw / 1023.0) * 5.0;
  float temperature = (tempVoltage - 0.5) * 100;
  if (temperature > 40) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Temp High: ");
    display.print(temperature);
    display.println("C");
    display.display();
    beepBuzzer(2000, 400);
    delay(1000);
  }
}

void drawAngleGraph() {
  display.clearDisplay();
  for (int i = 0; i < WINDOW_SIZE - 1; i++) {
    int x1 = i * (SCREEN_WIDTH / WINDOW_SIZE);
    int y1 = map(angleBuffer[i], 0, 90, SCREEN_HEIGHT - 1, 0);
    int x2 = (i + 1) * (SCREEN_WIDTH / WINDOW_SIZE);
    int y2 = map(angleBuffer[i + 1], 0, 90, SCREEN_HEIGHT - 1, 0);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
  display.display();
}

void checkButtonLongPress() {
  int buttonState = digitalRead(BUTTON_PIN);
  unsigned long pressTime = 0;
  if (buttonState == LOW) {
    pressTime = millis();
    while (digitalRead(BUTTON_PIN) == LOW) {
      if (millis() - pressTime > 2000) {
        resetSession();
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Reset Complete");
        display.display();
        delay(1000);
        break;
      }
    }
  }
}

void updateSessionTime() {
  static unsigned long startTime = 0;
  if (sessionActive && startTime == 0) {
    startTime = millis();
  } else if (!sessionActive) {
    startTime = 0;
  }
  if (sessionActive) {
    unsigned long elapsed = (millis() - startTime) / 1000;
    display.setCursor(0, 60);
    display.print("Time: ");
    display.print(elapsed);
    display.println("s");
  }
}

void adjustBuzzerVolume() {
  int potValue = analogRead(A4);
  int volume = map(potValue, 0, 1023, 0, 255);
  analogWrite(BUZZER_PIN, volume);
}

void syncWithApp() {
  if (bluetooth.available()) {
    String data = bluetooth.readStringUntil('\n');
    if (data == "SYNC") {
      bluetooth.println("ACK");
      sendBluetoothData(angle, classifyExercise(maxAngle, avgAngle));
    }
  }
}
