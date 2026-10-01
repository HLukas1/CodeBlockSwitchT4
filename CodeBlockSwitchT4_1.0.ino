#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>



// =====================================================
// ESP32-C3 / L9637D / KWP1281
//

#define KLINE_TX 10
#define KLINE_RX 1

#define MSG_ADDRESS 0x01

// =====================================================
// TESTBENCH-MODUS
// 0 = normaler Fahrzeugbetrieb
// 1 = Fahrzeugverbindung wird simuliert
// =====================================================
#define TESTBENCH_MODE 0 

HardwareSerial obd(1);

//Displayeinstellungen
#define OLED_WIDTH 64
#define OLED_HEIGHT 48
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

#define OLED_SDA 6
#define OLED_SCL 7

Adafruit_SSD1306 display(
  OLED_WIDTH,
  OLED_HEIGHT,
  &Wire,
  OLED_RESET);

//Taster
#define BUTTON_INPUT 9
#define BUTTON_OUTPUT 5
// Eingebaute LED des ESP32-C3 SuperMini
#define LED_PIN 8

const uint32_t BUTTON_LONG_PRESS_MS = 3000;

// =====================================================
// EINSTELLUNGEN
// =====================================================

uint32_t KLINE_BAUD = 9600;

// 5 Baud = 200 ms pro Bit
const uint32_t FIVE_BAUD_BIT_US = 200000UL;

// Busruhe vor neuer Adressierung
const uint32_t BUS_IDLE_TIME_MS = 300;

// Antwortabstand nach empfangenem Byte
const uint32_t TESTER_RESPONSE_DELAY_MS = 5;

// Abstand vor Komplement des zweiten Keybytes
const uint32_t KEYBYTE_COMPLEMENT_DELAY_MS = 30;

// Timeout innerhalb eines Blocks
const uint32_t INTERBYTE_TIMEOUT_MS = 55;

// Timeout zwischen zwei Blocks
const uint32_t INTERBLOCK_TIMEOUT_MS = 1100;

// Timeout waehrend Kommunikationsaufbau
const uint32_t INIT_TIMEOUT_MS = 250;

// Anzahl Verbindungsversuche
const uint8_t MAX_CONNECT_ATTEMPTS = 4;




// =====================================================
// GLOBALE VARIABLEN
// =====================================================

uint8_t blockCounter = 1;

uint16_t Coding = 0;
uint32_t WSC = 0;

uint8_t lastRpmRaw = 0;
uint16_t lastRpm = 0;
uint16_t lastBoostRaw = 0;
uint16_t lastBoostMbar = 0;
float injectionDriver = 0.0;
float injectionTorque = 0.0;
float injectionSmoke = 0.0;
float injectionActual = 0.0;

float currentSpeed = 0.0;
float fuelConsumption = 0.0;

bool kwpConnected = false;

bool codeblockMode = false;

// Aktuell im MSG vorhandene Codierung
uint16_t currentCodeblock = 0;



// Anzeige:
// false = Drehzahl
// true  = Ladedruck
bool showBoost = false;



// =====================================================
// TASTER - INTERRUPT
// =====================================================



// Von der ISR gesetzt: roher Pegel und Zeitpunkt der
// letzten Flanke. Keine Entprelllogik in der ISR.
volatile bool btnRawPressed = false;
volatile uint32_t btnRawChangeTime = 0;

// Vom Hauptprogramm verwaltet: entprellter Zustand
bool btnStablePressed = false;
uint32_t btnPressStart = 0;

const uint32_t BUTTON_DEBOUNCE_MS = 40;

// Messwertaktualisierung
uint32_t lastMeasurementTime = 0;
const uint32_t MEASUREMENT_INTERVAL_MS = 500;


// BETRIEBSMODI

enum DisplayMode {
  MODE_VOLTAGE,
  MODE_IDLE,
  MODE_CONSUMPTION,
  MODE_RPM,
  MODE_INJECTION,
  MODE_CODEBLOCK
};

DisplayMode displayMode = MODE_VOLTAGE;

// =====================================================
// VW STARTLOGO BITMAP 64x48
// =====================================================

const unsigned char vwLogo[] PROGMEM = {
  // 'Volkswagen-Logo-1967 (1)', 64x48px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xf0, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x1f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x03, 0xb0, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00, 0x03, 0x18, 0x19, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x06, 0x18, 0x30, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x18, 0x30, 0x60, 0x00, 0x00,
  0x00, 0x00, 0x0c, 0x0c, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x0c, 0x60, 0x70, 0x00, 0x00,
  0x00, 0x00, 0x1c, 0x06, 0x60, 0x70, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x06, 0x40, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x12, 0x07, 0xc0, 0xd8, 0x00, 0x00, 0x00, 0x00, 0x13, 0x03, 0xc0, 0x98, 0x00, 0x00,
  0x00, 0x00, 0x33, 0x03, 0x81, 0x98, 0x00, 0x00, 0x00, 0x00, 0x31, 0x80, 0x01, 0x98, 0x00, 0x00,
  0x00, 0x00, 0x31, 0x80, 0x03, 0x08, 0x00, 0x00, 0x00, 0x00, 0x31, 0x83, 0x83, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x10, 0xc3, 0x83, 0x18, 0x00, 0x00, 0x00, 0x00, 0x10, 0xc7, 0xc6, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x18, 0xc6, 0xc6, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x66, 0x64, 0x10, 0x00, 0x00,
  0x00, 0x00, 0x18, 0x6c, 0x6c, 0x30, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x3c, 0x7c, 0x20, 0x00, 0x00,
  0x00, 0x00, 0x0c, 0x3c, 0x38, 0x60, 0x00, 0x00, 0x00, 0x00, 0x06, 0x38, 0x38, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x03, 0x18, 0x39, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x90, 0x13, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x01, 0xf0, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x1e, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};





// =====================================================
// FUNKTIONSPROTOTYPEN
// =====================================================

void printHex(uint8_t value);
void waitUntilMicros(uint32_t targetTime);

void stopKLine(bool waitForBusIdle);
void startKLine();
void clearKLineInput();

bool obdReadByte(uint8_t &value, uint32_t timeoutMs);
void obdWriteByte(uint8_t value);

void send5BaudAddress(uint8_t address);
bool readInitialResponse();

bool kwpReceiveBlock(
  uint8_t *buffer,
  uint16_t maxSize,
  uint16_t &size);

bool kwpSendBlockOnce(
  const uint8_t *buffer,
  uint16_t size);

bool kwpSendBlock(
  const uint8_t *buffer,
  uint16_t size);

bool kwpSendAck();
bool kwpSendEnd();

bool parseCodingBlock(
  const uint8_t *buffer,
  uint16_t size);

bool readIdentificationBlocks();

bool connectMSGOnce();
bool connectMSG();
void disconnectMSG();

bool sendCoding(
  uint16_t requestedCoding,
  uint32_t requestedWSC);

bool readMeasurementBlock000(bool printAllFields);
bool readMeasurementBlock001(bool printAllFields);
bool readMeasurementBlock006(bool printAllFields);
bool readMeasurementBlock011(bool printAllFields);
bool readMeasurementBlock008(bool printAllFields);

void calculateFuelConsumption();

void clearSerialInput();

bool readSerialNumber(
  uint32_t &value,
  uint32_t timeoutMs,
  bool allowEmpty,
  uint32_t defaultValue);

int16_t getCenteredX(String text) {
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(
    text,
    0,
    0,
    &x1,
    &y1,
    &w,
    &h);

  return (OLED_WIDTH - w) / 2;
}
// =====================================================
// HEX-AUSGABE
// =====================================================

void printHex(uint8_t value) {
  if (value < 0x10) {
    Serial.print("0");
  }

  Serial.print(value, HEX);
}


// =====================================================
// ZEITGENAUES WARTEN
// =====================================================

void waitUntilMicros(uint32_t targetTime) {
  while ((int32_t)(targetTime - micros()) > 0) {
    delayMicroseconds(50);
  }
}


// =====================================================
// K-LINE STOPPEN
// =====================================================

void stopKLine(bool waitForBusIdle) {
  obd.end();

  pinMode(KLINE_TX, OUTPUT);
  digitalWrite(KLINE_TX, HIGH);

  pinMode(KLINE_RX, INPUT);

  if (waitForBusIdle) {
    delay(BUS_IDLE_TIME_MS);
  }
}


// =====================================================
// K-LINE UART STARTEN
// =====================================================

void startKLine() {
  obd.end();

  obd.begin(
    KLINE_BAUD,
    SERIAL_8N1,
    KLINE_RX,
    KLINE_TX);
}


// =====================================================
// K-LINE-EINGANGSPUFFER LEEREN
// =====================================================

void clearKLineInput() {
  while (obd.available()) {
    obd.read();
  }
}




// =====================================================
// BYTE VON K-LINE LESEN
// =====================================================

bool obdReadByte(
  uint8_t &value,
  uint32_t timeoutMs) {
  uint32_t startTime = millis();

  while (!obd.available()) {
    if ((uint32_t)(millis() - startTime) >= timeoutMs) {
      return false;
    }

    delay(1);
  }

  value = (uint8_t)obd.read();

  return true;
}


// =====================================================
// BYTE AUF K-LINE SENDEN
// =====================================================

