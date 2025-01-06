
#include <WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>

#define LED_BUILTIN 8

const char* ssid = "Haste_II_Beacon";
const char* password = "Cisco123";

const char* mqtt_server = "192.168.0.107";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

#define NUM_LEDS 4
#define DATA_PIN 0
CRGB leds[NUM_LEDS];

int counter = 0;
u_int32_t color = 0;

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);

  Serial.begin(115200);
  setup_wifi();

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(10);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
}

void setup_wifi() {
  delay(10);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++)
    messageTemp += (char)message[i];

  if (String(topic) == "counter/tick") {
    if(messageTemp == "Tick") 
      counter++;
  }

  if (String(topic) == "counter/value/one") {
    counter = messageTemp.toInt();
  }
  if (String(topic) == "counter/color/one") {
      color = messageTemp.toInt();
  }

  if (String(topic) == "will/two") {
    client.subscribe("counter/color/two");
  }

  if (String(topic) == "counter/color/two") {
    color = messageTemp.toInt();
    client.unsubscribe("counter/color/two");
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ClientOne", "One", "One", "will/one", 1, false, "Client one offline", false)) {
      client.unsubscribe("#");
      client.subscribe("counter/+/one");
      client.subscribe("counter/tick");
      client.subscribe("will/two");
    } else {
      delay(5000);
    }
  }
}

void counter_to_leds(int ticks, CRGB leds[NUM_LEDS], int color) {
  for(int i = 0; i < NUM_LEDS; i++) {
    bool mask = ticks & (0b1 << i);
    leds[i] = mask * color;
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (color != 0){
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  counter_to_leds(counter, leds, color);
  FastLED.show();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;

    char c[10];
    String(color).toCharArray(c, 10);
    client.publish("counter/color/one", c, true);
  }

}




