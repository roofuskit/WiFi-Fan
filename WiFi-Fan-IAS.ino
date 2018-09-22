#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                                        // Button pin on the esp for selecting modes. D3 for the Wemos!

#include <PubSubClient.h>
#include <IOTAppStory.h>                                    // IotAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IotAppStory
WiFiClient espClient;
PubSubClient client(espClient);


// ================================================ EXAMPLE VARS =========================================
const char* outTopic = "stat/kiddo_fan/speed";
const char* inTopic = "cmd/kiddo_fan/speed";
const char* mqtt_server = "192.168.1.115";
const int  buzzerPin = 5;    // the pin that the pushbutton is attached to
const int buttonPin = 15;       // the pin that the LED is attached to

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
long lastMsg = 0;
char msg[50];
int value = 0;

void button_up() {
      digitalWrite(buttonPin, HIGH);
      delay(250);
      digitalWrite(buttonPin, LOW);
      delay(250);
      buttonPushCounter++;
      status();
}

void button_off() {
      digitalWrite(buttonPin, HIGH);
      delay(250);
      digitalWrite(buttonPin, LOW);
      delay(500);
      buttonPushCounter = 0;
      Serial.println("OFF");
      client.publish(outTopic, "OFF");
      status();
}

void status() {
        if (buttonPushCounter > 4) {
        buttonPushCounter = 0;
        Serial.println("OFF");
        client.publish(outTopic, "OFF");
        }
      else if (buttonPushCounter == 1)  {
        Serial.println("LOW");
        client.publish(outTopic, "LOW");
      }
      else if (buttonPushCounter == 2)  {
        Serial.println("MED");  
        client.publish(outTopic, "MED");
      }
      else if (buttonPushCounter == 3)  {
        Serial.println("HIGH");  
        client.publish(outTopic, "HIGH");
      }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '4') {
    button_up();
  } 
  else if ((char)payload[0] == '1') {
    if (buttonPushCounter == 0){
      button_up();
    }
    else if (buttonPushCounter == 1){
      delay(250);
      status();
    }
    else if (buttonPushCounter == 2){
      button_up();
      button_off();
      button_up();
    }
    else if (buttonPushCounter == 3){
      button_off();
      button_up();
    }
  }
  else if ((char)payload[0] == '2') {
    if (buttonPushCounter == 0){
      button_up();
      button_up();
    }
    else if (buttonPushCounter == 1){
      button_up();
    }
    else if (buttonPushCounter == 2){
      delay(250);
      status();
    }
    else if (buttonPushCounter == 3){
      button_off();
      button_up();
      button_up();
    }
  }
  else if ((char)payload[0] == '3') {
    if (buttonPushCounter == 0){
      button_up();
      button_up();
      button_up();
    }
    else if (buttonPushCounter == 1){
      button_up();
      button_up();
    }
    else if (buttonPushCounter == 2){
      button_up();
    }
    else if (buttonPushCounter == 3){
      delay(250);
      status();
    }
  }
  else if ((char)payload[0] == '0') {
    if (buttonPushCounter == 0){
      delay(250);
      status();
    }
    else if (buttonPushCounter == 1){
      button_up();
      button_up();
      button_off();
    }
    else if (buttonPushCounter == 2){
      button_up();
      button_off();
    }
    else if (buttonPushCounter == 3){
      button_off();
    }
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "OFF");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


// ================================================ SETUP ================================================
void setup() {
  /* TIP! delete lines below when not used */
  IAS.preSetDeviceName("WiFi-Fan");                       // preset deviceName this is also your MDNS responder
  //IAS.preSetAutoUpdate(false);                            // automaticUpdate (true, false)
  //IAS.preSetAutoConfig(false);                            // automaticConfig (true, false)
  //IAS.preSetWifi("ssid","password");                      // preset Wifi
  /* TIP! Delete Wifi cred. when you publish your App. */
  


  // You can configure callback functions that can give feedback to the app user about the current state of the application.
  // In this example we use serial print to demonstrate the call backs. But you could use leds etc.

  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonVeryLongPress([]() {
    Serial.println(F(" If mode button is released, I won't do anything unless you program me to."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    /* TIP! You can use this callback to put your app on it's own configuration mode */
  });
  
  /* 
  IAS.onModeButtonNoPress([]() {
    Serial.println(F(" Mode Button is not pressed."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  
  IAS.onFirstBoot([]() {                              
    Serial.println(F(" Run or display something on the first time this app boots"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateCheck([]() {
    Serial.println(F(" Checking if there is a firmware update available."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateDownload([]() {
    Serial.println(F(" Downloading and Installing firmware update."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateError([]() {
    Serial.println(F(" Update failed...Check your logs"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onConfigMode([]() {
    Serial.println(F(" Starting configuration mode. Search for my WiFi and connect to 192.168.4.1."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  */

	/* TIP! delete the lines above when not used */
 
  //IAS.begin();
  IAS.begin('P');                                     // Optional parameter: What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact

  IAS.setCallHome(true);                              // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(60);                        // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production


  //-------- Your Setup starts from here ---------------
  
  // initialize the Buzzer pin as a input:
  pinMode(buzzerPin, INPUT);
  // initialize the Button as an output:
  pinMode(buttonPin, OUTPUT);
  // initialize serial communication:
  Serial.begin(115200);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();                                   // this routine handles the calling home functionality and reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration


  //-------- Your Sketch starts from here ---------------
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // read the pushbutton input pin:
  buttonState = digitalRead(buzzerPin);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button
      // went from off to on:
      buttonPushCounter++;
      //Serial.print("number of button pushes:  ");
      //Serial.println(buttonPushCounter);
      delay(150);
      status();
    } 
    else {
      // if the current state is LOW then the button
      // went from on to off:
      delay(150);
    }
  }
  // save the current state as the last state, 
  //for next time through the loop
  lastButtonState = buttonState;
  
}
