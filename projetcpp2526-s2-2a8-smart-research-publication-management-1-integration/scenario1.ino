const int LED_TEST = 13;

void setup() {
  Serial.begin(9600);
  pinMode(LED_TEST, OUTPUT);
  digitalWrite(LED_TEST, LOW);   // Éteinte au démarrage
}

void loop() {
  Serial.println("RFID:01020304");  // println ajoute \r\n
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "AUTORISE") {
      digitalWrite(LED_TEST, HIGH);   // Allume la LED
      delay(600);
      digitalWrite(LED_TEST, LOW);
    } 
    else if (cmd == "REFUSE") {
      digitalWrite(LED_TEST, LOW);    // Éteint la LED
    }
  }
  delay(5000);
}
