#include <DMXSerial.h>
#include <Arduino.h>

// Definieren der Tastenpins
const int BUT_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // D2 bis D13
const int NUM_BUTTONS = 9; // Nur die ersten 9 Tasten werden genutzt

// Namen der Tasten (für DMX-Mapping)
const String BUTTON_NAMES[] = {
  "1_FINE",
  "2_FLIP_X",
  "3_FLIP_Y",
  "4_UP",
  "5_DOWN",
  "6_LEFT",
  "7_RIGHT",
  "8_FOCUS+",
  "9_FOCUS-"
};

// Definieren der LED-Pins
const int LED_1_PIN = A0;
const int LED_2_PIN = A1;
const int LED_3_PIN = A2;

// Zustände der Tasten und LEDs
bool buttonState[NUM_BUTTONS];
bool lastButtonState[NUM_BUTTONS];

// Globale Variablen für die Modus-Steuerung
bool fine_active = false; // Steuert die Pan- und Tilt-Geschwindigkeit (50% oder 100%)
bool flipX_active = false; // Steuert den Wertebereich/Geschwindigkeit für LEFT/RIGHT
bool flipY_active = false; // Steuert den Wertebereich/Geschwindigkeit für UP/DOWN

// Funktion zum Zurücksetzen aller DMX-Kanäle auf 0
void resetDMX() {
  for (int i = 1; i <= 512; i++) {
    DMXSerial.write(i, 0);
  }
  // DMX-Daten senden
  Serial.println("DMX Kanäle auf 0 zurückgesetzt.");
}

// Funktion zum Senden spezifischer DMX-Parameter basierend auf der gedrückten Taste
void sendDMXPattern(int buttonIndex) {
  // Zuerst Pan, Tilt und Focus auf 'keine Bewegung' setzen, bevor neue Werte gesendet werden
  DMXSerial.write(1, 0); // Pan Kanal auf 0 (keine Bewegung)
  DMXSerial.write(3, 0); // Tilt Kanal auf 0 (keine Bewegung)
  DMXSerial.write(501, 0); // Fokus Kanal auf 0 (keine Bewegung)
  
  // Die DMX-Werte für FINE, FLIP_X, FLIP_Y werden bereits in loop() gesetzt.

  switch (buttonIndex) {
    case 0: // 1_FINE
      // DMX-Aktion erfolgt bereits in loop() beim Umschalten des Modus
      break;
    case 1: // 2_FLIP_X (Modus-Umschalter für Pan Speed)
      // DMX-Aktion erfolgt bereits in loop() beim Umschalten des Modus
      break;
    case 2: // 3_FLIP_Y (Modus-Umschalter für Tilt Speed)
      // DMX-Aktion erfolgt bereits in loop() beim Umschalten des Modus
      break;
    case 3: // 4_UP
      // Kanal 3: 0-127 tilt up
      DMXSerial.write(3, 63); // Beispiel für eine Up-Bewegung (Mitte des Up-Bereichs)
      Serial.println("DMX Pattern für UP (Kanal 3: 0-127 tilt up).");
      break;
    case 4: // 5_DOWN
      // Kanal 3: 128-255 tilt down
      DMXSerial.write(3, 191); // Beispiel für eine Down-Bewegung (Mitte des Down-Bereichs)
      Serial.println("DMX Pattern für DOWN (Kanal 3: 128-255 tilt down).");
      break;
    case 5: // 6_LEFT
      // Kanal 1: 0-127 pan_left
      DMXSerial.write(1, 63); // Beispiel für eine Left-Bewegung (Mitte des Left-Bereichs)
      Serial.println("DMX Pattern für LEFT (Kanal 1: 0-127 pan_left).");
      break;
    case 6: // 7_RIGHT
      // Kanal 1: 128-255 pan_right
      DMXSerial.write(1, 191); // Beispiel für eine Right-Bewegung (Mitte des Right-Bereichs)
      Serial.println("DMX Pattern für RIGHT (Kanal 1: 128-255 pan_right).");
      break;
    case 7: // 8_FOCUS+
      // Kanal 501: 192-255 focus+
      DMXSerial.write(501, 223); // Beispiel für Focus+ (Mitte des Focus+ Bereichs)
      Serial.println("DMX Pattern für FOCUS+ (Kanal 501: 192-255 focus+).");
      break;
    case 8: // 9_FOCUS-
      // Kanal 501: 128-191 focus-
      DMXSerial.write(501, 159); // Beispiel für Focus- (Mitte des Focus- Bereichs)
      Serial.println("DMX Pattern für FOCUS- (Kanal 501: 128-191 focus-).");
      break;
    default:
      break;
  }
  // Wichtig: Sendet die DMX-Daten nur einmal am Ende der Funktion, nachdem alle Werte gesetzt wurden.
 
}

// Funktion zum Aktualisieren der LEDs
void updateLEDs(int buttonNumber) {
  // LEDs für Modus-Tasten zeigen den Modus-Zustand an, nicht nur den Tastendruck
  if (buttonNumber == 1) { // LED für FINE
    digitalWrite(LED_1_PIN, fine_active ? HIGH : LOW);
  } else if (buttonNumber == 2) { // LED für FLIP_X
    digitalWrite(LED_2_PIN, flipX_active ? HIGH : LOW);
  } else if (buttonNumber == 3) { // LED für FLIP_Y
    digitalWrite(LED_3_PIN, flipY_active ? HIGH : LOW);
  }
  // Für andere Tasten (die keine LEDs haben oder deren LEDs kurz aufleuchten sollen)
  // ... hier könnten weitere Logiken für LED-Anzeigen implementiert werden, falls gewünscht.
}

