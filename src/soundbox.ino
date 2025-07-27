#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Audio.h"

// OLED-Display Konfiguration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SD-Karten-Pins
#define SD_CS 5

// Taster-Pins
#define BUTTON_NEXT 12
#define BUTTON_PREV 13
#define BUTTON_POWER 15  // Neuer Power-Taster

// Drehregler-Pins (Encoder)
#define ENCODER_CLK 14
#define ENCODER_DT 32
#define ENCODER_SW 33

// Audio-Konfiguration
Audio audio;
bool audioInitialized = false;

// Globale Variablen
String mp3Files[100];
int fileCount = 0;
int currentFileIndex = 0;
int volume = 8;  // Standardlautstärke (0-21)
bool isPlaying = true;
bool volumeChanged = false;
unsigned long volumeDisplayTime = 0;
bool systemOn = true;  // Systemzustand

// Encoder Variablen
volatile int encoderPos = 0;
int lastReportedPos = 0;

// Titel Scrolling Variablen
unsigned long lastScrollTime = 0;
int scrollPosition = 0;
const int scrollSpeed = 200;

void updateEncoder() {
  static uint8_t old_AB = 3;
  static int8_t encval = 0;
  static const int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};

  old_AB <<= 2;
  old_AB |= (digitalRead(ENCODER_CLK) << 1) | digitalRead(ENCODER_DT);
  encval += enc_states[(old_AB & 0x0f)];
  
  if (encval > 3) {
    encoderPos++;
    encval = 0;
  } else if (encval < -3) {
    encoderPos--;
    encval = 0;
  }
}

void setup() {
  Serial.begin(115200);

  // Hardware-Pins konfigurieren
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(BUTTON_PREV, INPUT_PULLUP);
  pinMode(BUTTON_POWER, INPUT_PULLUP);
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  // OLED initialisieren
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    while(true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("System starting..."));
  display.display();

  // SD-Karte initialisieren
  if(!SD.begin(SD_CS)) {
    Serial.println("SD Mount Failed!");
    displayError("SD Mount Failed!");
    return;
  }

  // MP3-Dateien laden
  listMP3Files(SD, "/");
  if(fileCount == 0) {
    displayError("No MP3 files found");
    return;
  }

  // Encoder-Interrupts
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), updateEncoder, CHANGE);

  // Audio initialisieren
  if(!audioInitialized) {
    audio.setPinout(26, 27, 25);
    audio.setVolume(volume);
    audioInitialized = true;
  }

  // Ersten Titel abspielen
  playMP3(currentFileIndex);
}

void loop() {
  // Power-Button überprüfen
  static unsigned long lastPowerCheck = 0;
  if(millis() - lastPowerCheck > 100) {
    lastPowerCheck = millis();
    checkPowerButton();
  }

  if(!systemOn) {
    // System ist ausgeschaltet
    delay(100);
    return;
  }

  // Normale Betriebsroutine
  audio.loop();
  checkEndOfTrack();
  handleButtons();
  handleVolumeControl();
  handlePlayPauseButton();
  updateDisplay();
}

void checkPowerButton() {
  static bool powerButtonActive = false;
  static unsigned long powerButtonTime = 0;
  
  if(digitalRead(BUTTON_POWER) == LOW) {
    if(!powerButtonActive) {
      powerButtonActive = true;
      powerButtonTime = millis();
    } else if(millis() - powerButtonTime > 1000) {
      // Langer Druck (1 Sekunde) zum Ein-/Ausschalten
      systemOn = !systemOn;
      powerButtonActive = false;
      
      if(systemOn) {
        // System einschalten
        display.clearDisplay();
        display.setCursor(0, 30);
        display.print(F("Powering ON..."));
        display.display();
        delay(500);
        playMP3(currentFileIndex);
      } else {
        // System ausschalten
        display.clearDisplay();
        display.setCursor(0, 30);
        display.print(F("Powering OFF..."));
        display.display();
        audio.stopSong();
        delay(500);
        display.clearDisplay();
        display.display();
      }
    }
  } else {
    powerButtonActive = false;
  }
}

void checkEndOfTrack() {
  static bool wasPlaying = false;
  
  if(audio.isRunning()) {
    wasPlaying = true;
  }
  else if(wasPlaying && isPlaying) {
    wasPlaying = false;
    nextTrack();
  }
}

void handleButtons() {
  // Nächster Titel
  if(digitalRead(BUTTON_NEXT) == LOW) {
    delay(200);
    nextTrack();
    while(digitalRead(BUTTON_NEXT) == LOW);
  }

  // Vorheriger Titel
  if(digitalRead(BUTTON_PREV) == LOW) {
    delay(200);
    previousTrack();
    while(digitalRead(BUTTON_PREV) == LOW);
  }
}

void handleVolumeControl() {
  if (encoderPos != lastReportedPos) {
    int change = (encoderPos - lastReportedPos);
    lastReportedPos = encoderPos;
    volume = constrain(volume + change, 0, 21);
    audio.setVolume(volume);
    volumeChanged = true;
    volumeDisplayTime = millis();
  }
}

