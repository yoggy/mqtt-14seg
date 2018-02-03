#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// for Wifi settings
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/

// Wif config
extern char *wifi_ssid;
extern char *wifi_password;
extern char *mqtt_server;
extern int  mqtt_port;

extern char *mqtt_client_id;
extern bool mqtt_use_auth;
extern char *mqtt_username;
extern char *mqtt_password;

extern char *mqtt_subscribe_topic;

WiFiClient wifi_client;
void mqtt_sub_callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqtt_client(mqtt_server, mqtt_port, mqtt_sub_callback, wifi_client);

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

void setup() {
  Serial.begin(115200);

  alpha4.begin(0x70);
  clear();

  scroll("MQTT-14SEG");

  Serial.println("connecting...");
  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.mode(WIFI_STA);
  int wifi_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    switch(wifi_count % 4) {
      case 0:
        disp("|");
        break;
      case 1:
        disp("/");
        break;
      case 2:
        disp("-");
        break;
      case 3:
        disp("\\");
        break;
    }
    wifi_count ++;
    delay(300);

    if (wifi_count > 100) reboot();
  }

  bool rv = false;
  if (mqtt_use_auth == true) {
    rv = mqtt_client.connect(mqtt_client_id, mqtt_username, mqtt_password);
  }
  else {
    rv = mqtt_client.connect(mqtt_client_id);
  }
  if (rv == false) {
    reboot();
  }
  Serial.println("connected!");

  disp("CONN");
  delay(1000);
  disp_with_dot("    .");

  mqtt_client.subscribe(mqtt_subscribe_topic);
}

void reboot() {
  Serial.println("reboot!");

  for (int i = 0; i < 5; ++i) {
    scroll("REBOOT");
  }

  disp(" . . . .");
  ESP.reset();

  while (true) {
    disp("!!!!");
    delay(500);
    disp("    ");
    delay(500);
  };
}

void loop() {
  if (!mqtt_client.connected()) {
    reboot();
  }
  mqtt_client.loop();
}

void mqtt_sub_callback(char* topic, byte* payload, unsigned int length) {
  char buf[9];
  memset(buf, 0, 9);
  int buf_len = length > 8 ? 8 : length;
  for (int i = 0; i < buf_len; ++i) {
    buf[i] = (char)payload[i];
  }

  Serial.println(buf);
  disp_with_dot(buf);
}

void clear()
{
  alpha4.clear();
  alpha4.writeDisplay();
}

void scroll(char *msg)
{
  if (msg == NULL || strlen(msg) == 0) {
    clear();
    return;
  }

  int str_len = 4 + strlen(msg) + 4;
  char *str = (char*)malloc(str_len);
  memset(str, ' ', str_len);
  memcpy(str + 4, msg, strlen(msg)); // "MESSAGE" -> "____MESSAGE____"

  char buf[5];

  for (int offset = 0; offset < 4 + strlen(msg); ++offset) {
    memset(buf, 0, 5);
    memcpy(buf, str + offset, 4);
    disp(buf);
    delay(200);
  }
    
  free(str);
}

void disp(char *buf) {  
  alpha4.clear();
  int buf_len = strlen(buf);

  if (buf == NULL || buf_len == 0) {
    alpha4.writeDisplay();
    return;
  }

  char c;
  for (int idx = 0; idx < 4; ++idx) {
    if (idx < buf_len) {
      c = buf[idx];
    }
    else {
      c = 0x20; // blank charactor)
    }
    alpha4.writeDigitAscii(idx, c, false);
  }
  alpha4.writeDisplay();
}

void disp_with_dot(char *buf) {  
  alpha4.clear();
  int buf_len = strlen(buf);

  if (buf == NULL || buf_len == 0) {
    alpha4.writeDisplay();
    return;
  }

  int buf_idx = 0;
  char c, d;
  for (int disp_idx = 0; disp_idx < 4; ++disp_idx) {
    if (buf_idx < buf_len) {
      c = buf[buf_idx];
      d = 0;
      if (buf_idx + 1 < buf_len) {
        d = buf[buf_idx + 1];
      }
    }
    else {
      c = 0x20; // blank charactor)
      d = 0;
    }

    if (d == '.') {
      alpha4.writeDigitAscii(disp_idx, c, true);
      buf_idx += 2;
    }
    else {
      alpha4.writeDigitAscii(disp_idx, c, false);
      buf_idx ++;
    }
  }
  alpha4.writeDisplay();
}

