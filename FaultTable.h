// FaultTable.h - Fehlercode-Tabelle T4 ACV + findFaultIndex()
#pragma once
#include <Arduino.h>

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
  {16485, "P0101", "Luftmassenmesser (G70): unplausibles Signal", "Luftmassenmesser (G70); Leitungsunterbrechung bzw. Kurzschluss nach Masse", "Verminderte Leistung; Schwarzrauch"},
  {16500, "P0116", "Kühlmitteltemperaturgeber (G62): unplausibles Signal", "Kühlmitteltemperaturgeber (G62)", "Schwarzrauch beim Start; es wird immer für ca. 20 s vorgeglüht"},
  {16705, "P0321", "Motordrehzahlgeber (G28): unplausibles Signal", "Motordrehzahlgeber (G28) defekt; Abstand zwischen Drehzahlgeber und Geberrad zu groß; Metallspäne am Geber bzw. Haltesockel lose", "Motor springt nicht an; Motor geht aus; Vorglühkontrolllampe blinkt; Drehzahlmesser ohne Anzeige"},
  {16706, "P0322", "Motordrehzahlgeber (G28): kein Signal", "Motordrehzahlgeber (G28) defekt; Leitungsunterbrechung bzw. Kurzschluss", "Motor springt nicht an; Motor geht aus; Vorglühkontrolllampe blinkt; Drehzahlmesser ohne Anzeige"},
  {16885, "P0501", "Fahrzeug-Geschwindigkeitssignal: unplausibles Signal", "Kein Signal vom Geber für Geschwindigkeitsmesser (G22)", "Geschwindigkeitsanzeige nicht i. O.; GRA abgeschaltet; Fahrzeug ruckelt beim Schalten; Klimaanlage wird abgeschaltet"},
  {16955, "P0571", "Bremslichtschalter (F): unplausibles Signal", "Bremslichtschalter (F) defekt; Bremspedalschalter (F47) defekt", "Vorglühkontrolllampe blinkt; Bremslicht defekt"},
  {16989, "P0605", "Steuergerät für Diesel-Direkteinspritzanlage (J248) defekt", "Steuergerät intern defekt", "Fahrverhaltensmängel; Motorstillstand"},
  {17552, "P1144", "Luftmassenmesser (G70): Unterbrechung/Kurzschluss nach Masse", "Luftmassenmesser (G70) defekt; Leitungsunterbrechung bzw. Kurzschluss nach Masse", "Verminderte Leistung; Schwarzrauch"},
  {17553, "P1145", "Luftmassenmesser (G70): Kurzschluss nach Plus", "Luftmassenmesser (G70) defekt; Leitungsunterbrechung bzw. Kurzschluss nach Plus", "Verminderte Leistung; Schwarzrauch"},
  {17554, "P1146", "Luftmassenmesser (G70): Versorgungsspannung", "Betriebsspannung zu groß bzw. zu klein; Leitungsunterbrechung", "Verminderte Leistung; Schwarzrauch"},
  {17563, "P1155", "Saugrohrdruckgeber (G71): Kurzschluss nach Plus", "Kurzschluss nach Plus; Saugrohrdruckgeber (G71) defekt", "Verminderte Leistung"},
  {17564, "P1156", "Saugrohrdruckgeber (G71): Unterbrechung/Kurzschluss nach Masse", "Saugrohrdruckgeber (G71) defekt; Leitungsunterbrechung bzw. Kurzschluss nach Masse", "Verminderte Leistung; erhöhte Emissionswerte"},
  {17565, "P1157", "Saugrohrdruckgeber (G71): Versorgungsspannung", "Saugrohrdruckgeber (G71) defekt; Leitungsunterbrechung bzw. Kurzschluss", "Verminderte Leistung; erhöhte Emissionswerte"},
  {17568, "P1160", "Saugrohrtemperaturgeber (G72): Kurzschluss nach Masse", "Saugrohrtemperaturgeber (G72) defekt; Leitung hat Kurzschluss nach Masse", "Messwert geht auf Vorgabewert 134,96 °C"},
  {17569, "P1161", "Saugrohrtemperaturgeber (G72): Unterbrechung/Kurzschluss nach Plus", "Saugrohrtemperaturgeber (G72) defekt; Leitungsunterbrechung bzw. Kurzschluss nach Plus", "Messwert geht auf Vorgabewert 134,96 °C"},
  {17570, "P1162", "Kraftstofftemperaturgeber (G81): Kurzschluss nach Masse", "Kraftstofftemperaturgeber (G81) defekt; Leitung hat Kurzschluss nach Masse", "Messwert geht auf Vorgabewert -5,34 °C; erhöhte Emissionswerte"},
  {17571, "P1163", "Kraftstofftemperaturgeber (G81): Unterbrechung/Kurzschluss nach Plus", "Kraftstofftemperaturgeber (G81) defekt; Leitungsunterbrechung bzw. Kurzschluss nach Plus", "Messwert geht auf Vorgabewert -5,34 °C; erhöhte Emissionswerte"},
  {17653, "P1245", "Nadelhubgeber (G80): Kurzschluss nach Masse", "Nadelhubgeber (G80) defekt; Leitung hat Kurzschluss nach Masse", "Vorglühkontrolllampe blinkt; rauer Motorlauf; verminderte Leistung; erhöhte Emissionswerte"},
  {17654, "P1246", "Nadelhubgeber (G80): unplausibles Signal", "Nadelhubgeber (G80) defekt; Einspritzleitung zur Einspritzdüse mit Nadelhubgeber nicht i. O.; Kraftstoffmangel; Luft im Kraftstoffsystem", "Vorglühkontrolllampe blinkt; rauer Motorlauf; verminderte Leistung; erhöhte Emissionswerte"},
  {17655, "P1247", "Nadelhubgeber (G80): Unterbrechung/Kurzschluss nach Plus", "Nadelhubgeber (G80) defekt; Leitungsunterbrechung bzw. Kurzschluss", "Vorglühkontrolllampe blinkt; rauer Motorlauf; verminderte Leistung; erhöhte Emissionswerte"},
  {17656, "P1248", "Spritzbeginnregelung: Regeldifferenz", "Ventil (N108) defekt; Nadelhubgeber (G80) defekt; Tank leer gefahren; Kraftstoffzufuhr nicht i. O.; Kraftstoffmangel; Förderbeginn nicht i. O.", "Rauer Motorlauf; verminderte Leistung; erhöhte Emissionswerte; schlechtes Kaltstartverhalten"},
  {17659, "P1251", "Ventil für Einspritzbeginn (N108): Kurzschluss nach Plus", "Kurzschluss nach Plus", "Motor nagelt im Leerlauf, da der Einspritzbeginn konstant auf „früh“ steht"},
  {17660, "P1252", "Ventil für Einspritzbeginn (N108): Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Ventil (N108) defekt; Kurzschluss nach Masse", "Motor nagelt im Leerlauf bei Einspritzbeginn „früh“; Leistungsmangel bei Einspritzbeginn „spät“"},
  {17663, "P1255", "Kühlmitteltemperaturgeber (G62): Kurzschluss nach Masse", "Kühlmitteltemperaturgeber (G62) defekt; Leitung hat Kurzschluss nach Masse", "Schwarzrauch beim Start; es wird immer für ca. 20 s vorgeglüht"},
  {17664, "P1256", "Kühlmitteltemperaturgeber (G62): Unterbrechung/Kurzschluss nach Plus", "Kühlmitteltemperaturgeber (G62) defekt; Leitungsunterbrechung bzw. Kurzschluss nach Plus", "Schwarzrauch beim Start; es wird immer für ca. 20 s vorgeglüht"},
  {17762, "P1354", "Regelschieberweggeber (G149): elektrischer Fehler im Stromkreis", "Einspritzpumpe defekt; Leitungsunterbrechung bzw. Kurzschluss", "Vorglühkontrolllampe blinkt; Fahrverhaltensmängel; Motorstillstand"},
  {17795, "P1387", "Steuergerät defekt", "Steuergerät intern defekt", "Fahrverhaltensmängel; Motorstillstand"},
  {17810, "P1402", "Abgasrückführungsventil (N18): Kurzschluss nach Plus", "Kurzschluss nach Plus", "Keine Abgasrückführung"},
  {17849, "P1441", "Abgasrückführungsventil (N18): Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Ventil (N18) defekt; Kurzschluss nach Masse", "Keine oder zu viel Abgasrückführung; verminderte Leistung; Schwarzrauch"},
  {17945, "P1537", "Kraftstoffabschaltventil (N109): Fehlfunktion", "Ventil (N109) defekt, undicht oder klemmt", "Verminderte Leistung; Motorstillstand"},
  {17946, "P1538", "Kraftstoffabschaltventil (N109): Unterbrechung/Kurzschluss nach Masse", "Ventil (N109) defekt; Leitungsunterbrechung bzw. Kurzschluss nach Masse", "Verminderte Leistung; Motorstillstand"},
  {17948, "P1540", "Fahrzeug-Geschwindigkeitssignal: Signal zu groß", "Fahrgeschwindigkeit größer 260 km/h; Schalttafeleinsatz defekt", "Geschwindigkeitsanzeige nicht i. O.; GRA abgeschaltet; Fahrzeug ruckelt beim Schalten; Klimaanlage wird abgeschaltet"},
  {17954, "P1546", "Magnetventil für Ladedruckbegrenzung (N75): Kurzschluss nach Plus", "Kurzschluss nach Plus", "Verminderte Leistung; Ladedruck zu niedrig"},
  {17957, "P1549", "Magnetventil für Ladedruckbegrenzung (N75): Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Magnetventil defekt; Kurzschluss nach Masse", "Verminderte Leistung; je nach Fehler Ladedruck zu niedrig oder zu hoch"},
  {17964, "P1556", "Ladedruckregelung: Regelgrenze unterschritten", "Ventil (N75) defekt; Turbolader defekt; Undichtigkeit zwischen Turbolader und Motor", "Ladedruck zu niedrig; verminderte Leistung"},
  {17965, "P1557", "Ladedruckregelung: Regelgrenze überschritten", "Schlauchanschlüsse vertauscht oder nicht aufgesteckt", "Ladedruck zu hoch; verminderte Leistung"},
  {17969, "P1561", "Mengensteller (N146): Regeldifferenz", "Einspritzpumpe defekt; Leitungsunterbrechung bzw. Kurzschluss", "Vorglühkontrolllampe blinkt; Fahrverhaltensmängel; Motorstillstand"},
  {17970, "P1562", "Mengensteller (N146): oberer Anschlagwert", "Mengensteller (N146) defekt oder blockiert; oberer Anschlagwert erreicht", "Verminderte Leistung; Ruckeln"},
  {17971, "P1563", "Mengensteller (N146): unterer Anschlagwert", "Mengensteller (N146) defekt oder blockiert; unterer Anschlagwert erreicht", "Schwarzrauch; unrunder Leerlauf"},
  {17977, "P1569", "Schalter für GRA (E45): unplausibles Signal", "Schalter (E45) defekt; Leitungsunterbrechung bzw. Kurzschluss", "Geschwindigkeitsregelanlage ist abgeschaltet"},
  {17978, "P1570", "Motorsteuergerät gesperrt", "Nicht berechtigter Schlüssel; Manipulationsversuch; Kurzschluss der Kommunikationsleitung; falsche Codierung; Fehler der Wegfahrsicherung; Wegfahrsicherungs-Steuergerät defekt oder fehlt; Motorsteuergerät nicht angepasst", "Motor springt kurz an und geht wieder aus; Vorglühkontrolllampe blinkt"},
  {18008, "P1600", "Spannungsversorgung Klemme 15: Spannung zu klein", "Spannung bei eingeschalteter Zündung an Klemme 15 nicht vorhanden", "Fahrverhaltensmängel bis zum Motorstillstand"},
  {18009, "P1601", "Relais für Spannungsversorgung Klemme 30 (J317)", "Relais (J317) defekt oder hängt sporadisch", "Motor startet nicht; Fahrverhaltensmängel bis zum Motorstillstand"},
  {18020, "P1612", "Motorsteuergerät falsch codiert", "Ungültige Steuergerätecodierung", "Vorglühkontrolllampe blinkt"},
  {18024, "P1616", "Kontrollleuchte für Vorglühzeit (K29): Kurzschluss nach Plus", "Kurzschluss nach Plus", "Vorglühkontrolle ohne Funktion"},
  {18025, "P1617", "Kontrollleuchte für Vorglühzeit (K29): Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Glühlampe defekt; Kurzschluss nach Masse", "Vorglühkontrolle ohne Funktion oder leuchtet ständig"},
  {18026, "P1618", "Relais für Glühkerzen (J52): Kurzschluss nach Plus", "Leitungskurzschluss nach Plus; Relais (J52) defekt", "Kein Vorglühen; schlechtes Kaltstartverhalten; Kontrolllampe (K29) blinkt"},
  {18027, "P1619", "Relais für Glühkerzen (J52): Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung bzw. Kurzschluss nach Masse; Relais (J52) defekt", "Kein Vorglühen; schlechtes Kaltstartverhalten; Kontrolllampe (K29) blinkt"},
  {18034, "P1626", "Datenbus Antrieb: fehlende Botschaft vom Getriebe-Steuergerät", "Fehler in den Datenleitungen zum Getriebe-Steuergerät", "Schaltruck; Lastwechselschlag"},
  {18039, "P1631", "Gaspedalstellungsgeber (G79): Signal zu groß", "Gaspedalstellungsgeber (G79) defekt", "Erhöhte Leerlaufdrehzahl"},
  {18040, "P1632", "Gaspedalstellungsgeber (G79): Versorgungsspannung", "Betriebsspannung zu groß bzw. zu klein; Leitungsunterbrechung", "Vorglühkontrolllampe blinkt; erhöhte Leerlaufdrehzahl"},
  {18047, "P1639", "Geber für Gaspedalstellung (G79): unplausibles Signal", "Gaspedalstellungsgeber (G79) defekt", "Erhöhte Leerlaufdrehzahl"},
  {18048, "P1640", "Steuergerät für Diesel-Direkteinspritzanlage (J248) defekt", "Steuergerät intern defekt", "Fahrverhaltensmängel; Motorstillstand"},
  {18056, "P1648", "Datenbus Antrieb defekt", "Fehler in den Datenleitungen zum Getriebe- bzw. ABS-Steuergerät; Fehler im automatischen Getriebe- bzw. ABS-System", "Schaltruck; Lastwechselschlag; keine Fahrdynamikregelung; keine ASR- oder MSR-Funktion"},
  {18057, "P1649", "Datenbus Antrieb: fehlende Botschaft vom ABS-Steuergerät", "Fehler in den Datenleitungen zum ABS-Steuergerät", "Keine Fahrdynamikregelung; keine ASR- oder MSR-Funktion"},
  {18058, "P1650", "Datenbus Antrieb: fehlende Botschaft vom Kombiinstrument", "Fehler in den Datenleitungen zum Kombiinstrument", "Im Dokument keine Auswirkung angegeben"},
  {18061, "P1653", "Fehlerspeicher des ABS-Steuergeräts auslesen", "Fehler im ABS-Steuergerät", "Keine Fahrdynamikregelung; keine ASR- oder MSR-Funktion"},
  {18062, "P1654", "Fehlerspeicher des Kombiinstruments auslesen", "Geber für Ölstand/Öltemperatur defekt", "Kontrolllampe für Ölstand leuchtet"},
  {18090, "P1682", "Datenbus Antrieb: unplausible Botschaft vom ABS-Steuergerät", "Fehler in den Datenleitungen zum ABS-Steuergerät", "Keine Fahrdynamikregelung; keine ASR- oder MSR-Funktion"},
  {19458, "P3002", "Kick-down-Schalter (F8): unplausibles Signal", "Gaspedalstellungsgeber (G79)", "Schaltpunkte für das automatische Getriebe nicht i. O."},
  {19560, "P3104", "Umschaltventil für Saugrohrklappe (N239): Kurzschluss nach Plus", "Kurzschluss nach Plus", "Harter Motorabstellschlag"},
  {19561, "P3105", "Umschaltventil für Saugrohrklappe (N239): Unterbrechung/Kurzschluss nach Masse", "Leitungsunterbrechung; Umschaltventil (N239) defekt", "Harter Motorabstellschlag"},

  // =====================================================
  // AIRBAG (Adresse 15)
  // =====================================================
  {532, "", "Versorgungsspannung", "Signal zu groß: Drehstromgenerator defekt. Signal zu klein: Batterie leer/defekt oder Leitungen/Steckverbindungen zum Airbag-Steuergerät", "Airbag-Kontrollleuchte leuchtet"},
  {588, "", "Zünder für Airbag Fahrerseite (N95)", "Leitungen/Steckverbindungen defekt; Airbag Fahrerseite (N95) defekt; Wickelfeder/Schleifring im Lenkrad (F138) defekt", "Airbag-Kontrollleuchte leuchtet; Fahrerairbag ggf. ohne Funktion"},
  {589, "", "Zünder 1 für Airbag Beifahrerseite (N131)", "Leitungen/Steckverbindungen defekt; Airbageinheit Beifahrerseite (N131) defekt", "Airbag-Kontrollleuchte leuchtet; Beifahrerairbag ggf. ohne Funktion"},
  {594, "", "Zündkreis Airbag", "Kurzschluss: Leitungen oder Steckverbindungen zu den Airbageinheiten defekt", "Airbag-Kontrollleuchte leuchtet"},
  {595, "", "Crashdaten gespeichert", "Airbag wurde ausgelöst oder Kurzschluss zwischen den Zündkreisen; Steuergerät und beschädigte Bauteile ersetzen", "Airbagsystem ohne Funktion"},
  {1025, "", "Ansteuerung Fehlerwarnlampe defekt", "Kontrollleuchte im Kombiinstrument defekt; Leitungen/Steckverbindungen defekt; Steuergerät defekt", "Airbag-Kontrollleuchte ohne Funktion"},
  {1280, "", "Airbag Beifahrerseite ist abgeschaltet", "Beifahrerairbag per Anpassung abgeschaltet (wird nur bei aktivierter Anpassung gespeichert)", "Beifahrerairbag außer Funktion"},

  // =====================================================
  // KOMBIINSTRUMENT (Adresse 17)
  // =====================================================
  {534, "", "Geber für Öltemperatur (G8)", "Leitungsunterbrechung oder Kurzschluss; Geber defekt (ab MJ 2000 durch G266 ersetzt)", "Ölstand-Kontrollleuchte blinkt ca. 5 s nach Zündung ein; Öltemperatur 155 °C und Ölstand n. i. O. im Messwerteblock"},
  {562, "", "Geber für Ölstand und Öltemperatur (G266)", "Leitungsunterbrechung oder Kurzschluss; Geber (G266) defekt", "Ölstand-Kontrollleuchte blinkt ca. 5 s nach Zündung ein; Öltemperatur 155 °C und Ölstand n. i. O. im Messwerteblock"},
  {667, "", "Außentemperatursignal", "Leitungsunterbrechung oder Kurzschluss; Temperaturfühler für Außentemperatur G17 defekt", "Display im Kombiinstrument zeigt Striche (- - -)"},
  {771, "", "Geber für Kraftstoffvorratsanzeige (G)", "Leitungsunterbrechung oder Kurzschluss zwischen Geber und Kombiinstrument; Geber (G) defekt", "Tankanzeige zeigt 0 (leer)"},
  {778, "", "Geber für Lenkwinkel (G85): keine Kommunikation", "Datenempfang vom Lenkwinkelgeber über Daten-BUS Antrieb gestört; Codierung des Gateways und Fehlerspeicher ABS prüfen", "Funktionen am Daten-BUS Antrieb gestört"},
  {779, "", "Temperaturfühler für Außentemperatur (G17)", "Leitungsunterbrechung oder Kurzschluss; Temperaturfühler (G17) defekt", "Display im Kombiinstrument zeigt Striche (- - -)"},
  {1039, "", "Geber für Kühlmitteltemperatur (G2)", "Leitungsunterbrechung oder Kurzschluss zwischen Geber und Kombiinstrument; Geber (G2) defekt", "Zeiger der Kühlmitteltemperatur steht auf Linksanschlag"},
  {1044, "", "Steuergerät falsch codiert", "Ein Steuergerät am Daten-BUS Antrieb ist falsch codiert oder defekt; Codierungen und Fehlerspeicher aller BUS-Teilnehmer prüfen", "Funktionen am Daten-BUS Antrieb gestört"},
  {1312, "", "Daten-BUS Antrieb defekt", "Fehler in den Datenleitungen oder Daten-BUS im BUS-Off-Zustand; Codierungen und Fehlerspeicher aller BUS-Teilnehmer prüfen", "Funktionen am Daten-BUS Antrieb gestört"},
  {1314, "", "Motorsteuergerät: keine Kommunikation", "Datenempfang vom Motorsteuergerät über Daten-BUS Antrieb gestört; Fehlerspeicher Motor auslesen, Leitungen prüfen", "Funktionen am Daten-BUS Antrieb gestört"},
  {1315, "", "Getriebesteuergerät: keine Kommunikation", "Datenempfang vom Getriebesteuergerät über Daten-BUS Antrieb gestört; Codierung des Gateways prüfen", "Funktionen am Daten-BUS Antrieb gestört"},
  {1316, "", "Bremsensteuergerät: keine Kommunikation", "Datenempfang vom ABS-Steuergerät über Daten-BUS Antrieb gestört; Codierung des Gateways und Fehlerspeicher ABS prüfen", "Funktionen am Daten-BUS Antrieb gestört"},
  {1317, "", "Kombiinstrument: keine Kommunikation", "Fehler in den Datenleitungen oder Steuergerät defekt", "Fahrverhaltensmängel (Schaltruck, Lastwechselschlag); keine Fahrdynamikregelung"},
  {1321, "", "Airbag-Steuergerät: keine Kommunikation", "Datenempfang vom Airbag-Steuergerät über Daten-BUS Antrieb gestört; Codierung des Gateways prüfen", "Funktionen am Daten-BUS Antrieb gestört"},
  {1402, "", "Datenleitung von Navigation: unplausibles Signal", "Leitungsführung defekt oder Radio-Navigationssystem defekt", "Keine Radio-/Navigationsdaten im Kombiinstrument"},

  // =====================================================
  // WEGFAHRSICHERUNG (im Kombiinstrument)
  // =====================================================
  {546, "", "Datenleitung defekt: unplausibles Signal", "W-Leitung zur Motorsteuerung gestört; falsche Baudrate der Wegfahrsicherung; Zündung nach Motorstopp zu früh wieder eingeschaltet (10 s warten); Steuergerät defekt", "Motor springt ggf. nicht an"},
  {1128, "", "Lesespule für Wegfahrsicherung (D2)", "Stecker am Steuergerät nicht gesteckt oder Lesespule mit Leitung defekt; Steuergerät defekt", "Schlüssel wird nicht erkannt, Motor springt ggf. nicht an"},
  {1176, "", "Schlüssel", "Signal zu klein: Lesespule/Leitung defekt oder Transponder im Schlüssel fehlt/defekt. Nicht berechtigt: Schlüssel nicht angepasst", "Motor springt nicht an"},
  {1177, "", "Motorsteuergerät nicht berechtigt", "Motorsteuergerät (oder Steuergerät für Kraftstoffabschaltventil) nicht an die Wegfahrsicherung angepasst", "Motor springt nicht an"},
  {1179, "", "Schlüsselprogrammierung falsch", "Anpassung der Zündschlüssel fehlerhaft oder Transponder fehlt/defekt; alle Schlüssel neu anpassen", "Motor springt mit betroffenem Schlüssel nicht an"},
  {1181, "", "Initialisierung: nur 2 Schlüssel gelernt", "Bei der Erstanpassung werden immer nur zwei Schlüssel angepasst", "Weitere Schlüssel funktionieren nicht"},
  {1202, "", "Diagnoseleitung: Kurzschluss", "Kurzschluss zwischen W- und K-Leitung am Stecker der Wegfahrsicherung; Steuergerät defekt", "Diagnose und Freigabe Motor ggf. gestört"},
  {1237, "", "Kraftstoffabschaltventil (N109) defekt", "Steuergerät für Kraftstoffabschaltventil oder Ventil (N109) defekt", "Motor springt nicht an oder geht aus"},

  // =====================================================
  // ALLE STEUERGERÄTE
  // =====================================================
  {65535, "", "Steuergerät defekt", "Steuergerät defekt; auch Störungen durch schlechte Masse-/Plusverbindungen oder Kurzschluss W-/K-Leitung möglich", "Funktion des Steuergeräts gestört"},
};

const int FAULT_TABLE_SIZE = sizeof(FAULT_TABLE) / sizeof(FAULT_TABLE[0]);

// Liefert Index in FAULT_TABLE oder -1
int findFaultIndex(uint16_t code) {
  for (int i = 0; i < FAULT_TABLE_SIZE; i++) {
    if (FAULT_TABLE[i].code == code) return i;
  }
  return -1;
}

