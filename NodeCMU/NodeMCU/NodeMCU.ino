#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include <dht.h> 
#include <Wire.h>    
#include <LiquidCrystal_I2C.h>
#define LED D0
#define BUTTON D1
#define DHTPIN D3; 
#define readtime  500
dht DHT;
 
WiFiClient wifiConnection;
PubSubClient mqttClient(wifiConnection);

int Time_counter = 0;
int buttonState = 0;
int TEMPERATURE=27;

enum {INIT, STATE_0, STATE_01, STATE_010};

int signal_button;
int state = INIT;

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
    pinMode(DHTPIN,INPUT);
    
    // put your setup code here, to run once
    Serial.begin(115200);
    lcd.init(); //initialize the lcd
    lcd.backlight(); //open the backlight 
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
    
    //mqttClient.subscribe("LEDToggle");
    mqttClient.subscribe("TempAdjust");
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
    
    /*if (String(message_buff) == "1"){
      digitalWrite(LED,1);  
    }
    else{
       digitalWrite(LED,0);
    }*/
  if(topic=="TempAdjust"){
      Serial.print("New temperature: ");
      if(messageTemp == "1"){
        if(TEMPERATURE<37)
          TEMPERATURE++;
      }
      else
        if(TEMPERATURE>17){
          TEMPERATURE--;
        }
        Serial.println(TEMPERATURE);
        snprintf (msg, 75, "%d",TEMPERATURE);
        mqttClient.publish("ButtonValue", msg); 
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

  switch(state){
    case INIT:
      state = STATE_0;
      break;
    case STATE_0:
      signal_button = digitalRead(BUTTON);
      if(!signal_button) state = STATE_01;
      break;
    case STATE_01:
      signal_button = digitalRead(BUTTON);
      if(signal_button) state = STATE_010;
      break;
    case STATE_010:
      signal_button = digitalRead(BUTTON);
      if(!signal_button) buttonState = 1;
      break;
    default:
      state = INIT;
      break;
  }
  Time_counter++;
  if(Time_counter==500){
    Time_counter=0;
    DHT.read11(DHTPIN);
    snprintf (msg, 75, "%d", (int)(DHT.temperature)));
    mqttClient.publish("TempSensor", msg);
  }

    if (buttonState){
      snprintf (msg, 75, "%d", 1);
      mqttClient.publish("ButtonValue", msg);
      buttonState = 0;
      state = INIT;
    }
  
    mqttClient.loop();
    delay(10);
}