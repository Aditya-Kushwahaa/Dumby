#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>


const unsigned char birdUp [] PROGMEM = {
  0b00011000,
  0b00111100,
  0b01111110,
  0b11111111,
  0b11100111,
  0b11111111,
  0b00111100,
  0b00011000
};

const unsigned char birdDown [] PROGMEM = {
  0b00011000,
  0b00111100,
  0b11111111,
  0b11111111,
  0b11100111,
  0b01111110,
  0b00111100,
  0b00011000
};

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define BUTTON_PIN D3
#define BUZZER_PIN D5

int score = 0;
int highScore = 0;

int menuIndex = 0;

const char* menuItems[] = {
  "Clock",
  "Weather",
  "Timer",
  "Flappy",
  "Mochi"
};

const unsigned char iconClock[] PROGMEM = {0x3C,0x42,0xA9,0x85,0x85,0xA9,0x42,0x3C};
const unsigned char iconWeather[] PROGMEM = {0x18,0x3C,0x3C,0x18,0x00,0x66,0x3C,0x18};
const unsigned char iconGame[] PROGMEM = {0x3C,0x7E,0xDB,0xFF,0xFF,0xDB,0x7E,0x3C};
const unsigned char iconMochi[] PROGMEM = {0x00,0x66,0x66,0x00,0x00,0x66,0x66,0x00};

const char* ssid = ""; // Wifi Name
const char* password = ""; // Wifi Password

// WEATHER
String weatherMain = "--";
float temperature = 0;
unsigned long lastWeatherUpdate = 0;

const char* apiKey = ""; // Paste Here api key

bool passedPipe = false;

enum Mode {
  BOOT,
  MENU,
  REAL_CLOCK,
  WEATHER,
  TIMER,
  FLAPPY,
  MOCHI
};

Mode currentMode = BOOT;

// BUTTON
bool lastButtonState = HIGH;
unsigned long pressStartTime = 0;
bool isHolding = false;
const int holdTime = 1000;

// TIME (SET THIS)
int setHour = 23;
int setMin = 29;
int setSec = 0;
unsigned long baseMillis;

// TIMER
bool timerRunning = false;
unsigned long remainingTime = 600; // default 10 min
unsigned long timerStart = 0;

int presetIndex = 0;
int presets[] = {600, 900, 1500}; // 10,15,25 min

int clickCount = 0;
unsigned long lastClickTime = 0;


// FLAPPY
float birdY = 30;
float velocity = 0;
int pipeX = 120;
int gapY = 30;
bool gameStarted = false;
bool gameOver = false;
unsigned long lastFrame = 0;

int wingFrame = 0;
unsigned long wingTimer = 0;

int groundX = 0;

int cloudX1 = 0;
int cloudX2 = 70;

bool isNight = false;
unsigned long dayTimer = 0;

int particleX[5];
int particleY[5];
bool showParticles = false;
unsigned long particleTimer = 0;

//Mochi
unsigned long lastPressTime = 0;
int pressCount = 0;

enum MochiState {
  NORMAL,
  LAUGH,
  CRY
};

MochiState mochiState = NORMAL;
unsigned long mochiAnimTimer = 0;

// SETUP
void setup() {
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println(WiFi.status());

  pinMode(BUZZER_PIN, OUTPUT);

  EEPROM.begin(512);
  highScore = EEPROM.read(0);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  baseMillis = millis();

  WiFi.begin(ssid, password);

display.clearDisplay();
display.setCursor(0, 0);
display.print("Connecting...");
display.display();

int retry = 0;

while (WiFi.status() != WL_CONNECTED && retry < 20) {
  delay(500);
  retry++;
}

display.clearDisplay();
display.setCursor(0, 20);

if (WiFi.status() == WL_CONNECTED) {
  display.print("WiFi OK");
} else {
  display.print("WiFi FAIL");
}

display.display();
delay(1000);

configTime(19800, 0, "pool.ntp.org", "time.nist.gov");

display.clearDisplay();
display.setCursor(0, 20);
display.print("Getting Time...");
display.display();

int retryTime = 0;

while (time(nullptr) < 1000 && retryTime < 40) {
  delay(500);
  retryTime++;

  Serial.println("Waiting for NTP...");
}
}

