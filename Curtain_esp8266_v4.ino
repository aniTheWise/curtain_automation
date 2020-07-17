/* code currently in esp01(curtain) -  2020-07-12 */
/* this change is to move the curtain esp8266 to the home assistant mqtt broker. */
/* changing the client id to be a constant identifier also */
/* lots of changes to make compatible with home assistant */

/* broker/server is 192.168.1.<homeassistantIP> */
/* port 1883 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// Update these with values suitable for your network.
const char* ssid = "*****";
const char* password = "*****";
const char* mqtt_server = "******";


// home assistant mqtt broker addon - login credentials
String mqttuser = "*****";
String mqttpass = "******";


// PubSub Client object setup
WiFiClient espClient;
PubSubClient client(espClient);


// Working Storage section
int change = 0;
int current_position = 0;
int position_request = 0;
String state = "open";
String availablity = "offline";

//****************************************************************************
// topics
//****************************************************************************
// subscriptions:
//   esp8266/curtain
//   home-assistant/cover/availability
//   home-assistant/cover/set
//   home-assistant/cover/set_position
//   home-assistant/cover/state

// publishings:
//   borker/curtain
//   home-assistant/cover/availability
//   home-assistant/cover/set
//   home-assistant/cover/set_position
//   home-assistant/cover/state

//****************************************************************************
// Functions
//****************************************************************************
int calculate_position_change(int pos_in) {
  if (pos_in > 28) {
    change = abs(current_position - 28);
    current_position = 28;
  }
  else if (pos_in < 0) {
    change = -current_position;
    current_position = 0;
  }
  else if (pos_in < current_position) {
    change = pos_in - current_position;
    current_position = pos_in;
  }
  else if (pos_in > current_position) {
    change = pos_in - current_position;
    current_position = pos_in;
  }
  else {
    change = 0;
  }
  return change;
}

void motor_command(int new_position) {
  if (new_position == 0) {
    ;
  }
  else if (new_position > 0) {
    Serial.print("[ESP8266|Serial]: curtain_down -r ");
    Serial.print(abs(new_position));
    Serial.println();
  }
  else {
    Serial.print("[ESP8266|Serial]: curtain_up -r ");
    Serial.print(abs(new_position));
    Serial.println();
  }
}

void topic_availability(String msg) {
  if (msg == "online") {
    ;
  }
  else if (msg == "offline") {
    ;
  }
  else {
    ;
  }
}

void topic_set(String msg) {
  if (msg == "curtain_up") {
    motor_command(calculate_position_change(-28));
  }
  else if (msg == "curtain_down") {
    motor_command(calculate_position_change(28));
  }
  else if (msg == "curtain_stop") {
    ;
  }
  else {
    ;
  }
}

void topic_set_position(String msg) {
  client.publish("broker/curtain", msg.c_str());
  position_request = msg.toInt();
  motor_command(calculate_position_change(position_request));
}

void topic_state(String msg) {
  if (msg == "open") {
    ;
  }
  else if (msg == "opening") {
    ;
  }
  else if (msg == "closed") {
    ;
  }
  else if (msg == "closing") {
    ;
  }
  else {
    ;
  }
}

//****************************************************************************
// Message Handler
//****************************************************************************
void callback(char* topic, byte* payload, unsigned int length) {
  //  Serial.print("[");
  //  Serial.print(topic);
  //  Serial.print("]: ");
  String message = "";

  // collect message (mqtt payload) from serial buffer
  for (int i = 0; i < length; i++) {
    //     Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  //  Serial.println(message);

  // process mqtt payload
  // Switch on the LED if an 1 was received as first character
  //  if ((char)payload[0] == '1') {
  //    // Turn the LED on (Note that LOW is the voltage level
  //    digitalWrite(BUILTIN_LED, LOW);
  //    // but actually the LED is on; this is because
  //    // it is active low on the ESP-01)
  //  }
  //
  //  else if ((char)payload[0] == '0') {
  //    // Turn the LED on (Note that LOW is the voltage level
  //    digitalWrite(BUILTIN_LED, HIGH);
  //    // but actually the LED is on; this is because
  //    // it is active low on the ESP-01)
  //  }
  //
  //  else {
  //  }

  if (String(topic) == "home-assistant/cover/availability") {
    topic_availability(message);
  }
  else if (String(topic) == "home-assistant/cover/set") {
    topic_set(message);
  }
  else if (String(topic) == "home-assistant/cover/set_position") {
    topic_set_position(message);
  }
  else if (String(topic) == "home-assistant/cover/state") {
    topic_state(message);
  }

  else if (message.indexOf("curtain_up") > -1) {
    if (message.length() == 10) {
      Serial.print("[ESP8266|Serial]: curtain_up -r default");
      Serial.println();
    }
    else {
      Serial.print("[ESP8266|Serial]: curtain_up -r ");
      Serial.print(message.substring(message.indexOf("-r ") + 3));
      Serial.println();
    }
    client.publish("broker/curtain", "[ESP8266|Mqtt]: curtain_up received");
  }

  else if (message.indexOf("curtain_down") > -1) {
    if (message.length() == 12) {
      Serial.print("[ESP8266|Serial]: curtain_down -r default");
      Serial.println();
    }
    else {
      Serial.print("[ESP8266|Serial]: curtain_down -r ");
      Serial.print(message.substring(message.indexOf("-r ") + 3));
      Serial.println();
    }
    client.publish("broker/curtain", "[ESP8266|Mqtt]: curtain_down received");
  }

  else {
    // Turn the LED off by making the voltage HIGH
    //digitalWrite(BUILTIN_LED, HIGH);
    ;
  }
}

void reconnect() {
  // Loop until we're reconnected  to MQTT Broker
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    //    String clientId = "ESP8266Client-";
    //    clientId += String(random(0xffff), HEX);

    // Create a very specfic client ID
    String clientId = "curtain_mqtt_client";

    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttuser.c_str(), mqttpass.c_str())) {
      Serial.println("connected");

      // reconnection message publish
      client.publish("broker/curtain", "curtain controller connected to broker");
      client.publish("home-assistant/cover/availability", "online");
      client.publish("home-assistant/cover/state", "open");

      // subscribe to topics
      client.subscribe("esp8266/curtain");
      client.subscribe("home-assistant/cover/set");
      client.subscribe("home-assistant/cover/set_position");
      client.subscribe("home-assistant/cover/state");

    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//****************************************************************************
// Arduino SETUP
//****************************************************************************
void setup() {
  // Serial monitor setup
  Serial.begin(9600);
  delay(10);
  // Wifi network connection
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(". ");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to MQTT server
  // Set Callback for MQTT communication
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // get the motor driver to sleep properly by moving the motor
  motor_command(1);
  delay(2000);
  motor_command(-1);
  delay(1000);
}

//****************************************************************************
// Arduino LOOP
//****************************************************************************
void loop() {
  //****************************************************************************
  // but actually the LED is on; this is because it is active low on the ESP-01)
  //****************************************************************************
  //  digitalWrite(1, LOW);
  //  digitalWrite(2, LOW);
  //  delay(2000);
  //  digitalWrite(1, HIGH);
  //  digitalWrite(2, HIGH);

  // Check MQTT connection and reconnect if not
  if (!client.connected()) {
    reconnect();
  }
  // loop for communication and connection (also)
  client.loop();

}