void obdWriteByte(uint8_t value) {
  obd.write(value);
  obd.flush();

  // Laut KW1281-Protokoll wird jedes vom µC gesendete
  // Byte vom Bus als Echo zurückgespiegelt, noch bevor
  // die eigentliche Antwort des Steuergeraets kommt.
  // Dieses Echo muss verworfen werden.
  uint8_t echo = 0;

  if (!obdReadByte(echo, INTERBYTE_TIMEOUT_MS)) {
    Serial.println("WARNUNG: Kein Echo auf gesendetes Byte.");
  } else if (echo != value) {
    Serial.print("WARNUNG: Echo 0x");
    printHex(echo);
    Serial.print(" weicht von gesendetem Byte 0x");
    printHex(value);
    Serial.println(" ab.");
  }
}


// =====================================================
// 5-BAUD-ADRESSE SENDEN
//
// 1 Startbit Low
// 7 Datenbits LSB zuerst
// 1 Bit ungerade Paritaet
// 1 Stopbit High
// =====================================================

void send5BaudAddress(uint8_t address) {
  Serial.println();
  Serial.println("========================================");
  Serial.println("       5-BAUD INITIALISIERUNG");
  Serial.println("========================================");

  stopKLine(true);

  uint8_t bits[10];

  // Startbit
  bits[0] = 0;

  uint8_t parity = 0;

  // Sieben Datenbits, LSB zuerst
  for (uint8_t i = 0; i < 7; i++) {
    uint8_t currentBit =
      (address >> i) & 0x01;

    bits[i + 1] = currentBit;

    parity ^= currentBit;
  }

  // Ungerade Paritaet
  bits[8] = parity ? 0 : 1;

  // Stopbit
  bits[9] = 1;

  Serial.print("Adresse: 0x");
  printHex(address);
  Serial.println();

  Serial.print("Bitfolge: ");

  for (uint8_t i = 0; i < 10; i++) {
    Serial.print(bits[i]);
  }

  Serial.println();

  uint32_t nextBitTime = micros();

  for (uint8_t i = 0; i < 10; i++) {
    digitalWrite(
      KLINE_TX,
      bits[i] ? HIGH : LOW);

    nextBitTime += FIVE_BAUD_BIT_US;

    waitUntilMicros(nextBitTime);
  }

  digitalWrite(KLINE_TX, HIGH);

  Serial.println("5-Baud-Adresse gesendet.");
}


// =====================================================
// INITIALANTWORT LESEN
//
// Erwartet:
//   55 01 8A
//
// Danach sendet der Tester:
//   75 = 8A XOR FF
// =====================================================

bool readInitialResponse() {
  Serial.println();
  Serial.println("----------------------------------------");
  Serial.println("        WARTE AUF 55 01 8A");
  Serial.println("----------------------------------------");

  uint8_t syncByte = 0;
  uint8_t keyByte1 = 0;
  uint8_t keyByte2 = 0;

  if (!obdReadByte(syncByte, INIT_TIMEOUT_MS)) {
    Serial.println(
      "FEHLER: Kein Synchronisationsbyte.");

    return false;
  }

  Serial.print("SYNC: 0x");
  printHex(syncByte);
  Serial.println();

  if (syncByte != 0x55) {
    Serial.println(
      "FEHLER: Synchronisationsbyte ist nicht 0x55.");

    return false;
  }

  if (!obdReadByte(keyByte1, INIT_TIMEOUT_MS)) {
    Serial.println("FEHLER: Keybyte 1 fehlt.");

    return false;
  }

  Serial.print("KB1: 0x");
  printHex(keyByte1);
  Serial.println();

  if (!obdReadByte(keyByte2, INIT_TIMEOUT_MS)) {
    Serial.println("FEHLER: Keybyte 2 fehlt.");

    return false;
  }

  Serial.print("KB2: 0x");
  printHex(keyByte2);
  Serial.println();

  if (
    keyByte1 != 0x01 || keyByte2 != 0x8A) {
    Serial.println(
      "FEHLER: Erwartetes Keyword 01 8A fehlt.");

    return false;
  }

  delay(KEYBYTE_COMPLEMENT_DELAY_MS);

  uint8_t complement =
    keyByte2 ^ 0xFF;

  Serial.print("TX KB2-Komplement: 0x");
  printHex(complement);
  Serial.println();

  obdWriteByte(complement);

  // Der erste Datenblock hat Blockzaehler 1.
  blockCounter = 1;

  return true;
}


// =====================================================
// KWP1281-BLOCK EMPFANGEN
// =====================================================

bool kwpReceiveBlock(
  uint8_t *buffer,
  uint16_t maxSize,
  uint16_t &size) {
  size = 0;

  uint8_t value = 0;

  Serial.println();
  Serial.print("Warte auf Block ");
  Serial.println(blockCounter);

  // Erstes Byte mit Interblock-Timeout
  if (!obdReadByte(
        value,
        INTERBLOCK_TIMEOUT_MS)) {
    Serial.println("FEHLER: Interblock-Timeout.");

    return false;
  }

  size = (uint16_t)value + 1U;

  if (size < 4) {
    Serial.println("FEHLER: Block zu klein.");

    return false;
  }

  if (size > maxSize) {
    Serial.println("FEHLER: Block zu gross.");

    return false;
  }

  buffer[0] = value;

  // Blocklaenge komplementieren
  delay(TESTER_RESPONSE_DELAY_MS);
  obdWriteByte(value ^ 0xFF);

  for (uint16_t index = 1; index < size; index++) {
    if (!obdReadByte(
          value,
          INTERBYTE_TIMEOUT_MS)) {
      Serial.print(
        "FEHLER: Interbyte-Timeout bei Index ");

      Serial.println(index);

      return false;
    }

    buffer[index] = value;

    // Das letzte Byte ETX wird nicht komplementiert.
    if (index < size - 1) {
      delay(TESTER_RESPONSE_DELAY_MS);
      obdWriteByte(value ^ 0xFF);
    }
  }

  if (buffer[size - 1] != 0x03) {
    Serial.print("FEHLER: Ungueltiges ETX 0x");
    printHex(buffer[size - 1]);
    Serial.println();

    return false;
  }

  if (buffer[1] != blockCounter) {
    Serial.print(
      "FEHLER: Blockzaehler empfangen 0x");

    printHex(buffer[1]);

    Serial.print(", erwartet 0x");
    printHex(blockCounter);

    Serial.println();

    return false;
  }

  Serial.print("RX BLOCK: ");

  for (uint16_t i = 0; i < size; i++) {
    printHex(buffer[i]);
    Serial.print(" ");
  }

  Serial.println();

  blockCounter++;

  return true;
}


// =====================================================
// KWP1281-BLOCK EINMAL SENDEN
// =====================================================

bool kwpSendBlockOnce(
  const uint8_t *buffer,
  uint16_t size) {
  if (size < 4) {
    Serial.println("FEHLER: TX-Block zu klein.");

    return false;
  }

  if (buffer[1] != blockCounter) {
    Serial.print(
      "FEHLER: TX-Blockzaehler 0x");

    printHex(buffer[1]);

    Serial.print(", erwartet 0x");
    printHex(blockCounter);

    Serial.println();

    return false;
  }

  if (buffer[size - 1] != 0x03) {
    Serial.println("FEHLER: TX-Block ohne ETX.");

    return false;
  }

  Serial.print("TX BLOCK: ");

  for (uint16_t i = 0; i < size; i++) {
    printHex(buffer[i]);
    Serial.print(" ");
  }

  Serial.println();

  for (uint16_t index = 0; index < size; index++) {
    delay(TESTER_RESPONSE_DELAY_MS);

    uint8_t sentByte = buffer[index];

    obdWriteByte(sentByte);

    // Kein Komplement fuer ETX
    if (index == size - 1) {
      continue;
    }

    uint8_t receivedComplement = 0;

    if (!obdReadByte(
          receivedComplement,
          INTERBYTE_TIMEOUT_MS)) {
      Serial.print(
        "FEHLER: Kein Komplement fuer 0x");

      printHex(sentByte);
      Serial.println();

      return false;
    }

    uint8_t expectedComplement =
      sentByte ^ 0xFF;

    if (receivedComplement != expectedComplement) {
      Serial.print(
        "FEHLER: Komplement empfangen 0x");

      printHex(receivedComplement);

      Serial.print(", erwartet 0x");
      printHex(expectedComplement);

      Serial.println();

      return false;
    }
  }

  return true;
}


// =====================================================
// KWP1281-BLOCK MIT WIEDERHOLUNG SENDEN
// =====================================================

bool kwpSendBlock(
  const uint8_t *buffer,
  uint16_t size) {
  const uint8_t maxAttempts = 3;

  for (
    uint8_t attempt = 1;
    attempt <= maxAttempts;
    attempt++) {
    if (kwpSendBlockOnce(buffer, size)) {
      blockCounter++;

      return true;
    }

    if (attempt < maxAttempts) {
      Serial.print("Blockwiederholung ");
      Serial.print(attempt + 1);
      Serial.print(" von ");
      Serial.println(maxAttempts);

      delay(INTERBYTE_TIMEOUT_MS * 2UL);

      clearKLineInput();
    }
  }

  return false;
}


// =====================================================
// B01 ACKNOWLEDGE
//
// 03 ZZ 09 03
// =====================================================

bool kwpSendAck() {
  uint8_t block[4];

  block[0] = 0x03;
  block[1] = blockCounter;
  block[2] = 0x09;
  block[3] = 0x03;

  return kwpSendBlock(
    block,
    sizeof(block));
}


