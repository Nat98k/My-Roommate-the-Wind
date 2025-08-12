// recieves data from esp_sensors
// sends data to mega
// recieves data from mega
// sends data to esp_lights
#include <esp_now.h>
#include <WiFi.h>

// Serial Communication with Arduino
#define RXp2 16
#define TXp2 17

// Define a data structure
typedef struct struct_message {
  float a;
  float b;
  float c;
} struct_message;

// Create a structured object to store incoming data
struct_message windData;

// Create a structured object to store outgoing data
struct_message lightData;

// Variables for test light data
float float_value1;
float float_value2;

// MAC Address of responder - edit as required (light)
//uint8_t broadcastAddress[] = { 0xFC, 0xB4, 0x67, 0xF6, 0x23, 0xA8 };  // light mockup ONLY FOR DEMODAY
//uint8_t broadcastAddressA[] = {0xE4, 0x65, 0xB8, 0xE7, 0x8D, 0xAC};  // kitchen light A
uint8_t broadcastAddress[] = { 0xEA, 0x65, 0xB8, 0xE7, 0x8D, 0xAC };   // living room light B
uint8_t broadcastAddressC[] = { 0xE8, 0x6B, 0xEA, 0xCF, 0xE1, 0xBC };   // small working room light C

// Callback function executed when data is received from 1
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&windData, incomingData, sizeof(windData));
  Serial.println("Data received from sensors: ");
  Serial.print("a: ");
  Serial.println(windData.a);
  Serial.print("b: ");
  Serial.println(windData.b);
}

// Peer info to 3
esp_now_peer_info_t peerInfo;

// Callback function called when data is sent to lights
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println("Data sent to Lights: ");
  Serial.print("a: ");
  Serial.println(lightData.a);
  Serial.print("b: ");
  Serial.println(lightData.b);
  Serial.print("c: ");
  Serial.println(lightData.c);
}

void setup() {
  Serial.begin(19200);                          // Setup serial monitor
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);  // Serial communication with Arduino

  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);

  // Register the send callback to lights
  esp_now_register_send_cb(OnDataSent);

  // Register peer to 3
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Register peer to 3 broadcastAddressC
  memcpy(peerInfo.peer_addr, broadcastAddressC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer to 3
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Add peer to 3 broadcastAddressC
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer C");
    return;
  }
}

void loop() {
  // MESSAGES TO ARDUINO
  float data1 = windData.a;
  float data2 = windData.b;
  //String DataToMega = "0.34;0.3";
  String DataToMega = String(data1) + ";" + String(data2);
  //Serial.println(DataToMega);
  Serial2.print(DataToMega);

  // MESSAGES FROM ARDUINO
  // Serial.println("Arduino told me: ");
  if (Serial2.available()) {
    messageRecieve();
  }

  delay(2000);

  // Send message via ESP-NOW to the lights
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&lightData, sizeof(lightData));
  //esp_err_t resultB = esp_now_send(broadcastAddressB, (uint8_t *)&lightData, sizeof(lightData));
  esp_err_t resultC = esp_now_send(broadcastAddressC, (uint8_t *)&lightData, sizeof(lightData));

  if (result == ESP_OK) { // &&resultC
    //Serial.println("Sending confirmed");
  } else {
    Serial.println("Sending error");
  }

  // // Send message via ESP-NOW to broadcastAddress1
  // result = esp_now_send(broadcastAddress1, (uint8_t *)&lightData, sizeof(lightData));
  // if (result == ESP_OK) {
  //   Serial.println("Sending to broadcastAddress1 confirmed");
  // } else {
  //   Serial.println("Sending to broadcastAddress1 error");
  // }
}
void messageRecieve() { // from mega
  String incomingMessage = Serial2.readString(); 
  Serial.println("recieved from mega: " + incomingMessage);

  int separator1Index = incomingMessage.indexOf(';');
  int separator2Index = incomingMessage.indexOf(';', separator1Index + 1);

  String data1 = incomingMessage.substring(0, separator1Index);
  String data2 = incomingMessage.substring(separator1Index + 1, separator2Index);
  String data3 = incomingMessage.substring(separator2Index + 1);

  lightData.a = data1.toFloat();
  lightData.b = data2.toFloat();
  lightData.c = data3.toFloat();
  Serial.println("translates to: a=" + String(lightData.a) + " b=" + String(lightData.b) + " c=" + String(lightData.c));
}
