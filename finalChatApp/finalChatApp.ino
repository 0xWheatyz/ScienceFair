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
    <title>ESP32 Chat Interface</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            text-align: center;
        }
        #chat-box {
            border: 1px solid #ccc;
            padding: 10px;
            height: 300px;
            overflow-y: auto;
            margin-bottom: 10px;
            background-color: #f9f9f9;
            border-radius: 5px;
        }
        #message-input {
            width: 80%;
            padding: 10px;
            margin-right: 10px;
            border: 1px solid #ccc;
            border-radius: 5px;
        }
        #send-btn {
            padding: 10px;
            border: none;
            background-color: #28a745;
            color: white;
            border-radius: 5px;
            cursor: pointer;
        }
        #send-btn:hover {
            background-color: #218838;
        }
    </style>
</head>
<body>
    <h1>ESP32 Chat Interface</h1>
    <div id="chat-box"></div>
    <input type="text" id="message-input" placeholder="Type your message...">
    <button id="send-btn">Send</button>

    <script>
        const chatBox = document.getElementById('chat-box');
        const messageInput = document.getElementById('message-input');
        const sendBtn = document.getElementById('send-btn');
        const ESP32_IP = "192.168.4.1"; // Replace with the actual ESP32 IP address

        // Function to add messages to the chat box
        function addMessageToChatBox(message) {
            chatBox.innerHTML += `<div>${message}</div>`;
            chatBox.scrollTop = chatBox.scrollHeight; // Auto-scroll to the bottom
        }

        // Function to send a message to the ESP32
        async function sendMessage() {
            const message = messageInput.value.trim();
            if (!message) return;

            // POST message to ESP32
            try {
                const response = await fetch('/send', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: 'message=' + encodeURIComponent(message) // Correctly format the body
                });

                if (response.ok) {
                    const data = await response.json();
                    addMessageToChatBox(`You: ${data.message}`);
                    messageInput.value = ''; // Clear input
                } else {
                    addMessageToChatBox("Error: Failed to send message");
                }
            } catch (error) {
                addMessageToChatBox("Error: Could not connect to ESP32");
            }
        }

        // Function to fetch the last received message
        async function fetchLastMessage() {
            try {
                const response = await fetch('/receive');
                if (response.ok) {
                    const data = await response.json();
                    if (data.message && data.message !== "No messages received yet.") {
                        addMessageToChatBox(`ESP32: ${data.message}`);
                    }
                }
            } catch (error) {
                console.error('Error fetching messages:', error);
            }
        }

        // Event listeners
        sendBtn.addEventListener('click', sendMessage);
        messageInput.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') sendMessage();
        });

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
        String lastReceivedMessage = "";
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
