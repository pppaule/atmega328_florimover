#include <DMXSerial.h>
#include <Arduino.h>
#define DEBUG
// --- Funktionsdeklarationen (Prototypen) ---
void resetDMX();
void sendDMXPattern(int buttonIndex, boolean flip_x, boolean flip_y, boolean fine);
void updateLEDs(int buttonNumber, boolean fine_active, boolean flipX_active, boolean flipY_active);


// ACHTUNG: Die Reihenfolge hier entspricht den physischen Pins D2, D3, D5, D6, D7, D8, D9, D10, D11
const int BUT_PINS[] = {2, 3, 11, 5, 6, 7, 8, 9, 10};
const int NUM_BUTTONS = 9; // Anzahl der physikalisch genutzten Tasten-Pins
const int lospeed=127;
const int hispeed = 255;

const String BUTTON_NAMES[] = {
  "FINE",
  "FLIP_X",
  "FLIP_Y",
  "UP",
  "DOWN",
  "LEFT",
  "RIGHT"
  "FOCUS-",
  "FOCUS+"
};

// Definieren der LED-Pins
const int LED_1_PIN = A0; // Für Taste "1_FINE"
const int LED_2_PIN = A1; // Für Taste "2_FLIP_X"
const int LED_3_PIN = A2; // Für Taste "3_UP" (Beachte, dass dies jetzt UP statt FLIP_Y ist)

// Zustände der Tasten und LEDs
bool buttonState[NUM_BUTTONS];
bool lastButtonState[NUM_BUTTONS];

// Globale Variablen für die Modus-Steuerung
bool fine_active = false; // Steuert die Pan- und Tilt-Geschwindigkeit (50% oder 100%)
bool flipX_active = false; // Steuert den Wertebereich/Geschwindigkeit für LEFT/RIGHT
bool flipY_active = false; // Steuert den Wertebereich/Geschwindigkeit für UP/DOWN


void setup() {

  DMXSerial.init(DMXController);
  resetDMX(); // Setzt alle DMX-Kanäle auf 0
  //Serial.begin(115200);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUT_PINS[i], INPUT_PULLUP);
    lastButtonState[i] = HIGH; 
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

  for (int i = 0; i < NUM_BUTTONS; i++) {
    // if (i >= sizeof(BUTTON_NAMES) / sizeof(BUTTON_NAMES[0])) {
    //   continue; 
    // }

    buttonState[i] = digitalRead(BUT_PINS[i]);
   
    if (buttonState[i] == LOW && lastButtonState[i] == HIGH) {
      if (BUTTON_NAMES[i] == "FINE") {
        fine_active = !fine_active; // Modus umschalten
      }
      else if (BUTTON_NAMES[i] == "FLIP_X") {
        flipX_active = !flipX_active; // Modus umschalten     
      } else if (BUTTON_NAMES[i] == "FLIP_Y") { 
        flipY_active = !flipY_active; // Modus umschalten
      }
      if (BUTTON_NAMES[i] != "FINE" && BUTTON_NAMES[i] != "FLIP_X" && BUTTON_NAMES[i] != "FLIP_Y") { 
        sendDMXPattern(i, flipX_active, flipY_active, fine_active); 
      }
      updateLEDs(i + 1, fine_active, flipX_active, flipY_active); // LED-Status aktualisieren
    }
   
   // Button release 
    else if (buttonState[i] == HIGH && lastButtonState[i] == LOW) {
      if (BUTTON_NAMES[i] != "FINE" && BUTTON_NAMES[i] != "FLIP_X" && BUTTON_NAMES[i] != "FLIP_Y") { 
        resetDMX();
      } 
    }
    lastButtonState[i] = buttonState[i];
  }


  delay(50);
}

// --- Funktionsdefinitionen ---

void resetDMX() {
  for (int i = 1; i <= 512; i++) {
    DMXSerial.write(i, 0);
  }
}

// Funktion zum Senden spezifischer DMX-Parameter basierend auf der gedrückten Taste
void sendDMXPattern(int buttonIndex, boolean flip_x, boolean flip_y, boolean fine) {

  switch (buttonIndex) {
    case 0: // 1_FINE - Wird in loop() behandelt
      break;
    case 1: // 2_FLIP_X - Wird in loop() behandelt
      break;
    case 2: //3_FLIP_Y - Wird in loop() behandelt
      break;
    case 3: // 4_ UP
      
      DMXSerial.write(3, flipY_active ? 0 : 255); // chan 3 0..127 tilt up, 128..255 tilt down
      DMXSerial.write(6, fine_active ? lospeed : hispeed); 
  
      break;
    case 4: // 5_DOWN
      DMXSerial.write(3, flipY_active ? 255 : 0); // chan 3 0..127 tilt up, 128..255 tilt down
      DMXSerial.write(6, fine_active ? lospeed : hispeed); 
      break;
    case 5: // 6_left
      DMXSerial.write(1, flipX_active ? 0:255);  // chan 1 pan left(0..127), pan right(128..255)
      DMXSerial.write(5, fine_active ? lospeed : hispeed); //
      break;
    case 6: // 7_right
      DMXSerial.write(1, flipX_active ? 255:0  ); // chan 1 pan left(0..127), pan right(128..255)
      DMXSerial.write(5, fine_active ? lospeed : hispeed); //
  
      break;
    case 7: // 8_focus -
      DMXSerial.write(501, 128); // Kanal 501 Fokus -128 
     
      break;
    case 8: // 9_focus +
      DMXSerial.write(501, 255); // Kanal 501 Fokus -128 
      
    default:
      break;
  }

}

// Funktion zum Aktualisieren der LEDs
void updateLEDs(int buttonNumber, boolean fine_active, boolean flipX_active, boolean flipY_active) {
  if (buttonNumber == 1) { 
    digitalWrite(LED_1_PIN, fine_active ? HIGH : LOW);
  } else if (buttonNumber == 2) { 
    digitalWrite(LED_2_PIN, flipX_active ? HIGH : LOW);
  } else if (buttonNumber == 3) { 
    digitalWrite(LED_3_PIN,flipY_active ? HIGH : LOW); 
  }
}