// LOOP
void loop() {
  runCurrentMode();
  handleButton();
  handleTimerClicks();
}

// BUTTON
void handleButton() {
  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW && lastButtonState == HIGH) {
    pressStartTime = millis();
    isHolding = false;
  }

  if (buttonState == LOW && millis() - pressStartTime > holdTime) {
    if (!isHolding) {
      isHolding = true;

      if (currentMode == MENU) {
        currentMode = (Mode)(menuIndex + 2);
      } else {
        currentMode = MENU;
      }
    }
  }

  if (buttonState == HIGH && lastButtonState == LOW) {
    if (!isHolding) onShortPress();
  }

  lastButtonState = buttonState;
}

// MODE SWITCH
void nextMode() {
  currentMode = (Mode)((currentMode + 1) % 4);

  if (currentMode == FLAPPY) {
    gameStarted = false;
    gameOver = false;
    birdY = 30;
    velocity = 0;
    pipeX = 120;
  }
}

// SHORT PRESS
void onShortPress() {

  if (currentMode == MENU) {
  menuIndex++;
  if (menuIndex > 4) menuIndex = 0;
  tone(BUZZER_PIN, 1500, 30);
}

  //Timer
  if (currentMode == TIMER) {

  unsigned long now = millis();

  if (now - lastClickTime < 400) {
    clickCount++;
  } else {
    clickCount = 1;
  }

  lastClickTime = now;
  }

  //Flappy
  if (currentMode == FLAPPY) {
    if (!gameStarted) {
      gameStarted = true;
    } else if (gameOver) {
      gameStarted = false;
      gameOver = false;
      birdY = 30;
      velocity = 0;
      pipeX = 120;
    } else {
      velocity = -4;
      jumpSound();
    }
  }

	if (currentMode == MOCHI) {
  unsigned long now = millis();

  if (now - lastPressTime < 400) {
    pressCount++;
  } else {
    pressCount = 1;
  }

  lastPressTime = now;

  if (pressCount == 1) {
    mochiState = LAUGH;
    mochiAnimTimer = millis();
  } 
  else if (pressCount == 2) {
    mochiState = CRY;
    mochiAnimTimer = millis();
    pressCount = 0;
  }
}
}

// MAIN DRAW
void runCurrentMode() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  switch (currentMode) {

  case BOOT: runBoot(); break;

  case MENU: runMenu(); break;

  case REAL_CLOCK: runRealClock(); break;

  case WEATHER: runWeather(); break;

  case TIMER: runTimer(); break;

  case FLAPPY: runFlappy(); break;

  case MOCHI: runMochi(); break;
}

  display.display();
}

// CLOCK
void runRealClock() {

  time_t now = time(nullptr);

  if (now < 1000) {
    display.setTextSize(1);
    display.setCursor(30, 25);
    display.print("Syncing...");
    return;
  }

  struct tm* t = localtime(&now);

  int hour = t->tm_hour;
  int min = t->tm_min;
  int sec = t->tm_sec;

  // -------- TOP (CHANNEL NAME) --------
  display.setTextSize(1);
  int textWidth = 6 * 7; // "CopyDIY" = 7 chars approx
  int xTop = (128 - textWidth) / 2;

  display.setCursor(xTop, 0);
  display.print("CopyDIY");

  // -------- TIME STRING --------
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", hour, min);

  // -------- CENTER ALIGN TIME --------
  display.setTextSize(3);

  int charWidth = 6 * 3; // approx width per char at size 3
  int totalWidth = charWidth * 5; // "HH:MM"

  int xCenter = (128 - totalWidth) / 2;
  int yCenter = 20;

  display.setCursor(xCenter, yCenter);

  // -------- BLINKING COLON --------
  if (sec % 2 == 0) {
    display.print(timeStr);
  } else {
    // hide colon
    char noColon[6];
    sprintf(noColon, "%02d %02d", hour, min);
    display.print(noColon);
  }

  // -------- DATE (BOTTOM) --------
  display.setTextSize(1);

  char dateStr[12];
  sprintf(dateStr, "%02d/%02d/%04d",
          t->tm_mday,
          t->tm_mon + 1,
          1900 + t->tm_year);

  int dateWidth = strlen(dateStr) * 6;
  int xDate = (128 - dateWidth) / 2;

  display.setCursor(xDate, 55);
  display.print(dateStr);
}

