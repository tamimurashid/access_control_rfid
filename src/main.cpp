#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

// Define pins for RFID
#define RST_PIN D1  // Reset pin
#define SS_PIN D2   // Slave select (SDA) pin
#define Green_led D0 // GPIO s3
#define Red_led D3
#define Card_led 10
#define Wait_led 9
#define alarm D4

// WiFi credentials
const char* ssid = "Reindeer";        // Replace with your WiFi SSID
const char* password = "200120022003"; // Replace with your WiFi password
const char* serverUrl = "http://192.168.10.103:8888/Iot_web_project/Access_control/Api/";
 // Replace with your server URL

// Create an instance of the MFRC522 class
MFRC522 rfid(SS_PIN, RST_PIN);
WiFiClient wifiClient;  // Create a WiFiClient instance

class Alert {
  public:
    void green_led(int Delay1, int Delay2) {
      digitalWrite(Green_led, HIGH);
      delay(Delay1);
      digitalWrite(Green_led, LOW);
      delay(Delay2);
    }

    void red_led(int Delay1, int Delay2) {
      digitalWrite(Red_led, HIGH);
      delay(Delay1);
      digitalWrite(Red_led, LOW);
      delay(Delay2);
    }
};
Alert alert;
void data_handler(String cardID) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Prepare the JSON payload
    String payload = "{\"cardID\": \"" + cardID + "\"}";

    // Begin the HTTP POST request
    http.begin(wifiClient, serverUrl);  // Use the updated API
    http.addHeader("Content-Type", "application/json");

    // Send the POST request and capture the response
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response: " + response);
      
      //parse the json 
      StaticJsonDocument<200> doc;

      DeserializationError error = deserializeJson(doc, response);

      if(!error){
        String code = doc["code"].as<String>();
        if(code == "001"){
          alert.green_led(1000, 1000);
        }
      }




    } else {
      Serial.println("Error in HTTP request: " + String(httpResponseCode));
    }

    // End the HTTP request
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}

String rfid_reader() {
  // Check for new RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return ""; // Return an empty string if no card is detected
  }

  // Construct the UID as a string
  String cardUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      cardUID += "0"; // Add leading zero if necessary
    }
    cardUID += String(rfid.uid.uidByte[i], HEX);
  }

  // Halt the RFID card to stop further communication
  rfid.PICC_HaltA();

  return cardUID;
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

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void loop() {
  String cardID = rfid_reader();
  if (cardID != "") {
    Serial.print("Card UID: ");
    Serial.println(cardID);

    // Handle the card data
    data_handler(cardID);

  }
}