// =====================================================
// B03 AUSGABE-ENDE
//
// 03 ZZ 06 03
// =====================================================

bool kwpSendEnd() {
  uint8_t block[4];

  block[0] = 0x03;
  block[1] = blockCounter;
  block[2] = 0x06;
  block[3] = 0x03;

  Serial.println();
  Serial.println("Sende B03 Ausgabe-Ende.");

  bool result =
    kwpSendBlock(
      block,
      sizeof(block));

  kwpConnected = false;

  return result;
}


// =====================================================
// CODIERUNGSBLOCK AUSWERTEN
//
// Empfang:
//   08 ZZ F6 00 PP0 PP1 PP2 PP3 03
//
// Coding:
//   Coding = (PP0 PP1) / 2
//
// WSC:
//   PP1 Bit 0 = WSC16
//   PP2       = WSC15 bis WSC08
//   PP3       = WSC07 bis WSC00
// =====================================================

bool parseCodingBlock(
  const uint8_t *buffer,
  uint16_t size) {
  if (size != 9) {
    return false;
  }

  if (
    buffer[2] != 0xF6 || buffer[3] != 0x00 || buffer[8] != 0x03) {
    return false;
  }

  uint8_t pp0 = buffer[4];
  uint8_t pp1 = buffer[5];
  uint8_t pp2 = buffer[6];
  uint8_t pp3 = buffer[7];

  uint16_t codingRaw =
    ((uint16_t)pp0 << 8) | (uint16_t)pp1;

  Coding = codingRaw >> 1;

  WSC =
    ((uint32_t)(pp1 & 0x01) << 16) | ((uint32_t)pp2 << 8) | (uint32_t)pp3;

  Serial.println();
  Serial.println("************************************");
  Serial.println("        CODIERUNG EMPFANGEN");
  Serial.println("************************************");

  Serial.print("PP0 = 0x");
  printHex(pp0);

  Serial.print("  PP1 = 0x");
  printHex(pp1);

  Serial.print("  PP2 = 0x");
  printHex(pp2);

  Serial.print("  PP3 = 0x");
  printHex(pp3);

  Serial.println();

  Serial.print("Codierung = ");
  Serial.println(Coding);

  Serial.print("WSC       = ");
  Serial.println(WSC);

  Serial.println("************************************");

  return true;
}


// =====================================================
// IDENTIFIKATIONSBLOECKE LESEN
//
// Nach abschliessendem B01 bleibt die Verbindung offen.
// Der Tester besitzt dann die Master-Funktion.
// =====================================================

bool readIdentificationBlocks() {
  String identificationText = "";

  uint8_t buffer[256];
  uint16_t size = 0;

  uint8_t safetyCounter = 0;

  while (safetyCounter < 40) {
    safetyCounter++;

    if (!kwpReceiveBlock(
          buffer,
          sizeof(buffer),
          size)) {
      return false;
    }

    uint8_t blockTitle = buffer[2];

    if (blockTitle == 0xF6) {
      if (!parseCodingBlock(buffer, size)) {
        for (
          uint16_t i = 3;
          i < size - 1;
          i++) {
          uint8_t data = buffer[i];

          if (
            data >= 0x20 && data <= 0x7E) {
            identificationText +=
              (char)data;
          }
        }
      }

      if (!kwpSendAck()) {
        Serial.println(
          "FEHLER: B01 konnte nicht gesendet werden.");

        return false;
      }

      continue;
    }

    if (blockTitle == 0x09) {
      Serial.println();
      Serial.println(
        "B01 vom Steuergeraet empfangen.");

      Serial.println(
        "Tester besitzt jetzt die Master-Funktion.");

      if (identificationText.length() > 0) {
        Serial.print("MSG-Identifikation: ");
        Serial.println(identificationText);
      }

      return true;
    }

    if (blockTitle == 0x06) {
      Serial.println(
        "B03 vom Steuergeraet empfangen.");

      kwpConnected = false;

      return false;
    }

    if (blockTitle == 0x0A) {
      Serial.println(
        "FEHLER: No-Acknowledge empfangen.");

      return false;
    }

    Serial.print(
      "FEHLER: Unerwarteter Blocktitel 0x");

    printHex(blockTitle);
    Serial.println();

    return false;
  }

  Serial.println(
    "FEHLER: Zu viele Identifikationsbloecke.");

  return false;
}


// =====================================================
// EIN VERBINDUNGSVERSUCH
// =====================================================

bool connectMSGOnce() {
  kwpConnected = false;

  blockCounter = 1;
  Coding = 0;
  WSC = 0;

  send5BaudAddress(MSG_ADDRESS);

  // UART unmittelbar nach 5-Baud-Adresswort starten.
  startKLine();

  // Eingangspuffer hier nicht leeren.
  // Sonst kann 0x55 verloren gehen.

  if (!readInitialResponse()) {
    stopKLine(false);

    return false;
  }

  kwpConnected = true;

  if (!readIdentificationBlocks()) {
    kwpConnected = false;

    stopKLine(false);

    return false;
  }

  return true;
}


// =====================================================
// VERBINDEN MIT WIEDERHOLUNG
// =====================================================

bool connectMSG() {

#if TESTBENCH_MODE
  kwpConnected = true;
  Coding = 2;
  WSC = 0;

  Serial.println("TESTBENCH: Fahrzeugverbindung simuliert.");

  return true;
#endif

  if (kwpConnected) {
    return true;
  }
  for (
    uint8_t attempt = 1;
    attempt <= MAX_CONNECT_ATTEMPTS;
    attempt++) {
    Serial.println();
    Serial.print("Verbindungsversuch ");
    Serial.print(attempt);
    Serial.print(" von ");
    Serial.println(MAX_CONNECT_ATTEMPTS);

    if (connectMSGOnce()) {
      Serial.println();
      Serial.println("MSG verbunden.");

      Serial.print("Aktuelle Codierung: ");
      Serial.println(Coding);

      Serial.print("Aktueller WSC: ");
      Serial.println(WSC);

      return true;
    }

    Serial.println(
      "Verbindungsversuch fehlgeschlagen.");

    stopKLine(true);
  }

  Serial.println();
  Serial.println(
    "FEHLER: Keine Verbindung zum MSG.");

  return false;
}


// =====================================================
// VERBINDUNG BEENDEN
// =====================================================

void disconnectMSG() {

  if (kwpConnected) {

    if (!kwpSendEnd()) {

      Serial.println(
        "WARNUNG: B03 konnte nicht gesendet werden.");

      // Verbindung trotzdem hart beenden
      stopKLine(false);
      kwpConnected = false;

      return;
    }
  }

  stopKLine(false);

  kwpConnected = false;
}


// =====================================================
// B13 CODIERUNG SENDEN
//
// 07 ZZ 10 PP0 PP1 PP2 PP3 03
// =====================================================

bool sendCoding(
  uint16_t requestedCoding,
  uint32_t requestedWSC) {

#if TESTBENCH_MODE

  Serial.println("TESTBENCH: Codeblock-Schreiben simuliert.");

  Coding = requestedCoding;
  WSC = requestedWSC;

  return true;

#endif

  if (!kwpConnected) {
    Serial.println(
      "FEHLER: Keine aktive Verbindung.");

    return false;
  }

  if (requestedCoding > 32767) {
    Serial.println(
      "FEHLER: Codierung muss 0 bis 32767 sein.");

    return false;
  }

  if (requestedWSC > 131071UL) {
    Serial.println(
      "FEHLER: WSC muss 0 bis 131071 sein.");

    return false;
  }

  uint16_t codingRaw =
    (uint16_t)(requestedCoding << 1);

  uint8_t pp0 =
    (uint8_t)((codingRaw >> 8) & 0xFF);

  uint8_t pp1 =
    (uint8_t)(codingRaw & 0xFE);

  if (requestedWSC & 0x10000UL) {
    pp1 |= 0x01;
  }

  uint8_t pp2 =
    (uint8_t)((requestedWSC >> 8) & 0xFF);

  uint8_t pp3 =
    (uint8_t)(requestedWSC & 0xFF);

  uint8_t block[8];

  block[0] = 0x07;
  block[1] = blockCounter;
  block[2] = 0x10;
  block[3] = pp0;
  block[4] = pp1;
  block[5] = pp2;
  block[6] = pp3;
  block[7] = 0x03;

  Serial.println();
  Serial.println("========================================");
  Serial.println("       SENDE NEUE CODIERUNG");
  Serial.println("========================================");

  Serial.print("Neue Codierung: ");
  Serial.println(requestedCoding);

  Serial.print("Neuer WSC: ");
  Serial.println(requestedWSC);

  if (!kwpSendBlock(
        block,
        sizeof(block))) {
    Serial.println(
      "FEHLER: B13 konnte nicht gesendet werden.");

    return false;
  }

  // Nach B13 sendet das MSG erneut
  // die Identifikationsbloecke.
  if (!readIdentificationBlocks()) {
    Serial.println(
      "FEHLER: Keine Identifikation nach B13.");

    return false;
  }

  if (Coding != requestedCoding) {
    Serial.println();
    Serial.println(
      "FEHLER: Codierung wurde nicht uebernommen.");

    Serial.print("Angefordert: ");
    Serial.println(requestedCoding);

    Serial.print("Zurueckgelesen: ");
    Serial.println(Coding);

    return false;
  }

  Serial.println();
  Serial.println(
    "Codierung erfolgreich uebernommen.");

  Serial.print("Zurueckgelesene Codierung: ");
  Serial.println(Coding);

  Serial.print("Zurueckgelesener WSC: ");
  Serial.println(WSC);

  return true;
}


