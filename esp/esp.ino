#include <WiFi.h> 
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "secrets.h"
#define RXD2 33
#define TXD2 4
#define MQTT_SERVER      "193.147.79.118"
#define MQTT_SERVERPORT  21883
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD    ""

WiFiClient wifiClient;
Adafruit_MQTT_Client mqtt(&wifiClient, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);
String topicStr = "/SETR/2025/4/";
Adafruit_MQTT_Publish pub_robot = Adafruit_MQTT_Publish(&mqtt, topicStr.c_str());

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }

  Serial.println("MQTT CONNECTED!");
}

void pubMsg(int action, unsigned long arg) {
  String json;

  switch(action) {
    
    case 0: // START_LAP
      json = "{\"team_name\":\"Maradona\",\"id\":\"4\",\"action\":\"START_LAP\"}";
      break;

    case 1: // END_LAP
      json = "{\"team_name\":\"Maradona\",\"id\":\"4\",\"action\":\"END_LAP\",\"time\":" 
        + String(arg) + "}";
      break;

    case 2: // OBSTACLE_DETECTED
      json = "{\"team_name\":\"Maradona\",\"id\":\"4\",\"action\":\"OBSTACLE_DETECTED\",\"distance\":" 
        + String(arg) + "}";
      break;

    case 3: // LINE_LOST
      json = "{\"team_name\":\"Maradona\",\"id\":\"4\",\"action\":\"LINE_LOST\"}";
      break;

    case 4: // PING
      json = "{\"team_name\":\"Maradona\",\"id\":\"4\",\"action\":\"PING\",\"time\":" 
        + String(arg) + "}";
      break;

    case 5: // INIT_LINE_SEARCH
      json = "{\"team_name\":\"Maradona\",\"id\":\"4\",\"action\":\"INIT_LINE_SEARCH\"}";
      break;

    case 6: // STOP_LINE_SEARCH
      json = "{\"team_name\":\"Maradona\",\"id\":\"4\",\"action\":\"STOP_LINE_SEARCH\"}";
      break;

    case 7: // LINE_FOUND
      json = "{\"team_name\":\"Maradona\",\"id\":\"4\",\"action\":\"LINE_FOUND\"}";
      break;

    case 8: // VISIBLE_LINE
      json = "{\"team_name\":\"Maradona\",\"id\":\"4\",\"action\":\"VISIBLE_LINE\",\"value\":" 
        + String(arg) + "}";
      break;

    default:
      Serial.println("Error receiving from Arduino");
      break;
  }
  pub_robot.publish(json.c_str());
}


void processMsg(String msg) {
  msg.replace("{", "");
  msg.replace("}", "");

  int action = -1, sep = msg.indexOf(':');
  unsigned long arg = 0;

  if (sep == -1) {
    action = msg.toInt();
  } else {
    action = msg.substring(0, sep).toInt();
    arg = msg.substring(sep + 1).toInt();
  }

  pubMsg(action, arg);
}

/*
//SSID NAME
const char* ssid = "eduroam"; // eduroam SSID


void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  delay(5000);

  Serial.print(F("Connecting to network: "));
  Serial.println(ssid);
  WiFi.disconnect(true); 

  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println("");
  Serial.println(F("WiFi is connected!"));
  Serial.println(F("IP address set: "));
  Serial.println(WiFi.localIP()); //print LAN IP

  MQTT_connect();

  Serial2.print("{OK}");
  Serial.print("Messase sent! to Arduino");
}

void loop() {
  yield();
}
*/

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  delay(5000);

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  MQTT_connect();

  Serial2.print("{OK}");
  Serial.print("Messase sent! to Arduino");
}

String sendBuff;

void loop() {
    if (Serial2.available()) {

      char c = Serial2.read();
      sendBuff += c;
      
      if (c == '}')  {            
        Serial.print("Received data in serial port from Arduino: ");
        Serial.println(sendBuff);

        processMsg(sendBuff);
        sendBuff = "";
      } 
  }

}




