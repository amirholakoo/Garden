#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


// Replace with your network credentials
const char* ssid = "VPN1";
const char* password = "09126141426";
int moist_count = 0;
int dry_count = 0;
int germination_count = 0;
float wet_soil_ratio = 0;
float germination_avg = 0;
uint16_t light_level = 0;
float temperature = 0;
float humidity = 0;
float pressure = 0;

WebServer server(80);

// Relay connected to GPIO pin
const int relayPin = 4;

const int builtInLedPin = 2;

// Create BME280 instance
Adafruit_BME280 bme;

// Create BH1750 instance
BH1750 lightMeter;

// Create OLED instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  pinMode(builtInLedPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  
  // Initialize BME280 sensor
  if (!bme.begin(0x77)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Initialize BH1750 sensor
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  Serial.println("BH1750 Advanced begin");

  // Initialize OLED
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Initializing...");
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,16);
  display.println(WiFi.localIP());
  display.display();
  delay(5000);

  // Set up server routes
  server.on("/capture", handleCapture);
  server.on("/control_pump", handleControlPump);
  server.on("/sensor_data", handleSensorData);
  server.on("/update_data", HTTP_POST, handleUpdateData);

  // Start the server
  server.begin();
  Serial.println("Server started");
}


//////////////////////////////////////////////////////////////////////////
void loop() {
  server.handleClient();
  Serial.println("Server handling Client");
  handleSensorData();
  Serial.println("Server handling SensorData");
  delay(10 * 1000); // XX SECONDS 
  handleUpdateData();
  Serial.println("Server handling UpdateData");
  delay(10 * 1000);
  handleControlPump();
  Serial.println("Server handling ControlPump");
  delay(10 * 1000);
  //handleUpdateData();
  //delay(10 * 1000);
  /*

  if (moist_count < 3000000 && light_level < 20000) {// 1300 is perfect cloudy light
    digitalWrite(relayPin, LOW); // Turn on the relay (active low)
    Serial.println("WATERING...");
    delay(60 * 1000); // Keep the pump on for 10 seconds (customize as needed)
    digitalWrite(relayPin, HIGH); // Turn off the relay (active low)
    Serial.println("DONE WATERING");
  }

    // Check if the soil is dry (soilMoisture < 0.3)
  if (moist_count < 3000000) {
    digitalWrite(builtInLedPin, HIGH); // Turn on the LED (active low)
    Serial.println("LED ON");
  } else {
    digitalWrite(builtInLedPin, LOW); // Turn off the LED (active low)
  } */

  
  
}


////////////////////////////////////////////////////////////////////
void handleCapture() {
  // Implement your image capture code here
  server.send(200, "text/plain", "Capture not implemented.");
}

void handleControlPump() {
  String state = server.arg("state");
  if (state == "on") {
    
    // Update OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Watering needed!");
    display.setCursor(0,16);
    display.print("Light: ");
    display.println(light_level);
    display.print("Dry Pixels: ");
    display.println(dry_count);
    display.println("PUMP ON");
    display.display();
    
    digitalWrite(relayPin, LOW);
    Serial.println("PUMP ON");
    delay(10);
    digitalWrite(builtInLedPin, HIGH); // Turn on the LED (active low)
    Serial.println("LED ON");

    server.send(200, "text/plain", "Pump turned on");
  } else if (state == "off") {
    digitalWrite(relayPin, HIGH);
    Serial.println("PUMP OFF");
    delay(10);
    digitalWrite(builtInLedPin, LOW); 
    Serial.println("LED OFF");
    
    server.send(200, "text/plain", "Pump turned off");
  } else {
    server.send(400, "text/plain", "Invalid state");
  }
}

void handleSensorData() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  light_level = lightMeter.readLightLevel();

  String json = "{\"temperature\": " + String(temperature) + ",";
  json += "\"humidity\": " + String(humidity) + ",";
  json += "\"pressure\": " + String(pressure) + ",";
  json += "\"light_level\": " + String(light_level) + "}";

  server.send(200, "application/json", json);

    // Update OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Germination %: ");
  display.println(germination_avg);
  display.print("Moist/Total %: ");
  display.println(wet_soil_ratio);
  display.setCursor(0,16);
  display.print("Temp    : ");
  display.print(temperature);
  display.println("   C");
  display.print("Humidity: ");
  display.print(humidity);
  display.println("   %");
  display.print("Pressure: ");
  display.print(pressure);
  display.println("  hPa");
  display.print("Light   : ");
  display.print(light_level);
  display.println("    lx");
  display.print("Moist    :");
  display.println(moist_count);
  display.print("Dry      :");
  display.println(dry_count);
  display.display();
}

float handleUpdateData() {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Updating... ");
    display.display();
    
    // Store the received values in variables
    moist_count = doc["moist_count"];
    dry_count = doc["dry_count"];
    germination_count = doc["germination_count"];
    wet_soil_ratio = doc["wet_soil_ratio"];
    germination_avg = doc["germination_avg"];

    // Update OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Germination Ratio: ");
    display.println(germination_avg);
    display.setCursor(0,16);
    display.print("Moist: ");
    display.println(moist_count);
    display.print("Dry  : ");
    display.println(dry_count);
    display.print("Ratio: % ");
    display.println(wet_soil_ratio);
    display.println("Germination Pixels: ");
    display.println(germination_count);
    display.display();
    server.send(200, "application/json", "{\"status\": \"ok\"}");
  }

  return moist_count;
}
