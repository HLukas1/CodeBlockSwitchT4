#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>


// =====================================================
// ESP32-C3 / L9637D / KWP1281 /
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


uint16_t lastRpm = 0;
uint16_t lastBoostMbar = 0;
float injectionActual = 0.0;

float currentSpeed = 0.0;
float fuelConsumption = 0.0;

// =====================================================
// FAHRT-DURCHSCHNITT (nur RAM -> nach Neustart 0)
// =====================================================

float averageFuelConsumption = 0.0;

double tripFuelLiters = 0.0;
double tripDistanceKm = 0.0;

uint32_t lastAverageUpdateTime = 0;
uint32_t lastAverageMeasurementTime = 0;

// Durchschnitt alle 2 s aktualisieren
const uint32_t AVERAGE_MEASUREMENT_INTERVAL_MS = 2000;

// Unter dieser Geschwindigkeit nicht mitzählen (Standzeit)
const float FUEL_MIN_SPEED_KMH = 4.0f;

// Durchschnitt erst ab 100 m Fahrstrecke anzeigen
const double AVERAGE_MIN_DISTANCE_KM = 0.10;

// Glaettung Verbrauch (gleitender Mittelwert)
const uint8_t FUEL_AVG_SAMPLES = 35;

float fuelHourBuf[FUEL_AVG_SAMPLES] = {0};
float speedBuf[FUEL_AVG_SAMPLES] = {0};
uint8_t fuelBufIndex = 0;
uint8_t fuelBufCount = 0;

bool kwpConnected = false;


// Aktuell im MSG vorhandene Codierung
uint16_t currentCodeblock = 0;

// =====================================================
// WLAN-HOTSPOT / WEBSERVER /SSID
// =====================================================

const char* AP_SSID = "TDI-Dashboard";
const char* AP_PASS = "12345678";   // mind. 8 Zeichen

WebServer server(80);

// Zeitpunkt der letzten Webanfrage
uint32_t lastWebRequest = 0;

// So lange gilt die Webseite nach der letzten Anfrage als offen
const uint32_t WEB_ACTIVE_TIMEOUT_MS = 3000;

bool webClientActive() {
  return lastWebRequest != 0 &&
         (millis() - lastWebRequest) < WEB_ACTIVE_TIMEOUT_MS;
}



// =====================================================
// TASTER - INTERRUPT
// =====================================================

// =====================================================
// TASTER-EVENTS
// Der Interrupt speichert einen vollständigen Tastendruck.
// Dadurch geht der Tastendruck auch während KWP-Lesevorgängen
// oder delay()-Aufrufen nicht verloren.
// =====================================================

portMUX_TYPE buttonMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool buttonWasPressed = false;
volatile bool buttonEventPending = false;

volatile uint32_t buttonPressStartMs = 0;
volatile uint32_t buttonEventDurationMs = 0;
volatile uint32_t buttonLastEdgeMs = 0;

const uint32_t BUTTON_DEBOUNCE_MS = 40;

// Messwertaktualisierung
uint32_t lastMeasurementTime = 0;
const uint32_t MEASUREMENT_INTERVAL_MS = 500;

uint32_t lastConnectAttempt = 0;
const uint32_t RECONNECT_PAUSE_MS = 15000;
// BETRIEBSMODI

enum DisplayMode {
  MODE_VOLTAGE,
  MODE_BOOST,
  MODE_CONSUMPTION,
  MODE_AVERAGE_CONSUMPTION,
  MODE_RPM,
  MODE_INJECTION,
  MODE_CUSTOM,
  MODE_CODEBLOCK
};

DisplayMode displayMode = MODE_VOLTAGE;

// =====================================================
// OLED-KONFIGURATION (wird im Flash gespeichert)
// =====================================================

Preferences prefs;

const uint8_t SCR_VOLTAGE     = 0;
const uint8_t SCR_RPM         = 1;
const uint8_t SCR_BOOST       = 2;
const uint8_t SCR_CONSUMPTION = 3;
const uint8_t SCR_AVERAGE     = 4;
const uint8_t SCR_INJECTION   = 5;
const uint8_t SCR_CUSTOM1     = 10;   // 10..13 = eigene Messwerte

const uint8_t MAX_SCREENS = 12;
const uint8_t MAX_CUSTOM  = 4;

// Standard = bisherige Reihenfolge
uint8_t screenList[MAX_SCREENS] = {
  SCR_VOLTAGE, SCR_RPM, SCR_BOOST,
  SCR_CONSUMPTION, SCR_AVERAGE, SCR_INJECTION
};
uint8_t screenCount = 6;
uint8_t screenIndex = 0;
uint8_t currentScreenId = SCR_VOLTAGE;

// Eigene Messwerte (group = 0 -> Platz leer)
struct CustomScreen {
  uint8_t group;
  uint8_t field;
  char    label[10];
};

CustomScreen customScreens[MAX_CUSTOM];
uint8_t activeCustom = 0;

String customValueText = "-";
String customUnitText  = "";

// Einheiten der festen Bildschirme (Index = ID 0..5)
const char DEFAULT_UNITS[6][10] = {
  "V", "U/min", "mBar", "L/100km", "AVG L/100", "mg/Hub"
};

char baseUnits[6][10] = {
  "V", "U/min", "mBar", "L/100km", "AVG L/100", "mg/Hub"
};

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
// KWP1281 MESSWERT
// =====================================================

struct KwpValue {
  bool    valid;    // Feld vorhanden und gelesen
  bool    isText;   // true = Ergebnis ist Text (z.B. COLD/WARM)
  uint8_t formula;  // Kennzahl (Byte 1)
  uint8_t a;        // Byte 2
  uint8_t b;        // Byte 3
  double  value;    // umgerechneter Messwert
  String  unit;     // Einheit
  String  text;     // Textergebnis
};

const uint8_t KWP_MAX_FIELDS = 4;

// =====================================================
// WEB: FREIE MESSWERTBLOCK-ANZEIGE
// =====================================================

uint8_t  webMwbGroup   = 0;      // gewünschter Block (0 = keiner)
uint32_t lastMwbRequest = 0;

KwpValue mwbData[KWP_MAX_FIELDS];
uint8_t  mwbCount     = 0;       // Anzahl gelesener Felder
uint8_t  mwbDataGroup = 0;       // Block, zu dem mwbData gehört
bool     mwbError     = false;

// =====================================================
// WEB: FEHLERSPEICHER
// =====================================================

struct KwpFault {
  uint16_t code;
  uint8_t type;
  bool sporadic;
};

const uint8_t KWP_MAX_FAULTS = 32;

KwpFault faultData[KWP_MAX_FAULTS];
uint8_t faultCount = 0;

bool faultMemoryValid = false;
bool faultMemoryError = false;
bool faultMemoryEmpty = false;
bool faultRequestRunning = false;

String faultStatusText = "Noch nicht gelesen";
// true = Fehlerspeicher wird gerade beim Start gelesen
bool startupFaultCheck = false;

enum FaultRequestType {
  FAULT_REQUEST_NONE,
  FAULT_REQUEST_READ,
  FAULT_REQUEST_CLEAR
};

volatile FaultRequestType pendingFaultRequest =
  FAULT_REQUEST_NONE;
  
  // =====================================================
// FEHLERCODE-TABELLE T4 ACV (aus T4 ACV FSP.xlsx)
// =====================================================

struct FaultInfo {
  uint16_t code;
  const char* obd;
  const char* text;
  const char* cause;
  const char* effect;
};

