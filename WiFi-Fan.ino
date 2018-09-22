#include <ESP8266WiFi.h>
#include <PubSubClient.h>
const char* outTopic = "stat/kiddo_fan/speed";
const char* inTopic = "cmd/kiddo_fan/speed";
const char* ssid = "Ashley";
const char* password = "9612030151";
const char* mqtt_server = "192.168.1.115";
const int  buzzerPin = 5;    // the pin that the pushbutton is attached to
const int buttonPin = 15;       // the pin that the LED is attached to

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

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

void setup() {
  // initialize the button pin as a input:
  pinMode(buzzerPin, INPUT);
  // initialize the LED as an output:
  pinMode(buttonPin, OUTPUT);
  // initialize serial communication:
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    //Serial.print("Publish message: ");
    //Serial.println(msg);
    //client.publish("outTopic", msg);
  }
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
