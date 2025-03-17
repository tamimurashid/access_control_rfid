/*-----------------------------------------------------------------------------------------------
//             -----
//           /    
//          |
//          |
             \
//        
//
//
-------------------------------------------------------------------------------------------------*/
#include <SPI.h>
#include <Servo.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

// Define pins for RFID and LEDs
#define RST_PIN D1  // Reset pin
#define SS_PIN D2   // Slave select (SDA) pin
#define Green_led D0 // GPIO s3
#define Red_led 10
#define Card_led D3
#define Wait_led 9
#define alarm D4
#define servo_motor D8

// WiFi credentials
// const char* ssid = "Reindeer";        
// const char* password = "200120022003";  
const char* serverUrl = "http://192.168.10.103:8888/Access_control/Api/"; //local test
// const char* serverUrl = "http://13.60.74.47/Acces_control_web/Api/";// ec2 instance 

// Create an instance of the WiFiClient
// WiFiClient wifiClient;
WiFiClient wifiClient;

Servo myServo;

// Alert Class: Handles LED blinking
class Alert {
public:
    void green_led(int Delay1, int Delay2, int times) {
        blink_led(Green_led, Delay1, Delay2, times);
    }

    void red_led(int Delay1, int Delay2, int times) {
        blink_led(Red_led, Delay1, Delay2, times);
    }

    void alarm_alert(int Delay1, int Delay2, int times){
         alarms(alarm, Delay1, Delay2, times);
    }

    void warningSound(int times) {
        for(int i = 0; i < times; i++){
            tone(alarm, 500, 500); // Play a 500Hz tone for 500ms
            delay(1000);
            noTone(alarm);
        }
    }

    // Function to play a success sound
    void successSound() {
    tone(alarm, 1000, 300); // Play a 1.5kHz tone for 300ms
    delay(300);
    noTone(alarm);
    tone(alarm, 2000, 300); // Play a 2kHz tone for 300ms
    delay(300);
    noTone(alarm);
    }

private:
    void alarms(int pin, int Delay1, int Delay2, int times){
      for(int i = 0; i < times; i++){
        digitalWrite(pin, HIGH);
        delay(Delay1);
        digitalWrite(pin, LOW);
        delay(Delay2);
      }

    }
    void blink_led(int pin, int Delay1, int Delay2, int times) {
        for (int i = 0; i < times; i++) {
            digitalWrite(pin, HIGH);
            delay(Delay1);
            digitalWrite(pin, LOW);
            delay(Delay2);
        }
    }
};

// RFID Reader Class
class RFIDReader {
private:
    MFRC522 rfid;
    
public:
    RFIDReader(int ssPin, int rstPin) : rfid(ssPin, rstPin) {}

    void init() {
        SPI.begin();  // Start SPI communication
        rfid.PCD_Init();
        Serial.println("RFID Reader initialized.");
    }

    String readCard() {
        if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
            return ""; // No card detected
        }

        String cardUID = "";
        for (byte i = 0; i < rfid.uid.size; i++) {
            if (rfid.uid.uidByte[i] < 0x10) {
                cardUID += "0"; // Add leading zero if necessary
            }
            cardUID += String(rfid.uid.uidByte[i], HEX);
        }

        rfid.PICC_HaltA(); // Stop communication with card
        return cardUID;
    }
};


// Access Control Class: Handles API communication
class AccessControl {
private:
    WiFiClient& client;
    Alert& alert;

public:
    AccessControl(WiFiClient& client, Alert& alertObj) : client(client), alert(alertObj) {}
    String mode = "auth_mod"; // Default mode is Authentication Mode

    void rotateServo() {
    myServo.write(45);  // Move to 45 degrees clockwise
    delay(5000);        // Hold position for 5 seconds
    myServo.write(0);   // Return to the initial position (0 degrees)
    } 
    void connect() {
         Serial.print("Connecting to WiFi");
    unsigned long startTime = millis();  // Track time

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        digitalWrite(Red_led, HIGH);
        delay(500);
        digitalWrite(Red_led, LOW);
        delay(500);

        // If WiFi doesn't connect within 15 seconds, restart ESP
        if (millis() - startTime > 15000) {  
            Serial.println("\nWiFi connection failed! Restarting...");
            ESP.restart(); // Restart the ESP8266
        }
    }

    Serial.println("\nWiFi connected.");
    digitalWrite(Card_led, HIGH);
    }

    void fetchMode() {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected.");
            return;
        }

        HTTPClient http;
        http.begin(client, serverUrl);
        http.addHeader("Content-Type", "application/json");

        String payload = "{\"code\": \"check_mode\"}";
        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Mode Response: " + response);

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, response);

            if (!error) {
                String newMode = doc["status"].as<String>();
                if (newMode == "reg_mod" || newMode == "auth_mod") {
                    mode = newMode;
                    Serial.println("Mode set to: " + mode);
                }
            } else {
                Serial.println("JSON Parsing Error: " + String(error.f_str()));
            }
        } else {
            Serial.println("Error in HTTP request: " + String(httpResponseCode));
        }

        http.end();
     }

    void processCard(String cardID) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected.");
            return;
        }
        HTTPClient http;
        http.begin(client, serverUrl);
        http.addHeader("Content-Type", "application/json");

        // Create JSON payload
        String payload = "{\"cardID\": \"" + cardID + "\", \"mode\": \"" + mode + "\"}";
        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Server Response: " + response);
            // parse json 
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, response);

            if (!error) {
                String code = doc["code"].as<String>();
                if (code == "001") {
                    alert.green_led(100, 100, 2);
                    alert.successSound(); 
                    rotateServo();
                    // alert.alarm_alert(100, 100, 3);
                } else if (code == "000") {
                    alert.red_led(100, 100, 3);
                    //alert.alarm_alert(100, 100, 5);
                    alert.warningSound(2);
                }
            } else {
                Serial.println("JSON Parsing Error: " + String(error.f_str()));
            }
        } else {
            Serial.println("Error in HTTP request: " + String(httpResponseCode));
            alert.red_led(100, 100, 10);
        }

        http.end();
    }
};

// Global Instances
Alert alert;
RFIDReader rfidReader(SS_PIN, RST_PIN);
WiFiManager wifiManager;
AccessControl accessControl(wifiClient, alert);


void setup() {
    // Initialize Serial Monitor
    Serial.begin(9600);
    while (!Serial);

    myServo.attach(servo_motor); // Attach servo to the defined pin
    myServo.write(0);  // Set the initial position to 0 degrees
    delay(1000);

    // Set pin modes
    pinMode(Green_led, OUTPUT);
    pinMode(Red_led, OUTPUT);
    pinMode(Card_led, OUTPUT);
    pinMode(Wait_led, OUTPUT);
    pinMode(alarm, OUTPUT);
    pinMode(servo_motor, OUTPUT);

    // Initialize Components
    rfidReader.init();
   
    wifiManager.autoConnect("AccessControlAP");
}

void loop() {
     if (WiFi.status() != WL_CONNECTED) { 
        accessControl.connect(); // Only connect if not already connected
    } else {
        digitalWrite(Card_led, HIGH); // Keep LED on when connected
    }

    accessControl.fetchMode();

    String cardID = rfidReader.readCard();
    if (cardID != "") {
        Serial.print("Card UID: ");
        Serial.println(cardID);
        accessControl.processCard(cardID);
    }
     delay(2000); // Prevents spamming the server
}