void setup() {
 // Serial.begin(9600); // Für Debugging

  // Initialisiere DMXSerial
    DMXSerial.init(DMXController);


  // Setze Tastenpins als INPUT_PULLUP
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUT_PINS[i], INPUT_PULLUP);
    lastButtonState[i] = HIGH; // Da INPUT_PULLUP, HIGH im Ruhezustand
  }

  // Setze LED-Pins als OUTPUT
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);

  // LEDs initial ausschalten
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
  digitalWrite(LED_3_PIN, LOW);
}

void loop() {
  // Überprüfe den Zustand der Tasten
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonState[i] = digitalRead(BUT_PINS[i]);

    // Wenn Taste von NICHT gedrückt auf gedrückt wechselt (HIGH zu LOW)
    if (buttonState[i] == LOW && lastButtonState[i] == HIGH) {
      Serial.print("Taste gedrückt: ");
      Serial.println(BUTTON_NAMES[i]);

      // Spezielle Behandlung für FINE als Umschalter für Geschwindigkeiten
      if (BUTTON_NAMES[i] == "1_FINE") {
        fine_active = !fine_active; // Modus umschalten
        Serial.print("FINE Modus (Gesamtgeschwindigkeit): ");
        Serial.println(fine_active ? "Aktiv (50%)" : "Inaktiv (100%)");
        // Pan- und Tilt-Geschwindigkeit anpassen
        DMXSerial.write(5, fine_active ? 127 : 255); // Kanal 5 Pan Speed (127 = ~50%, 255 = 100%)
        DMXSerial.write(6, fine_active ? 127 : 255); // Kanal 6 Tilt Speed (127 = ~50%, 255 = 100%)
       
      }
      // Spezielle Behandlung für FLIP_X und FLIP_Y als Umschalter
      else if (BUTTON_NAMES[i] == "2_FLIP_X") {
        flipX_active = !flipX_active; // Modus umschalten
        Serial.print("FLIP_X Modus (Pan Speed): ");
        Serial.println(flipX_active ? "Aktiv (Langsamere Geschwindigkeit)" : "Inaktiv (Schnellere Geschwindigkeit)");
        // Wenn FLIP_X aktiviert/deaktiviert wird, setzen wir die Pan-Geschwindigkeit
        // Berücksichtigt den FINE-Modus für die Grundgeschwindigkeit
        DMXSerial.write(5, flipX_active ? (fine_active ? 25 : 50) : (fine_active ? 127 : 255)); // Beispielwerte
       
      } else if (BUTTON_NAMES[i] == "3_FLIP_Y") {
        flipY_active = !flipY_active; // Modus umschalten
        Serial.print("FLIP_Y Modus (Tilt Speed): ");
        Serial.println(flipY_active ? "Aktiv (Langsamere Geschwindigkeit)" : "Inaktiv (Schnellere Geschwindigkeit)");
        // Wenn FLIP_Y aktiviert/deaktiviert wird, setzen wir die Tilt-Geschwindigkeit
        // Berücksichtigt den FINE-Modus für die Grundgeschwindigkeit
        DMXSerial.write(6, flipY_active ? (fine_active ? 25 : 50) : (fine_active ? 127 : 255)); // Beispielwerte
       
      }

      // DMX-Werte für die gedrückte Taste senden (außer für reine Umschalter)
      // Die DMX-Werte für FINE, FLIP_X, FLIP_Y werden bereits oben gesetzt
      if (BUTTON_NAMES[i] != "1_FINE" && BUTTON_NAMES[i] != "2_FLIP_X" && BUTTON_NAMES[i] != "3_FLIP_Y") {
        sendDMXPattern(i);
      }

      // Status-LEDs aktualisieren
      updateLEDs(i + 1); // +1, da Tasten von 0-Index bis 8 laufen, aber LEDs von 1-3
    }
    // Wenn Taste von gedrückt auf NICHT gedrückt wechselt (LOW zu HIGH)
    else if (buttonState[i] == HIGH && lastButtonState[i] == LOW) {
      Serial.print("Taste losgelassen: ");
      Serial.println(BUTTON_NAMES[i]);

      // Nur Bewegung stoppen, wenn es keine Modus-Taste ist
      if (BUTTON_NAMES[i] != "1_FINE" && BUTTON_NAMES[i] != "2_FLIP_X" && BUTTON_NAMES[i] != "3_FLIP_Y") {
        // Pan/Tilt Bewegung stoppen
        DMXSerial.write(1, 0); // Pan Kanal auf 0 (keine Bewegung)
        DMXSerial.write(3, 0); // Tilt Kanal auf 0 (keine Bewegung)
        DMXSerial.write(501, 0); // Fokus Kanal auf 0 (keine Bewegung)
       
        Serial.println("Pan/Tilt/Focus-Bewegung gestoppt.");
      }
     
      // LEDs ausschalten, wenn die zugehörige Taste losgelassen wird (außer Modus-LEDs)
      // Die LEDs für FINE, FLIP_X und FLIP_Y zeigen den Modus an, nicht nur den Tastendruck
      if (i == 0 && !fine_active) digitalWrite(LED_1_PIN, LOW);
      if (i == 1 && !flipX_active) digitalWrite(LED_2_PIN, LOW);
      if (i == 2 && !flipY_active) digitalWrite(LED_3_PIN, LOW);
    }
    lastButtonState[i] = buttonState[i];
  }

  // Optional: Eine kleine Verzögerung, um Prellen zu reduzieren
  delay(50);
}

