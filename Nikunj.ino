#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi Credentials
const char* ssid = "Aldxhc";            // 🔹 Replace with your WiFi name
const char* password = "1234567890";    // 🔹 Replace with your WiFi password
const char* server_url = "http://192.168.42.176:8000/chat";  // 🔹 Replace with your FastAPI server IP

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LED Pin
#define LED_PIN 2  

void setup() {
    Serial.begin(115200);
    delay(1000);  // Ensure Serial Monitor is ready

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Ensure LED is off at start

    // Initialize OLED Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        for (;;);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("Connecting WiFi...");
    display.display();

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");

    display.clearDisplay();
    display.setCursor(0, 10);
    display.print("Connected!");
    display.display();
    delay(1000);
}

String getAIResponse(String userInput) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("http://192.168.42.176:8000/chat");  // Use your actual FastAPI IP
        http.addHeader("Content-Type", "application/json");

        // Prepare JSON Payload with the user's question
        String jsonPayload = "{\"text\":\"" + userInput + "\"}";

        int httpResponseCode = http.POST(jsonPayload);

        String response = "";
        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("Response: " + payload);

            // Parse JSON response
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                Serial.println("JSON Parsing Error!");
                response = "Error!";
            } else {
                response = doc["response"].as<String>();
            }

            // 🔹 Blink LED only when response is received
            digitalWrite(LED_PIN, HIGH);
            delay(300);  // LED ON for 300ms
            digitalWrite(LED_PIN, LOW);
        } else {
            Serial.println("Error in HTTP request");
            Serial.println(httpResponseCode);
            response = "Error!";
        }
        http.end();  // 🔹 Ensure HTTP connection is properly closed
        delay(500);  // 🔹 Short delay to allow next request
        return response;
    }
    return "No WiFi!";
}



void scrollText(String text) {
    int textWidth = text.length() * 6;  // Approximate width of text in pixels
    int xStart = SCREEN_WIDTH;  // Start text outside the right edge

    for (int x = xStart; x > -textWidth; x--) {
        display.clearDisplay();
        display.setCursor(x, 10);
        display.print(text);
        display.display();
        delay(10);  // Adjust speed of scrolling
    }
}


void loop() {
    Serial.println("Waiting for user input...");
    
    // Wait for user input via Serial Monitor
    while (Serial.available() == 0) {}

    // Read user input
    String userInput = Serial.readStringUntil('\n');  
    userInput.trim();  // Remove extra spaces or newlines

    if (userInput.length() > 0) {
        Serial.print("You typed: ");
        Serial.println(userInput);

        // Send input to FastAPI server
        String aiResponse = getAIResponse(userInput);  // ✅ FIXED - Pass userInput as an argument

        // Display response on OLED
        scrollText(aiResponse);
    }
}