// =====================================================
// MESSWERTBLOCK 000 LESEN
//
// KWP1281 B10:
//   03 ZZ 12 03
//
// Kein Gruppenbyte mitsenden.
//
// Antwort:
//   N-1 ZZ F4 Feld1 ... Feld10 03
// =====================================================

bool readMeasurementBlock000(bool printAllFields) {
  if (!kwpConnected) {
    Serial.println(
      "FEHLER: Keine aktive Verbindung.");

    return false;
  }

  uint8_t request[4];

  request[0] = 0x03;
  request[1] = blockCounter;
  request[2] = 0x12;
  request[3] = 0x03;

  if (printAllFields) {
    Serial.println();
    Serial.println("========================================");
    Serial.println("         LESE MWB 000");
    Serial.println("========================================");
  }

  if (!kwpSendBlock(
        request,
        sizeof(request))) {
    Serial.println(
      "FEHLER: B10 konnte nicht gesendet werden.");

    return false;
  }

  uint8_t response[64];
  uint16_t responseSize = 0;

  if (!kwpReceiveBlock(
        response,
        sizeof(response),
        responseSize)) {
    Serial.println(
      "FEHLER: Keine Antwort auf B10.");

    return false;
  }

  if (responseSize < 5) {
    Serial.println(
      "FEHLER: F4-Antwort ist zu kurz.");

    return false;
  }

  if (response[2] == 0x0A) {
    Serial.println(
      "FEHLER: MSG meldet No-Acknowledge.");

    return false;
  }

  if (response[2] != 0xF4) {
    Serial.print(
      "FEHLER: Antworttitel ist 0x");

    printHex(response[2]);

    Serial.println(", erwartet wurde 0xF4.");

    return false;
  }

  uint16_t fieldCount =
    responseSize - 4U;

  if (fieldCount == 0) {
    Serial.println(
      "FEHLER: F4 enthaelt keine Messwerte.");

    return false;
  }

  // Feld 1 liegt direkt hinter F4.
  lastRpmRaw = response[3];

lastRpm = (uint16_t)lastRpmRaw;

  if (printAllFields) {
    Serial.println();
    Serial.print("MWB000 RAW: ");

    for (
      uint16_t i = 0;
      i < responseSize;
      i++) {
      printHex(response[i]);
      Serial.print(" ");
    }

    Serial.println();

    Serial.println();
    Serial.print("Anzahl Messwertfelder: ");
    Serial.println(fieldCount);

    for (
      uint16_t field = 0;
      field < fieldCount;
      field++) {
      Serial.print("Feld ");
      Serial.print(field + 1);
      Serial.print(" = ");

      Serial.print(response[3 + field]);

      Serial.print("  [0x");
      printHex(response[3 + field]);
      Serial.println("]");
    }
  }

  Serial.println();
  Serial.print("Drehzahl Rohwert: ");
  Serial.println(lastRpmRaw);

  Serial.print("Drehzahl berechnet: ");
  Serial.print(lastRpm);
  Serial.println(" U/min");

  return true;
}

// =====================================================
// KWP1281 MESSWERT UMRECHNUNG
// =====================================================

float decodeKwpValue(uint8_t formula, uint8_t a, uint8_t b) {
  switch (formula) {
    // Kennzahl 0x12 = 18
    // Ladedruck / Druck
    // Formel: 0,04 × a × b
    case 0x12:
      return 0.04f * (float)a * (float)b;

    // Kennzahl 0x27 = 39
    // Einspritzmenge
    // Formel: b / 256 × a
    case 0x27:
      return ((float)b / 256.0f) * (float)a;

    // Noch unbekannte Formel
    default:
      return 0.0f;
  }
}

// =====================================================
// MESSWERTBLOCK 011 LESEN
//
// KWP1281 Group Reading:
//
//   04 ZZ 29 0B 03
//
// Antwort:
//
//   LL ZZ E7
//   Formel Wert_H Wert_L
//   Formel Wert_H Wert_L
//   Formel Wert_H Wert_L
//   Formel Wert_H Wert_L
//   03
//
// MWB 011:
//
//   Feld 1 = Motordrehzahl
//   Feld 2 = Ladedruck Soll
//   Feld 3 = Ladedruck Ist
//   Feld 4 = Regelung
// =====================================================

bool readMeasurementBlock011(bool printAllFields) {
  if (!kwpConnected) {
    Serial.println("FEHLER: Keine aktive Verbindung.");
    return false;
  }

  uint8_t request[5];

  request[0] = 0x04;
  request[1] = blockCounter;
  request[2] = 0x29;
  request[3] = 0x0B;
  request[4] = 0x03;

  if (printAllFields) {
    Serial.println();
    Serial.println("========================================");
    Serial.println("         LESE MWB 011");
    Serial.println("========================================");
  }

  if (!kwpSendBlock(
        request,
        sizeof(request))) {
    Serial.println(
      "FEHLER: Group Reading MWB 011 konnte nicht gesendet werden.");

    return false;
  }

  uint8_t response[64];
  uint16_t responseSize = 0;

  if (!kwpReceiveBlock(
        response,
        sizeof(response),
        responseSize)) {
    Serial.println(
      "FEHLER: Keine Antwort auf MWB 011.");

    return false;
  }

  if (responseSize < 6) {
    Serial.println(
      "FEHLER: MWB 011 Antwort ist zu kurz.");

    return false;
  }

  if (response[2] == 0x0A) {
    Serial.println(
      "FEHLER: MSG meldet No-Acknowledge.");

    return false;
  }

  if (response[2] != 0xE7) {
    Serial.print(
      "FEHLER: Antworttitel ist 0x");

    printHex(response[2]);

    Serial.println(
      ", erwartet wurde 0xE7.");

    return false;
  }

  if (response[responseSize - 1] != 0x03) {
    Serial.println(
      "FEHLER: MWB 011 Antwort ohne ETX.");

    return false;
  }

  // Anzahl Messwertfelder
  uint16_t dataBytes = responseSize - 4;

  if ((dataBytes % 3) != 0) {
    Serial.println(
      "FEHLER: MWB 011 Datenformat ist ungueltig.");

    return false;
  }

  uint16_t fieldCount = dataBytes / 3;

  if (printAllFields) {
    Serial.print("Anzahl Messwertfelder: ");
    Serial.println(fieldCount);

    Serial.println();
    Serial.print("MWB011 RAW: ");

    for (uint16_t i = 0; i < responseSize; i++) {
      printHex(response[i]);
      Serial.print(" ");
    }

    Serial.println();
  }

  // Bis zu 4 Felder auswerten
  for (uint16_t field = 0; field < fieldCount && field < 4; field++) {
    uint16_t pos = 3 + (field * 3);

    uint8_t formula = response[pos];

    uint8_t a = response[pos + 1];
    uint8_t b = response[pos + 2];

    float value = decodeKwpValue(formula, a, b);

    if (printAllFields) {
      Serial.print("Feld ");
      Serial.print(field + 1);

      Serial.print("  Formel=0x");
      printHex(formula);

      Serial.print("  RAW=0x");
      printHex(response[pos + 1]);
      printHex(response[pos + 2]);

      Serial.print("  Wert=");
      Serial.println(value);
    }

    // Feld 3 = Ladedruck Ist
    if (field == 2) {
      lastBoostRaw = value;
      lastBoostMbar = 0.04 * a * b;

      Serial.println();
      Serial.println("----------------------------------------");
      Serial.println("        LADEDRUCK IST");
      Serial.println("----------------------------------------");

      Serial.print("Formel: 0x");
      printHex(formula);
      Serial.println();

      Serial.print("Rohwert: ");
      Serial.print(value);
      Serial.println(" mbar");

      Serial.println("----------------------------------------");
    }
  }

  return true;
}

// =====================================================
// MESSWERTBLOCK 001 LESEN
//
// MWB 001:
//
// Feld 1 = Drehzahl
// Feld 2 = Einspritzmenge
//
// Berechnung:
// Feld 1 Drehzahl:
//   0.2 × a × b
//
// Feld 2 Einspritzmenge:
//   b / 256 × a
// =====================================================

