#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* STA_SSID = "your_ssid";
const char* STA_PASS = "your_pass";

const char* TARGET_DOMAIN = "pioneer.vtuner.com";
const IPAddress TARGET_IP(23, 238, 115, 210);

const IPAddress REAL_DNS(8, 8, 8, 8);

WiFiUDP udp;
const unsigned int DNS_PORT = 53;

void setup() {
  Serial.begin(115200);
  WiFi.begin(STA_SSID, STA_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
  udp.begin(DNS_PORT);
}

void loop() {
  handleDNS();
}

void handleDNS() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    uint8_t packetBuffer[512];
    udp.read(packetBuffer, packetSize);

    String domain = readDNSQuery(packetBuffer, packetSize);
    Serial.println("Query: " + domain);

    if (domain.equals(TARGET_DOMAIN)) {
      sendDNSResponse(packetBuffer, packetSize, TARGET_IP);
    } else {
      forwardDNSRequest(packetBuffer, packetSize);
    }
  }
}

String readDNSQuery(uint8_t *buffer, int size) {
  String domain = "";
  int pos = 12; // Skip DNS header
  
  while (buffer[pos] != 0) {
    int labelLen = buffer[pos++];
    for (int i = 0; i < labelLen; i++) {
      domain += (char)buffer[pos++];
    }
    domain += ".";
  }
  
  if (domain.length() > 0) {
    domain.remove(domain.length() - 1); // Remove last dot
  }
  
  return domain;
}

void sendDNSResponse(uint8_t *buffer, int size, IPAddress ip) {
  // Set response flags
  buffer[2] = 0x81; // Response + recursion desired
  buffer[3] = 0x80; // Recursion available
  
  // Set answer count to 1
  buffer[7] = 1;

  // Prepare answer section
  int answerPos = size;
  buffer[answerPos++] = 0xC0; // Pointer to domain name
  buffer[answerPos++] = 0x0C; // Offset to domain name
  
  // Type A (IPv4)
  buffer[answerPos++] = 0x00;
  buffer[answerPos++] = 0x01;
  
  // Class IN
  buffer[answerPos++] = 0x00;
  buffer[answerPos++] = 0x01;
  
  // TTL (60 seconds)
  buffer[answerPos++] = 0x00;
  buffer[answerPos++] = 0x00;
  buffer[answerPos++] = 0x00;
  buffer[answerPos++] = 0x3C;
  
  // Data length (IPv4 = 4)
  buffer[answerPos++] = 0x00;
  buffer[answerPos++] = 0x04;
  
  // IP address bytes
  buffer[answerPos++] = ip[0];
  buffer[answerPos++] = ip[1];
  buffer[answerPos++] = ip[2];
  buffer[answerPos++] = ip[3];

  // Send response
  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  udp.write(buffer, answerPos);
  udp.endPacket();
}

void forwardDNSRequest(uint8_t *buffer, int size) {
  WiFiUDP udpExt;
  udpExt.begin(0); // Random local port
  
  // Forward to real DNS server
  udpExt.beginPacket(REAL_DNS, DNS_PORT);
  udpExt.write(buffer, size);
  
  if (udpExt.endPacket()) {
    // Wait for response (max 1 second)
    unsigned long start = millis();
    while (millis() - start < 1000) {
      if (udpExt.parsePacket() > 0) {
        uint8_t response[512];
        int respSize = udpExt.read(response, 512);
        
        // Forward response to original client
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.write(response, respSize);
        udp.endPacket();
        break;
      }
      delay(10);
    }
  }
  
  udpExt.stop();
}