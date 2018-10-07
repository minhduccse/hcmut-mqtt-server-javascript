#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#define LED D0
#define BUTTON D1

WiFiClient wifiConnection;
PubSubClient mqttClient(wifiConnection);

int Time_counter = 0;
int buttonState = 0;
int tempButton = 0;
char msg[100];

  //wifi settings
  const char* wifiSsid = "Raspberry Pi 3";
  const char* wifiPassword = "bkumtce16";

  //mqtt settings
  const char* mqttHost = "192.168.0.101";
  const int mqttPort = 1883;

void setup() {
    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    // put your setup code here, to run once
    Serial.begin(115200);

    //print what the programme is connecting to
    Serial.print("Connecting to ");
    Serial.println(wifiSsid);

    //start the wifi library by connection to the wifi
    WiFi.begin(wifiSsid, wifiPassword);

    //while the wifi library is not fully connected
    while (WiFi.status() != WL_CONNECTED) {
      //print '.' every 0.5s
      delay(500);
      Serial.print(".");
    }

    //print that the wifi library is connected and the IP
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //connect to the wifiConnection with mqtt
    mqttClient.setServer(mqttHost, mqttPort);

    //set the callback when the programme recieves a callback
    mqttClient.setCallback(callback);

    while (!mqttClient.connected()) {
      Serial.print("Attempting MQTT connection...");
      if (mqttClient.connect("device1")) {
        Serial.println("connected");
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in one second");
        // Wait a second before retrying
        delay(1000);
      }
    }

    mqttClient.subscribe("LEDToggle");
}

void callback(char *topic, byte *payload, unsigned int length) {
    
    char message_buff[3];
    int i = 0;

    //for loop that loops through the playload array
    for(i = 0; i < length; i++) {
      message_buff[i] = payload[i];
    }

    //\0 is null, this makes it that the String typecast understands where to stop
    message_buff[i] = *"\0";

    Serial.println(String(message_buff));
    
    if (String(message_buff) == "1"){
      digitalWrite(LED,1);  
    }
    else{
       digitalWrite(LED,0);
    }
}

void loop() {
   while (!mqttClient.connected()) {
      Serial.print("Attempting MQTT connection...");
      if (mqttClient.connect("device1")) {
        Serial.println("connected");
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in one second");
        // Wait a second before retrying
        delay(1000);
      }
    }

    tempButton = digitalRead(BUTTON);
    if (tempButton != buttonState){
      buttonState = tempButton;
      snprintf (msg, 75, "%d", tempButton);
      mqttClient.publish("ButtonValue", msg);
    }
  
    mqttClient.loop();
    delay(10);
}
