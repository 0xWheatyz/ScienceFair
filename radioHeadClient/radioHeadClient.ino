// nrf905_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_NRF905 class. RH_NRF905 class does not provide for addressing or
// reliability, so you should only use RH_NRF905 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example nrf905_server.
// Tested on Teensy3.1 with nRF905 module
// Tested on Arduino Due with nRF905 module (Caution: use the SPI headers for connecting)
 
#include <SPI.h>
#include <RH_NRF905.h>
 
// Singleton instance of the radio driver
RH_NRF905 nrf905(
  15, // CE
  4, // TXEN
  5 //CSN
);
 
void setup() 
{
  Serial.begin(115200);
  if (!nrf905.init())
    Serial.println("init failed");
  // Defaults after init are 433.2 MHz (channel 108), -10dBm
}
 
 
void loop()
{
  Serial.println("Sending to nrf905_server");
  // Send a message to nrf905_server
  uint8_t data[] = "Hello World!";
  nrf905.send(data, sizeof(data));
  
  nrf905.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_NRF905_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
 
  if (nrf905.waitAvailableTimeout(500))
  { 
    // Should be a reply message for us now   
    if (nrf905.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is nrf905_server running?");
  }
  delay(400);
}