bool readMeasurementBlock001(bool printAllFields) {

  if (!kwpConnected) {
    Serial.println("FEHLER: Keine aktive Verbindung.");
    return false;
  }

  uint8_t request[5];

  request[0] = 0x04;
  request[1] = blockCounter;
  request[2] = 0x29;
  request[3] = 0x01;
  request[4] = 0x03;

  if (printAllFields) {
    Serial.println();
    Serial.println("========================================");
    Serial.println("         LESE MWB 001");
    Serial.println("========================================");
  }

  // -------------------------------------------------
  // MWB 001 anfordern
  // -------------------------------------------------

  if (!kwpSendBlock(request, sizeof(request))) {
    Serial.println(
      "FEHLER: Group Reading MWB 001 konnte nicht gesendet werden.");

    return false;
  }

  // -------------------------------------------------
  // Antwort empfangen
  // -------------------------------------------------

  uint8_t response[64];
  uint16_t responseSize = 0;

  if (!kwpReceiveBlock(
        response,
        sizeof(response),
        responseSize)) {

    Serial.println(
      "FEHLER: Keine Antwort auf MWB 001.");

    return false;
  }

  // -------------------------------------------------
  // Grundprüfung
  // -------------------------------------------------

  if (responseSize < 10) {
    Serial.println(
      "FEHLER: MWB 001 Antwort ist zu kurz.");

    return false;
  }

  if (response[2] == 0x0A) {
    Serial.println(
      "FEHLER: MSG meldet No-Acknowledge.");

    return false;
  }

  if (response[2] != 0xE7) {
    Serial.print(
      "FEHLER: Antworttitel ist 0x");

    printHex(response[2]);

    Serial.println(
      ", erwartet wurde 0xE7.");

    return false;
  }

  // -------------------------------------------------
  // Datenlänge
  // -------------------------------------------------

  uint16_t dataBytes = responseSize - 4;

  if ((dataBytes % 3) != 0) {
    Serial.println(
      "FEHLER: MWB 001 Datenformat ist ungueltig.");

    return false;
  }

  uint16_t fieldCount = dataBytes / 3;

  if (fieldCount < 2) {
    Serial.println(
      "FEHLER: MWB 001 enthaelt weniger als 2 Felder.");

    return false;
  }

  // -------------------------------------------------
  // FELD 1 = DREHZAHL
  // -------------------------------------------------

  uint16_t pos1 = 3;

  uint8_t formula1 = response[pos1];
  uint8_t a1 = response[pos1 + 1];
  uint8_t b1 = response[pos1 + 2];

  float rpm =
    0.2f * (float)a1 * (float)b1;

  lastRpmRaw = a1;
  lastRpm = (uint16_t)rpm;

  // -------------------------------------------------
  // FELD 2 = EINSPRITZMENGE
  // -------------------------------------------------

  uint16_t pos2 = 6;

  uint8_t formula2 = response[pos2];
  uint8_t a2 = response[pos2 + 1];
  uint8_t b2 = response[pos2 + 2];

  float injection =
    ((float)b2 / 256.0f) * (float)a2;

  injectionActual = injection;

  // -------------------------------------------------
  // DEBUG
  // -------------------------------------------------

  if (printAllFields) {

    Serial.println();
    Serial.print("MWB001 RAW: ");

    for (uint16_t i = 0; i < responseSize; i++) {
      printHex(response[i]);
      Serial.print(" ");
    }

    Serial.println();

    Serial.println();
    Serial.println("FELD 1 - DREHZAHL");

    Serial.print("Formel: 0x");
    printHex(formula1);
    Serial.println();

    Serial.print("A: ");
    Serial.println(a1);

    Serial.print("B: ");
    Serial.println(b1);

    Serial.print("Berechnung: 0.2 x ");
    Serial.print(a1);
    Serial.print(" x ");
    Serial.println(b1);

    Serial.print("Drehzahl: ");
    Serial.print(lastRpm);
    Serial.println(" U/min");

    Serial.println();
    Serial.println("FELD 2 - EINSPRITZMENGE");

    Serial.print("Formel: 0x");
    printHex(formula2);
    Serial.println();

    Serial.print("A: ");
    Serial.println(a2);

    Serial.print("B: ");
    Serial.println(b2);

    Serial.print("Berechnung: ");
    Serial.print(b2);
    Serial.print(" / 256 x ");
    Serial.println(a2);

    Serial.print("Einspritzmenge: ");
    Serial.print(injectionActual, 2);
    Serial.println(" mg/Hub");
  }

  return true;
}

bool readMeasurementBlock006(bool printAllFields) {

  if (!kwpConnected) {
    Serial.println("FEHLER: Keine aktive Verbindung.");
    return false;
  }

  uint8_t request[5];

  request[0] = 0x04;
  request[1] = blockCounter;
  request[2] = 0x29;
  request[3] = 0x06;
  request[4] = 0x03;

  if (printAllFields) {
    Serial.println();
    Serial.println("========================================");
    Serial.println("         LESE MWB 006");
    Serial.println("========================================");
  }

  if (!kwpSendBlock(request, sizeof(request))) {
    Serial.println(
      "FEHLER: Group Reading MWB 006 konnte nicht gesendet werden.");
    return false;
  }

  uint8_t response[64];
  uint16_t responseSize = 0;

  if (!kwpReceiveBlock(response, sizeof(response), responseSize)) {
    Serial.println(
      "FEHLER: Keine Antwort auf MWB 006.");
    return false;
  }

  if (responseSize < 7) {
    Serial.println(
      "FEHLER: MWB 006 Antwort ist zu kurz.");
    return false;
  }

  if (response[2] == 0x0A) {
    Serial.println(
      "FEHLER: MSG meldet No-Acknowledge.");
    return false;
  }

  if (response[2] != 0xE7) {
    Serial.print(
      "FEHLER: Antworttitel ist 0x");
    printHex(response[2]);
    Serial.println(
      ", erwartet wurde 0xE7.");
    return false;
  }

  uint16_t dataBytes = responseSize - 4;

  if ((dataBytes % 3) != 0) {
    Serial.println(
      "FEHLER: MWB 006 Datenformat ist ungueltig.");
    return false;
  }

  uint16_t fieldCount = dataBytes / 3;

  if (fieldCount < 1) {
    Serial.println(
      "FEHLER: MWB 006 enthaelt kein Feld.");
    return false;
  }

  // -----------------------------------------
  // FELD 1 = GESCHWINDIGKEIT
  // -----------------------------------------

  uint16_t pos1 = 3;

  uint8_t formula1 = response[pos1];
  uint8_t a1 = response[pos1 + 1];
  uint8_t b1 = response[pos1 + 2];

  float speed =
    0.01f * (float)a1 * (float)b1;

  currentSpeed = speed;

  if (printAllFields) {

    Serial.println();

    Serial.print("MWB006 RAW: ");

    for (uint16_t i = 0; i < responseSize; i++) {
      printHex(response[i]);
      Serial.print(" ");
    }

    Serial.println();

    Serial.println();
    Serial.println("FELD 1 - GESCHWINDIGKEIT");

    Serial.print("Formel: 0x");
    printHex(formula1);
    Serial.println();

    Serial.print("A: ");
    Serial.println(a1);

    Serial.print("B: ");
    Serial.println(b1);

    Serial.print("Berechnung: 0.01 x ");
    Serial.print(a1);
    Serial.print(" x ");
    Serial.println(b1);

    Serial.print("Geschwindigkeit: ");
    Serial.print(currentSpeed, 2);
    Serial.println(" km/h");
  }

  return true;
}

void calculateFuelConsumption() {

  // Keine sinnvolle L/100-km-Berechnung im Stand
  if (currentSpeed < 1.0f || lastRpm < 1) {
    fuelConsumption = 0.0f;
    return;
  }

  // 5-Zylinder, 4-Takt:
  // 5 Einspritzungen pro 2 Kurbelwellenumdrehungen
  const float INJECTIONS_PER_REV = 2.5f;

  // Dieseldichte ca. 0.832 kg/L
  const float DIESEL_DENSITY_MG_PER_L = 832000.0f;

  // mg/Hub -> mg/min
  float mgPerMinute =
    injectionActual *
    (float)lastRpm *
    INJECTIONS_PER_REV;

  // mg/min -> Liter/min
  float litersPerMinute =
    mgPerMinute /
    DIESEL_DENSITY_MG_PER_L;

  // Liter/min -> Liter/100km
  fuelConsumption =
    (litersPerMinute * 60.0f * 100.0f) /
    currentSpeed;
}

// =====================================================
// MESSWERTBLOCK 008 LESEN
//
// MWB 008:
//
// Feld 1 = Einspritzmenge Fahrerwunsch
// Feld 2 = Einspritzmengenbegrenzung Drehmoment
// Feld 3 = Einspritzmengenbegrenzung Rauch
//
// Es wird immer der niedrigste Wert verwendet.
// Dieser entspricht der tatsächlich begrenzenden
// Einspritzmenge.
// =====================================================

