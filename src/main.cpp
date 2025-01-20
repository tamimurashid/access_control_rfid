#include <SPI.h>
#include <MFRC522.h>

// Define pins for RFID
#define RST_PIN D1  // Reset pin
#define SS_PIN D2   // Slave select (SDA) pin
#define Green_led D0
#define Red_led D3

// Create an instance of the MFRC522 class
MFRC522 rfid(SS_PIN, RST_PIN);
class Alert{
  public: 
  void green_led(int Delay1, int Delay2){
   digitalWrite(Green_led, HIGH);
   delay(Delay1);
   digitalWrite(Green_led, LOW);
   delay(Delay2);
  }
};

void rfid_reader(){
    // Check for new RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Print the UID of the RFID card
  Serial.print("Card UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print("");
  }
  Serial.println();

  // Halt the RFID card to stop further communication
  rfid.PICC_HaltA();
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial);

  // Initialize SPI bus and RFID reader
  SPI.begin();  // SCK, MISO, MOSI, and SS (SDA) pins
  rfid.PCD_Init();
  Serial.println("RFID Reader initialized.");
}

void loop() {

}
