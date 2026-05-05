#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String codeBuffer = "";

void setup() {
  Serial.begin(9600);
}

void loop() {
  char key = keypad.getKey();
  if (!key) return;

  if (key == '*') {
    Serial.println("CANCEL");
    codeBuffer = "";

  } else if (key == '#') {
    if (codeBuffer.length() > 0) {
      Serial.println("CODE:" + codeBuffer);
      codeBuffer = "";
    }

  } else if (key >= '0' && key <= '9') {
    codeBuffer += key;
    Serial.println("KEY:*");   // <-- envoie une étoile à Qt
  }
}