bool readMeasurementBlock008(bool printAllFields) {

  if (!kwpConnected) {
    Serial.println("FEHLER: Keine aktive Verbindung.");
    return false;
  }

  uint8_t request[5];

  request[0] = 0x04;
  request[1] = blockCounter;
  request[2] = 0x29;
  request[3] = 0x08;
  request[4] = 0x03;

  if (printAllFields) {
    Serial.println();
    Serial.println("========================================");
    Serial.println("         LESE MWB 008");
    Serial.println("========================================");
  }

  // -------------------------------------------------
  // MWB 008 anfordern
  // -------------------------------------------------

  if (!kwpSendBlock(
        request,
        sizeof(request))) {

    Serial.println(
      "FEHLER: Group Reading MWB 008 konnte nicht gesendet werden.");

    return false;
  }

  // -------------------------------------------------
  // Antwort empfangen
  // -------------------------------------------------

  uint8_t response[64];
  uint16_t responseSize = 0;

  if (!kwpReceiveBlock(
        response,
        sizeof(response),
        responseSize)) {

    Serial.println(
      "FEHLER: Keine Antwort auf MWB 008.");

    return false;
  }

  // -------------------------------------------------
  // Grundprüfung
  // -------------------------------------------------

  if (responseSize < 6) {
    Serial.println(
      "FEHLER: MWB 008 Antwort ist zu kurz.");

    return false;
  }

  if (response[2] == 0x0A) {
    Serial.println(
      "FEHLER: MSG meldet No-Acknowledge.");

    return false;
  }

  if (response[2] != 0xE7) {

    Serial.print(
      "FEHLER: Antworttitel ist 0x");

    printHex(response[2]);

    Serial.println(
      ", erwartet wurde 0xE7.");

    return false;
  }

  // -------------------------------------------------
  // Datenlänge
  // -------------------------------------------------

  uint16_t dataBytes =
    responseSize - 4;

  if ((dataBytes % 3) != 0) {

    Serial.println(
      "FEHLER: MWB 008 Datenformat ist ungueltig.");

    return false;
  }

  uint16_t fieldCount =
    dataBytes / 3;

  if (fieldCount < 3) {

    Serial.println(
      "FEHLER: MWB 008 enthaelt weniger als 3 Felder.");

    return false;
  }

  // -------------------------------------------------
  // Felder auslesen
  // -------------------------------------------------

  float values[3];

  for (uint16_t field = 0; field < 3; field++) {

    uint16_t pos =
      3 + (field * 3);

    uint8_t formula = response[pos];

    uint8_t a = response[pos + 1];
    uint8_t b = response[pos + 2];

    float value = decodeKwpValue(formula, a, b);

    values[field] = value;

    if (printAllFields) {

      Serial.print("Feld ");
      Serial.print(field + 1);

      Serial.print("  Formel=0x");
      printHex(formula);



      Serial.print("  Wert=");
      Serial.println(values[field], 2);
    }
  }

  // -------------------------------------------------
  // Werte speichern
  // -------------------------------------------------

  injectionDriver = values[0];
  injectionTorque = values[1];
  injectionSmoke = values[2];

  // -------------------------------------------------
  // Niedrigsten Wert bestimmen
  // -------------------------------------------------

  injectionActual =
    injectionDriver;

  if (injectionTorque < injectionActual) {
    injectionActual =
      injectionTorque;
  }

  if (injectionSmoke < injectionActual) {
    injectionActual =
      injectionSmoke;
  }

  if (printAllFields) {

    Serial.println();
    Serial.println("----------------------------------------");
    Serial.print("Fahrerwunsch:       ");
    Serial.println(injectionDriver, 2);

    Serial.print("Drehmomentbegrenz.: ");
    Serial.println(injectionTorque, 2);

    Serial.print("Rauchbegrenzung:    ");
    Serial.println(injectionSmoke, 2);

    Serial.println("----------------------------------------");

    Serial.print("EINSPRITZMENGE IST: ");
    Serial.println(injectionActual, 2);

    Serial.println("----------------------------------------");
  }

  return true;
}

// =====================================================
// CODIERUNG LESEN
// =====================================================

void readCodingCommand() {
  if (!connectMSG()) {
    return;
  }

  Serial.println();
  Serial.println("========================================");
  Serial.println("        AKTUELLE MSG-DATEN");
  Serial.println("========================================");

  Serial.print("Codierung: ");
  Serial.println(Coding);

  Serial.print("WSC: ");
  Serial.println(WSC);

  Serial.println("========================================");

  disconnectMSG();
}




// =====================================================
// CODIERUNG AENDERN
// =====================================================
void changeCodingCommand() {
  // 1. Erst verbinden, um aktuelle Werte zu lesen
  if (!connectMSG()) {
    return;
  }

  uint16_t oldCoding = Coding;
  uint32_t oldWSC = WSC;

  Serial.println();
  Serial.println("========================================");
  Serial.println("         CODIERUNG AENDERN");
  Serial.println("========================================");

  Serial.print("Aktuelle Codierung: ");
  Serial.println(oldCoding);

  Serial.print("Aktueller WSC: ");
  Serial.println(oldWSC);

  // 2. Verbindung sauber trennen, WAEHREND der User tippt
  disconnectMSG();

  clearSerialInput();

  Serial.println();
  Serial.println(
    "Neue Codierung eingeben und Enter druecken:");

  uint32_t inputCoding = 0;

  if (!readSerialNumber(inputCoding, 30000, false, 0)) {
    Serial.println("FEHLER: Keine Codierung eingegeben.");
    return;
  }

  if (inputCoding > 32767) {
    Serial.println("FEHLER: Codierung muss 0 bis 32767 sein.");
    return;
  }

  clearSerialInput();

  Serial.println();
  Serial.print("Neuen WSC eingeben oder nur Enter fuer ");
  Serial.println(oldWSC);

  uint32_t inputWSC = oldWSC;

  if (!readSerialNumber(inputWSC, 30000, true, oldWSC)) {
    inputWSC = oldWSC;
  }

  if (inputWSC > 131071UL) {
    Serial.println("FEHLER: WSC muss 0 bis 131071 sein.");
    return;
  }

  if (inputCoding == oldCoding && inputWSC == oldWSC) {
    Serial.println();
    Serial.println("Codierung und WSC sind bereits identisch.");
    return;
  }

  // 3. Jetzt ERNEUT verbinden, direkt bevor gesendet wird
  if (!connectMSG()) {
    Serial.println("FEHLER: Erneute Verbindung fehlgeschlagen.");
    return;
  }

  sendCoding((uint16_t)inputCoding, inputWSC);

  disconnectMSG();
}

// =====================================================
// OLED - NORMALE MESSWERTANZEIGE Drehzahl, Ladedruck
// =====================================================


void oledShowNormal() {

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // =====================================================
  // DREHZAHL
  // =====================================================

  if (displayMode == MODE_RPM) {

    String title = "";
    int16_t tx = getCenteredX(title);

    display.setCursor(tx, 8);
    display.print(title);

    display.setTextSize(2);

    String value = String(lastRpm);
    tx = getCenteredX(value);

    display.setCursor(tx, 23);
    display.print(value);

    display.setTextSize(1);

    String unit = "U/min";
    tx = getCenteredX(unit);

    display.setCursor(tx, 40);
    display.print(unit);
  }

  // =====================================================
  // LADEDRUCK
  // =====================================================

  else if (displayMode == MODE_IDLE) {

    String title = "";
    int16_t tx = getCenteredX(title);

    display.setCursor(tx, 8);
    display.print(title);

    display.setTextSize(2);

    String value = String(lastBoostMbar);
    tx = getCenteredX(value);

    display.setCursor(tx, 23);
    display.print(value);

    display.setTextSize(1);

    String unit = "mBar";
    tx = getCenteredX(unit);

    display.setCursor(tx, 40);
    display.print(unit);
  }

// =====================================================
// VERBRAUCH
// =====================================================

else if (displayMode == MODE_CONSUMPTION) {

  display.setTextSize(1);

  String title = "";
  int16_t tx = getCenteredX(title);

  display.setCursor(tx, 8);
  display.print(title);

  display.setTextSize(2);

  String value = String(fuelConsumption, 1);
  tx = getCenteredX(value);

  display.setCursor(tx, 23);
  display.print(value);

  display.setTextSize(1);

  String unit = "L/100km";
  tx = getCenteredX(unit);

  display.setCursor(tx, 40);
  display.print(unit);
}

  // =====================================================
  // OLED EINSPRITZMENGE
  // =====================================================

  else if (displayMode == MODE_INJECTION) {

    display.setTextSize(1);

    String title = "";
    int16_t tx = getCenteredX(title);

    display.setCursor(tx, 8);
    display.print(title);

    display.setTextSize(2);

    String value =
      String(injectionActual, 1);

    tx = getCenteredX(value);

    display.setCursor(tx, 23);
    display.print(value);

    display.setTextSize(1);

    String unit = "mg/Hub";
    tx = getCenteredX(unit);

    display.setCursor(tx, 40);
    display.print(unit);
  }
  display.display();
}
// =====================================================
// OLED - CODEBLOCK MODUS
// =====================================================

void oledShowCodeblock() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // -------------------------------------------------
  // CB: 2 / CB: 3
  // -------------------------------------------------

  display.setTextSize(1);

  String title = "CB: " + String(currentCodeblock);

  int16_t tx = getCenteredX(title);

  display.setCursor(tx, 9);
  display.print(title);

  // -------------------------------------------------
  // LEISTUNG
  // -------------------------------------------------

  String power;

  if (currentCodeblock == 2) {
    power = "102PS";
  } else if (currentCodeblock == 3) {
    power = "138PS";
  } else {
    power = "???";
  }

  display.setTextSize(2);

  tx = getCenteredX(power);

  display.setCursor(tx, 24);
  display.print(power);

  display.display();
}

// =====================================================
// OLED - SCHREIBEN
// =====================================================

void oledShowWriting() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 20);
  display.println("CODEBLOCK");

  display.setCursor(0, 20);
  display.println("wird");

  display.setCursor(0, 35);
  display.println("geschrieben...");

  display.display();
}


// =====================================================
// OLED - ERFOLG
// =====================================================

void oledShowSuccess() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 12);
  display.println("CODEBLOCK");

  display.setCursor(0, 23);
  display.println("erfolgreich");

  display.setCursor(0, 34);
  display.println("geschrieben!");

  display.display();

  delay(1500);
}


// =====================================================
// OLED - FEHLER
// =====================================================

void oledShowError() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 20);
  display.println("FEHLER!");

  display.setCursor(0, 28);
  display.println("Vorgang");

  display.setCursor(0, 35);
  display.println("fehlgeschlagen");

  display.display();

  delay(1500);
}

// =====================================================
// SPANNUNG MESSEN
// =====================================================

const int VOLTAGE_PIN = 0;

// Spannungsteiler
const float R_TOP = 47000000.0;
const float R_BOTTOM = 10000000.0;

// Dein bisheriger Korrekturwert
float voltageOffset = -1.24;

