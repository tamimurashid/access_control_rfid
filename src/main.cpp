#include <SPI.h>
#include <MFRC522.h>

// Define pins for RFID
#define RST_PIN D1  // Reset pin
#define SS_PIN D2   // Slave select (SDA) pin
#define Green_led D0 // gpio  s3
#define Red_led D3
#define Card_led 10
#define Wait_led 9
#define alarm D4



// Create an instance of the MFRC522 class
MFRC522 rfid(SS_PIN, RST_PIN);

  void green_led(int Delay1, int Delay2){
   digitalWrite(Green_led, HIGH);
   delay(Delay1);
   digitalWrite(Green_led, LOW);
   delay(Delay2);
  }

  void red_led(int Delay1, int Delay2){
   digitalWrite(Red_led, HIGH);
   delay(Delay1);
   digitalWrite(Red_led, LOW);
   delay(Delay2);
  }



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
  pinMode(Green_led, OUTPUT);
  pinMode(Red_led, OUTPUT);
  pinMode(Card_led, OUTPUT);
  pinMode(Wait_led, OUTPUT);
  pinMode(alarm, OUTPUT);

  Serial.begin(9600);
  while (!Serial);
  SPI.begin();  // SCK, MISO, MOSI, and SS (SDA) pins
  rfid.PCD_Init();
  Serial.println("RFID Reader initialized.");
}

void loop() {

}