//Buzzer
void beep(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
}

void jumpSound() {
  tone(BUZZER_PIN, 1200, 40);
}

void scoreSound() {
  tone(BUZZER_PIN, 1500, 50);
  delay(10);
  tone(BUZZER_PIN, 1800, 50);
}

void gameOverSound() {
  tone(BUZZER_PIN, 500, 200);
  delay(50);
  tone(BUZZER_PIN, 300, 200);
}

// TIMER Function
void runTimer() {

  unsigned long currentTime;

  if (timerRunning) {
    unsigned long elapsed = (millis() - timerStart) / 1000;

    if (elapsed >= remainingTime) {
      currentTime = 0;
      timerRunning = false;

      // 🔊 BUZZER WHEN DONE
      tone(BUZZER_PIN, 1000, 500);
    } else {
      currentTime = remainingTime - elapsed;
    }

  } else {
    currentTime = remainingTime;
  }

  int minutes = currentTime / 60;
  int seconds = currentTime % 60;

  // -------- TITLE --------
  display.setTextSize(1);
  display.setCursor(25, 0);
  display.print("SMART TIMER");

  // -------- PRESET --------
  display.setCursor(40, 10);
  if (presetIndex == 0) display.print("10 MIN");
  if (presetIndex == 1) display.print("15 MIN");
  if (presetIndex == 2) display.print("25 MIN");

  // -------- BIG TIME --------
  display.setTextSize(3);

  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", minutes, seconds);

  int xCenter = (128 - (6 * 3 * 5)) / 2;

  display.setCursor(xCenter, 22);
  display.print(timeStr);

  // -------- STATUS --------
  display.setTextSize(1);
  display.setCursor(35, 55);

  if (currentTime == 0)
    display.print("Done!");
  else if (timerRunning)
    display.print("Running");
  else
    display.print("Paused");
}
void handleTimerClicks() {

  if (currentMode != TIMER) return;

  if (millis() - lastClickTime > 400 && clickCount > 0) {

    // SINGLE CLICK
    if (clickCount == 1) {

      timerRunning = !timerRunning;

      if (timerRunning) {
        timerStart = millis();
      } else {
        remainingTime -= (millis() - timerStart) / 1000;
      }

      tone(BUZZER_PIN, 1200, 40);
    }
    // DOUBLE CLICK (CHANGE PRESET)
    else if (clickCount == 2) {

      presetIndex++;
      if (presetIndex > 2) presetIndex = 0;

      remainingTime = presets[presetIndex];
      timerRunning = false;

      tone(BUZZER_PIN, 1500, 80);
    }

    clickCount = 0;
  }
}