// Messung
const int NUM_SAMPLES = 50;
const unsigned long VOLTAGE_DISPLAY_TIME = 500;

unsigned long lastVoltageDisplay = 0;

float measuredVoltage = 0.0;


// =====================================================
// SPANNUNG MESSEN UND ANZEIGEN
// =====================================================

void updateVoltageDisplay() {

  if (displayMode != MODE_VOLTAGE) {
    return;
  }

  if (millis() - lastVoltageDisplay < VOLTAGE_DISPLAY_TIME) {
    return;
  }



  lastVoltageDisplay = millis();

  // ------------------------------------------
  // ADC Mittelwert
  // ------------------------------------------

  long adcSum = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    adcSum += analogRead(VOLTAGE_PIN);
    delayMicroseconds(500);
  }

  float adcValue =
    adcSum / (float)NUM_SAMPLES;

  // ------------------------------------------
  // ADC Spannung
  // ------------------------------------------

  float adcVoltage =
    (adcValue / 4095.0) * 3.3;

  // ------------------------------------------
  // Eingangsspannung
  // ------------------------------------------

  measuredVoltage =
    adcVoltage * ((R_TOP + R_BOTTOM) / R_BOTTOM);

  measuredVoltage += voltageOffset;

  // ------------------------------------------
  // Serieller Monitor
  // ------------------------------------------

  Serial.print("Spannung: ");
  Serial.print(measuredVoltage, 2);
  Serial.println(" V");

  // ------------------------------------------
  // OLED
  // ------------------------------------------

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ----------------------------------------
  // Schriftzug oben Spannung
  // ----------------------------------------

  display.setTextSize(1);

  String title = "";

  int16_t tx1, ty1;
  uint16_t tw, th;

  display.getTextBounds(
    title,
    0,
    0,
    &tx1,
    &ty1,
    &tw,
    &th);

  int tx = (64 - tw) / 2;

  display.setCursor(tx, 25);
  display.print(title);

  // ----------------------------------------
  // Spannung
  // ----------------------------------------


  String voltageText =
    String(measuredVoltage, 2) + "V";

  // Eigene GFX-Schrift aktivieren
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);

  // Schrift zentrieren
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(
    voltageText,
    0,
    0,
    &x1,
    &y1,
    &w,
    &h);

  int x = (64 - w) / 2;

  // Bei GFX-Fonts ist die Y-Position die Baseline
  display.setCursor(x, 43);
  display.print(voltageText);

  // Danach wieder Standardschrift verwenden
  display.setFont(NULL);

  display.display();
}

// =====================================================
// CODEBLOCK-MODUS ÖFFNEN
// =====================================================

void enterCodeblockMode() {
  Serial.println();
  Serial.println("========================================");
  Serial.println("        CODEBLOCK MODUS");
  Serial.println("========================================");

  displayMode = MODE_CODEBLOCK;
  codeblockMode = true;

  // -------------------------------------------------
  // Verbindung herstellen
  // -------------------------------------------------

  if (!connectMSG()) {
    oledShowError();
    return;
  }

  // -------------------------------------------------
  // Aktuelle Codierung übernehmen
  // -------------------------------------------------

  currentCodeblock = Coding;

  // Nur 2 und 3 erlauben
  if (currentCodeblock != 2 && currentCodeblock != 3) {
    Serial.print("Unbekannter Codeblock: ");
    Serial.println(currentCodeblock);

    currentCodeblock = 2;
  }

  Serial.print("Aktueller Codeblock: ");
  Serial.println(currentCodeblock);

  // Verbindung nicht offen halten.
  // changeCodeblock() verbindet bei Bedarf neu.
  disconnectMSG();

  oledShowCodeblock();
}
// =====================================================
// CODEBLOCK 2 <-> 3
// =====================================================

void changeCodeblock() {

  if (displayMode != MODE_CODEBLOCK) {
    return;
  }

  // =====================================================
  // ZIEL BESTIMMEN
  // =====================================================

  uint16_t newCodeblock;

  if (currentCodeblock == 2) {
    newCodeblock = 3;
  } else {
    newCodeblock = 2;
  }

  Serial.println();
  Serial.println("========================================");
  Serial.println("       CODEBLOCK WECHSEL");
  Serial.println("========================================");

  Serial.print("Aktuell: ");
  Serial.println(currentCodeblock);

  Serial.print("Neu:     ");
  Serial.println(newCodeblock);

  // =====================================================
  // WICHTIG:
  // ALTE KWP-VERBINDUNG IMMER BEENDEN
  // =====================================================

  if (kwpConnected) {

    Serial.println();
    Serial.println("Beende alte KWP-Verbindung...");

    disconnectMSG();
  }

  // =====================================================
  // KURZ WARTEN
  // =====================================================

  delay(100);

  // =====================================================
  // NEU MIT MSG VERBINDEN
  // =====================================================

  Serial.println();
  Serial.println("Verbinde MSG fuer Codeblock-Schreiben...");

  if (!connectMSG()) {

    Serial.println();
    Serial.println("FEHLER: Verbindung zum MSG fehlgeschlagen.");

    oledShowError();
    return;
  }

  Serial.println();
  Serial.println("MSG erneut verbunden.");

  Serial.print("Aktuelle Codierung: ");
  Serial.println(Coding);

  Serial.print("Aktueller WSC: ");
  Serial.println(WSC);

  // =====================================================
  // WSC ÜBERNEHMEN
  // =====================================================

  uint32_t unchangedWSC = WSC;

  // =====================================================
  // CODEBLOCK SCHREIBEN
  // =====================================================

  oledShowWriting();

  bool result = sendCoding(
    newCodeblock,
    unchangedWSC);

  // =====================================================
  // VERBINDUNG BEENDEN
  // =====================================================

  if (result) {

    disconnectMSG();

  } else {

    Serial.println();
    Serial.println("B13 fehlgeschlagen.");

    // Keine weitere B03-Kommunikation erzwingen,
    // wenn das MSG bereits nicht mehr antwortet.

    stopKLine(false);
    kwpConnected = false;
  }

  // =====================================================
  // ERGEBNIS
  // =====================================================

  if (result) {

    currentCodeblock = newCodeblock;

    Serial.println();
    Serial.println("========================================");
    Serial.print("Codeblock erfolgreich geändert auf: ");
    Serial.println(currentCodeblock);
    Serial.println("========================================");

    oledShowCodeblock();

  } else {

    Serial.println();
    Serial.println("Codeblock-Wechsel fehlgeschlagen.");

    oledShowError();

    // Alten Codeblock weiter anzeigen
    oledShowCodeblock();
  }
}



// =====================================================
// TASTER - INTERRUPT-SERVICE-ROUTINE
//
// Zeichnet nur den rohen Pegel und den Zeitpunkt der
// Flanke auf. Keine Entprellung hier, das passiert
// im Hauptprogramm.
// =====================================================

void IRAM_ATTR handleButtonISR() {
  btnRawPressed = (digitalRead(BUTTON_INPUT) == LOW);
  btnRawChangeTime = millis();
}
// =====================================================
// TASTER AUSWERTEN
//
// KURZ:
//   Spannung       -> Drehzahl
//   Drehzahl       -> Ladedruck
//   Ladedruck      -> Verbrauch
//   Verbrauch      -> Einspritzmenge
//   Einspritzmenge -> Codeblock
//   Codeblock      -> Spannung
//
// LANG:
//   Im Codeblock-Modus:
//   Codeblock 2 <-> 3
// =====================================================

void handleButton() {

  // Rohen Zustand sicher aus der ISR uebernehmen
  noInterrupts();
  bool rawPressed = btnRawPressed;
  uint32_t rawChangeTime = btnRawChangeTime;
  interrupts();

  uint32_t now = millis();

  // ------------------------------------------
  // ENTPRELLUNG
  // Der Pegel gilt erst als stabil, wenn seit
  // der letzten Flanke die Entprellzeit ohne
  // weitere Aenderung vergangen ist.
  // ------------------------------------------

  if ((now - rawChangeTime) < BUTTON_DEBOUNCE_MS) {
    return;
  }

  // Kein Wechsel gegenueber dem letzten stabilen Zustand
  if (rawPressed == btnStablePressed) {
    return;
  }

  // ------------------------------------------
  // ECHTER, STABILER ZUSTANDSWECHSEL
  // ------------------------------------------

  btnStablePressed = rawPressed;

  if (btnStablePressed) {
    // Neu gedrueckt
    btnPressStart = now;
    return;
  }

  // Neu losgelassen -> Ereignis auswerten
  uint32_t pressTime = now - btnPressStart;

  Serial.print("TASTER LOSGELASSEN: ");
  Serial.print(pressTime);
  Serial.println(" ms");

  // ===================================================
  // LANGER DRUCK
  // ===================================================

  if (pressTime >= BUTTON_LONG_PRESS_MS) {

    Serial.println("LANGER DRUCK");

    if (displayMode == MODE_CODEBLOCK) {

      Serial.println("Codeblock-Modus aktiv");
      Serial.println("Wechsle Codeblock...");

      changeCodeblock();

    } else {

      Serial.println("Langer Druck ausserhalb Codeblock ignoriert.");
    }
  }

  // ===================================================
  // KURZER DRUCK
  // ===================================================

  else {

    Serial.println("KURZER DRUCK");

    if (displayMode == MODE_VOLTAGE) {

      displayMode = MODE_RPM;
      Serial.println("MODE: DREHZAHL");
      oledShowNormal();

    } else if (displayMode == MODE_RPM) {

      displayMode = MODE_IDLE;
      Serial.println("MODE: LADEDRUCK");
      oledShowNormal();

    } else if (displayMode == MODE_IDLE) {

      displayMode = MODE_CONSUMPTION;
      Serial.println("MODE: VERBRAUCH");
      oledShowNormal();

    } else if (displayMode == MODE_CONSUMPTION) {

      displayMode = MODE_INJECTION;
      Serial.println("MODE: EINSPRITZMENGE");
      oledShowNormal();

    } else if (displayMode == MODE_INJECTION) {

      Serial.println("MODE: CODEBLOCK");
      enterCodeblockMode();

    } else if (displayMode == MODE_CODEBLOCK) {

      if (kwpConnected) {
        disconnectMSG();
      }

      displayMode = MODE_VOLTAGE;
      Serial.println("MODE: SPANNUNG");
      oledShowNormal();
    }
  }
}

