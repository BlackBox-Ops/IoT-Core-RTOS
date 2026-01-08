/**
 * LedController
 * Mengelola perilaku LED dengan enkapsulasi logika Active LOW/HIGH.
 */
class LedController {
  private:
    int ledPin;
    bool isActiveLow;
    unsigned long interval;
    unsigned long prevTime;
    bool internalState;

  public:
    LedController(int pin, bool activeLow, int frequency) {
      this->ledPin = pin;
      this->isActiveLow = activeLow;
      this->interval = (frequency > 0) ? (500 / frequency) : 0;
      this->prevTime = 0;
      this->internalState = false;
    }

    void begin() {
      pinMode(this->ledPin, OUTPUT);
      this->turnOff();
    }

    void turnOn() {
      digitalWrite(this->ledPin, this->isActiveLow ? LOW : HIGH);
    }

    void turnOff() {
      digitalWrite(this->ledPin, this->isActiveLow ? HIGH : LOW);
    }

    void syncTime() {
      this->prevTime = millis();
    }

    void updateBlink() {
      unsigned long currentTime = millis();
      if (currentTime - this->prevTime >= this->interval) {
        this->prevTime = currentTime;
        this->internalState = !this->internalState;
        
        bool physicalSignal = this->isActiveLow ? 
                              (this->internalState ? LOW : HIGH) : 
                              (this->internalState ? HIGH : LOW);
        digitalWrite(this->ledPin, physicalSignal);
      }
    }
};

// ==========================================
// INSTANSIASI OBJEK
// ==========================================
LedController ledRed(13, true, 10);   
LedController ledGreen(12, false, 5);
LedController ledBlue(11, false, 2);

bool isBlinkMode = false;

void setup() {
  Serial.begin(9600);
  ledRed.begin();
  ledGreen.begin();
  ledBlue.begin();
  Serial.println(F("=== SISTEM LED: EXCLUSIVE STATIC & GLOBAL BLINK ==="));
}

void loop() {
  unsigned long currentMillis = millis();

  // --- DEMO SEQUENCE (Setiap 2 detik agar transisi terlihat jelas) ---
  static unsigned long lastChange = 0;
  static int scenario = 0;

  if (currentMillis - lastChange >= 2000) {
    lastChange = currentMillis;
    scenario++;
    if (scenario > 4) scenario = 0;

    Serial.print(F("\n[SCENARIO ")); Serial.print(scenario); Serial.print(F("]: "));

    switch (scenario) {
      case 0: // SEMUA OFF
        isBlinkMode = false;
        ledRed.turnOn(); ledGreen.turnOff(); ledBlue.turnOff();
        Serial.println(F("Semua LED OFF"));
        break;

      case 1: // HANYA RED ON
        isBlinkMode = false;
        ledRed.turnOff(); ledGreen.turnOff(); ledBlue.turnOff();
        Serial.println(F("Red ON (Green & Blue OFF)"));
        break;

      case 2: // HANYA GREEN ON
        isBlinkMode = false;
        ledRed.turnOn(); ledGreen.turnOn(); ledBlue.turnOff();
        Serial.println(F("Green ON (Red & Blue OFF)"));
        break;

      case 3: // HANYA BLUE ON
        isBlinkMode = false;
        ledRed.turnOn(); ledGreen.turnOff(); ledBlue.turnOn();
        Serial.println(F("Blue ON (Red & Green OFF)"));
        break;

      case 4: // MODE BLINK BERSAMAAN
        isBlinkMode = true;
        ledRed.syncTime(); ledGreen.syncTime(); ledBlue.syncTime();
        Serial.println(F("Mode BLINK Aktif (Semua Berkedip Independen)"));
        break;
    }
  }

  // --- EKSEKUSI LOGIKA ---
  if (isBlinkMode) {
    ledRed.updateBlink();
    ledGreen.updateBlink();
    ledBlue.updateBlink();
  }
}
