// MwbTable.h - Messwertblock-Tabelle + findMwbIndex()
#pragma once
#include <Arduino.h>

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

