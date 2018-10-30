#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include <dht.h>
#include <Wire.h>    
#include <LiquidCrystal_I2C.h>
#define LED D0
#define BUTTON D6   //button up
#define BUTTON1 D5  //button down
#define DHTPIN D3
#define readtime  500
dht DHT;
LiquidCrystal_I2C lcd(0x27,16,2);
/// LED : no need
/// BUTTON UP PIN :D6
/// BUTTON DOWN PIN :D5
/// SENSOR PIN :D3
///SDA  D2
///SDL  D1
WiFiClient wifiConnection;
PubSubClient mqttClient(wifiConnection);

int Time_counter = 0;
int buttonState = 0;
int button1State = 0;
int TEMPERATURE=27;

enum {INIT, STATE_0, STATE_01, STATE_010};

int signal_button;
int state = INIT;
int state1 = INIT;

char msg[100];

  //wifi settings
  const char* wifiSsid = "Raspberry Pi 3";
  const char* wifiPassword = "bkumtce16";

  //mqtt settings
  const char* mqttHost ="192.168.1.100";
  const int mqttPort = 1883;

void setup() {
    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    pinMode(BUTTON1, INPUT);
    pinMode(DHTPIN,INPUT);
    digitalWrite(BUTTON, LOW);
    digitalWrite(BUTTON1, LOW);
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
      if(String(message_buff) == "1"){
        if(TEMPERATURE<37)
          TEMPERATURE++;
      }
      else
        if(TEMPERATURE>17){
          TEMPERATURE--;
        }
        Serial.println(TEMPERATURE);
        snprintf (msg, 75, "%d",TEMPERATURE);
        mqttClient.publish("TempAirConditioner", msg); 
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
      if(!signal_button) {
        buttonState =0;
        state = STATE_01;
      }
      break;
    case STATE_01:
      signal_button = digitalRead(BUTTON);
      if(signal_button) state = STATE_010;
      break;
    case STATE_010:
      signal_button = digitalRead(BUTTON);
      if(!signal_button) {
        buttonState = 1;
        state = STATE_0;
      }
      break;
    default:
      state = INIT;
      break;
  }
  switch(state1){
    case INIT:
      state1 = STATE_0;
      break;
    case STATE_0:
      signal_button = digitalRead(BUTTON1);
      if(!signal_button) {
        button1State =0;
        state1 = STATE_01;
      }
      break;
    case STATE_01:
      signal_button = digitalRead(BUTTON1);
      if(signal_button) state1 = STATE_010;
      break;
    case STATE_010:
      signal_button = digitalRead(BUTTON1);
      if(!signal_button) {
        button1State = 1;
        state1 = STATE_0;
      }
      break;
    default:
      state1 = INIT;
      break;
  }

  Time_counter++;
  if(Time_counter==500){
    lcd.clear();
    Time_counter=0;
    DHT.read11(DHTPIN);
    snprintf (msg, 75, "%d", (int)(DHT.temperature));
    Serial.println(DHT.temperature);
    mqttClient.publish("TempSensor", msg);
    lcd.setCursor(0,0); 
    lcd.print(DHT.temperature);
  }

    if (buttonState){
      Serial.print("New temperature: ");
      if(TEMPERATURE<37)
        TEMPERATURE++;
      Serial.println(TEMPERATURE);
      snprintf (msg, 75, "%d",TEMPERATURE);
      mqttClient.publish("TempAirConditioner", msg); 
    }
    
    if (button1State){
      Serial.print("New temperature: ");
      if(TEMPERATURE>17)
        TEMPERATURE--;
      Serial.println(TEMPERATURE);
      snprintf (msg, 75, "%d",TEMPERATURE);
      mqttClient.publish("TempAirConditioner", msg); 
    }
  
    mqttClient.loop();
    delay(10);
}
