// esp32 connected to led ring 
// receiving data from esp_display
// Include Libraries
#include <esp_now.h>
#include <WiFi.h>
#include <FastLED.h>

// Define the number of LEDs
#define NUM_LEDS 12 // 24 living and kitchen, 12 office
#define DATA_PIN 5

// Create a structured object for received info
typedef struct struct_message {
  float a;
  float b;
  float c;
} struct_message;

// Create a structured object
struct_message lightData;
struct_message myData;

// LED stuff
CRGB leds[NUM_LEDS];
int brightness_highbase = 125;  // can be value between 0 and 255
int brightness_middlebase = 75;
int brightness_lowbase = -55;
uint16_t x = 0;      // Starting point for the noise function
uint16_t y = 0;      // Constant value to simulate 1D noise


int myPosition = 4;  // N(kitchen)=1 E=2 W(living)=3 S(office)=4
//noise modifiers
int flickerSpeed = 300; //normal = 300
float flickerStrength = 0.25; //normal = 0.25


// Callback function executed when data is received from 2
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&lightData, incomingData, sizeof(lightData));
  Serial.println("Light data received from display: ");
  Serial.println("a: ");
  Serial.println(lightData.a); //  speed
  Serial.println("b: ");
  Serial.println(lightData.b); // direction
  Serial.println("c: ");
  Serial.println(lightData.c); // buttonmod
}

void setup() {
  // Set up Serial Monitor
  Serial.begin(9600);

  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);

  // LED stuff
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(50);

  // Initial color setup (a default color)
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255, 224, 55);  // warm color
  }
  FastLED.show();
}

void loop() {
  // For testing, change the light value manually
  // lightData.b = 2;
  // lightData.a = 1;
  // lightData.c = 0.8;

  // Adjust brightness based on light value
  int brightness = calculateBrightness();
  FastLED.setBrightness(brightness);
  FastLED.show();

  // Debug print to monitor brightness value
  // Serial.print("Light: ");
  // Serial.print(light);
  // Serial.print(" | Brightness: ");
  // Serial.println(brightness);
  // Serial.println("received");

  delay(200);  // Delay to slow down the loop for better visibility
}

int calculateBrightness() {
  int brightness = brightness_middlebase;

  // median
  if (lightData.b == myPosition) {
    brightness += brightness_highbase * lightData.c;
  } else {
    if ((lightData.b - 1 == myPosition || (myPosition == 4 && lightData.b == 1)) || (lightData.b + 1 == myPosition || (myPosition == 1 && lightData.b == 4))) {
      brightness = brightness_middlebase;
    } else {
      brightness += brightness_lowbase * lightData.c;
    }
  }

  // noise
  float noiseMod = lightData.a * lightData.c;
  int noise = inoise8(noiseMod * flickerSpeed * x, y);  // generate noise
  int brightness_noise = noiseMod * flickerStrength * noise;
  x += 1;  // Increment x to change the noise value over time
  Serial.println("a= " + String(brightness_noise));
  brightness -= brightness_noise;
  Serial.println(brightness);
  return brightness;
}