const FaultInfo FAULT_TABLE[] = {
  {16485, "P0101", "Luftmassenmesser -G70-: unplausibles Signal", "Luftmassenmesser -G70-; Leitungsunterbrechung bzw. Kurzschluss nach Masse", "Verminderte Leistung; Schwarzrauch"},
  {16500, "P0116", "Kühlmitteltemperaturgeber -G62-: unplausibles Signal", "Kühlmitteltemperaturgeber -G62-", "Schwarzrauch beim Start; es wird immer für ca. 20 s vorgeglüht"},
  {16705, "P0321", "Motordrehzahlgeber -G28-: unplausibles Signal", "Motordrehzahlgeber -G28- defekt; Abstand zwischen Drehzahlgeber und Geberrad zu groß; Metallspäne am Geber bzw. Haltesockel lose", "Motor springt nicht an; Motor geht aus; Vorglühkontrolllampe blinkt; Drehzahlmesser ohne Anzeige"},
  {16706, "P0322", "Motordrehzahlgeber -G28-: kein Signal", "Motordrehzahlgeber -G28- defekt; Leitungsunterbrechung bzw. Kurzschluss", "Motor springt nicht an; Motor geht aus; Vorglühkontrolllampe blinkt; Drehzahlmesser ohne Anzeige"},
  {16885, "P0501", "Fahrzeug-Geschwindigkeitssignal: unplausibles Signal", "Kein Signal vom Geber für Geschwindigkeitsmesser -G22-", "Geschwindigkeitsanzeige nicht i. O.; GRA abgeschaltet; Fahrzeug ruckelt beim Schalten; Klimaanlage wird abgeschaltet"},
  {16955, "P0571", "Bremslichtschalter -F-: unplausibles Signal", "Bremslichtschalter -F- defekt; Bremspedalschalter -F47- defekt", "Vorglühkontrolllampe blinkt; Bremslicht defekt"},
  {16989, "P0605", "Steuergerät für Diesel-Direkteinspritzanlage -J248- defekt", "Steuergerät intern defekt", "Fahrverhaltensmängel; Motorstillstand"},
  {17552, "P1144", "Luftmassenmesser -G70-: Unterbrechung/Kurzschluss nach Masse", "Luftmassenmesser -G70- defekt; Leitungsunterbrechung bzw. Kurzschluss nach Masse", "Verminderte Leistung; Schwarzrauch"},
  {17553, "P1145", "Luftmassenmesser -G70-: Kurzschluss nach Plus", "Luftmassenmesser -G70- defekt; Leitungsunterbrechung bzw. Kurzschluss nach Plus", "Verminderte Leistung; Schwarzrauch"},
  {17554, "P1146", "Luftmassenmesser -G70-: Versorgungsspannung", "Betriebsspannung zu groß bzw. zu klein; Leitungsunterbrechung", "Verminderte Leistung; Schwarzrauch"},
  {17563, "P1155", "Saugrohrdruckgeber -G71-: Kurzschluss nach Plus", "Kurzschluss nach Plus; Saugrohrdruckgeber -G71- defekt", "Verminderte Leistung"},
  {17564, "P1156", "Saugrohrdruckgeber -G71-: Unterbrechung/Kurzschluss nach Masse", "Saugrohrdruckgeber -G71- defekt; Leitungsunterbrechung bzw. Kurzschluss nach Masse", "Verminderte Leistung; erhöhte Emissionswerte"},
  {17565, "P1157", "Saugrohrdruckgeber -G71-: Versorgungsspannung", "Saugrohrdruckgeber -G71- defekt; Leitungsunterbrechung bzw. Kurzschluss", "Verminderte Leistung; erhöhte Emissionswerte"},
  {17568, "P1160", "Saugrohrtemperaturgeber -G72-: Kurzschluss nach Masse", "Saugrohrtemperaturgeber -G72- defekt; Leitung hat Kurzschluss nach Masse", "Messwert geht auf Vorgabewert 134,96 °C"},
  {17569, "P1161", "Saugrohrtemperaturgeber -G72-: Unterbrechung/Kurzschluss nach Plus", "Saugrohrtemperaturgeber -G72- defekt; Leitungsunterbrechung bzw. Kurzschluss nach Plus", "Messwert geht auf Vorgabewert 134,96 °C"},
  {17570, "P1162", "Kraftstofftemperaturgeber -G81-: Kurzschluss nach Masse", "Kraftstofftemperaturgeber -G81- defekt; Leitung hat Kurzschluss nach Masse", "Messwert geht auf Vorgabewert -5,34 °C; erhöhte Emissionswerte"},
  {17571, "P1163", "Kraftstofftemperaturgeber -G81-: Unterbrechung/Kurzschluss nach Plus", "Kraftstofftemperaturgeber -G81- defekt; Leitungsunterbrechung bzw. Kurzschluss nach Plus", "Messwert geht auf Vorgabewert -5,34 °C; erhöhte Emissionswerte"},
  {17653, "P1245", "Nadelhubgeber -G80-: Kurzschluss nach Masse", "Nadelhubgeber -G80- defekt; Leitung hat Kurzschluss nach Masse", "Vorglühkontrolllampe blinkt; rauer Motorlauf; verminderte Leistung; erhöhte Emissionswerte"},
  {17654, "P1246", "Nadelhubgeber -G80-: unplausibles Signal", "Nadelhubgeber -G80- defekt; Einspritzleitung zur Einspritzdüse mit Nadelhubgeber nicht i. O.; Kraftstoffmangel; Luft im Kraftstoffsystem", "Vorglühkontrolllampe blinkt; rauer Motorlauf; verminderte Leistung; erhöhte Emissionswerte"},
  {17655, "P1247", "Nadelhubgeber -G80-: Unterbrechung/Kurzschluss nach Plus", "Nadelhubgeber -G80- defekt; Leitungsunterbrechung bzw. Kurzschluss", "Vorglühkontrolllampe blinkt; rauer Motorlauf; verminderte Leistung; erhöhte Emissionswerte"},
  {17656, "P1248", "Spritzbeginnregelung: Regeldifferenz", "Ventil -N108- defekt; Nadelhubgeber -G80- defekt; Tank leer gefahren; Kraftstoffzufuhr nicht i. O.; Kraftstoffmangel; Förderbeginn nicht i. O.", "Rauer Motorlauf; verminderte Leistung; erhöhte Emissionswerte; schlechtes Kaltstartverhalten"},
  {17659, "P1251", "Ventil für Einspritzbeginn -N108-: Kurzschluss nach Plus", "Kurzschluss nach Plus", "Motor nagelt im Leerlauf, da der Einspritzbeginn konstant auf „früh“ steht"},
  {17660, "P1252", "Ventil für Einspritzbeginn -N108-: Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Ventil -N108- defekt; Kurzschluss nach Masse", "Motor nagelt im Leerlauf bei Einspritzbeginn „früh“; Leistungsmangel bei Einspritzbeginn „spät“"},
  {17663, "P1255", "Kühlmitteltemperaturgeber -G62-: Kurzschluss nach Masse", "Kühlmitteltemperaturgeber -G62- defekt; Leitung hat Kurzschluss nach Masse", "Schwarzrauch beim Start; es wird immer für ca. 20 s vorgeglüht"},
  {17664, "P1256", "Kühlmitteltemperaturgeber -G62-: Unterbrechung/Kurzschluss nach Plus", "Kühlmitteltemperaturgeber -G62- defekt; Leitungsunterbrechung bzw. Kurzschluss nach Plus", "Schwarzrauch beim Start; es wird immer für ca. 20 s vorgeglüht"},
  {17762, "P1354", "Regelschieberweggeber -G149-: elektrischer Fehler im Stromkreis", "Einspritzpumpe defekt; Leitungsunterbrechung bzw. Kurzschluss", "Vorglühkontrolllampe blinkt; Fahrverhaltensmängel; Motorstillstand"},
  {17795, "P1387", "Steuergerät defekt", "Steuergerät intern defekt", "Fahrverhaltensmängel; Motorstillstand"},
  {17810, "P1402", "Abgasrückführungsventil -N18-: Kurzschluss nach Plus", "Kurzschluss nach Plus", "Keine Abgasrückführung"},
  {17849, "P1441", "Abgasrückführungsventil -N18-: Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Ventil -N18- defekt; Kurzschluss nach Masse", "Keine oder zu viel Abgasrückführung; verminderte Leistung; Schwarzrauch"},
  {17945, "P1537", "Kraftstoffabschaltventil -N109-: Fehlfunktion", "Ventil -N109- defekt, undicht oder klemmt", "Verminderte Leistung; Motorstillstand"},
  {17946, "P1538", "Kraftstoffabschaltventil -N109-: Unterbrechung/Kurzschluss nach Masse", "Ventil -N109- defekt; Leitungsunterbrechung bzw. Kurzschluss nach Masse", "Verminderte Leistung; Motorstillstand"},
  {17948, "P1540", "Fahrzeug-Geschwindigkeitssignal: Signal zu groß", "Fahrgeschwindigkeit größer 260 km/h; Schalttafeleinsatz defekt", "Geschwindigkeitsanzeige nicht i. O.; GRA abgeschaltet; Fahrzeug ruckelt beim Schalten; Klimaanlage wird abgeschaltet"},
  {17954, "P1546", "Magnetventil für Ladedruckbegrenzung -N75-: Kurzschluss nach Plus", "Kurzschluss nach Plus", "Verminderte Leistung; Ladedruck zu niedrig"},
  {17957, "P1549", "Magnetventil für Ladedruckbegrenzung -N75-: Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Magnetventil defekt; Kurzschluss nach Masse", "Verminderte Leistung; je nach Fehler Ladedruck zu niedrig oder zu hoch"},
  {17964, "P1556", "Ladedruckregelung: Regelgrenze unterschritten", "Ventil -N75- defekt; Turbolader defekt; Undichtigkeit zwischen Turbolader und Motor", "Ladedruck zu niedrig; verminderte Leistung"},
  {17965, "P1557", "Ladedruckregelung: Regelgrenze überschritten", "Schlauchanschlüsse vertauscht oder nicht aufgesteckt", "Ladedruck zu hoch; verminderte Leistung"},
  {17969, "P1561", "Mengensteller -N146-: Regeldifferenz", "Einspritzpumpe defekt; Leitungsunterbrechung bzw. Kurzschluss", "Vorglühkontrolllampe blinkt; Fahrverhaltensmängel; Motorstillstand"},
  {17970, "P1562", "Mengensteller -N146-: oberer Anschlagwert", "Mengensteller -N146- defekt oder blockiert; oberer Anschlagwert erreicht", "Verminderte Leistung; Ruckeln"},
  {17971, "P1563", "Mengensteller -N146-: unterer Anschlagwert", "Mengensteller -N146- defekt oder blockiert; unterer Anschlagwert erreicht", "Schwarzrauch; unrunder Leerlauf"},
  {17977, "P1569", "Schalter für GRA -E45-: unplausibles Signal", "Schalter -E45- defekt; Leitungsunterbrechung bzw. Kurzschluss", "Geschwindigkeitsregelanlage ist abgeschaltet"},
  {17978, "P1570", "Motorsteuergerät gesperrt", "Nicht berechtigter Schlüssel; Manipulationsversuch; Kurzschluss der Kommunikationsleitung; falsche Codierung; Fehler der Wegfahrsicherung; Wegfahrsicherungs-Steuergerät defekt oder fehlt; Motorsteuergerät nicht angepasst", "Motor springt kurz an und geht wieder aus; Vorglühkontrolllampe blinkt"},
  {18008, "P1600", "Spannungsversorgung Klemme 15: Spannung zu klein", "Spannung bei eingeschalteter Zündung an Klemme 15 nicht vorhanden", "Fahrverhaltensmängel bis zum Motorstillstand"},
  {18009, "P1601", "Relais für Spannungsversorgung Klemme 30 -J317-", "Relais -J317- defekt oder hängt sporadisch", "Motor startet nicht; Fahrverhaltensmängel bis zum Motorstillstand"},
  {18020, "P1612", "Motorsteuergerät falsch codiert", "Ungültige Steuergerätecodierung", "Vorglühkontrolllampe blinkt"},
  {18024, "P1616", "Kontrollleuchte für Vorglühzeit -K29-: Kurzschluss nach Plus", "Kurzschluss nach Plus", "Vorglühkontrolle ohne Funktion"},
  {18025, "P1617", "Kontrollleuchte für Vorglühzeit -K29-: Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Glühlampe defekt; Kurzschluss nach Masse", "Vorglühkontrolle ohne Funktion oder leuchtet ständig"},
  {18026, "P1618", "Relais für Glühkerzen -J52-: Kurzschluss nach Plus", "Leitungskurzschluss nach Plus; Relais -J52- defekt", "Kein Vorglühen; schlechtes Kaltstartverhalten; Kontrolllampe -K29- blinkt"},
  {18027, "P1619", "Relais für Glühkerzen -J52-: Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung bzw. Kurzschluss nach Masse; Relais -J52- defekt", "Kein Vorglühen; schlechtes Kaltstartverhalten; Kontrolllampe -K29- blinkt"},
  {18034, "P1626", "Datenbus Antrieb: fehlende Botschaft vom Getriebe-Steuergerät", "Fehler in den Datenleitungen zum Getriebe-Steuergerät", "Schaltruck; Lastwechselschlag"},
  {18039, "P1631", "Gaspedalstellungsgeber -G79-: Signal zu groß", "Gaspedalstellungsgeber -G79- defekt", "Erhöhte Leerlaufdrehzahl"},
  {18040, "P1632", "Gaspedalstellungsgeber -G79-: Versorgungsspannung", "Betriebsspannung zu groß bzw. zu klein; Leitungsunterbrechung", "Vorglühkontrolllampe blinkt; erhöhte Leerlaufdrehzahl"},
  {18047, "P1639", "Geber für Gaspedalstellung -G79-: unplausibles Signal", "Gaspedalstellungsgeber -G79- defekt", "Erhöhte Leerlaufdrehzahl"},
  {18048, "P1640", "Steuergerät für Diesel-Direkteinspritzanlage -J248- defekt", "Steuergerät intern defekt", "Fahrverhaltensmängel; Motorstillstand"},
  {18056, "P1648", "Datenbus Antrieb defekt", "Fehler in den Datenleitungen zum Getriebe- bzw. ABS-Steuergerät; Fehler im automatischen Getriebe- bzw. ABS-System", "Schaltruck; Lastwechselschlag; keine Fahrdynamikregelung; keine ASR- oder MSR-Funktion"},
  {18057, "P1649", "Datenbus Antrieb: fehlende Botschaft vom ABS-Steuergerät", "Fehler in den Datenleitungen zum ABS-Steuergerät", "Keine Fahrdynamikregelung; keine ASR- oder MSR-Funktion"},
  {18058, "P1650", "Datenbus Antrieb: fehlende Botschaft vom Kombiinstrument", "Fehler in den Datenleitungen zum Kombiinstrument", "Im Dokument keine Auswirkung angegeben"},
  {18061, "P1653", "Fehlerspeicher des ABS-Steuergeräts auslesen", "Fehler im ABS-Steuergerät", "Keine Fahrdynamikregelung; keine ASR- oder MSR-Funktion"},
  {18062, "P1654", "Fehlerspeicher des Kombiinstruments auslesen", "Geber für Ölstand/Öltemperatur defekt", "Kontrolllampe für Ölstand leuchtet"},
  {18090, "P1682", "Datenbus Antrieb: unplausible Botschaft vom ABS-Steuergerät", "Fehler in den Datenleitungen zum ABS-Steuergerät", "Keine Fahrdynamikregelung; keine ASR- oder MSR-Funktion"},
  {19458, "P3002", "Kick-down-Schalter -F8-: unplausibles Signal", "Gaspedalstellungsgeber -G79-", "Schaltpunkte für das automatische Getriebe nicht i. O."},
  {19560, "P3104", "Umschaltventil für Saugrohrklappe -N239-: Kurzschluss nach Plus", "Kurzschluss nach Plus", "Harter Motorabstellschlag"},
  {19561, "P3105", "Umschaltventil für Saugrohrklappe -N239-: Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Umschaltventil -N239- defekt", "Harter Motorabstellschlag"},
};

const int FAULT_TABLE_SIZE = sizeof(FAULT_TABLE) / sizeof(FAULT_TABLE[0]);

// Liefert Index in FAULT_TABLE oder -1
int findFaultIndex(uint16_t code) {
  for (int i = 0; i < FAULT_TABLE_SIZE; i++) {
    if (FAULT_TABLE[i].code == code) return i;
  }
  return -1;
}

// Wird die MWB-Seite gerade abgefragt?
bool mwbActive() {
  return webMwbGroup > 0 &&
         lastMwbRequest != 0 &&
         (millis() - lastMwbRequest) < 3000;
}

String jsonEscape(const String &s) {
  String o;
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') { o += '\\'; o += c; }
    else if ((uint8_t)c < 0x20) { o += '?'; }
    else o += c;   // UTF-8 (Umlaute, °) bleibt erhalten
  }
  return o;
}

// =====================================================
// MWB-TABELLE: TDI-Motoren ab 05/1999 (T4-Wiki)
// =====================================================

struct MwbField {
  const char* name;
  const char* unit;
  const char* soll;
};

struct MwbInfo {
  uint8_t group;
  const char* title;
  const char* cond;
  MwbField f[4];
};