//Flappy Function
void runFlappy() {

  // -------- START SCREEN --------
  // Day/Night toggle every 10 sec
  if (millis() - dayTimer > 10000) {
  dayTimer = millis();
  isNight = !isNight;
}

  if (!gameStarted) {
    score = 0;
    passedPipe = false;

    display.setTextSize(1);

    // Top Left
    display.setCursor(0, 0);
    display.print("CopyDIY");

    // Center
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print("Flappy");

    display.setCursor(20, 40);
    display.print("Bird");

    // Bottom
    display.setTextSize(1);
    display.setCursor(10, 55);
    display.print("Press to Play");

    return;
  }

  // -------- GAME OVER --------
  if (gameOver) {

  if (score > highScore) {
    highScore = score;
    EEPROM.write(0, highScore);
    EEPROM.commit();
  }

  display.setCursor(30, 15);
  display.print("GAME OVER");

  display.setCursor(20, 30);
  display.print("Score: ");
  display.print(score);

  display.setCursor(10, 45);
  display.print("High: ");
  display.print(highScore);

  return;
}

  // -------- GAME UPDATE --------
  if (millis() - lastFrame > 40) {
  lastFrame = millis();

  velocity += 0.5;   // gravity
  velocity *= 0.95;  // damping (smooth motion)
  birdY += velocity;

  if (birdY > 60) {
    gameOver = true;
    gameOverSound();
  }

  pipeX -= 2;

  // Pipe reset
  if (pipeX < 0) {
    pipeX = 128;
    gapY = random(18, 40);
    passedPipe = false;
  }

  // Score system
  if (pipeX < 20 && !passedPipe) {
    score++;
    passedPipe = true;
    scoreSound();
    showParticles = true;
    particleTimer = millis();

    for (int i = 0; i < 5; i++) {
    particleX[i] = 30;
    particleY[i] = birdY;
    }
  }

  // Collision
  if (pipeX < 25 && pipeX > 10) {
    if (birdY < gapY || birdY > gapY + 22) {
      gameOver = true;
      gameOverSound();
    }
  }
}

  // -------- DRAW --------
  // Wing animation
  if (millis() - wingTimer > 150) {
  wingTimer = millis();
  wingFrame = !wingFrame;
}

  if (wingFrame)
  display.drawBitmap(20, (int)birdY, birdUp, 8, 8, WHITE);
  else
  display.drawBitmap(20, (int)birdY, birdDown, 8, 8, WHITE);

  // Eye
  display.drawPixel(25, (int)birdY + 2, BLACK);

  if (showParticles) {
  for (int i = 0; i < 5; i++) {
    particleX[i] += random(1, 3);
    particleY[i] += random(-2, 3);
    display.drawPixel(particleX[i], particleY[i], WHITE);
  }

  if (millis() - particleTimer > 300) {
    showParticles = false;
    }
  }

  if (isNight) {
  // Stars
  display.drawPixel(10, 5, WHITE);
  display.drawPixel(50, 15, WHITE);
  display.drawPixel(90, 8, WHITE);
} else {
  // Sun
  display.drawCircle(110, 10, 5, WHITE);
}

  // Move clouds
  cloudX1 -= 1;
  cloudX2 -= 1;

  if (cloudX1 < -20) cloudX1 = 128;
  if (cloudX2 < -20) cloudX2 = 128;

  // Draw clouds
  display.fillCircle(cloudX1, 10, 4, WHITE);
  display.fillCircle(cloudX1 + 5, 10, 4, WHITE);

  display.fillCircle(cloudX2, 20, 3, WHITE);
  display.fillCircle(cloudX2 + 4, 20, 3, WHITE);

  int pipeWidth = 10;
  int capHeight = 4;

  // Top pipe
  display.fillRect(pipeX, 0, pipeWidth, gapY, WHITE);
  // Shadow
  display.drawLine(pipeX + pipeWidth - 2, 0, pipeX + pipeWidth - 2, gapY, BLACK);

  // Cap
  display.fillRect(pipeX - 2, gapY - capHeight, pipeWidth + 4, capHeight, WHITE);

  // Bottom pipe
  display.fillRect(pipeX, gapY + 22, pipeWidth, 64, WHITE);
  // Shadow
  display.drawLine(pipeX + pipeWidth - 2, gapY + 22, pipeX + pipeWidth - 2, 64, BLACK);

  // Cap
  display.fillRect(pipeX - 2, gapY + 22, pipeWidth + 4, capHeight, WHITE);

  // Move ground
  groundX -= 2;
  if (groundX < -10) groundX = 0;

  // Draw ground line
  for (int i = 0; i < 128; i += 10) {
  display.fillRect(i + groundX, 60, 8, 4, WHITE);
}

  // SCORE (PUT HERE)
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Score:");
  display.print(score);
}
// MOCHI
void runMochi() {
  static int frame = 0;
  frame++;

  int eyeY = 22;

  // Eye size
  int eyeW = 30;
  int eyeH = 18;

  // Blink effect
  bool isBlink = (frame % 50 < 5);

  // Reset animation after 2 sec
  if (millis() - mochiAnimTimer > 2000) {
    mochiState = NORMAL;
  }

  // -------- NORMAL (ANIME EYES) --------
  if (mochiState == NORMAL) {

    if (isBlink) {
      // Closed eyes
      display.drawLine(18, eyeY + 10, 48, eyeY + 10, WHITE);
      display.drawLine(80, eyeY + 10, 110, eyeY + 10, WHITE);
    } 
    else {
      // Outer eye
      display.fillRoundRect(18, eyeY, eyeW, eyeH, 8, WHITE);
      display.fillRoundRect(80, eyeY, eyeW, eyeH, 8, WHITE);

      // Inner pupil (black cut)
      display.fillCircle(33, eyeY + 10, 6, BLACK);
      display.fillCircle(95, eyeY + 10, 6, BLACK);

      // Sparkle
      display.fillCircle(30, eyeY + 6, 2, WHITE);
      display.fillCircle(92, eyeY + 6, 2, WHITE);
    }
  }

  // -------- LAUGH --------
  else if (mochiState == LAUGH) {
    display.drawLine(18, eyeY + 10, 48, eyeY + 10, WHITE);
    display.drawLine(80, eyeY + 10, 110, eyeY + 10, WHITE);

    // big smile
    display.drawCircle(64, 48, 12, WHITE);
  }

  // -------- CRY --------
  else if (mochiState == CRY) {
    display.fillRoundRect(18, eyeY, eyeW, 5, 5, WHITE);
    display.fillRoundRect(80, eyeY, eyeW, 5, 5, WHITE);

    // tears falling
    int tearY = 40 + (frame % 12);
    display.fillCircle(28, tearY, 3, WHITE);
    display.fillCircle(100, tearY, 3, WHITE);
  }
}
  //Boot
  void runBoot() {
  static int progress = 0;

  display.setTextSize(2);
  display.setCursor(20, 20);
  display.print("CopyDIY");

  display.drawRect(14, 50, 100, 8, WHITE);
  display.fillRect(16, 52, progress, 4, WHITE);

  progress += 2;

  if (progress > 96) {
    currentMode = MENU;
  }
  }

  //Menu
  void runMenu() {

  display.setTextSize(1);
  display.setCursor(40, 0);
  display.print("CopyDIY");

  for (int i = 0; i < 5; i++) {

    int y = 15 + i * 10;

    if (i == menuIndex) {
      display.fillRect(0, y - 2, 128, 10, WHITE);
      display.setTextColor(BLACK);
    } else {
      display.setTextColor(WHITE);
    }

    display.setCursor(20, y);

    switch(i) {
      case 0: display.drawBitmap(2, y, iconClock, 8, 8, WHITE); break;
      case 1: display.drawBitmap(2, y, iconWeather, 8, 8, WHITE); break;
      case 2: display.drawBitmap(2, y, iconClock, 8, 8, WHITE); break;
      case 3: display.drawBitmap(2, y, iconGame, 8, 8, WHITE); break;
      case 4: display.drawBitmap(2, y, iconMochi, 8, 8, WHITE); break;
    }

    display.print(menuItems[i]);
  }

  display.setTextColor(WHITE);
  }

  //Fetch Weather
  void fetchWeather() {
  if (WiFi.status() == WL_CONNECTED) {

    WiFiClient client;
    HTTPClient http;

    String url = "http://api.openweathermap.org/data/2.5/weather?q=Delhi&appid=" + String(apiKey) + "&units=metric";

    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();

      StaticJsonDocument<512> doc;
      deserializeJson(doc, payload);

      temperature = doc["main"]["temp"];
      weatherMain = doc["weather"][0]["main"].as<String>();
    }

    http.end();
  }
  }
  //Run Weather
  void runWeather() {

  if (millis() - lastWeatherUpdate > 600000) {
    fetchWeather();
    lastWeatherUpdate = millis();
  }

  display.setTextSize(1);
  display.setCursor(40, 0);
  display.print("CopyDIY");

  display.setTextSize(3);
  display.setCursor(20, 20);
  display.print(temperature);
  display.print("C");

  display.setTextSize(1);
  display.setCursor(30, 55);
  display.print(weatherMain);
}
