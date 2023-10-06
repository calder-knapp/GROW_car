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

float gain = 0.01;
float prediction = 0;

signed int R_set;
signed int L_set;

signed int R_PWM = 0;
signed int L_PWM = 0;

WiFiUDP Udp;

long timeoutTargetMS = 0;

void setup() {
  Serial.begin(230400);
  Serial.println("\n\nESP Booted.");
  setupWifi();
  
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);

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


//OSC from Neuromore
void mentalImagery(OSCMessage &msg) {
  Serial.println("OUTPUTTING FROM MESSAGE RECIEVED");
  prediction = msg.getFloat(0);
  Serial.println(int(prediction));

  TestControl(int(prediction));
  
}


void RightPWM(int duty)
{

    digitalWrite(D4,HIGH);

    analogWrite(D1, duty);
    digitalWrite(D0, LOW);

    analogWrite(D2, duty);
    digitalWrite(D3, LOW);
}

void LeftPWM(int duty)
{
    digitalWrite(D4,HIGH);

    digitalWrite(D8, LOW);
    analogWrite(D7, duty);

    digitalWrite(D6, LOW);
    analogWrite(D5, duty);

}

void RightPWM_REV(int duty)
{

    duty = 255 - duty;

    digitalWrite(D4,HIGH);

    analogWrite(D1, duty);
    digitalWrite(D0, HIGH);

    analogWrite(D2, duty);
    digitalWrite(D3, HIGH);
}

void LeftPWM_REV(int duty)
{

    duty = 255 - duty;
    digitalWrite(D4,HIGH);

    digitalWrite(D8, HIGH);
    analogWrite(D7, duty);

    digitalWrite(D6, HIGH);
    analogWrite(D5, duty);

}

void DutyLR(int PWM_L, int PWM_R)
{

  if (PWM_L > 255)
  {
    PWM_L = 255;
  }
  if (PWM_R > 255)
  {
    PWM_R = 255;
  }

  if (PWM_L < -255)
  {
    PWM_L = -255;
  }
  if (PWM_R < -255)
  {
    PWM_R = -255;
  }


  if (PWM_L < 0)
  {
    LeftPWM_REV(abs(PWM_L));
  }
  else
  {
    LeftPWM(PWM_L);
  }

  if (PWM_R < 0)
  {
    RightPWM_REV(abs(PWM_R));
  }
  else
  {
    RightPWM(PWM_R);
  }

}

void TestControl(int prediction)
{
  
  //Serial.println(prediction);
  if (prediction == 0)
  {
    // Stop state
    R_set = 0;
    L_set = 0;

  }
  else if (prediction == 1){
    R_set = 255;
    L_set = 255;

  }
  else if (prediction == 2){
    R_set = 0;
    L_set = 255;

  }
  else if (prediction == 3)
  {
    R_set = -255;
    L_set = 255;

  }
  else if (prediction == 4)
  {
    R_set = -255;
    L_set = 0;
  }
  else if (prediction == 5)
  {
    R_set = -255;
    L_set = -255;
    
  }
  else if (prediction == 6)
  {
    R_set = 0;
    L_set = -255; 
  }
  else if (prediction == 7)
  {
    R_set = 255;
    L_set = -255;
  }
  else if (prediction == 8)
  {
    R_set = 255;
    L_set = 0;
  }

  R_PWM = R_PWM + (R_set - R_PWM) * gain;
  L_PWM = L_PWM + (L_set - L_PWM) * gain;
  /*
  Serial.print("RIGHT: ");
  Serial.println(R_PWM);
  Serial.print("LEFT: ");
  Serial.println(L_PWM);
  */  

  DutyLR(L_PWM, R_PWM);
}

void loop() {
  OSCBundle bundle;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }

    if (!bundle.hasError()) {
      bundle.dispatch("/neuropype", mentalImagery);
    }
    
    //Reset timeout
    timeoutTargetMS = millis() + TIMEOUT_MILLIS;
  }

  TestControl(int(prediction));


}