const MwbInfo MWB_TABLE[] = {
  {1, "Einspritzmenge", "Leerlauf, Kühlmittel > 80°C", {
    {"Motordrehzahl", "U/min", "740 - 800"},
    {"Einspritzmenge", "mg/H", "2,2 - 9"},
    {"Spannung Geber Regelschieberweg G149", "V", "1,25 - 2,00"},
    {"Kühlmitteltemperatur", "°C", "80 - 110"}}},

  {2, "Leerlaufdrehzahl", "Leerlauf, Kühlmittel > 80°C", {
    {"Motordrehzahl", "U/min", "740 - 800"},
    {"Gaspedalstellung", "%", "0 - 100 (0 = nicht betätigt)"},
    {"Betriebszustand", "-", "010 = Klima aus + Gaspedal nicht betätigt, 101 = Klima ein"},
    {"Kühlmitteltemperatur", "°C", "80 - 110"}}},

  {3, "Abgasrückführung", "Leerlauf, Kühlmittel > 80°C", {
    {"Motordrehzahl", "U/min", "740 - 800"},
    {"Luftmasse (Soll)", "mg/H", "-"},
    {"Luftmasse (Ist)", "mg/H", "-"},
    {"Tastverhältnis Ventil AGR N18", "%", "-"}}},

  {4, "Einspritzbeginn", "Leerlauf bzw. Volllastfahrt, Kühlmittel > 80°C", {
    {"Motordrehzahl", "U/min", "Leerlauf: 740 - 800 | Volllast: 2900 - 3100"},
    {"Einspritzbeginn (Soll)", "° vor/nach OT", "Leerlauf: 3,8 vor - 0,5 nach OT | Volllast: 9 - 14 vor OT"},
    {"Einspritzbeginn (Ist)", "° vor/nach OT", "Leerlauf: 3,8 vor - 0,5 nach OT | Volllast: ca. Sollwert"},
    {"Tastverhältnis Ventil Spritzbeginn N108", "%", "15 - 95"}}},

  {5, "Startverhältnisse", "", {
    {"Motordrehzahl", "U/min", "-"},
    {"Startmenge", "mg/H", "-"},
    {"Einspritzbeginn (Ist)", "° vor/nach OT", "-"},
    {"Kühlmitteltemperatur", "°C", "-"}}},

  {6, "Schalterstellungen", "Leerlauf, Kühlmittel > 80°C", {
    {"Geschwindigkeit", "km/h", "0"},
    {"Bremspedalüberwachung", "xxx", "000 = Leerlauf, 011 = Bremspedal betätigt, 100 = nur Kupplungspedal betätigt"},
    {"Geschwindigkeitsregelanlage", "xxxxxx", "siehe Artikel GRA"},
    {"Geschwindigkeitsregelanlage", "x", "000: GRA aus, 001: GRA ein, 255: nicht freigeschaltet"}}},

  {7, "Temperaturen", "Zündung aus, Motor abgekühlt", {
    {"Kraftstofftemperatur", "°C", "Umgebungstemperatur (-5,6°C bei Defekt)"},
    {"-", "-", "-"},
    {"Saugrohrtemperatur", "°C", "Umgebungstemperatur (134,9°C bei Defekt)"},
    {"Kühlmitteltemperatur", "°C", "Umgebungstemperatur"}}},

  {8, "Einspritzmengenbegrenzung", "Volllastfahrt, Kühlmittel > 80°C", {
    {"Motordrehzahl", "U/min", "2900 - 3100"},
    {"Einspritzmenge (Fahrerwunsch)", "mg/H", "40 - 45"},
    {"Begrenzung Drehmoment-Kennfeld", "mg/H", "30 - 36"},
    {"Begrenzung Rauch-Kennfeld", "mg/H", "35 - 41"}}},

  {9, "Einspritzmengenbegrenzung", "", {
    {"Motordrehzahl", "U/min", "-"},
    {"Einspritzmenge (GRA aktiv)", "mg/H", "-"},
    {"Begrenzung Drehmoment Automatikgetriebe", "mg/H", "-"},
    {"Spannung Geber Regelschieberweg G149", "%", "-"}}},

  {10, "Luftgrößen", "Volllastfahrt, Kühlmittel > 80°C", {
    {"Luftmasse (Ist)", "mg/H", "800 - 950"},
    {"Atmosphärischer Druck", "mbar", "-"},
    {"Saugrohrdruck (Ladedruck)", "mbar", "1700 - 1950"},
    {"Gaspedalstellung", "%", "100"}}},

  {11, "Ladedruckregelung", "Volllastfahrt, Kühlmittel > 80°C", {
    {"Motordrehzahl", "U/min", "2900 - 3100"},
    {"Ladedruck (Soll)", "mbar", "1700 - 1950"},
    {"Ladedruck (Ist)", "mbar", "1700 - 1950"},
    {"Tastverhältnis Ventil Ladedruckbegrenzung N75", "%", "9 - 90"}}},

  {12, "Vorglühanlage", "", {
    {"-", "-", "-"},
    {"Vorglühzeit", "s", "-"},
    {"Versorgungsspannung vom Steuergerät", "V", "-"},
    {"Kühlmitteltemperatur", "°C", "-"}}},

  {13, "Leerlaufruheregelung", "Leerlauf, Kühlmittel > 80°C", {
    {"Abweichende Einspritzmenge Zyl. 1 zu Zyl. 3", "mg/H", "+/- 2,0"},
    {"Abweichende Einspritzmenge Zyl. 2 zu Zyl. 3", "mg/H", "+/- 2,0"},
    {"Abweichende Einspritzmenge Zyl. 4 zu Zyl. 3", "mg/H", "+/- 2,0"},
    {"Abweichende Einspritzmenge Zyl. 5 zu Zyl. 3", "mg/H", "+/- 2,0"}}},



  {18, "Schalterstellungen", "", {
    {"Geschwindigkeit", "km/h", "-"},
    {"Status GRA, Geschwindigkeitsmesser oder Fahrtschreiber", "xx", "-"},
    {"-", "-", "-"},
    {"-", "-", "-"}}},

  {19, "Mengenstellwerk", "Motor aus, Zündung ein", {
    {"Spannung G149 (Stop-Anschlag)", "V", "0,540 - 0,880"},
    {"Spannung G149 (Start-Anschlag)", "V", "4,100 - 4,880"},
    {"-", "-", "-"},
    {"-", "-", "-"}}},

  {125, "Datenbus", "Leerlauf", {
    {"Status Automatikgetriebe", "-", "1 = Verbindung"},
    {"Status ABS", "-", "1 = Verbindung"},
    {"Status Kombiinstrument", "-", "1 = Verbindung"},
    {"Status Airbag", "-", "1 = Verbindung"}}},
};

const int MWB_TABLE_SIZE = sizeof(MWB_TABLE) / sizeof(MWB_TABLE[0]);

// Index in MWB_TABLE oder -1, wenn der Block nicht in der Tabelle steht
int findMwbIndex(uint8_t group) {
  for (int i = 0; i < MWB_TABLE_SIZE; i++) {
    if (MWB_TABLE[i].group == group) return i;
  }
  return -1;
}

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
bool connectMSG(uint8_t maxAttempts = MAX_CONNECT_ATTEMPTS);
void disconnectMSG();

bool sendCoding(
  uint16_t requestedCoding,
  uint32_t requestedWSC);


void decodeKwpValue(KwpValue &v);

bool readMeasurementGroup(
  uint8_t group,
  KwpValue values[],
  uint8_t &fieldCount,
  bool printAllFields);

bool readMeasurement(
  uint8_t group,
  uint8_t field,
  KwpValue &out,
  bool printAllFields);

bool faultPageActive();

void calculateFuelConsumption();
float calculateLitersPerHour(uint16_t rpm, float injection);
void updateAverageFuelConsumption(float litersPerHour, float speedKmh);
bool readTripAverageValues();

void oledShowCustom();
void updateVoltageDisplay();
void applyScreen(uint8_t id);
void setCustomValue(bool isText, const String &text, double value, const String &unit);

bool readFaultMemory();
bool clearFaultMemory();
bool receiveFaultMemoryResponse();
bool parseFaultBlock(const uint8_t *buffer, uint16_t size);
void processFaultRequest();
String faultTypeText(uint8_t type);
String faultCodeText(uint16_t code);

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

