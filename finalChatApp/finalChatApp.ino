#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <RH_NRF905.h>
#include <SPIFFS.h>

// Wi-Fi credentials for the Access Point (AP) network
const char *ssid = "ESP_MODULE2";      // Wi-Fi network name for ESP32
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
String content = "<html lang=\"en\">\n"
"    <head>\n"
"        <meta charset=\"UTF-8\">\n"
"        <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
"        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"        <title>Science Fair</title>\n"
"        <style>\n"
"            body {\n"
"                font-family: Arial, sans-serif;\n"
"                margin: 20px;\n"
"            }\n"
"\n"
"            #chat-box {\n"
"                border: 1px solid #ccc;\n"
"                padding: 10px;\n"
"                height: 300px;\n"
"                overflow-y: auto;\n"
"                margin-bottom: 10px;\n"
"                display: flex;\n"
"                flex-direction: column;\n"
"                gap: 10px;\n"
"            }\n"
"\n"
"            .message {\n"
"                max-width: 70%;\n"
"                padding: 10px;\n"
"                border-radius: 15px;\n"
"                display: inline-block;\n"
"                word-wrap: break-word;\n"
"            }\n"
"\n"
"            .sent {\n"
"                background-color: #0084ff;\n"
"                color: #fff;\n"
"                align-self: flex-end;\n"
"            }\n"
"\n"
"            .received {\n"
"                background-color: #f1f0f0;\n"
"                color: #000;\n"
"                align-self: flex-start;\n"
"            }\n"
"\n"
"            #message-input {\n"
"                width: 80%;\n"
"                padding: 10px;\n"
"                margin-right: 10px;\n"
"            }\n"
"\n"
"            #send-btn {\n"
"                padding: 10px;\n"
"            }\n"
"        </style>\n"
"    </head>\n"
"    <body>\n"
"        <h1>Science Fair Peer-to-Peer Communication</h1>\n"
"        <h2>Chat Interface</h2>\n"
"        <div id=\"chat-box\"></div>\n"
"        <input type=\"text\" id=\"message-input\" placeholder=\"Type your message\">\n"
"        <button id=\"send-btn\">Send</button>\n"
"        <script>\n"
"            const chatBox = document.getElementById('chat-box');\n"
"            const messageInput = document.getElementById('message-input');\n"
"            const sendBtn = document.getElementById('send-btn');\n"
"            let lastMessages = [];\n"
"\n"
"            async function fetchMessages() {\n"
"                try {\n"
"                    const response = await fetch('/receive');\n"
"                    const messages = await response.json();\n"
"\n"
"                    // Only update the chat box if new messages are received\n"
"                    if (JSON.stringify(messages) !== JSON.stringify(lastMessages)) {\n"
"                        lastMessages = messages;\n"
"                        chatBox.innerHTML = messages.map(msg =>\n"
"                            `<div class=\"message ${msg.sent ? 'sent' : 'received'}\">${msg.text}</div>`\n"
"                        ).join('');\n"
"                        chatBox.scrollTop = chatBox.scrollHeight;\n"
"                    }\n"
"                } catch (error) {\n"
"                    console.error('Error fetching messages:', error);\n"
"                }\n"
"            }\n"
"\n"
"            async function sendMessage() {\n"
"                const message = messageInput.value.trim();\n"
"                if (!message) return;\n"
"\n"
"                try {\n"
"                    await fetch('/send', {\n"
"                        method: 'POST',\n"
"                        headers: {\n"
"                            'Content-Type': 'application/json',\n"
"                        },\n"
"                        body: JSON.stringify({ text: message, sent: true }),\n"
"                    });\n"
"                    messageInput.value = '';\n"
"                    fetchMessages();\n"
"                } catch (error) {\n"
"                    console.error('Error sending message:', error);\n"
"                }\n"
"            }\n"
"\n"
"            sendBtn.addEventListener('click', sendMessage);\n"
"            messageInput.addEventListener('keypress', (e) => {\n"
"                if (e.key === 'Enter') sendMessage();\n"
"            });\n"
"\n"
"            setInterval(fetchMessages, 1000);\n"
"\n"
"            fetchMessages();\n"
"        </script>\n"
"    </body>\n"
"</html>\n";

// Set up HTTP server
void setup() {
  // Begin local flash storage on esp32
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

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
      lastReceivedMessage = receivedMessage;
    }
  }
}