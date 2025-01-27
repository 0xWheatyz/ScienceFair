#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <RH_NRF905.h>

// Wi-Fi credentials for the Access Point (AP) network
const char *ssid = "ESP_MODULE1";      // Wi-Fi network name for ESP32
const char *password = "123456789"; // Wi-Fi network password for ESP32

// NRF905 setup
RH_NRF905 nrf905(
  15, // CE
  4, // TXEN
  5 //CSN
);
// HTTP server on this ESP32
WebServer server(80);

// Message buffer
String lastReceivedMessage = "";

// Set up HTTP server
void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize NRF905
  if (!nrf905.init()) {
    Serial.println("nRF905 init failed");
    while (1);
  }
  Serial.println("nRF905 initialized");

  // Set ESP32 as an Access Point (AP)
  WiFi.softAP(ssid, password);
  Serial.println("ESP32 is now an Access Point.");
  Serial.println("IP Address: " + WiFi.softAPIP().toString());

  // Set up HTTP server endpoints
  server.on("/send", HTTP_POST, []() {
    if (server.hasArg("message")) {
      String message = server.arg("message");
      uint8_t data[RH_NRF905_MAX_MESSAGE_LEN];
      message.getBytes(data, sizeof(data));
      if (nrf905.send(data, message.length())) {
        nrf905.waitPacketSent();
        Serial.println("Sent via NRF905: " + message);
        server.send(200, "text/plain", "Message sent via NRF905: " + message);
      } else {
        server.send(500, "text/plain", "Failed to send message via NRF905");
      }
    } else {
      server.send(400, "text/plain", "Message parameter is missing");
    }
  });

  server.on("/receive", HTTP_GET, []() {
    if (lastReceivedMessage.length() > 0) {
      server.send(200, "text/plain", "Last received message: " + lastReceivedMessage);
    } else {
      server.send(200, "text/plain", "No messages received yet.");
    }
  });

  server.begin();
}

void loop() {
  server.handleClient();

  // Check for incoming messages via NRF905
  uint8_t buf[RH_NRF905_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (nrf905.waitAvailableTimeout(500)) {
    if (nrf905.recv(buf, &len)) {
      String receivedMessage = String((char *)buf);
      Serial.println("Received via NRF905: " + receivedMessage);
      lastReceivedMessage = receivedMessage;
    }
  }
}

// Function to send an HTTP POST request to the peer
/*
void sendMessageToPeer(const String &message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://" + String(peerIP) + ":" + String(peerPort) + "/send";
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "message=" + message;
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      Serial.println("Message forwarded to peer: " + message);
      Serial.println("Response: " + http.getString());
    } else {
      Serial.println("Failed to send message to peer. HTTP error: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("Wi-Fi not connected. Cannot send message to peer.");
  }
}*/
