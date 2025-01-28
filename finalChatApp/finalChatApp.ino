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
String content = "<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"  <meta charset=\"UTF-8\">"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"  <title>Chat Box</title>"
"  <style>"
"    body {"
"      font-family: Arial, sans-serif;"
"      margin: 20px;"
"    }"
"    #chat-box {"
"      border: 1px solid #ccc;"
"      padding: 10px;"
"      height: 300px;"
"      overflow-y: auto;"
"      margin-bottom: 10px;"
"    }"
"    #message-input {"
"      width: 80%;"
"      padding: 10px;"
"      margin-right: 10px;"
"    }"
"    #send-btn {"
"      padding: 10px;"
"    }"
"  </style>"
"</head>"
"<body>"
"  <h1>Chat Interface</h1>"
"  <div id=\"chat-box\"></div>"
"  <input type=\"text\" id=\"message-input\" placeholder=\"Type your message...\">"
"  <button id=\"send-btn\">Send</button>"
""
"  <script>"
"    const chatBox = document.getElementById('chat-box');"
"    const messageInput = document.getElementById('message-input');"
"    const sendBtn = document.getElementById('send-btn');"
""
"    // Mock messages array to simulate chat history"
"    let mockMessages = [];"
"    // Assume \"messages\" array already exists"
"    let messages = [];"
""
"    // Function to fetch messages from the server"
"    async function fetchMessages() {"
"    try {"
"      const response = await fetch('/receive');"
"      if (!response.ok) {"
"        throw new Error('Failed to fetch messages');"
"      }"
""
"        const newMessages = await response.json(); // Assuming the response is JSON"
"        newMessages.forEach(msg => {"
"        // Add only unique messages to the array"
"        if (!messages.some(existingMsg => existingMsg.id === msg.id)) {"
"          messages.push(msg);"
"        }"
"      });"
""
"      console.log('Messages updated:', messages);"
"    } catch (error) {"
"      console.error('Error fetching messages:', error);"
"    }"
"  }"
""
"  // Function to send a message to the server"
"  async function sendMessage(message) {"
"    try {"
"      const response = await fetch('/send', {"
"        method: 'POST',"
"        headers: { 'Content-Type': 'application/json' },"
"        body: JSON.stringify({ message }),"
"      });"
""
"      if (!response.ok) {"
"        throw new Error('Failed to send message');"
"      }"
""
"      const result = await response.json(); // Assuming the server responds with a status"
"      console.log('Message sent successfully:', result);"
"    } catch (error) {"
"      console.error('Error sending message:', error);"
"    }"
"  }"
""
"  // Automatically check for new messages every 5 seconds"
"  function pollMessages(interval = 5000) {"
"    setInterval(fetchMessages, interval);"
"  }"
""
"  // Start polling for messages"
"  pollMessages();"
""
""
"    // Event listeners"
"    sendBtn.addEventListener('click', sendMessage);"
"    messageInput.addEventListener('keypress', (e) => {"
"      if (e.key === 'Enter') sendMessage();"
"    });"
"  </script>"
"</body>"
"</html>";

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