bool connectMSG(uint8_t maxAttempts) { 

#if TESTBENCH_MODE
  kwpConnected = true;
  Coding = 2;
  WSC = 0;
 
  Serial.println("TESTBENCH: Fahrzeugverbindung simuliert.");
  return true;
#endif

  // Ab hier bleibt der bisherige Code von connectMSG()

  if (kwpConnected) {
    return true;
  }
  for (
    uint8_t attempt = 1;
    attempt <= maxAttempts;
    attempt++) {
    Serial.println();
    Serial.print("Verbindungsversuch ");
    Serial.print(attempt);
    Serial.print(" von ");
    Serial.println(maxAttempts);   

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
// FEHLERSPEICHER: HILFSFUNKTIONEN
// =====================================================

String faultCodeText(uint16_t code) {
  String text = String(code);
  while (text.length() < 5) text = "0" + text;
  return text;
}

// Bit 7 = sporadisch, Bit 0..6 = Fehlerart
String faultTypeText(uint8_t type) {
  switch (type & 0x7F) {
    case 0x00: return "keine Angabe";
    case 0x01: return "Kurzschluss nach Plus";
    case 0x02: return "Kurzschluss nach Masse";
    case 0x03: return "kein Signal";
    case 0x04: return "mechanischer Fehler";
    case 0x05: return "Eingang offen";
    case 0x06: return "Signal zu groß";
    case 0x07: return "Signal zu klein";
    case 0x08: return "unplausibles Signal";
    case 0x09: return "Unterbrechung / Kurzschluss nach Masse";
    case 0x0A: return "Unterbrechung / Kurzschluss nach Plus";
    case 0x0B: return "Kurzschluss";
    case 0x0C: return "elektrischer Fehler";
    case 0x0D: return "bitte Fehlerspeicher auslesen";
    case 0x0E: return "defekt";
	case 0x23: return "";   // SAE/P-Code-Kennung, keine eigene Fehlerart
    default:   return "Fehlerart " + String(type & 0x7F);
  }
}

// =====================================================
// 0xFC-BLOCK AUSWERTEN
// LL ZZ FC [HH LL ART] x n 03
// =====================================================

bool parseFaultBlock(const uint8_t *buffer, uint16_t size) {
  if (size < 7 || buffer[2] != 0xFC) {
    Serial.println("FEHLER: Ungueltiger Fehlerspeicherblock.");
    return false;
  }

  if (((size - 4) % 3) != 0) {
    Serial.println("FEHLER: Ungueltige Anzahl Fehlerbytes.");
    return false;
  }

  for (uint16_t pos = 3; pos + 2 < size - 1; pos += 3) {
    uint16_t code = ((uint16_t)buffer[pos] << 8) | buffer[pos + 1];
    uint8_t type = buffer[pos + 2];

    // FFFF 88 = kein Fehler erkannt
    if (code == 0xFFFF && type == 0x88) {
      faultMemoryEmpty = true;
      continue;
    }

    if (faultCount >= KWP_MAX_FAULTS) continue;

    faultData[faultCount].code = code;
    faultData[faultCount].type = type;
    faultData[faultCount].sporadic = (type & 0x80) != 0;

    Serial.print("Fehler ");
    Serial.print(faultCodeText(code));
    Serial.print("  Art 0x");
    printHex(type);
    Serial.println(faultData[faultCount].sporadic ? "  sporadisch" : "  statisch");

    faultCount++;
  }

  return true;
}

// =====================================================
// ANTWORT EMPFANGEN (mehrere 0xFC-Bloecke moeglich)
// Jeder 0xFC wird mit B01 bestaetigt, Ende = B01 vom SG
// =====================================================

bool receiveFaultMemoryResponse() {
  uint8_t response[256];
  uint16_t responseSize = 0;

  for (uint8_t i = 0; i < 40; i++) {
    if (!kwpReceiveBlock(response, sizeof(response), responseSize)) {
      return false;
    }

    uint8_t title = response[2];

    if (title == 0xFC) {
      if (!parseFaultBlock(response, responseSize)) return false;
      if (!kwpSendAck()) return false;
      continue;
    }

    if (title == 0x09) return true;          // fertig

    if (title == 0x0A) {
      Serial.println("FEHLER: No-Acknowledge.");
      return false;
    }

    if (title == 0x06) {
      kwpConnected = false;
      return false;
    }

    Serial.print("FEHLER: Unerwarteter Blocktitel 0x");
    printHex(title);
    Serial.println();
    return false;
  }

  return false;
}

// =====================================================
// B06 LESEN: 03 ZZ 07 03
// =====================================================

bool readFaultMemory() {
  faultCount = 0;
  faultMemoryEmpty = false;

#if TESTBENCH_MODE
  faultData[0] = {257, 0x05, false};
  faultData[1] = {515, 0x88, true};
  faultCount = 2;
  return true;
#endif

  if (!kwpConnected) return false;

  uint8_t request[4] = {0x03, blockCounter, 0x07, 0x03};

  Serial.println("\n===== FEHLERSPEICHER LESEN =====");

  if (!kwpSendBlock(request, sizeof(request))) return false;

  return receiveFaultMemoryResponse();
}

// =====================================================
// B07 LOESCHEN: 03 ZZ 05 03
// SG gibt danach den Fehlerspeicher erneut aus
// =====================================================

bool clearFaultMemory() {
  faultCount = 0;
  faultMemoryEmpty = false;

#if TESTBENCH_MODE
  faultMemoryEmpty = true;
  return true;
#endif

  if (!kwpConnected) return false;

  uint8_t request[4] = {0x03, blockCounter, 0x05, 0x03};

  Serial.println("\n===== FEHLERSPEICHER LOESCHEN =====");

  if (!kwpSendBlock(request, sizeof(request))) return false;

  return receiveFaultMemoryResponse();
}

// =====================================================
// KWP1281 MESSWERT UMRECHNUNG ANHAND KENNZAHL
// Quelle: Klien Kennzahlen.xlsx
// =====================================================

void decodeKwpValue(KwpValue &v) {
  double a = v.a;
  double b = v.b;

  v.value  = 0.0;
  v.unit   = "";
  v.text   = "";
  v.isText = false;

  switch (v.formula) {
    case 1:  v.value = 0.2 * a * b;               v.unit = "U/min";   break;
    case 2:  v.value = a * 0.002 * b;             v.unit = "%";       break;
    case 3:  v.value = 0.002 * a * b;             v.unit = "Deg";     break;
    case 4:  v.value = fabs(b - 127) * 0.01 * a;
             v.unit  = (v.b > 127) ? "ATDC" : "BTDC";                break;
    case 5:  v.value = a * (b - 100) * 0.1;       v.unit = "C";       break;
    case 6:  v.value = 0.001 * a * b;             v.unit = "V";       break;
    case 7:  v.value = 0.01 * a * b;              v.unit = "km/h";    break;
    case 8:  v.value = 0.1 * a * b;               v.unit = "";        break;
    case 9:  v.value = (b - 127) * 0.02 * a;      v.unit = "Deg";     break;
    case 10: v.isText = true; v.text = (v.b == 0) ? "COLD" : "WARM";  break;
    case 11: v.value = 0.0001 * a * (b - 128) + 1; v.unit = "";       break;
    case 12: v.value = 0.001 * a * b;             v.unit = "Ohm";     break;
    case 13: v.value = (b - 127) * 0.001 * a;     v.unit = "mm";      break;
    case 14: v.value = 0.005 * a * b;             v.unit = "bar";     break;
    case 15: v.value = 0.01 * a * b;              v.unit = "ms";      break;
    case 16: v.isText = true; v.value = v.b;
             v.text = String(v.b, BIN);           v.unit = "bit";     break;
    case 17: v.isText = true;
             v.text = String((char)v.a) + String((char)v.b);          break;
    case 18: v.value = 0.04 * a * b;              v.unit = "mBar";    break;
    case 19: v.value = 0.01 * a * b;              v.unit = "l";       break;
    case 20: v.value = a * (b - 128) / 128.0;     v.unit = "%";       break;
    case 21: v.value = 0.001 * a * b;             v.unit = "V";       break;
    case 22: v.value = 0.001 * a * b;             v.unit = "ms";      break;
    case 23: v.value = b / 256.0 * a;             v.unit = "%";       break;
    case 24: v.value = 0.001 * a * b;             v.unit = "A";       break;
    case 25: v.value = (b * 1.421) + (a / 182.0); v.unit = "g/s";     break;
    case 26: v.value = b - a;                     v.unit = "C";       break;
    case 27: v.value = fabs(b - 128) * 0.01 * a;
             v.unit  = (v.b < 128) ? "ATDC" : "BTDC";                break;
    case 28: v.value = b - a;                     v.unit = "";        break;
    case 29: v.isText = true;
             v.text = (v.b < v.a) ? "1.Kennfeld" : "2.Kennfeld";      break;
    case 30: v.value = b / 12.0 * a;              v.unit = "Deg k/w"; break;
    case 31: v.value = b / 2560.0 * a;            v.unit = "C";       break;
    case 32: v.value = (v.b > 128) ? (b - 256) : b; v.unit = "";      break;
    case 33: v.value = (v.a == 0) ? (100 * b) : (100 * b / a);
             v.unit  = "%";                                           break;
    case 34: v.value = (b - 128) * 0.01 * a;      v.unit = "kW";      break;
    case 35: v.value = 0.01 * a * b;              v.unit = "l/h";     break;
    case 36: v.value = a * 2560 + b * 10;         v.unit = "km";      break;
    case 37: v.isText = true; v.value = v.b;
             v.text = "Oil Pr. 2 < min";                              break;
    case 38: v.value = (b - 128) * 0.001 * a;     v.unit = "Deg k/w"; break;
    case 39: v.value = b / 256.0 * a;             v.unit = "mg/Hub";  break;
    case 40: v.value = b * 0.1 + (25.5 * a) - 400; v.unit = "A";      break;
    case 41: v.value = b + a * 255;               v.unit = "Ah";      break;
    case 42: v.value = b * 0.1 + (25.5 * a) - 400; v.unit = "kW";     break;
    case 43: v.value = b * 0.1 + (25.5 * a);      v.unit = "V";       break;
    case 44: v.isText = true;
             v.text = String(v.a) + ":" + (v.b < 10 ? "0" : "") + String(v.b);
             v.unit = "h:m";                                          break;
    case 45: v.value = 0.1 * a * b / 100.0;       v.unit = "";        break;
    case 46: v.value = (a * b - 3200) * 0.0027;   v.unit = "Deg k/w"; break;
    case 47: v.value = (b - 128) * a;             v.unit = "ms";      break;
    case 48: v.value = b + a * 255;               v.unit = "";        break;
    case 49: v.value = (b / 4.0) * a * 0.1;       v.unit = "mg/Hub";  break;
    case 50: v.value = (v.a == 0) ? ((b - 128) / 0.01)
                                  : ((b - 128) / (0.01 * a));
             v.unit  = "mBar";                                        break;
    case 51: v.value = ((b - 128) / 255.0) * a;   v.unit = "mg/Hub";  break;
    case 52: v.value = b * 0.02 * a - a;          v.unit = "Nm";      break;
    case 53: v.value = (b - 128) * 1.4222 + 0.006 * a; v.unit = "g/s"; break;
    case 54: v.value = a * 256 + b;               v.unit = "Count";   break;
    case 55: v.value = a * b / 200.0;             v.unit = "s";       break;
    case 56: v.value = a * 256 + b;               v.unit = "WSC";     break;
    case 57: v.value = a * 256 + b + 65536;       v.unit = "WSC";     break;
    case 58: v.value = (v.b > 128) ? (1.0225 * (256 - b)) : (1.0225 * b);
             v.unit  = "/s";                                          break;
    case 59: v.value = (a * 256 + b) / 32768.0;   v.unit = "";        break;
    case 60: v.value = (a * 256 + b) * 0.01;      v.unit = "s";       break;
    case 61: v.value = (v.a == 0) ? (b - 128) : ((b - 128) / a);
             v.unit  = "";                                            break;
    case 62: v.value = 0.256 * a * b;             v.unit = "S";       break;
    case 63: v.isText = true;
             v.text = String((char)v.a) + String((char)v.b) + "?";   break;
    case 64: v.value = a + b;                     v.unit = "Ohm";     break;
    case 65: v.value = 0.01 * a * (b - 127);      v.unit = "mm";      break;
    case 66: v.value = (a * b) / 511.12;          v.unit = "V";       break;
    case 67: v.value = (640 * a) + b * 2.5;       v.unit = "Deg";     break;
    case 68: v.value = (256 * a + b) / 7.365;     v.unit = "deg/s";   break;
    case 69: v.value = (256 * a + b) * 0.3254;    v.unit = "bar";     break;
    case 70: v.value = (256 * a + b) * 0.192;     v.unit = "m/s2";    break;

    default:
      // Unbekannte Kennzahl: Rohwert b zurueckgeben
      v.value = b;
      v.unit  = "?";
      break;
  }
}


// =====================================================
// MESSWERTBLOCK KOMPLETT LESEN
//
// Anfrage:  04 ZZ 29 GG 03
// Antwort:  LL ZZ E7 [K a b] x max. 4  03
// =====================================================

bool readMeasurementGroup(
  uint8_t group,
  KwpValue values[],
  uint8_t &fieldCount,
  bool printAllFields) {

  fieldCount = 0;

  for (uint8_t i = 0; i < KWP_MAX_FIELDS; i++) {
    values[i].valid = false;
  }

  if (!kwpConnected) {
    Serial.println("FEHLER: Keine aktive Verbindung.");
    return false;
  }

  uint8_t request[5];
  request[0] = 0x04;
  request[1] = blockCounter;
  request[2] = 0x29;
  request[3] = group;
  request[4] = 0x03;

  if (!kwpSendBlock(request, sizeof(request))) {
    Serial.print("FEHLER: Group Reading MWB ");
    Serial.print(group);
    Serial.println(" konnte nicht gesendet werden.");
    return false;
  }

  uint8_t response[64];
  uint16_t responseSize = 0;

  if (!kwpReceiveBlock(response, sizeof(response), responseSize)) {
    Serial.print("FEHLER: Keine Antwort auf MWB ");
    Serial.println(group);
    return false;
  }

  if (response[2] == 0x0A) {
    Serial.println("FEHLER: MSG meldet No-Acknowledge.");
    return false;
  }

  if (response[2] != 0xE7) {
    Serial.print("FEHLER: Antworttitel ist 0x");
    printHex(response[2]);
    Serial.println(", erwartet wurde 0xE7.");
    return false;
  }

  uint16_t dataBytes = responseSize - 4;

  if (dataBytes == 0 || (dataBytes % 3) != 0) {
    Serial.println("FEHLER: Datenformat ist ungueltig.");
    return false;
  }

  uint16_t count = dataBytes / 3;
  if (count > KWP_MAX_FIELDS) {
    count = KWP_MAX_FIELDS;
  }

  for (uint8_t f = 0; f < count; f++) {
    uint16_t pos = 3 + (f * 3);

    values[f].formula = response[pos];
    values[f].a       = response[pos + 1];
    values[f].b       = response[pos + 2];

    decodeKwpValue(values[f]);
    values[f].valid = true;
  }

  fieldCount = (uint8_t)count;

  if (printAllFields) {
    Serial.println();
    Serial.print("MWB ");
    Serial.print(group);
    Serial.print(" RAW: ");
    for (uint16_t i = 0; i < responseSize; i++) {
      printHex(response[i]);
      Serial.print(" ");
    }
    Serial.println();

    for (uint8_t f = 0; f < fieldCount; f++) {
      Serial.print("Feld ");
      Serial.print(f + 1);
      Serial.print("  K=");
      Serial.print(values[f].formula);
      Serial.print("  a=");
      Serial.print(values[f].a);
      Serial.print("  b=");
      Serial.print(values[f].b);
      Serial.print("  -> ");

      if (values[f].isText) {
        Serial.print(values[f].text);
      } else {
        Serial.print(values[f].value, 2);
      }

      Serial.print(" ");
      Serial.println(values[f].unit);
    }
  }

  return true;
}


// =====================================================
// EINZELNEN MESSWERT LESEN
//
// group = Messwertblock (z.B. 1, 6, 11)
// field = Feld 1..4
// =====================================================

bool readMeasurement(
  uint8_t group,
  uint8_t field,
  KwpValue &out,
  bool printAllFields) {

  out.valid = false;

  if (field < 1 || field > KWP_MAX_FIELDS) {
    Serial.println("FEHLER: Feld muss 1 bis 4 sein.");
    return false;
  }

  KwpValue values[KWP_MAX_FIELDS];
  uint8_t fieldCount = 0;

  if (!readMeasurementGroup(group, values, fieldCount, printAllFields)) {
    return false;
  }

  if (field > fieldCount) {
    Serial.print("FEHLER: MWB ");
    Serial.print(group);
    Serial.print(" hat kein Feld ");
    Serial.println(field);
    return false;
  }

  out = values[field - 1];
  return true;
}



// =====================================================
// KRAFTSTOFF PRO STUNDE (L/h)
// 5-Zylinder, 4-Takt: 2,5 Einspritzungen pro Umdrehung
// Dieseldichte ca. 0,832 kg/L
// =====================================================

float calculateLitersPerHour(uint16_t rpm, float injection) {
  const float INJECTIONS_PER_REV = 2.5f;
  const float DIESEL_DENSITY_MG_PER_L = 832000.0f;

  float mgPerMinute =
    injection * (float)rpm * INJECTIONS_PER_REV;

  return (mgPerMinute / DIESEL_DENSITY_MG_PER_L) * 60.0f;
}

void calculateFuelConsumption() {

  float litersPerHour =
    calculateLitersPerHour(lastRpm, injectionActual);

  // -------------------------------------------------
  // BEI FAHRZEUGSTILLSTAND ZURUECKSETZEN
  // Wichtig: vor dem Eintragen in den Puffer prüfen.
  // -------------------------------------------------

  if (currentSpeed < 4.0f) {

    fuelConsumption = 0.0f;

    for (uint8_t i = 0; i < FUEL_AVG_SAMPLES; i++) {
      fuelHourBuf[i] = 0.0f;
      speedBuf[i] = 0.0f;
    }

    fuelBufCount = 0;
    fuelBufIndex = 0;

    Serial.print("Verbrauch: ");
    Serial.print(litersPerHour, 2);
    Serial.println(" L/h roh | 0.0 L/100km, Fahrzeug steht");

    return;
  }

  // -------------------------------------------------
  // AKTUELLEN WERT IN DEN RINGPUFFER SCHREIBEN
  // -------------------------------------------------

  fuelHourBuf[fuelBufIndex] = litersPerHour;
  speedBuf[fuelBufIndex] = currentSpeed;

  fuelBufIndex =
    (fuelBufIndex + 1) % FUEL_AVG_SAMPLES;

  if (fuelBufCount < FUEL_AVG_SAMPLES) {
    fuelBufCount++;
  }

  // -------------------------------------------------
  // MITTELWERTE BILDEN
  // -------------------------------------------------

  float sumFuel = 0.0f;
  float sumSpeed = 0.0f;

  for (uint8_t i = 0; i < fuelBufCount; i++) {
    sumFuel += fuelHourBuf[i];
    sumSpeed += speedBuf[i];
  }

  float avgFuelHour =
    sumFuel / (float)fuelBufCount;

  float avgSpeed =
    sumSpeed / (float)fuelBufCount;

  // -------------------------------------------------
  // VERBRAUCH BERECHNEN
  //
  // Unter 10 km/h Durchschnittsgeschwindigkeit wird
  // 0.0 angezeigt, damit beim Losfahren keine Werte
  // wie 50 bis 70 L/100km auftreten.
  // -------------------------------------------------

  if (avgSpeed < 10.0f) {

    fuelConsumption = 0.0f;

  } else {

    fuelConsumption =
      avgFuelHour * 100.0f / avgSpeed;
  }

  // -------------------------------------------------
  // SERIELLE AUSGABE
  // -------------------------------------------------

  Serial.print("Verbrauch: ");
  Serial.print(litersPerHour, 2);
  Serial.print(" L/h roh | ");

  Serial.print(fuelConsumption, 1);
  Serial.print(" L/100km geglaettet | ");

  Serial.print("Geschwindigkeit aktuell: ");
  Serial.print(currentSpeed, 1);
  Serial.print(" km/h | Mittel: ");
  Serial.print(avgSpeed, 1);
  Serial.println(" km/h");
}

// =====================================================
// FAHRT-DURCHSCHNITT AUFSUMMIEREN
//
// Durchschnitt = Gesamtliter / Gesamtkilometer * 100
// Standzeiten (< 4 km/h) werden nicht gezählt.
// =====================================================

void updateAverageFuelConsumption(float litersPerHour, float speedKmh) {

  uint32_t now = millis();

  // Erster Aufruf oder nach Pause: nur Zeitstempel setzen
  if (lastAverageUpdateTime == 0) {
    lastAverageUpdateTime = now;
    return;
  }

  uint32_t elapsedMs = now - lastAverageUpdateTime;
  lastAverageUpdateTime = now;

  // Lücken > 10 s ignorieren (Spannungsmodus, Fehlerspeicher,
  // Verbindungsabbruch) -> keine Sprünge
  if (elapsedMs == 0 || elapsedMs > 10000UL) {
    return;
  }

  // Standzeit nicht berücksichtigen
  if (speedKmh < FUEL_MIN_SPEED_KMH || litersPerHour < 0.0f) {
    return;
  }

  double hours = (double)elapsedMs / 3600000.0;

  tripFuelLiters += (double)litersPerHour * hours;
  tripDistanceKm += (double)speedKmh * hours;

  if (tripDistanceKm >= AVERAGE_MIN_DISTANCE_KM) {
    averageFuelConsumption =
      (float)(tripFuelLiters * 100.0 / tripDistanceKm);
  }

  Serial.print("Fahrt: ");
  Serial.print(tripDistanceKm, 3);
  Serial.print(" km | ");
  Serial.print(tripFuelLiters, 4);
  Serial.print(" L | AVG ");
  Serial.print(averageFuelConsumption, 1);
  Serial.println(" L/100km");
}

// =====================================================
// MWB 1 + MWB 6 FÜR DEN DURCHSCHNITT LESEN
// (wird in Modi ohne Verbrauchsanzeige alle 2 s genutzt)
// =====================================================

bool readTripAverageValues() {
  KwpValue g1[KWP_MAX_FIELDS];
  uint8_t n = 0;
  KwpValue speedValue;

  if (!readMeasurementGroup(1, g1, n, false) || n < 2) {
    return false;
  }

  uint16_t rpm = (uint16_t)g1[0].value;
  float injection = (float)g1[1].value;

  if (!readMeasurement(6, 1, speedValue, false)) {
    return false;
  }

  // Frische Werte gleich mit übernehmen
  lastRpm = rpm;
  injectionActual = injection;

  updateAverageFuelConsumption(
    calculateLitersPerHour(rpm, injection),
    (float)speedValue.value);

  return true;
}

// =====================================================
// OLED - NORMALE MESSWERTANZEIGE
// =====================================================

void oledShowValue(String value, String unit) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(getCenteredX(value), 23);
  display.print(value);

  display.setTextSize(1);
  display.setCursor(getCenteredX(unit), 40);
  display.print(unit);

  display.display();
}

void oledShowNormal() {
  if (displayMode == MODE_RPM) {
    oledShowValue(String(lastRpm), baseUnits[SCR_RPM]);
  } else if (displayMode == MODE_BOOST) {
    oledShowValue(String(lastBoostMbar), baseUnits[SCR_BOOST]);
  } else if (displayMode == MODE_CONSUMPTION) {
    oledShowValue(String(fuelConsumption, 1), baseUnits[SCR_CONSUMPTION]);
  } else if (displayMode == MODE_AVERAGE_CONSUMPTION) {
    oledShowValue(String(averageFuelConsumption, 1), baseUnits[SCR_AVERAGE]);
  } else if (displayMode == MODE_INJECTION) {
    oledShowValue(String(injectionActual, 1), baseUnits[SCR_INJECTION]);
  } else if (displayMode == MODE_CUSTOM) {
    oledShowCustom();
  }
}

// Eigener Messwert: oben Wert, darunter Kurzname (oder Einheit)
void oledShowCustom() {
  CustomScreen &c = customScreens[activeCustom];

  String label = strlen(c.label) > 0
                   ? String(c.label)
                   : customUnitText;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Lange Texte (z. B. Bitmuster) kleiner darstellen
  display.setTextSize(customValueText.length() <= 5 ? 2 : 1);
  display.setCursor(getCenteredX(customValueText), 23);
  display.print(customValueText);

  display.setTextSize(1);
  display.setCursor(getCenteredX(label), 40);
  display.print(label);

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
// SPANNUNG NUR MESSEN (ohne OLED)
// =====================================================

void measureVoltage() {
  long adcSum = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    adcSum += analogRead(VOLTAGE_PIN);
    delayMicroseconds(500);
  }

  float adcValue = adcSum / (float)NUM_SAMPLES;
  float adcVoltage = (adcValue / 4095.0) * 3.3;

  measuredVoltage =
    adcVoltage * ((R_TOP + R_BOTTOM) / R_BOTTOM);

  measuredVoltage += voltageOffset;
}

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

  measureVoltage();

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



  // Fehlerhinweis, wenn FSP-Einträge vorhanden sind
  if (faultMemoryValid && faultCount > 0) {
    display.setTextSize(1);

    String warn = "! " + String(faultCount) + " FSP";

    display.setCursor(getCenteredX(warn), 22);
    display.print(warn);
  }

  // ----------------------------------------
  // Spannung
  // ----------------------------------------


  String voltageText = String(measuredVoltage, 1);

  if (strlen(baseUnits[SCR_VOLTAGE]) > 0) {
    voltageText += " ";
    voltageText += baseUnits[SCR_VOLTAGE];
  }

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
// OLED-KONFIGURATION: LADEN / SPEICHERN / ANZEIGEN
// =====================================================

bool screenValid(int id) {
  if (id >= SCR_VOLTAGE && id <= SCR_INJECTION) return true;

  if (id >= SCR_CUSTOM1 && id < SCR_CUSTOM1 + MAX_CUSTOM) {
    return customScreens[id - SCR_CUSTOM1].group != 0;
  }

  return false;
}

void loadOledConfig() {
  memset(customScreens, 0, sizeof(customScreens));
  memcpy(baseUnits, DEFAULT_UNITS, sizeof(baseUnits));

  prefs.begin("oled", true);

  size_t n = prefs.getBytesLength("list");
  if (n > 0 && n <= MAX_SCREENS) {
    prefs.getBytes("list", screenList, n);
    screenCount = n;
  }

  if (prefs.getBytesLength("custom") == sizeof(customScreens)) {
    prefs.getBytes("custom", customScreens, sizeof(customScreens));
  }

  if (prefs.getBytesLength("units") == sizeof(baseUnits)) {
    prefs.getBytes("units", baseUnits, sizeof(baseUnits));
  }

  prefs.end();

  // Ungültige Einträge entfernen
  uint8_t w = 0;
  for (uint8_t i = 0; i < screenCount; i++) {
    if (screenValid(screenList[i])) {
      screenList[w++] = screenList[i];
    }
  }
  screenCount = w;

  if (screenCount == 0) {
    screenList[0] = SCR_VOLTAGE;
    screenCount = 1;
  }

  Serial.print("OLED-Konfiguration geladen, Bildschirme: ");
  Serial.println(screenCount);
}

void saveOledConfig() {
  prefs.begin("oled", false);
  prefs.putBytes("list", screenList, screenCount);
  prefs.putBytes("custom", customScreens, sizeof(customScreens));
  prefs.putBytes("units", baseUnits, sizeof(baseUnits));
  prefs.end();

  Serial.println("OLED-Konfiguration gespeichert.");
}

void setCustomValue(bool isText, const String &text, double value, const String &unit) {
  if (isText) {
    customValueText = text;
  } else if (fabs(value) >= 100.0) {
    customValueText = String(value, 0);
  } else {
    customValueText = String(value, 1);
  }

  customUnitText = unit;
}

// Bildschirm aktivieren
void applyScreen(uint8_t id) {
  currentScreenId = id;
  lastVoltageDisplay = 0;

  switch (id) {
    case SCR_VOLTAGE:
      displayMode = MODE_VOLTAGE;
      Serial.println("MODE: SPANNUNG");
      updateVoltageDisplay();
      return;

    case SCR_RPM:         displayMode = MODE_RPM;                 break;
    case SCR_BOOST:       displayMode = MODE_BOOST;               break;
    case SCR_CONSUMPTION: displayMode = MODE_CONSUMPTION;         break;
    case SCR_AVERAGE:     displayMode = MODE_AVERAGE_CONSUMPTION; break;
    case SCR_INJECTION:   displayMode = MODE_INJECTION;           break;

    default:
      displayMode = MODE_CUSTOM;
      activeCustom = id - SCR_CUSTOM1;
      customValueText = "-";
      customUnitText = "";
      break;
  }

  Serial.print("MODE-ID: ");
  Serial.println(id);

  oledShowNormal();
}

// Nach Änderung: aktuellen Bildschirm in neuer Liste wiederfinden
void syncScreenIndex() {
  if (displayMode == MODE_CODEBLOCK) return;

  for (uint8_t i = 0; i < screenCount; i++) {
    if (screenList[i] == currentScreenId &&
        screenValid(currentScreenId)) {
      screenIndex = i;
      return;
    }
  }

  // Aktueller Bildschirm wurde entfernt/gelöscht -> ersten anzeigen
  screenIndex = 0;
  applyScreen(screenList[0]);
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


  // -------------------------------------------------
  // Verbindung herstellen
  // -------------------------------------------------

  // Verbindung herstellen (nur 1 Versuch, dann Pause)
  if (!kwpConnected) {
    if (lastConnectAttempt != 0 &&
        millis() - lastConnectAttempt < RECONNECT_PAUSE_MS) {
      return;
    }

    bool ok = connectMSG(1);

    // Pause nur nach fehlgeschlagenem Versuch
    lastConnectAttempt = ok ? 0 : millis();

    if (!ok) {
      return;
    }
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
// Erfasst Drücken und Loslassen unabhängig vom loop().
// Flanken innerhalb der Entprellzeit werden ignoriert.
// Nach dem Loslassen wird die Druckdauer als Ereignis
// für das Hauptprogramm gespeichert.
// =====================================================

void IRAM_ATTR handleButtonISR() {
  uint32_t now = millis();
  bool pressed = (digitalRead(BUTTON_INPUT) == LOW);

  portENTER_CRITICAL_ISR(&buttonMux);

  // Flanken innerhalb der Entprellzeit ignorieren
  if ((uint32_t)(now - buttonLastEdgeMs) < BUTTON_DEBOUNCE_MS) {
    portEXIT_CRITICAL_ISR(&buttonMux);
    return;
  }

  buttonLastEdgeMs = now;

  // Taste wurde gedrückt
  if (pressed && !buttonWasPressed) {
    buttonWasPressed = true;
    buttonPressStartMs = now;
  }

  // Taste wurde losgelassen
  else if (!pressed && buttonWasPressed) {
    buttonWasPressed = false;

    // Bereits gespeicherten Tastendruck nicht überschreiben
    if (!buttonEventPending) {
      buttonEventDurationMs =
        (uint32_t)(now - buttonPressStartMs);

      buttonEventPending = true;
    }
  }

  portEXIT_CRITICAL_ISR(&buttonMux);
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

  // ---------------------------------------------------
  // Gespeichertes Taster-Ereignis atomar übernehmen
  // ---------------------------------------------------

  bool eventAvailable = false;
  uint32_t pressTime = 0;

  portENTER_CRITICAL(&buttonMux);

  if (buttonEventPending) {
    pressTime = buttonEventDurationMs;
    buttonEventPending = false;
    eventAvailable = true;
  }

  portEXIT_CRITICAL(&buttonMux);

  // Kein vollständiger Tastendruck vorhanden
  if (!eventAvailable) {
    return;
  }

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

      Serial.println(
        "Langer Druck ausserhalb Codeblock ignoriert.");
    }

    return;
  }

  // ===================================================
  // KURZER DRUCK
  // ===================================================

   Serial.println("KURZER DRUCK");

  // Codeblock -> zurück zum ersten Bildschirm
  if (displayMode == MODE_CODEBLOCK) {

    if (kwpConnected) {
      disconnectMSG();
    }

    screenIndex = 0;
    applyScreen(screenList[0]);
    return;
  }

  // Nächster Bildschirm aus der Liste
  screenIndex++;

  if (screenIndex >= screenCount) {
    // Nach dem letzten Eintrag kommt immer der Codeblock
    Serial.println("MODE: CODEBLOCK");
    enterCodeblockMode();
  } else {
    applyScreen(screenList[screenIndex]);
  }
}

// =====================================================
// MESSWERTE AUTOMATISCH AKTUALISIEREN
// =====================================================

void updateMeasurements() {

  bool webActive = webClientActive();
  
  // Fehlerspeicher-Seite offen -> keine Live-Messwerte auf der K-Line,
  // Spannungsanzeige am OLED läuft weiter
  if (faultPageActive()) {
    if (kwpConnected && !faultRequestRunning) {
      disconnectMSG();
    }
    updateVoltageDisplay();
    return;
  }

  // =====================================================
  // SPANNUNG
  // =====================================================

  if (displayMode == MODE_VOLTAGE) {

    // Ohne Webseite: MSG-Verbindung wie bisher beenden
    if (!webActive && kwpConnected) {
      disconnectMSG();
    }

    updateVoltageDisplay();

    // Ohne Webseite kein KWP-Lesen im Spannungsmodus
    if (!webActive) {
      return;
    }
  }

  // =====================================================
  // CODEBLOCK -> nie für Web lesen
  // =====================================================

  if (displayMode == MODE_CODEBLOCK) {
    return;
  }

  // =====================================================
  // INTERVALL
  // =====================================================

  if (millis() - lastMeasurementTime < MEASUREMENT_INTERVAL_MS) {
    return;
  }

  lastMeasurementTime = millis();

  // Spannung für Web auch außerhalb des Spannungsmodus
  if (webActive && displayMode != MODE_VOLTAGE) {
    measureVoltage();
  }

  // Verbindung herstellen (1 Versuch, danach 15 s Pause)
  if (!kwpConnected) {
    if (lastConnectAttempt != 0 &&
        millis() - lastConnectAttempt < RECONNECT_PAUSE_MS) {
      return;
    }

    lastConnectAttempt = millis();

    if (!connectMSG(1)) {
      return;
    }

    // Verbindung erfolgreich -> beim nächsten Abbruch sofort neu versuchen
    lastConnectAttempt = 0;
  }
  // =====================================================
  // MESSWERT LESEN
  // =====================================================

#if TESTBENCH_MODE

  lastRpm = 900;
  lastBoostMbar = 1000;
  injectionActual = 42.5;

  bool readOk = true;

#else
  bool readOk = false;

  KwpValue v;

  // ================================================
  // WEBSEITE OFFEN -> ALLE LIVEWERTE
  // MWB 001 Feld 1+2, MWB 011 Feld 3
  // ================================================

  if (webActive && mwbActive()) {
    // MWB-Seite offen: nur den gewählten Block lesen
    uint8_t n = 0;
    readOk = readMeasurementGroup(webMwbGroup, mwbData, n, false);
    if (readOk) {
      mwbCount = n;
      mwbDataGroup = webMwbGroup;
      mwbError = false;

      // OLED zeigt eigenen Messwert
      if (displayMode == MODE_CUSTOM) {
        CustomScreen &c = customScreens[activeCustom];

        if (c.group == webMwbGroup) {
          if (c.field <= n) {
            KwpValue &cv = mwbData[c.field - 1];
            setCustomValue(cv.isText, cv.text, cv.value, cv.unit);
          }
        } else if (readMeasurement(c.group, c.field, v, false)) {
          setCustomValue(v.isText, v.text, v.value, v.unit);
        }
      }
    } else {
      mwbCount = 0;
      mwbError = true;
    }
  }

  else if (webActive) {
    KwpValue g1[KWP_MAX_FIELDS];
    uint8_t n = 0;

    if (readMeasurementGroup(1, g1, n, false) && n >= 2) {
      lastRpm = (uint16_t)g1[0].value;
      injectionActual = (float)g1[1].value;

      if (readMeasurement(11, 3, v, false)) {
        lastBoostMbar = (uint16_t)v.value;
        readOk = true;
      }

      // OLED zeigt eigenen Messwert -> zusätzlich lesen
      if (readOk && displayMode == MODE_CUSTOM) {
        CustomScreen &c = customScreens[activeCustom];
        readOk = readMeasurement(c.group, c.field, v, false);
        if (readOk) {
          setCustomValue(v.isText, v.text, v.value, v.unit);
        }
      }

      // OLED zeigt Verbrauch -> zusätzlich Geschwindigkeit
      if (readOk && displayMode == MODE_CONSUMPTION) {
        readOk = readMeasurement(6, 1, v, false);
        if (readOk) {
          currentSpeed = (float)v.value;
          calculateFuelConsumption();

          updateAverageFuelConsumption(
            calculateLitersPerHour(lastRpm, injectionActual),
            currentSpeed);
          lastAverageMeasurementTime = millis();
        }
      }
    }
  }

  // ================================================
  // DREHZAHL -> MWB 001 Feld 1
  // ================================================

  else if (displayMode == MODE_RPM) {
    readOk = readMeasurement(1, 1, v, false);
    if (readOk) {
      lastRpm = (uint16_t)v.value;
    }
  }

  // ================================================
  //   -> MWB 011 Feld 3
  // ================================================

  else if (displayMode == MODE_BOOST) {
    readOk = readMeasurement(11, 3, v, false);
    if (readOk) {
      lastBoostMbar = (uint16_t)v.value;
    }
  }

  // ================================================
  // VERBRAUCH -> MWB 001 Feld 1+2, MWB 006 Feld 1
  // ================================================

  else if (displayMode == MODE_CONSUMPTION) {
    KwpValue g1[KWP_MAX_FIELDS];
    uint8_t n = 0;

    if (readMeasurementGroup(1, g1, n, false) && n >= 2) {
      lastRpm = (uint16_t)g1[0].value;
      injectionActual = (float)g1[1].value;

      if (readMeasurement(6, 1, v, false)) {
        currentSpeed = (float)v.value;
        calculateFuelConsumption();

        updateAverageFuelConsumption(
          calculateLitersPerHour(lastRpm, injectionActual),
          currentSpeed);
        lastAverageMeasurementTime = millis();

        readOk = true;
      }
    }
  }

  // ================================================
  // DURCHSCHNITT -> Werte kommen aus Block 8d
  // ================================================

  else if (displayMode == MODE_AVERAGE_CONSUMPTION) {
    // Jeden Zyklus lesen -> Verbindung bleibt aktiv
    // und der Durchschnitt wird genauer
    readOk = readTripAverageValues();
    lastAverageMeasurementTime = millis();
  }

  // ================================================
  // EIGENER MESSWERT -> gespeicherter Block / Feld
  // ================================================

  else if (displayMode == MODE_CUSTOM) {
    CustomScreen &c = customScreens[activeCustom];
    readOk = readMeasurement(c.group, c.field, v, false);
    if (readOk) {
      setCustomValue(v.isText, v.text, v.value, v.unit);
    }
  }

  // ================================================
  // EINSPRITZMENGE -> MWB 001 Feld 2
  // ================================================

  else if (displayMode == MODE_INJECTION) {
    readOk = readMeasurement(1, 2, v, false);
    if (readOk) {
      injectionActual = (float)v.value;
    }
  }

  // ================================================
  // FAHRT-DURCHSCHNITT IN ALLEN ANDEREN MODI
  // Alle 2 s MWB 1 + MWB 6 zusätzlich lesen.
  // Nicht, wenn die freie MWB-Webseite offen ist.
  // Im Durchschnittsmodus selbst läuft es ebenfalls hierüber.
  // ================================================

  if (readOk &&
      !mwbActive() &&
      millis() - lastAverageMeasurementTime >=
        AVERAGE_MEASUREMENT_INTERVAL_MS) {

    lastAverageMeasurementTime = millis();

    if (!readTripAverageValues()) {
      // Verbindung vermutlich gestört -> wie gewohnt zurücksetzen
      readOk = false;
    }
  }

  // ================================================
  // FEHLER: VERBINDUNG ZURUECKSETZEN
  // ================================================

  if (!readOk) {

    Serial.println();
    Serial.println("Messwert fehlgeschlagen.");
    Serial.println("Verbindung wird zurueckgesetzt.");

    stopKLine(true);
    kwpConnected = false;

    return;
  }

#endif

  // =====================================================
  // OLED (im Spannungsmodus macht oledShowNormal nichts)
  // =====================================================

  oledShowNormal();
}

// =====================================================
// FEHLERSPEICHER-ANFORDERUNG AUS DEM WEB ABARBEITEN
// =====================================================

void processFaultRequest() {
  if (pendingFaultRequest == FAULT_REQUEST_NONE) return;

  FaultRequestType request = pendingFaultRequest;
  pendingFaultRequest = FAULT_REQUEST_NONE;

  faultRequestRunning = true;
  faultMemoryError = false;
  faultMemoryValid = false;
  


  // Laufende Messwert-Verbindung sauber beenden
  if (kwpConnected) disconnectMSG();
  delay(100);

  bool result = false;

  if (connectMSG(startupFaultCheck ? 1 : MAX_CONNECT_ATTEMPTS)) {
    result = (request == FAULT_REQUEST_READ)
               ? readFaultMemory()
               : clearFaultMemory();
  }

  if (kwpConnected) disconnectMSG();
  else stopKLine(false);
  kwpConnected = false;

  if (result) {
    faultMemoryValid = true;

    if (faultCount == 0) {
      faultStatusText = (request == FAULT_REQUEST_CLEAR)
                          ? "Fehlerspeicher gelöscht"
                          : "Kein Fehler erkannt";
    } else {
      if (request == FAULT_REQUEST_CLEAR) {
        faultStatusText = "Gelöscht, aber Fehler weiterhin vorhanden";
      } else {
        faultStatusText = String(faultCount) + " Fehler gelesen";
      }
    }
  } else {
    faultMemoryError = true;
    faultStatusText = "Vorgang fehlgeschlagen";
  }

  faultRequestRunning = false;
  lastMeasurementTime = millis();
  
    startupFaultCheck = false;

  // Spannungsanzeige sofort neu zeichnen (mit/ohne "!")
  lastVoltageDisplay = 0;
  updateVoltageDisplay();
}


// =====================================================
// WEBSEITE
// =====================================================

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>TDI Dashboard</title>
<style>
body{margin:0;font-family:system-ui,sans-serif;background:#111;color:#eee}
header{padding:14px 16px;font-size:20px;font-weight:600;display:flex;justify-content:space-between;align-items:center}
#st{font-size:13px;padding:4px 10px;border-radius:12px;background:#555}
#st.ok{background:#1b7f3a}
#st.err{background:#a12a2a}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:0 12px 12px}
.card{background:#1e1e1e;border-radius:12px;padding:16px;text-align:center}
.lbl{font-size:13px;color:#999}
.val{font-size:34px;font-weight:700;margin:6px 0 2px}
.unit{font-size:13px;color:#999}
</style>
</head>
<body>
<header>TDI Dashboard<span id="st">...</span></header>
<div class="grid">
  <div class="card"><div class="lbl">Drehzahl</div><div class="val" id="rpm">-</div><div class="unit">U/min</div></div>
  <div class="card"><div class="lbl">Ladedruck</div><div class="val" id="boost">-</div><div class="unit">mbar</div></div>
  <div class="card"><div class="lbl">Einspritzmenge</div><div class="val" id="inj">-</div><div class="unit">mg/Hub</div></div>
  <div class="card"><div class="lbl">Spannung</div><div class="val" id="volt">-</div><div class="unit">V</div></div>
</div>
<div style="padding:0 12px 16px;display:grid;grid-template-columns:1fr 1fr;gap:12px">
  <button onclick="location.href='/mwb'" style="background:#1e1e1e;color:#6cf;border:0;border-radius:11px;padding:14px;font-size:15px">Messwerteblöcke →</button>
  <button onclick="location.href='/faults'" style="background:#1e1e1e;color:#6cf;border:0;border-radius:11px;padding:14px;font-size:15px">Fehlerspeicher →</button>
  <button onclick="location.href='/oled'" style="grid-column:1/3;background:#1e1e1e;color:#6cf;border:0;border-radius:11px;padding:14px;font-size:15px">OLED-Anzeige →</button>
</div>
<script>
const st = document.getElementById('st');

async function upd() {
  try {
    const r = await fetch('/api/live', { cache: 'no-store' });
    const d = await r.json();

    for (const k of ['rpm', 'boost', 'inj']) {
      document.getElementById(k).textContent = d.connected ? d[k] : '-';
    }
    document.getElementById('volt').textContent = d.volt;

    st.textContent = d.connected ? 'MSG verbunden' : 'MSG getrennt';
    st.className = d.connected ? 'ok' : 'err';
  } catch (e) {
    st.textContent = 'ESP offline';
    st.className = 'err';
  }

  // Erst nach Antwort neu anfragen -> kein Request-Stau
  setTimeout(upd, 500);
}

upd();
</script>
</body>
</html>
)rawliteral";


// =====================================================
// WEB-HANDLER
// =====================================================

void handleRoot() {
  lastWebRequest = millis();
  server.send(200, "text/html", INDEX_HTML);
}

void handleApiLive() {
  lastWebRequest = millis();

  String json = "{";
  json += "\"connected\":";
  json += kwpConnected ? "true" : "false";
  json += ",\"rpm\":";
  json += String(lastRpm);
  json += ",\"boost\":";
  json += String(lastBoostMbar);
  json += ",\"inj\":";
  json += String(injectionActual, 1);
  json += ",\"volt\":";
  json += String(measuredVoltage, 1);
  json += "}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

const char MWB_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Messwerteblock</title>
<style>
body{margin:0;font-family:system-ui,sans-serif;background:#111;color:#eee}
header{padding:14px 16px;font-size:20px;font-weight:600;display:flex;justify-content:space-between;align-items:center}
a{color:#6cf;text-decoration:none;font-size:14px}
#st{font-size:13px;padding:4px 10px;border-radius:12px;background:#555}
#st.ok{background:#1b7f3a}
#st.err{background:#a12a2a}
.sel{display:flex;gap:8px;padding:0 12px 12px;align-items:center}
.sel button{background:#2a2a2a;color:#eee;border:0;border-radius:10px;font-size:22px;width:52px;height:48px}
.sel input{flex:1;min-width:0;background:#1e1e1e;color:#eee;border:1px solid #333;border-radius:10px;font-size:22px;height:48px;text-align:center}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:0 12px 12px}
.card{background:#1e1e1e;border-radius:12px;padding:16px;text-align:center}
.lbl{font-size:13px;color:#999}
.val{font-size:30px;font-weight:700;margin:6px 0 2px;word-break:break-all}
.unit{font-size:13px;color:#999}
.raw{font-size:11px;color:#666;margin-top:6px}
.soll{font-size:12px;color:#8c8;margin-top:6px}
#msg{padding:0 16px 12px;color:#999;font-size:14px}
</style>
</head>
<body>
<header><span>Messwerteblock</span><span id="st">...</span></header>
<div style="padding:0 16px 12px"><a href="/">&larr; Dashboard</a></div>
<div class="sel">
  <button id="prev">&minus;</button>
  <input id="grp" type="number" min="1" max="255" inputmode="numeric">
  <button id="next">+</button>
</div>
<div id="msg"></div>
<div class="grid" id="grid"></div>
<script>
const st = document.getElementById('st');
const grp = document.getElementById('grp');
const grid = document.getElementById('grid');
const msg = document.getElementById('msg');

let g = parseInt(localStorage.getItem('mwbGroup')) || 1;
grp.value = g;

function setG(v) {
  v = Math.max(1, Math.min(255, parseInt(v) || 1));
  g = v;
  grp.value = v;
  localStorage.setItem('mwbGroup', v);
  grid.innerHTML = '';
  msg.textContent = 'Lade Block ' + v + '...';
}

document.getElementById('prev').onclick = () => setG(g - 1);
document.getElementById('next').onclick = () => setG(g + 1);
grp.onchange = () => setG(grp.value);

function card(i, f) {
  const c = document.createElement('div');
  c.className = 'card';
  const mk = (cls, txt) => {
    const e = document.createElement('div');
    e.className = cls;
    e.textContent = txt;
    c.appendChild(e);
  };
  const named = f.n && f.n !== '-';
  mk('lbl', named ? f.n : 'Feld ' + (i + 1));
  mk('val', f.v);
  mk('unit', f.u || '\u00a0');
  mk('raw', 'Feld ' + (i + 1) + '  K=' + f.k + '  a=' + f.a + '  b=' + f.b);
  c.style.cursor = 'pointer';
  c.onclick = () => pick(i, f);
  return c;
}

async function pick(i, f) {
  const def = (f.u || '').substring(0, 9);
  const n = prompt('Auf OLED anzeigen: Block ' + g + ' Feld ' + (i + 1) +
                   '\nKurzname (max. 9 Zeichen, ohne Umlaute):', def);
  if (n === null) return;

  try {
    const r = await fetch('/api/oled/custom?g=' + g + '&f=' + (i + 1) +
                          '&n=' + encodeURIComponent(n), { method: 'POST' });
    const d = await r.json();
    alert(d.ok ? 'Wird jetzt auf dem OLED angezeigt und ist gespeichert.' : d.msg);
  } catch (e) {
    alert('ESP nicht erreichbar');
  }
}

async function upd() {
  try {
    const r = await fetch('/api/mwb?group=' + g, { cache: 'no-store' });
    const d = await r.json();

    st.textContent = d.connected ? 'MSG verbunden' : 'MSG getrennt';
    st.className = d.connected ? 'ok' : 'err';

    if (d.group === g && d.dataGroup === g && d.fields.length > 0) {
      grid.innerHTML = '';
      d.fields.forEach((f, i) => grid.appendChild(card(i, f)));
      msg.textContent = 'Block ' + g +
        (d.title ? ' – ' + d.title : '') +
        (d.cond ? ' (' + d.cond + ')' : '');
    } else if (d.error && d.group === g) {
      msg.textContent = 'Block ' + g + ' konnte nicht gelesen werden.';
    } else if (!d.connected) {
      msg.textContent = 'Warte auf Verbindung zum MSG...';
    }
  } catch (e) {
    st.textContent = 'ESP offline';
    st.className = 'err';
  }
  setTimeout(upd, 500);
}

msg.textContent = 'Lade Block ' + g + '...';
upd();
</script>
</body>
</html>
)rawliteral";

void handleMwbPage() {
  lastWebRequest = millis();
  server.send(200, "text/html", MWB_HTML);
}

void handleApiMwb() {
  lastWebRequest = millis();
  lastMwbRequest = millis();

  if (server.hasArg("group")) {
    int g = server.arg("group").toInt();
    if (g >= 1 && g <= 255 && g != webMwbGroup) {
      webMwbGroup = (uint8_t)g;
      mwbCount = 0;
      mwbDataGroup = 0;
      mwbError = false;
    }
  }

  int idx = findMwbIndex(webMwbGroup);

  String json = "{\"connected\":";
  json += kwpConnected ? "true" : "false";
  json += ",\"group\":";
  json += String(webMwbGroup);
  json += ",\"dataGroup\":";
  json += String(mwbDataGroup);
  json += ",\"error\":";
  json += mwbError ? "true" : "false";
  json += ",\"title\":\"";
  json += idx >= 0 ? jsonEscape(String(MWB_TABLE[idx].title)) : String("");
  json += "\",\"cond\":\"";
  json += idx >= 0 ? jsonEscape(String(MWB_TABLE[idx].cond)) : String("");
  json += "\",\"fields\":[";

  for (uint8_t i = 0; i < mwbCount; i++) {
    if (i > 0) json += ",";

    String val = mwbData[i].isText
                   ? mwbData[i].text
                   : String(mwbData[i].value, 2);

    String name = "";
    String unit = mwbData[i].unit;

    if (idx >= 0 && i < 4) {
      name = MWB_TABLE[idx].f[i].name;

      // Einheit aus der Tabelle, falls die Formel keine liefert
      if (unit == "" || unit == "?") {
        unit = MWB_TABLE[idx].f[i].unit;
        if (unit == "-") unit = "";
      }
    }

    json += "{\"k\":";  json += String(mwbData[i].formula);
    json += ",\"a\":";  json += String(mwbData[i].a);
    json += ",\"b\":";  json += String(mwbData[i].b);
    json += ",\"v\":\""; json += jsonEscape(val);
    json += "\",\"u\":\""; json += jsonEscape(unit);
    json += "\",\"n\":\""; json += jsonEscape(name);
    json += "\"}";
  }

  json += "]}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", json);
}

// =====================================================
// FEHLERSPEICHER-WEBSEITE
// =====================================================

const char FAULT_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Fehlerspeicher</title>
<style>
body{margin:0;font-family:system-ui,sans-serif;background:#111;color:#eee}
header{padding:14px 16px;font-size:20px;font-weight:600;display:flex;justify-content:space-between;align-items:center}
a{color:#6cf;text-decoration:none;font-size:14px}
#st{font-size:13px;padding:4px 10px;border-radius:12px;background:#555}
#st.ok{background:#1b7f3a}
#st.err{background:#a12a2a}
#st.run{background:#9a6500}
.act{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:0 12px 12px}
button{border:0;border-radius:11px;padding:14px;font-size:16px;font-weight:600;color:#fff}
button:disabled{opacity:.4}
.rd{background:#1b6ca8}
.cl{background:#a12a2a}
#msg{margin:0 12px 12px;padding:12px;background:#1e1e1e;border-radius:11px;color:#bbb;font-size:14px}
.list{padding:0 12px 18px;display:flex;flex-direction:column;gap:10px}
.f{background:#1e1e1e;border-radius:12px;padding:14px;border-left:5px solid #d13b3b}
.f.sp{border-left-color:#e3a11a}
.top{display:flex;justify-content:space-between;align-items:center}
.code{font-size:24px;font-weight:700;letter-spacing:1px}
.tag{font-size:12px;padding:4px 9px;border-radius:12px;background:#8e2525}
.sp .tag{background:#8a620d}
.txt{color:#bbb;margin-top:8px;font-size:14px}
.raw{color:#666;margin-top:5px;font-size:11px}
.empty{text-align:center;color:#8bd29c;background:#17351f;padding:18px;border-radius:12px}
.obd{font-size:13px;color:#6cf;margin-left:8px;font-weight:600}
.ttl{font-size:16px;font-weight:600;margin-top:8px;color:#fff}
.sec{margin-top:10px;font-size:13px;line-height:1.4}
.sec b{display:block;color:#999;font-size:11px;text-transform:uppercase;letter-spacing:.5px;margin-bottom:2px}
.unk{margin-top:8px;font-size:13px;color:#e3a11a}
</style>
</head>
<body>
<header><span>Fehlerspeicher</span><span id="st">...</span></header>
<div style="padding:0 16px 12px"><a href="/">&larr; Dashboard</a></div>
<div class="act">
  <button class="rd" id="rb">Fehler lesen</button>
  <button class="cl" id="cb">Fehler löschen</button>
</div>
<div id="msg">Noch nicht gelesen</div>
<div class="list" id="list"></div>
<script>
const st=document.getElementById('st');
const msg=document.getElementById('msg');
const list=document.getElementById('list');
const rb=document.getElementById('rb');
const cb=document.getElementById('cb');

function btn(dis){rb.disabled=dis;cb.disabled=dis;}

async function cmd(a){
  if(a==='clear' && !confirm('Fehlerspeicher wirklich löschen?')) return;
  btn(true);
  msg.textContent = a==='read' ? 'Lese Fehlerspeicher...' : 'Lösche Fehlerspeicher...';
  try{
    await fetch('/api/faults/'+a,{method:'POST',cache:'no-store'});
  }catch(e){
    msg.textContent='Anforderung fehlgeschlagen';
    btn(false);
  }
}

rb.onclick=()=>cmd('read');
cb.onclick=()=>cmd('clear');

function card(f){
  const c=document.createElement('div');
  c.className='f'+(f.sporadic?' sp':'');

  const top=document.createElement('div');
  top.className='top';

  const left=document.createElement('div');
  const code=document.createElement('span');
  code.className='code';
  code.textContent=f.code;
  left.appendChild(code);

  if(f.known){
    const obd=document.createElement('span');
    obd.className='obd';
    obd.textContent=f.obd;
    left.appendChild(obd);
  }

  const tag=document.createElement('div');
  tag.className='tag';
  tag.textContent=f.sporadic?'sporadisch':'statisch';

  top.appendChild(left);
  top.appendChild(tag);
  c.appendChild(top);

  const add=(cls,txt)=>{
    const e=document.createElement('div');
    e.className=cls;
    e.textContent=txt;
    c.appendChild(e);
    return e;
  };

  const sec=(label,txt)=>{
    const e=document.createElement('div');
    e.className='sec';
    const b=document.createElement('b');
    b.textContent=label;
    e.appendChild(b);
    e.appendChild(document.createTextNode(txt));
    c.appendChild(e);
  };

  if(f.known){
    add('ttl',f.title);
    if(f.text) add('txt',f.text);
    sec('Ursache',f.cause);
    sec('Auswirkung',f.effect);
  }else{
    if(f.text) add('txt',f.text);
    add('unk','Fehlercode nicht in der Tabelle hinterlegt');
  }

  add('raw','Fehlerort 0x'+f.hex+' | Fehlerart 0x'+f.typeHex);
  return c;
}

async function upd(){
  try{
    const r=await fetch('/api/faults',{cache:'no-store'});
    const d=await r.json();

    msg.textContent=d.status;

    if(d.running){
      st.textContent='läuft...'; st.className='run'; btn(true);
    }else{
      st.textContent=d.error?'Fehler':(d.valid?'Bereit':'Nicht gelesen');
      st.className=d.error?'err':(d.valid?'ok':'');
      btn(false);
    }

    list.innerHTML='';
    if(d.valid && d.faults.length===0){
      const e=document.createElement('div');
      e.className='empty';
      e.textContent='Kein Fehler erkannt';
      list.appendChild(e);
    }else{
      d.faults.forEach(f=>list.appendChild(card(f)));
    }
  }catch(e){
    st.textContent='ESP offline'; st.className='err';
  }
  setTimeout(upd,700);
}
upd();
</script>
</body>
</html>
)rawliteral";

// =====================================================
// FEHLERSPEICHER-HANDLER
// =====================================================

uint32_t lastFaultPageRequest = 0;

bool faultPageActive() {
  return lastFaultPageRequest != 0 &&
         (millis() - lastFaultPageRequest) < 3000;
}

void handleFaultPage() {
  lastWebRequest = millis();
  lastFaultPageRequest = millis();
  server.send(200, "text/html; charset=utf-8", FAULT_HTML);
}

void handleApiFaultRead() {
  lastWebRequest = millis();
  lastFaultPageRequest = millis();

  if (faultRequestRunning || pendingFaultRequest != FAULT_REQUEST_NONE) {
    server.send(409, "application/json", "{\"accepted\":false}");
    return;
  }

  pendingFaultRequest = FAULT_REQUEST_READ;
  faultStatusText = "Fehlerspeicher wird gelesen...";
  server.send(202, "application/json", "{\"accepted\":true}");
}

void handleApiFaultClear() {
  lastWebRequest = millis();
  lastFaultPageRequest = millis();

  if (faultRequestRunning || pendingFaultRequest != FAULT_REQUEST_NONE) {
    server.send(409, "application/json", "{\"accepted\":false}");
    return;
  }

  pendingFaultRequest = FAULT_REQUEST_CLEAR;
  faultStatusText = "Fehlerspeicher wird gelöscht...";
  server.send(202, "application/json", "{\"accepted\":true}");
}

void handleApiFaults() {
  lastWebRequest = millis();
  lastFaultPageRequest = millis();

  bool busy = faultRequestRunning ||
              pendingFaultRequest != FAULT_REQUEST_NONE;

  String json = "{\"running\":";
  json += busy ? "true" : "false";
  json += ",\"valid\":";
  json += faultMemoryValid ? "true" : "false";
  json += ",\"error\":";
  json += faultMemoryError ? "true" : "false";
  json += ",\"status\":\"";
  json += jsonEscape(faultStatusText);
  json += "\",\"faults\":[";

  for (uint8_t i = 0; i < faultCount; i++) {
    if (i > 0) json += ",";

    char hexCode[5];
    char hexType[3];
    sprintf(hexCode, "%04X", faultData[i].code);
    sprintf(hexType, "%02X", faultData[i].type);

    json += "{\"code\":\"";
    json += faultCodeText(faultData[i].code);
    json += "\",\"hex\":\"";
    json += hexCode;
    json += "\",\"typeHex\":\"";
    json += hexType;
    json += "\",\"sporadic\":";
    json += faultData[i].sporadic ? "true" : "false";
    json += ",\"text\":\"";
    json += jsonEscape(faultTypeText(faultData[i].type));
    json += "\"";

    int fi = findFaultIndex(faultData[i].code);

    json += ",\"known\":";
    json += (fi >= 0) ? "true" : "false";

    if (fi >= 0) {
      json += ",\"obd\":\"";    json += jsonEscape(String(FAULT_TABLE[fi].obd));
      json += "\",\"title\":\""; json += jsonEscape(String(FAULT_TABLE[fi].text));
      json += "\",\"cause\":\""; json += jsonEscape(String(FAULT_TABLE[fi].cause));
      json += "\",\"effect\":\""; json += jsonEscape(String(FAULT_TABLE[fi].effect));
      json += "\"";
    }

    json += "}";
  }

  json += "]}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", json);
}

// =====================================================
// OLED-KONFIGURATIONSSEITE
// =====================================================

const char OLED_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>OLED-Anzeige</title>
<style>
body{margin:0;font-family:system-ui,sans-serif;background:#111;color:#eee}
header{padding:14px 16px;font-size:20px;font-weight:600;display:flex;justify-content:space-between;align-items:center}
#st{font-size:13px;padding:4px 10px;border-radius:12px;background:#555}
.back{background:none;border:0;color:#6cf;font-size:14px;padding:0 16px 4px}
h3{font-size:13px;color:#999;text-transform:uppercase;letter-spacing:.5px;margin:16px 16px 8px}
.row{display:flex;align-items:center;gap:6px;background:#1e1e1e;border-radius:10px;padding:8px 10px;margin:0 12px 8px}
.row span{flex:1;min-width:0;font-size:15px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.row button{flex:none;background:#2a2a2a;color:#eee;border:0;border-radius:8px;width:36px;height:36px;font-size:16px}
.row button.del{background:#5a1f1f}
.hint{color:#777;font-size:12px;margin:0 16px 8px}
.save{display:block;width:calc(100% - 24px);margin:16px 12px 24px;background:#1b7f3a;color:#fff;border:0;border-radius:11px;padding:14px;font-size:16px;font-weight:600}
</style>
</head>
<body>
<header><span>OLED-Anzeige</span><span id="st">...</span></header>
<button class="back" onclick="location.href='/'">&larr; Dashboard</button>

<h3>Reihenfolge (Taster)</h3>
<div id="act"></div>
<div class="hint">Erster Eintrag = Startbildschirm. Danach folgt immer: Codeblock.</div>

<h3>Nicht angezeigt</h3>
<div id="off"></div>

<h3>Eigene Messwerte</h3>
<div id="cus"></div>
<div class="hint">Hinzufügen: auf der Seite Messwerteblöcke eine Karte antippen. Löschen: 🗑</div>

<button class="save" id="sv">Reihenfolge speichern</button>

<script>
const st = document.getElementById('st');
const BASE = {0:'Spannung',1:'Drehzahl',2:'Ladedruck',3:'Verbrauch',4:'Durchschnitt',5:'Einspritzmenge'};
let list = [], custom = [], units = [];

function name(id) {
  if (id >= 10) {
    const c = custom[id - 10];
    return c && c.g ? '★ ' + (c.n || ('MWB ' + c.g + '/' + c.f)) : '?';
  }
  return BASE[id] + ' [' + (units[id] || '') + ']';
}

function avail() {
  const a = [0,1,2,3,4,5];
  custom.forEach((c, i) => { if (c.g) a.push(10 + i); });
  return a;
}

function row(txt, btns) {
  const r = document.createElement('div');
  r.className = 'row';
  const s = document.createElement('span');
  s.textContent = txt;
  r.appendChild(s);
  btns.forEach(([t, f, cls]) => {
    const b = document.createElement('button');
    b.textContent = t;
    if (cls) b.className = cls;
    b.onclick = f;
    r.appendChild(b);
  });
  return r;
}

function changed() {
  st.textContent = 'Nicht gespeichert';
  draw();
}

function move(i, d) {
  const j = i + d;
  if (j < 0 || j >= list.length) return;
  [list[i], list[j]] = [list[j], list[i]];
  changed();
}

function hide(i) {
  if (list.length > 1) { list.splice(i, 1); changed(); }
}

function draw() {
  const act = document.getElementById('act');
  const off = document.getElementById('off');
  const cus = document.getElementById('cus');
  act.innerHTML = ''; off.innerHTML = ''; cus.innerHTML = '';

  list.forEach((id, i) => act.appendChild(row((i + 1) + '. ' + name(id), [
    ['✎', () => edit(id)],
    ['▲', () => move(i, -1)],
    ['▼', () => move(i, 1)],
    ['✕', () => hide(i)]
  ])));

  avail().filter(id => !list.includes(id)).forEach(id =>
    off.appendChild(row(name(id), [
      ['✎', () => edit(id)],
      ['+', () => { list.push(id); changed(); }]
    ])));

  let any = false;
  custom.forEach((c, i) => {
    if (!c.g) return;
    any = true;
    cus.appendChild(row((c.n || '-') + '  (MWB ' + c.g + ' Feld ' + c.f + ')', [
      ['✎', () => edit(10 + i)],
      ['🗑', () => del(i), 'del']
    ]));
  });

  if (!any) {
    const h = document.createElement('div');
    h.className = 'hint';
    h.textContent = 'Keine eigenen Messwerte angelegt.';
    cus.appendChild(h);
  }
}

async function load() {
  try {
    const r = await fetch('/api/oled', { cache: 'no-store' });
    const d = await r.json();
    list = d.list;
    custom = d.custom;
    units = d.units || [];
    st.textContent = 'Geladen';
    draw();
  } catch (e) {
    st.textContent = 'ESP offline';
  }
}

async function edit(id) {
  const cur = id >= 10 ? (custom[id - 10].n || '') : (units[id] || '');
  const n = prompt('Einheit / Bezeichnung auf dem OLED\n' +
                   '(max. 9 Zeichen, ohne Umlaute, leer = Standard):', cur);
  if (n === null) return;

  try {
    const r = await fetch('/api/oled/unit?id=' + id + '&n=' + encodeURIComponent(n),
                          { method: 'POST' });
    const d = await r.json();
    if (!d.ok) { st.textContent = 'Fehler'; return; }

    if (id >= 10) custom[id - 10].n = d.n;
    else units[id] = d.n;

    st.textContent = 'Einheit gespeichert';
    draw();
  } catch (e) {
    st.textContent = 'ESP offline';
  }
}

// Eigenen Messwert endgültig löschen (ohne Neuladen der Seite)
async function del(i) {
  const c = custom[i];
  if (!confirm('Eigenen Messwert "' + (c.n || ('MWB ' + c.g + '/' + c.f)) +
               '" endgültig löschen?')) return;

  try {
    const r = await fetch('/api/oled/delete?slot=' + i, { method: 'POST' });
    const d = await r.json();
    if (!d.ok) { st.textContent = 'Fehler'; return; }

    custom[i] = { g: 0, f: 0, n: '' };
    list = list.filter(id => id !== 10 + i);
    if (list.length === 0) list = [0];

    st.textContent = 'Gelöscht';
    draw();
  } catch (e) {
    st.textContent = 'ESP offline';
  }
}

document.getElementById('sv').onclick = async () => {
  try {
    const r = await fetch('/api/oled/save?list=' + list.join(','), { method: 'POST' });
    st.textContent = r.ok ? 'Gespeichert' : 'Fehler';
  } catch (e) {
    st.textContent = 'ESP offline';
  }
};

load();
</script>
</body>
</html>
)rawliteral";

// Nur druckbare ASCII-Zeichen, max. 9
void sanitizeLabel(String n, char *out) {
  n.trim();
  uint8_t k = 0;

  for (size_t i = 0; i < n.length() && k < 9; i++) {
    char ch = n[i];
    if (ch >= 0x20 && ch <= 0x7E) out[k++] = ch;
  }

  out[k] = 0;
}

void handleOledPage() {
  lastWebRequest = millis();
  server.send(200, "text/html; charset=utf-8", OLED_HTML);
}

void handleApiOled() {
  lastWebRequest = millis();

  String json = "{\"list\":[";
  for (uint8_t i = 0; i < screenCount; i++) {
    if (i > 0) json += ",";
    json += String(screenList[i]);
  }

  json += "],\"custom\":[";
  for (uint8_t i = 0; i < MAX_CUSTOM; i++) {
    if (i > 0) json += ",";
    json += "{\"g\":";  json += String(customScreens[i].group);
    json += ",\"f\":";  json += String(customScreens[i].field);
    json += ",\"n\":\""; json += jsonEscape(String(customScreens[i].label));
    json += "\"}";
  }

  json += "],\"units\":[";
  for (uint8_t i = 0; i < 6; i++) {
    if (i > 0) json += ",";
    json += "\"";
    json += jsonEscape(String(baseUnits[i]));
    json += "\"";
  }
  json += "]}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", json);
}

void handleApiOledSave() {
  lastWebRequest = millis();

  String s = server.arg("list");
  uint8_t tmp[MAX_SCREENS];
  uint8_t n = 0;
  int start = 0;

  while (start < (int)s.length() && n < MAX_SCREENS) {
    int comma = s.indexOf(',', start);
    if (comma < 0) comma = s.length();

    int id = s.substring(start, comma).toInt();

    if (screenValid(id)) {
      bool dup = false;
      for (uint8_t i = 0; i < n; i++) {
        if (tmp[i] == id) dup = true;
      }
      if (!dup) tmp[n++] = (uint8_t)id;
    }

    start = comma + 1;
  }

  if (n == 0) {
    server.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  memcpy(screenList, tmp, n);
  screenCount = n;

  saveOledConfig();
  syncScreenIndex();

  server.send(200, "application/json", "{\"ok\":true}");
}

// Eigenen Messwert anlegen (Klick auf MWB-Karte)
void handleApiOledCustom() {
  lastWebRequest = millis();

  int g = server.arg("g").toInt();
  int f = server.arg("f").toInt();

  if (g < 1 || g > 255 || f < 1 || f > 4) {
    server.send(400, "application/json; charset=utf-8",
      "{\"ok\":false,\"msg\":\"Ungültige Angabe\"}");
    return;
  }

  // Gleicher Block/Feld schon vorhanden -> aktualisieren
  int slot = -1;
  for (uint8_t i = 0; i < MAX_CUSTOM; i++) {
    if (customScreens[i].group == g && customScreens[i].field == f) slot = i;
  }

  // Sonst ersten freien Platz nehmen
  if (slot < 0) {
    for (uint8_t i = 0; i < MAX_CUSTOM; i++) {
      if (customScreens[i].group == 0) { slot = i; break; }
    }
  }

  if (slot < 0) {
    server.send(409, "application/json; charset=utf-8",
      "{\"ok\":false,\"msg\":\"Alle 4 Plätze belegt. Bitte unter OLED-Anzeige einen löschen.\"}");
    return;
  }

  char label[10];
  sanitizeLabel(server.arg("n"), label);

  customScreens[slot].group = (uint8_t)g;
  customScreens[slot].field = (uint8_t)f;
  memcpy(customScreens[slot].label, label, sizeof(label));

  // In die Liste aufnehmen, falls noch nicht drin
  uint8_t id = SCR_CUSTOM1 + slot;
  bool inList = false;
  for (uint8_t i = 0; i < screenCount; i++) {
    if (screenList[i] == id) { inList = true; screenIndex = i; }
  }
  if (!inList && screenCount < MAX_SCREENS) {
    screenList[screenCount] = id;
    screenIndex = screenCount;
    screenCount++;
  }

  saveOledConfig();

  // Sofort auf dem OLED anzeigen
  applyScreen(id);

  server.send(200, "application/json", "{\"ok\":true}");
}

// Eigenen Messwert löschen
void handleApiOledDelete() {
  lastWebRequest = millis();

  int slot = server.arg("slot").toInt();
  if (slot < 0 || slot >= MAX_CUSTOM) {
    server.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  memset(&customScreens[slot], 0, sizeof(CustomScreen));

  // Aus der Reihenfolge entfernen
  uint8_t id = SCR_CUSTOM1 + slot;
  uint8_t w = 0;
  for (uint8_t i = 0; i < screenCount; i++) {
    if (screenList[i] != id) screenList[w++] = screenList[i];
  }
  screenCount = w;

  if (screenCount == 0) {
    screenList[0] = SCR_VOLTAGE;
    screenCount = 1;
  }

  saveOledConfig();

  // Wird der gelöschte Wert gerade angezeigt -> ersten Bildschirm zeigen
  syncScreenIndex();

  server.send(200, "application/json", "{\"ok\":true}");
}

// Einheit / Bezeichnung ändern
void handleApiOledUnit() {
  lastWebRequest = millis();

  int id = server.arg("id").toInt();

  char label[10];
  sanitizeLabel(server.arg("n"), label);

  if (id >= SCR_VOLTAGE && id <= SCR_INJECTION) {

    // Leer -> Standard wiederherstellen
    if (label[0] == 0) {
      strcpy(label, DEFAULT_UNITS[id]);
    }
    memcpy(baseUnits[id], label, sizeof(label));

  } else if (id >= SCR_CUSTOM1 &&
             id < SCR_CUSTOM1 + MAX_CUSTOM &&
             customScreens[id - SCR_CUSTOM1].group != 0) {

    // Leer -> Einheit aus der Kennzahl wird angezeigt
    memcpy(customScreens[id - SCR_CUSTOM1].label, label, sizeof(label));

  } else {
    server.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  saveOledConfig();

  // Läuft der Bildschirm gerade, sofort neu zeichnen
  if (displayMode != MODE_CODEBLOCK && currentScreenId == id) {
    if (displayMode == MODE_VOLTAGE) {
      lastVoltageDisplay = 0;
      updateVoltageDisplay();
    } else {
      oledShowNormal();
    }
  }

  String json = "{\"ok\":true,\"n\":\"";
  json += jsonEscape(String(label));
  json += "\"}";

  server.send(200, "application/json; charset=utf-8", json);
}

// =====================================================
// SETUP
// =====================================================

void setup() {
    // Serieller Monitor für Debug-Ausgaben
  Serial.begin(115200);

  // Gespeicherte OLED-Konfiguration laden
  loadOledConfig();

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

// Aktuellen Tastenzustand übernehmen
buttonWasPressed =
  (digitalRead(BUTTON_INPUT) == LOW);

buttonLastEdgeMs = millis();

// Falls der Taster beim Einschalten bereits gedrückt ist,
// beginnt die Zeitmessung ab jetzt.
if (buttonWasPressed) {
  buttonPressStartMs = millis();
}

// Interrupt auf Drücken und Loslassen aktivieren
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
  // WLAN-HOTSPOT + WEBSERVER
  // ------------------------------------------

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  // Beim ESP32-C3 SuperMini ist WLAN bei voller
  // Sendeleistung oft instabil -> reduzieren
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  server.on("/", handleRoot);
  server.on("/api/live", handleApiLive);
  server.on("/mwb", handleMwbPage);
  server.on("/api/mwb", handleApiMwb);

  server.on("/faults", HTTP_GET, handleFaultPage);
  server.on("/api/faults", HTTP_GET, handleApiFaults);
  server.on("/api/faults/read", HTTP_POST, handleApiFaultRead);
  server.on("/api/faults/clear", HTTP_POST, handleApiFaultClear);

  server.on("/oled", HTTP_GET, handleOledPage);
  server.on("/api/oled", HTTP_GET, handleApiOled);
  server.on("/api/oled/save", HTTP_POST, handleApiOledSave);
  server.on("/api/oled/custom", HTTP_POST, handleApiOledCustom);
  server.on("/api/oled/delete", HTTP_POST, handleApiOledDelete);
  server.on("/api/oled/unit", HTTP_POST, handleApiOledUnit);

  server.begin();

  Serial.print("Hotspot gestartet, IP: ");
  Serial.println(WiFi.softAPIP());

  // Beim Start einmal den Fehlerspeicher lesen.
  // Ausgeführt wird das im ersten loop()-Durchlauf.
  startupFaultCheck = true;
  pendingFaultRequest = FAULT_REQUEST_READ;
  faultStatusText = "Startcheck: Fehlerspeicher wird gelesen...";
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

  // Erster Bildschirm aus der gespeicherten Liste
  screenIndex = 0;
  applyScreen(screenList[0]);
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  server.handleClient();

  handleButton();

  // Fehlerspeicher hat Vorrang
  processFaultRequest();

  if (!faultRequestRunning &&
      pendingFaultRequest == FAULT_REQUEST_NONE) {
    updateMeasurements();
  }

  delay(5);
}