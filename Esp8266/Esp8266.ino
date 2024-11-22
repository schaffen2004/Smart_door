#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

FirebaseData firebase;
String a;

void ketnoiwifi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin("Oppo", "12345678");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}

void setup() {
    Serial.begin(9600);
    ketnoiwifi();

    // Initialize Firebase with URL and authentication key (leave auth key empty if not required)
    Firebase.begin("https://smart-door-99a3b-default-rtdb.asia-southeast1.firebasedatabase.app", "325haUmqxWM70ht5lJRlvePD8t1T2q042o96U81u");
}

void loop() {
    // Ensure WiFi connection before attempting to read from Firebase
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected. Reconnecting...");
        ketnoiwifi();
    }

    // Try to get the value from Firebase
    if (Firebase.getString(firebase, "/status/door")) {
        // Check if the data retrieval was successful
        a = firebase.stringData();
        Serial.println("Door Status: " + a);
    } else {
        // Print error message if data retrieval failed
        Serial.print("Error: ");
        Serial.println(firebase.errorReason());
    }

    delay(2000); // Wait a bit before the next read to avoid flooding the network
}
