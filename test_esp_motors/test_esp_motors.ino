//Settings
#define WIFI_SSID "NodeMCU"
#define WIFI_PASSWORD "nodemcu22"
#define LISTEN_PORT 9002
#define TIMEOUT_MILLIS 5000

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <Wire.h>

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define RX 3
#define TX 1
#define SD2 9

char incomingPacket[256];

/* define L298N or L293D motor control pins */
int MotorENB = D4; /* GPIO14(D5) -> Motor-A Enable */

int rightMotorForward1 = D0;   /* GPIO15(D8) -> IN1  */
int rightMotorBackward1 = D1;  /* GPIO13(D1) -> IN2  */
int leftMotorForward1 = D2;     /* GPIO2(D4) -> IN3   */
int leftMotorBackward1 = D3;    /* GPIO0(D3) -> IN4   */

int rightMotorForward2 = D5;   /* GPIO15(D8) -> IN1  */
int rightMotorBackward2 = D6;  /* GPIO13(D1) -> IN2  */
int leftMotorForward2 = D7;     /* GPIO2(D4) -> IN3   */
int leftMotorBackward2 = D8;    /* GPIO0(D3) -> IN4   */

/* define L298N or L293D enable pins */



WiFiUDP Udp;

long timeoutTargetMS = 0;


void setup() {
  Serial.begin(230400);
  Serial.println("\n\nESP Booted.");
  setupWifi();
  /* initialize motor control pins as output */
  pinMode(leftMotorForward1, OUTPUT);
  pinMode(rightMotorForward1, OUTPUT); 
  pinMode(leftMotorBackward1, OUTPUT);  
  pinMode(rightMotorBackward1, OUTPUT);

  pinMode(leftMotorForward2, OUTPUT);
  pinMode(rightMotorForward2, OUTPUT); 
  pinMode(leftMotorBackward2, OUTPUT);  
  pinMode(rightMotorBackward2, OUTPUT);

  /* initialize motor enable pins as output */
  pinMode(MotorENB, OUTPUT);

}


String localIP(){
  IPAddress ip;
  if (WIFI_SSID==""){
    ip = WiFi.softAPIP();
  } else {
    ip = WiFi.localIP();
  }
  String sip = String(ip[0]);
  sip += ".";sip += ip[1];
  sip += ".";sip += ip[2];
  sip += ".";sip += ip[3];
  return(sip);
}

void setupWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    yield();
    Serial.print(".");
  }
  Serial.println(localIP());
  Udp.begin(LISTEN_PORT);
}


/********************************************* FORWARD *****************************************************/
void MotorForward(void)   
{
  digitalWrite(MotorENB,HIGH);
  digitalWrite(leftMotorForward1,HIGH);
  digitalWrite(rightMotorForward1,HIGH);
  digitalWrite(leftMotorBackward1,LOW);
  digitalWrite(rightMotorBackward1,LOW);

  digitalWrite(leftMotorForward2,HIGH);
  digitalWrite(rightMotorForward2,HIGH);
  digitalWrite(leftMotorBackward2,LOW);
  digitalWrite(rightMotorBackward2,LOW);
}

/********************************************* BACKWARD *****************************************************/
void MotorBackward(void)   
{
  digitalWrite(MotorENB,HIGH);
  digitalWrite(leftMotorBackward1,HIGH);
  digitalWrite(rightMotorBackward1,HIGH);
  digitalWrite(leftMotorForward1,LOW);
  digitalWrite(rightMotorForward1,LOW);

  digitalWrite(leftMotorBackward2,HIGH);
  digitalWrite(rightMotorBackward2,HIGH);
  digitalWrite(leftMotorForward2,LOW);
  digitalWrite(rightMotorForward2,LOW);
}

/********************************************* TURN LEFT *****************************************************/
void TurnRight(void)   
{
  digitalWrite(MotorENB,HIGH); 
  digitalWrite(leftMotorForward1,HIGH);
  digitalWrite(rightMotorForward1,LOW);
  digitalWrite(rightMotorBackward1,LOW);
  digitalWrite(leftMotorBackward1,LOW);

  digitalWrite(leftMotorForward2,HIGH);
  digitalWrite(rightMotorForward2,LOW);
  digitalWrite(rightMotorBackward2,LOW);
  digitalWrite(leftMotorBackward2,LOW); 
}

/********************************************* TURN RIGHT *****************************************************/
void TurnLeft(void)   
{
  digitalWrite(MotorENB,HIGH);
  digitalWrite(leftMotorForward1,LOW);
  digitalWrite(rightMotorForward1,HIGH);
  digitalWrite(rightMotorBackward1,LOW);
  digitalWrite(leftMotorBackward1,LOW);

  digitalWrite(leftMotorForward2,LOW);
  digitalWrite(rightMotorForward2,HIGH);
  digitalWrite(rightMotorBackward2,LOW);
  digitalWrite(leftMotorBackward2,LOW);
}

/********************************************* STOP *****************************************************/
void MotorStop(void)   
{
  digitalWrite(MotorENB,HIGH);
  digitalWrite(leftMotorForward1,LOW);
  digitalWrite(leftMotorBackward1,LOW);
  digitalWrite(rightMotorForward1,LOW);
  digitalWrite(rightMotorBackward1,LOW);

  digitalWrite(leftMotorForward2,LOW);
  digitalWrite(leftMotorBackward2,LOW);
  digitalWrite(rightMotorForward2,LOW);
  digitalWrite(rightMotorBackward2,LOW);
}


//OSC from Neuromore
void mentalImagery(OSCMessage &msg) {
  Serial.println("OUTPUTTING FROM MESSAGE RECIEVED");
  float right_prediction = msg.getFloat(0);
  float left_prediction = msg.getFloat(1); 
  Serial.println(right_prediction);
  Serial.println(left_prediction);
  if (left_prediction > 0.6 && right_prediction < 0.4) {
    Serial.println("LEFT");
    TurnLeft();
  }
  else if (right_prediction > 0.6 && left_prediction < 0.4){
    Serial.println("RIGHT");
    TurnRight();
  }
  else if (right_prediction > 0.6 && left_prediction > 0.6){
    Serial.println("FORWARD");
    MotorForward();
  }
  else{
    Serial.println("STOP");
    MotorStop();
  }
}

void loop() {
  OSCBundle bundle;
  int size = Udp.parsePacket();
  if (size > 0) {
    Serial.println("RECIEVED");
    while (size--) {
      bundle.fill(Udp.read());
    }

    if (!bundle.hasError()) {
      Serial.println("DISPATCHING");
      bundle.dispatch("/neuropype", mentalImagery);
    }
    //comment
    //Reset timeout
    timeoutTargetMS = millis() + TIMEOUT_MILLIS;
  }
}
