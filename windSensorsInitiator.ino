//Iteration 2+ sending wind data to sci-esp
// Include Libraries
#include <esp_now.h>
#include <WiFi.h>

//sensor variables
int directionPin = 35;  // select the input pins for the sensors
int speedPin = 34;
float directionValue = 0;  // variable to store the value coming from the sensor
float speedValue = 0;

// MAC Address of receiver 
//uint8_t broadcastAddress[] = { 0xFC, 0xB4, 0x67, 0xF6, 0x23, 0xA8 }; //light mockup
uint8_t broadcastAddress[] = { 0xE8, 0x6B, 0xEA, 0xCF, 0x29, 0x30 }; //display


// Define a data structure
typedef struct struct_message {
  float a;
  float b;
  float c;
} struct_message;

// Create a structured object
struct_message windData;

// Peer info
esp_now_peer_info_t peerInfo;

// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {

  // Set up Serial Monitor to check what is going on
  Serial.begin(19200);

  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the send callback
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  //read the sensor data
  float convert = 0.00122070312;  //to convert to volt in float
  directionValue = convert * analogRead(directionPin);
  speedValue = convert * analogRead(speedPin);
  Serial.println(speedValue);
  Serial.println(directionValue);

  windData.a = directionValue;
  windData.b = speedValue;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&windData, sizeof(windData));

  if (result == ESP_OK) {
    Serial.println("Sending confirmed from sensor to lightmockup and display");
    Serial.println(windData.a);
    Serial.println(windData.b);
  } else {
    Serial.println("Sending error");
  }
  delay(2000);
}