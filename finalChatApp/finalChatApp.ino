#include <WiFi.h>
#include <WebServer.h>
#include <RH_NRF905.h>
#include <SPIFFS.h>

// Wi-Fi credentials for the Access Point (AP) network
const char *ssid = "ESP_MODULE1";      // Wi-Fi network name for ESP32
const char *password = "123456789"; // Wi-Fi network password for ESP32

// NRF905 setup
RH_NRF905 nrf905(15, 4, 5); // CE, TXEN, CSN

// HTTP server on this ESP32
WebServer server(80);

// Message buffer
String lastReceivedMessage = "";

// HTML content for the web page
const char* content = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Message Sender</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; }
        input[type="text"] { width: 300px; }
        button { margin-top: 10px; }
        #receivedMessage { margin-top: 20px; font-weight: bold; }
    </style>
</head>
<body>
    <h1>ESP32 Message Sender</h1>
    <input type="text" id="messageInput" placeholder="Enter your message here">
    <button onclick="sendMessage()">Send Message</button>
    <h2>Last Received Message:</h2>
    <p id="receivedMessage"></p>

    <script>
        function sendMessage() {
            const message = document.getElementById('messageInput').value;
            fetch('/send', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: 'message=' + encodeURIComponent(message) // Correctly format the body
            })
            .then(response => {
                if (!response.ok) {
                    throw new Error('Network response was not ok ' + response.statusText);
                }
                return response.json();
            })
            .then(data => {
                document.getElementById('receivedMessage').innerText = data.message;
            })
            .catch(error => console.error('Error:', error));
        }

        // Function to fetch the last received message
        function fetchLastMessage() {
            fetch('/receive')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('receivedMessage').innerText = data.message;
                })
                .catch(error => console.error('Error:', error));
        }

        // Automatically fetch the last message every 5 seconds
        setInterval(fetchLastMessage, 5000);
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

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
        if (server.hasArg("message")) { // Check for the correct argument
            String message = server.arg("message");
            uint8_t data[RH_NRF905_MAX_MESSAGE_LEN];
            message.getBytes(data, sizeof(data));
            nrf905.send(data, message.length());
            nrf905.waitPacketSent();
            Serial.println("Sent via NRF905: " + message);
            server.send(200, "application/json", "{\"message\":\"" + message + "\"}");
        } else {
            server.send(400, "application/json", "{\"message\":\"Message parameter is missing\"}");
        }
    });

    server.on("/receive", HTTP_GET, []() {
        String jsonResponse = "{\"message\": \"" + lastReceivedMessage + "\"}";
        server.send(200, "application/json", jsonResponse);
    });

    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", content);
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
            lastReceivedMessage = receivedMessage; // Store the last received message
        }
    }
}
