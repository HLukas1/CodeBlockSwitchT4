# VW-MSG-Anzeige mit ESP32-C3

Der Sketch verbindet einen ESP32-C3 über die K-Leitung mit einem VW-Motorsteuergerät (MSG), liest ausgewählte Messwertblöcke über KWP1281 und zeigt Messwerte auf einem kleinen OLED an. Zusätzlich misst der ESP32 die Bordspannung und kann im Codeblock-Modus zwischen den Codierungen 2 und 3 umschalten.

> **Hinweis:** Das README wurde aus dem bereitgestellten Sketch abgeleitet. Fahrzeugtyp, MSG-Variante und genaue Bauteile der K-Line- und Spannungsteiler-Schaltung sind daraus nicht vollständig ersichtlich. Vor dem Anschluss müssen diese mit der eigenen Hardware abgeglichen werden.

## Funktionen

- Verbindung zum MSG über KWP1281 mit 5-Baud-Adressierung und anschließender Kommunikation mit 9600 Baud.
- Anzeige der Bordspannung, Motordrehzahl, des Ladedrucks, des berechneten Verbrauchs und der Einspritzmenge.
- Auslesen und Anzeigen der MSG-Codierung und des WSC (Werkstattcode) während des Verbindungsaufbaus.
- Codeblock-Modus: Codierung 2 und 3 können gewechselt werden; der vorhandene WSC wird dabei übernommen.
- Testbench-Modus mit Beispielwerten für die Anzeige, ohne eine Fahrzeugverbindung aufzubauen.

## Benötigte Hardware

- ESP32-C3 SuperMini
- VW-MSG mit KWP1281-Unterstützung
- K-Line-Transceiver, im Kommentar des Sketches als L9637D bezeichnet
- OLED mit SSD1306-Controller, 64 × 48 Pixel, I²C-Adresse `0x3C`
- Taster zwischen GPIO 9 und GND
- Spannungsteiler für die Bordspannungsmessung an GPIO 0

Die K-Leitung darf **nicht direkt** an einen ESP32-GPIO angeschlossen werden. Verwende einen geeigneten K-Line-Transceiver und eine für das Fahrzeug ausgelegte Beschaltung. Auch der Spannungsteiler muss für die maximale Eingangsspannung und die zulässige ADC-Spannung des konkreten Boards ausgelegt sein.

## Anschlussbelegung

| Funktion | ESP32-C3-Pin | Hinweise |
|---|---:|---|
| K-Line TX | GPIO 10 | Über den K-Line-Transceiver |
| K-Line RX | GPIO 1 | Vom K-Line-Transceiver |
| OLED SDA | GPIO 6 | I²C |
| OLED SCL | GPIO 7 | I²C |
| Taster-Eingang | GPIO 9 | Intern mit `INPUT_PULLUP`; Taster nach GND |
| Taster-Ausgang | GPIO 5 | Im Sketch als LOW-Ausgang konfiguriert |
| Eingebaute LED | GPIO 8 | Wird beim Start als Ausgang konfiguriert |
| Spannungsmessung | GPIO 0 | Über externen Spannungsteiler |

Für die Spannungsmessung verwendet der Sketch derzeit `R_TOP = 47 MΩ`, `R_BOTTOM = 10 MΩ` und einen Korrekturwert von `-1,24 V`. Diese Werte müssen zur tatsächlich aufgebauten Schaltung passen. Der Korrekturwert sollte anhand einer bekannten Spannung kalibriert werden.

## Software und Einrichtung

Der Sketch verwendet die Arduino-ESP32-Umgebung sowie diese Bibliotheken:

- `Adafruit GFX Library`
- `Adafruit SSD1306`

Wähle in der Arduino-IDE ein passendes ESP32-C3-Board und den zugehörigen seriellen Port. Lade den Sketch hoch und öffne bei Bedarf den seriellen Monitor mit **115200 Baud**. Die serielle Ausgabe dient vor allem zur Diagnose und zeigt Verbindungsstatus, gelesene MSG-Daten und Fehler.

Die Parameter für Pins, OLED, K-Line-Zeiten, Spannungsteiler und Testbench sind am Anfang des Sketches definiert. Für einen simulierten Betrieb ohne angeschlossenes Fahrzeug setze:

```cpp
#define TESTBENCH_MODE 1
```

Im normalen Fahrzeugbetrieb bleibt der Wert `0`.

## Bedienung

Nach dem Start wird kurz ein VW-Logo gezeigt. Danach beginnt die Anzeige mit der Bordspannung. Ein kurzer Tastendruck wechselt durch die Ansichten:

1. Bordspannung
2. Drehzahl
3. Ladedruck
4. Verbrauch
5. Einspritzmenge
6. Codeblock-Modus
7. zurück zur Bordspannung

Im Codeblock-Modus wechselt ein langer Tastendruck (mindestens 3 Sekunden) zwischen Codierung 2 und 3. Der Sketch verbindet sich dazu mit dem MSG, schreibt die neue Codierung und beendet die Verbindung anschließend.

## Messwerte und Berechnung

- Messwerte werden ungefähr alle 500 ms aktualisiert.
- Die Drehzahl wird aus dem ersten Feld des Messwertblocks 000 beziehungsweise aus den dafür verwendeten Messwertblöcken übernommen.
- Der Ladedruck wird über KWP1281-Formelkennung `0x12` umgerechnet: `0,04 × a × b`.
- Einspritzmengen mit Formelkennung `0x27` werden als `b / 256 × a` berechnet. Für die tatsächliche Einspritzmenge nimmt der Sketch den kleinsten Wert aus Fahrerwunsch sowie Drehmoment- und Rauchbegrenzung.
- Der Verbrauch wird aus Einspritzmenge und Drehzahl abgeleitet. Dabei nimmt die Berechnung einen Fünfzylinder-Viertaktmotor und eine Dieseldichte von 0,832 kg/l an. Im Stand zeigt sie 0 l/100 km.

Die Annahmen zur Verbrauchsberechnung sind motorspezifisch. Prüfe außerdem vor dem Fahrzeugeinsatz, ob die verwendeten Messwertblöcke und Umrechnungsformeln zum konkreten MSG passen.

## Hinweise zum Fahrzeugbetrieb

Das Schreiben einer Codierung verändert MSG-Daten. Verwende den Codeblock-Wechsel nur, wenn die Werte für dein Steuergerät bekannt und geeignet sind. Eine stabile Versorgung, korrekte Masseführung und ein passender K-Line-Transceiver sind Voraussetzung für zuverlässige Kommunikation. Der Sketch versucht eine Verbindung bis zu viermal; bei fehlgeschlagener Kommunikation wird die Verbindung zurückgesetzt.

## Einschränkungen des bereitgestellten Sketches

Der Sketch enthält Funktionen zum seriellen Einlesen einer neuen Codierung und eines WSC. Im gezeigten Hauptprogramm wird jedoch kein serieller Befehlsdialog aufgerufen. Der beschriebene Bedienweg für die Codierungsänderung ist daher der Codeblock-Modus mit dem Taster.

