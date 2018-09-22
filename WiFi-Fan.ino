#include <ESP8266WiFi.h>
#include <PubSubClient.h>
const char* outTopic = "YOUR_TOPIC_HERE"; // status topic for your fan.
const char* inTopic = "YOUR_TOPIC_HERE"; // command topic for your fan.
const int pyldOFF = 0;
const int pyldLOW = 1;
const int pyldMED = 2;
const int pyldHIGH = 3;
const int pyldPRESS = 4;
const char* ssid = "SSID_HERE"; // SSID
const char* password = "PASSWORD"; // PASSWORD
const char* mqtt_server = "MQTT_ADDRESS"; // address of your MQTT broker, do not include the port.
const int  buzzerPin = 5;    // the pin that the buzzer is attached to
const int buttonPin = 15;       // the pin that the pushbutton is attached to

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

//WiFi and MQTT network setup

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// END WiFi and MQTT setup

void button_up() {  // normal button press
      digitalWrite(buttonPin, HIGH);
      delay(250);
      digitalWrite(buttonPin, LOW);
      delay(250);
      buttonPushCounter++;  //increment the button
      status();
}

void button_off() {  // button press with an extra delay for long tone when the fan turns off
      digitalWrite(buttonPin, HIGH);
      delay(250);
      digitalWrite(buttonPin, LOW);
      delay(500);
      buttonPushCounter = 0; // reset the counter to 0 as there are only 4 states for the fan
      Serial.println("OFF");
      client.publish(outTopic, "OFF");
}

void status() {  // broadcast the current status of the fan via serial and MQTT
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


void callback(char* topic, byte* payload, unsigned int length) { // callback function to receive incomming MQTT messages
  Serial.print("Message arrived ["); // announce the message
  Serial.print(topic); // print the topic
  Serial.print("] "); 
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  // print the payload
  }
  Serial.println();

  if ((char)payload[0] == pyldPRESS) { // this payload is a single button press
    button_up();
  } 
  else if ((char)payload[0] == pyldLOW) { // this payload is LOW
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
  else if ((char)payload[0] == pyldMED) {  // this payload is MED
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
  else if ((char)payload[0] == pyldHIGH) {  // this payload is HIGH
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
  else if ((char)payload[0] == pyldOFF) {  // this payload is OFF
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

void reconnect() { // connect to MQTT broker
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

void setup() {  // set pinmodes, open serial connection, setup WiFi and MQTT
  // initialize the Buzzer pin as a input:
  pinMode(buzzerPin, INPUT);
  // initialize the Button as an output:
  pinMode(buttonPin, OUTPUT);
  // initialize serial communication:
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void loop() {
  if (!client.connected()) { // if the MQTT broker isn't connected, reconnect
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