// =====================================================
// MESSWERTE AUTOMATISCH AKTUALISIEREN
// =====================================================

void updateMeasurements() {

  // =====================================================
  // SPANNUNG
  // =====================================================

  if (displayMode == MODE_VOLTAGE) {

    if (kwpConnected) {
      disconnectMSG();
    }
    updateVoltageDisplay();
    return;
  }

  // =====================================================
  // CODEBLOCK
  // =====================================================

  if (displayMode == MODE_CODEBLOCK) {
    return;
  }

  // =====================================================
  // LEERLAUF / DREHZAHL
  // =====================================================

  if (millis() - lastMeasurementTime < MEASUREMENT_INTERVAL_MS) {
    return;
  }

  lastMeasurementTime = millis();

  // Verbindung herstellen
  if (!kwpConnected) {

#if TESTBENCH_MODE

    kwpConnected = true;

#else

    if (!connectMSG()) {
      return;
    }

#endif
  }
  // =====================================================
  // MESSWERT LESEN
  // =====================================================

#if TESTBENCH_MODE

  // Im TestBench Beispielwerte
  lastRpmRaw = 36;
  lastRpm = 900;

  lastBoostRaw = 1000;
  lastBoostMbar = 1000;

  // Testwert Einspritzmenge
  injectionActual = 42.5;

  bool readOk = true;

#else
  bool readOk = false;

  // ================================================
  // DREHZAHL -> MWB 001
  // ================================================

  if (displayMode == MODE_RPM) {
    readOk = readMeasurementBlock001(false);
  }

  // ================================================
  // LADEDRUCK -> MWB 011
  // ================================================

  else if (displayMode == MODE_IDLE) {
    readOk = readMeasurementBlock011(false);
  }

  // ================================================
  // VERBRAUCH -> MWB 001 + MWB 006
  // ================================================

  else if (displayMode == MODE_CONSUMPTION) {

    if (readMeasurementBlock001(false)) {
      if (readMeasurementBlock006(false)) {
        calculateFuelConsumption();
        readOk = true;
      }
    }
  }

  // ================================================
  // EINSPRITZMENGE -> MWB 008
  // ================================================

  else if (displayMode == MODE_INJECTION) {
    readOk = readMeasurementBlock001(false);
  }

  // ================================================
  // FEHLER: VERBINDUNG ZURUECKSETZEN
  // ================================================

  if (!readOk) {

    Serial.println();
    Serial.println("Messwert fehlgeschlagen.");
    Serial.println("Verbindung wird zurueckgesetzt.");

    // Hart trennen, kein B03 mehr senden.
    // Das MSG antwortet ohnehin nicht mehr.
    stopKLine(true);
    kwpConnected = false;

    // Beim naechsten Durchlauf wird neu verbunden.
    return;
  }

#endif
  // =====================================================
  // OLED
  // =====================================================

  oledShowNormal();
}

// =====================================================
// SERIELLEN EINGANG LEEREN
// =====================================================

void clearSerialInput() {
  while (Serial.available()) {
    Serial.read();
  }
}


// =====================================================
// ZAHL ÜBER SERIAL EINLESEN
//
// value         = Ergebnis
// timeoutMs     = maximale Wartezeit
// allowEmpty    = leere Eingabe erlaubt?
// defaultValue  = Wert bei leerer Eingabe
//
// Nur Ziffern 0-9 werden akzeptiert.
// Enter beendet die Eingabe.
// =====================================================

bool readSerialNumber(
  uint32_t &value,
  uint32_t timeoutMs,
  bool allowEmpty,
  uint32_t defaultValue) {
  String input = "";

  uint32_t startTime = millis();

  while (true) {
    // ------------------------------------------
    // TIMEOUT
    // ------------------------------------------

    if (
      (uint32_t)(millis() - startTime) >= timeoutMs) {
      Serial.println();
      Serial.println("FEHLER: Eingabe-Timeout.");

      return false;
    }


    // ------------------------------------------
    // AUF ZEICHEN WARTEN
    // ------------------------------------------

    if (!Serial.available()) {
      delay(1);
      continue;
    }

    char c = Serial.read();


    // ------------------------------------------
    // ENTER
    // ------------------------------------------

    if (c == '\n' || c == '\r') {
      // ----------------------------------------
      // LEERE EINGABE
      // ----------------------------------------

      if (input.length() == 0) {
        if (allowEmpty) {
          value = defaultValue;

          Serial.println();
          Serial.print("Verwende Wert: ");
          Serial.println(value);

          return true;
        }

        Serial.println();
        Serial.println(
          "FEHLER: Es muss eine Zahl eingegeben werden.");

        Serial.print("Bitte erneut eingeben: ");

        startTime = millis();

        continue;
      }


      // ----------------------------------------
      // STRING PRÜFEN
      // ----------------------------------------

      bool valid = true;

      for (uint16_t i = 0; i < input.length(); i++) {
        char digit = input.charAt(i);

        if (
          digit < '0' || digit > '9') {
          valid = false;
          break;
        }
      }


      // ----------------------------------------
      // UNGÜLTIGE EINGABE
      // ----------------------------------------

      if (!valid) {
        Serial.println();
        Serial.println(
          "FEHLER: Nur Zahlen 0-9 erlaubt.");

        input = "";

        Serial.print("Bitte erneut eingeben: ");

        startTime = millis();

        continue;
      }


      // ----------------------------------------
      // ZAHL UMWANDELN
      // ----------------------------------------

      uint32_t result = 0;

      bool overflow = false;

      for (uint16_t i = 0; i < input.length(); i++) {
        uint8_t digit =
          input.charAt(i) - '0';

        if (
          result > (UINT32_MAX - digit) / 10UL) {
          overflow = true;
          break;
        }

        result =
          result * 10UL + digit;
      }


      // ----------------------------------------
      // ZAHL ZU GROSS
      // ----------------------------------------

      if (overflow) {
        Serial.println();
        Serial.println(
          "FEHLER: Zahl ist zu gross.");

        input = "";

        Serial.print("Bitte erneut eingeben: ");

        startTime = millis();

        continue;
      }


      // ----------------------------------------
      // ERGEBNIS
      // ----------------------------------------

      value = result;

      Serial.println();
      Serial.print("Eingabe: ");
      Serial.println(value);

      return true;
    }


    // ------------------------------------------
    // BACKSPACE
    // ------------------------------------------

    if (c == '\b' || c == 127) {
      if (input.length() > 0) {
        input.remove(
          input.length() - 1);

        Serial.print("\b \b");
      }

      continue;
    }


    // ------------------------------------------
    // ZIFFER
    // ------------------------------------------

    if (c >= '0' && c <= '9') {
      input += c;

      Serial.print(c);

      continue;
    }


    // ------------------------------------------
    // ANDERE ZEICHEN IGNORIEREN
    // ------------------------------------------

    // Buchstaben, Leerzeichen usw. werden ignoriert.
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  // Serial ist deaktiviert und macht nichts.
  Serial.begin(115200);

  // ------------------------------------------
  // K-LINE
  // ------------------------------------------

  pinMode(KLINE_TX, OUTPUT);
  digitalWrite(KLINE_TX, HIGH);

  pinMode(KLINE_RX, INPUT);


// ------------------------------------------
// TASTER
// ------------------------------------------

pinMode(BUTTON_OUTPUT, OUTPUT);
digitalWrite(BUTTON_OUTPUT, LOW);

pinMode(BUTTON_INPUT, INPUT_PULLUP);

pinMode(BUTTON_INPUT, INPUT_PULLUP);

attachInterrupt(
  digitalPinToInterrupt(BUTTON_INPUT),
  handleButtonISR,
  CHANGE);

  // ------------------------------------------
  // LED TEST
  // ------------------------------------------

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ------------------------------------------
  // OLED
  // ------------------------------------------

  Wire.begin(
    OLED_SDA,
    OLED_SCL);

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS)) {

    // Keine Serial-Ausgabe mehr.
    // Falls OLED nicht funktioniert,
    // läuft die K-Line trotzdem weiter.

    return;
  }

  // ------------------------------------------
  // VW STARTLOGO
  // ------------------------------------------

  display.clearDisplay();

  display.drawBitmap(
    0,
    15,
    vwLogo,
    64,
    40,
    SSD1306_WHITE);

  display.display();

  delay(1500);

  // Danach normale Anzeige
  oledShowNormal();
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  // Taster überwachen
  handleButton();

  // Messwerte aktualisieren
  updateMeasurements();

  delay(5);
}
