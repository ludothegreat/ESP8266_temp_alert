#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_AHTX0.h>
#include <EEPROM.h>

ESP8266WebServer server(80);

// Wi-Fi credentials
const char* ssid = "YOURSSID";
const char* password = "YOURPASSWORD";

Adafruit_AHTX0 aht;
const int buzzerPin = 12;  // Replace with your buzzer GPIO pin
const int ledPin = 13;     // Replace with your LED GPIO pin

float temperatureThreshold;  // Global variable for the temperature threshold

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  EEPROM.begin(4); // Allocate 4 bytes for a float
  EEPROM.get(0, temperatureThreshold); // Read the stored threshold
  if (isnan(temperatureThreshold) || temperatureThreshold < -100 || temperatureThreshold > 100) {
    temperatureThreshold = 55.0; // Set a default or last known good value
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!aht.begin()) {
    Serial.println("Could not find AHT sensor!");
    while (1) delay(10);
  }
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
 
  server.on("/", handleRoot); // Define the handler for the root URL
  server.on("/set-threshold", handleSetThreshold);
  server.on("/test-heartbeat", handleTestHeartbeat);
  server.on("/reboot", handleReboot);
  server.begin(); // Start the server
}

unsigned long lastNotificationTime = 0; // Stores the last time a notification was sent
const unsigned long notificationInterval = 3600000; // 1 hour interval between notifications
//const unsigned long notificationInterval = 30000; // 30 seconds interval between notifications - FOR TESTING ONLY

unsigned long lastRSSICheck = 0; // Stores the last time the RSSI was checked
const unsigned long rssiInterval = 10000; // 10 seconds interval between RSSI checks

unsigned long lastHeartbeatTime = 0; // Stores the last time a heartbeat was sent
const unsigned long heartbeatInterval = 86400000; // 24 hours in milliseconds

void loop() {
  server.handleClient(); // Handle client requests
  unsigned long currentTime = millis(); // Current time for various checks

  // Temperature reading
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // Read the data
  float temperatureF = temp.temperature * 9 / 5 + 32; // Convert to Fahrenheit
  temperatureF -= 10; // Adjust the temperature by subtracting 10°F
  Serial.print("Temperature: ");
  Serial.print(temperatureF);
  Serial.println(" degrees F");

  // Buzzer and LED control based on temperature
  if (temperatureF < temperatureThreshold) {
    tone(buzzerPin, 2600);  // Buzzer on
    digitalWrite(ledPin, HIGH); // LED on
    delay(1000);  // Duration for buzzer and LED
    noTone(buzzerPin);  // Buzzer off
    digitalWrite(ledPin, LOW); // LED off
    
    // Notification check
    if (currentTime - lastNotificationTime > notificationInterval) {
      sendIFTTTNotification();
      lastNotificationTime = currentTime; // Update the last notification time
    }
  }

  // Wi-Fi signal strength check
  if (currentTime - lastRSSICheck >= rssiInterval) {
    long rssi = WiFi.RSSI();
    Serial.print("Wi-Fi Signal Strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
    lastRSSICheck = currentTime;
  }

  // Daily heartbeat check
  if (currentTime - lastHeartbeatTime > heartbeatInterval) {
    sendHeartbeat(temperatureF);
    lastHeartbeatTime = currentTime;
  }

  // Serial command processing
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');

    if (input.equals("test_heartbeat")) {
      sendHeartbeat(temperatureF);
    }

    if (input.equals("test_beeper")) {
      tone(buzzerPin, 2600);
      delay(1000);
      noTone(buzzerPin);
    }

    if (input.equals("test_led")) {
      static bool ledState = false;
      ledState = !ledState;
      digitalWrite(ledPin, ledState ? HIGH : LOW);
    }

    if (input.equals("get_ip")) {
      Serial.print("Current IP Address: ");
      Serial.println(WiFi.localIP());
    }

    if (input.equals("get_wifi_strength")) {
      long rssi = WiFi.RSSI();
      Serial.print("Current Wi-Fi Signal Strength (RSSI): ");
      Serial.print(rssi);
      Serial.println(" dBm");
    }

    if (input.equals("heartbeat_status")) {
      unsigned long timeSinceLastHeartbeat = currentTime - lastHeartbeatTime;
      unsigned long timeUntilNextHeartbeat = heartbeatInterval - timeSinceLastHeartbeat;

      Serial.print("Time since last heartbeat: ");
      Serial.print(timeSinceLastHeartbeat / 1000); // Convert milliseconds to seconds
      Serial.println(" seconds");

      Serial.print("Time until next heartbeat: ");
      Serial.print(timeUntilNextHeartbeat / 1000); // Convert milliseconds to seconds
      Serial.println(" seconds");
    }

    if (input.equals("help")) {
      Serial.println("Available Commands:");
      Serial.println("  test_heartbeat - Manually trigger the heartbeat function");
      Serial.println("  test_beeper - Test the beeper");
      Serial.println("  test_led - Toggle the LED state");
      Serial.println("  get_ip - Get the current IP address");
      Serial.println("  get_wifi_strength - Get the current Wi-Fi signal strength");
      Serial.println("  hearbeat_status - Get the current heartbeat status");
      Serial.println("  help - Display this help message");
    }
  }

  delay(2000); // Wait for 2 seconds before next read
}