void handlePlayPauseButton() {
  static unsigned long lastPressTime = 0;
  static bool buttonActive = false;
  
  if(digitalRead(ENCODER_SW) == LOW) {
    if(!buttonActive && (millis() - lastPressTime > 300)) {
      buttonActive = true;
      lastPressTime = millis();
      isPlaying = !isPlaying;
      
      display.clearDisplay();
      display.setCursor(0, 30);
      display.print(isPlaying ? F("Resuming...") : F("Pausing..."));
      display.display();
      
      if(isPlaying) {
        delay(50);
      }
      audio.pauseResume();
    }
  } else {
    buttonActive = false;
  }
}

void updateDisplay() {
  if(!systemOn) return;
  
  static unsigned long lastUpdate = 0;
  if(millis() - lastUpdate < 100) return;
  lastUpdate = millis();

  display.clearDisplay();
  
  // Titel-Anzeige
  display.setCursor(0, 0);
  display.print(F("Track: "));
  display.print(currentFileIndex + 1);
  display.print(F("/"));
  display.print(fileCount);

  // Status (Play/Pause)
  display.setCursor(70, 0);
  display.print(isPlaying ? F("[Playing]") : F("[Paused]"));

  // Titel (mit Scrolling)
  String title = mp3Files[currentFileIndex];
  title.replace(".mp3", "");
  if(title.length() > 16) {
    if(millis() - lastScrollTime > scrollSpeed) {
      lastScrollTime = millis();
      scrollPosition = (scrollPosition + 1) % (title.length() - 12);
    }
    title = title.substring(scrollPosition, scrollPosition + 16);
  }
  display.setCursor(0, 20);
  display.println(title);

  // Lautstärke oder Fortschritt anzeigen
  if(volumeChanged || (millis() - volumeDisplayTime < 2000)) {
    display.setCursor(0, 40);
    display.print(F("Volume: "));
    display.print(volume);
    display.print(F("/21"));
    
    int barWidth = map(volume, 0, 21, 0, 120);
    display.fillRect(0, 50, barWidth, 10, SSD1306_WHITE);
    display.drawRect(0, 50, 120, 10, SSD1306_WHITE);
    
    if(millis() - volumeDisplayTime >= 2000) {
      volumeChanged = false;
    }
  } else {
    // Zeit in Minuten:Sekunden anzeigen
    int currentTime = audio.getAudioCurrentTime();
    int totalTime = audio.getAudioFileDuration();
    
    display.setCursor(0, 40);
    display.print(F("Time: "));
    display.print(currentTime / 60);
    display.print(F(":"));
    if(currentTime % 60 < 10) display.print(F("0"));
    display.print(currentTime % 60);
    
    display.print(F("/"));
    display.print(totalTime / 60);
    display.print(F(":"));
    if(totalTime % 60 < 10) display.print(F("0"));
    display.print(totalTime % 60);
    
    float progress = (float)currentTime / totalTime;
    display.fillRect(0, 50, (int)(120 * progress), 10, SSD1306_WHITE);
    display.drawRect(0, 50, 120, 10, SSD1306_WHITE);
  }

  display.display();
}

void nextTrack() {
  if(fileCount == 0) return;
  currentFileIndex = (currentFileIndex + 1) % fileCount;
  playMP3(currentFileIndex);
}

void previousTrack() {
  if(fileCount == 0) return;
  currentFileIndex = (currentFileIndex - 1 + fileCount) % fileCount;
  playMP3(currentFileIndex);
}

void playMP3(int index) {
  if(index < 0 || index >= fileCount) return;
  
  audio.stopSong();
  delay(100);
  
  String filePath = "/" + mp3Files[index];
  if(!audio.connecttoFS(SD, filePath.c_str())) {
    Serial.println("Playback failed!");
    displayError("Playback error");
    return;
  }
  
  isPlaying = true;
  scrollPosition = 0;
  
  unsigned long timeout = millis() + 2000;
  while(!audio.isRunning() && millis() < timeout) {
    delay(10);
    audio.loop();
  }
  
  if(!audio.isRunning()) {
    Serial.println("Playback timeout!");
    displayError("Playback timeout");
  }
}

void listMP3Files(fs::FS &fs, const char *dirname) {
  File root = fs.open(dirname);
  if(!root || !root.isDirectory()) return;

  File file = root.openNextFile();
  while(file) {
    if(!file.isDirectory()) {
      String fileName = file.name();
      if(fileName.endsWith(".mp3") && !fileName.startsWith("._")) {
        mp3Files[fileCount++] = fileName;
        if(fileCount >= 100) break;
      }
    }
    file = root.openNextFile();
  }
}

void displayError(const char *message) {
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println(message);
  display.display();
}

void audio_info(const char *info) {
  Serial.print("info: "); Serial.println(info);
}