//Website
void handleRoot() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // Read the data
  float temperatureF = temp.temperature * 9 / 5 + 32; // Convert to Fahrenheit
  //adjust the temp sensor if needed. If not, comment out
  temperatureF -= 10;

  unsigned long currentTime = millis();
  long rssi = WiFi.RSSI(); // Get Wi-Fi signal strength
  unsigned long timeSinceLastHeartbeat = currentTime - lastHeartbeatTime;
  unsigned long timeUntilNextHeartbeat = heartbeatInterval - timeSinceLastHeartbeat;
//Start of HTML for site
  String html = "<html><head><title>Greenhouse Temp Alert Sensor Readings</title></head><body>";
  html += "<h1>Greenhouse Temp Alert</h1>";
  
  // Add form for setting the threshold
  html += "<form action='/set-threshold' method='GET'>";
  html += "Temperature Threshold (F): <input type='number' name='threshold' value='" + String(temperatureThreshold) + "' step='0.1'>";
  html += "<input type='submit' value='Set Threshold'>";
  html += "</form></p>";
  
  // Current Sensor Readings
  html += "<h2>Current Sensor Readings</h2>";
  html += "<p>Temperature: " + String(temperatureF) + "F</p>";
  html += "<p>Wi-Fi Signal Strength: " + String(rssi) + " dBm (less is better)</p>";
  html += "<p>Time since last heartbeat: " + String(timeSinceLastHeartbeat / 1000) + " seconds</p>";
  html += "<p>Time until next heartbeat: " + String(timeUntilNextHeartbeat / 1000) + " seconds</p>";
  html += "<h2>System</h2>";
    // Add buttons for actions
  html += "<button onclick=\"sendCommand('test-heartbeat')\">Test Heartbeat</button>";
  html += "<button onclick=\"sendCommand('reboot')\">Reboot Device</button></p>";

  // JavaScript to handle button clicks
  html += "<script>"
          "function sendCommand(command) {"
          "  var xhttp = new XMLHttpRequest();"
          "  xhttp.onreadystatechange = function() {"
          "    if (this.readyState == 4 && this.status == 200) {"
          "      alert('Command executed: ' + command);"
          "    }"
          "  };"
          "  xhttp.open('GET', command, true);"
          "  xhttp.send();"
          "}"
          "</script>";
  
   html += "</body></html>";
  //End HTML for site
  server.send(200, "text/html", html);
}

void handleSetThreshold() {
  if (server.hasArg("threshold")) {
    temperatureThreshold = server.arg("threshold").toFloat();
    EEPROM.put(0, temperatureThreshold); // Store the new threshold
    EEPROM.commit(); // Ensure data is written to EEPROM
  }

  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "Threshold updated. Redirecting...");
}

void handleTestHeartbeat() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // Read the data
  float currentTemp = temp.temperature * 9 / 5 + 32; // Convert to Fahrenheit and get current temperature
  currentTemp -= 10; // Adjust the temperature if needed

  sendHeartbeat(currentTemp);
  server.send(200, "text/plain", "Heartbeat sent");
}

void handleReboot() {
  server.send(200, "text/plain", "Rebooting...");
  delay(1000);
  ESP.restart();
}

void sendIFTTTNotification() {
  HTTPClient http;
  http.begin("http://maker.ifttt.com/trigger/ASSETNAME/with/key/YOURKEY");
  
  Serial.println("Sending IFTTT Notification...");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("IFTTT Response: " + response);
  } else {
    Serial.println("Error on sending GET: " + String(httpResponseCode));
  }
}

  void sendHeartbeat(float temperature) {
  HTTPClient http;
  String url = "http://maker.ifttt.com/trigger/ASSETNAME/with/key/YOURKEY";
  url += "?value1=" + String(temperature); // Add temperature as a parameter

  http.begin(url);
  
  Serial.println("Sending Daily Heartbeat...");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("IFTTT Response: " + response);
  } else {
    Serial.println("Error on sending GET: " + String(httpResponseCode));
  }

  http.end();
}
