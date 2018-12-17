#include <ESP8266WiFi.h>
#include "Gsender.h"
#include <FirebaseArduino.h>


// Set these to run example.
#define FIREBASE_HOST "nodemcu-30f53.firebaseio.com"
#define FIREBASE_AUTH "HMfJ8WuF3OUIJBRA96FMhdJAPFY92Cv0UdvoJDGB"

#pragma region Globals
const char* ssid = "PJCIP";                           // WIFI network name
const char* password = "24121999";                       // WIFI network password
uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
#pragma endregion Globals

int pirPin = D8;

int minSecsBetweenEmails = 60; // 1 min

long lastSend = -minSecsBetweenEmails * 1000l;


int count = 0;

String c;
uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);  
        Serial.println(nSSID);
    } else {
        WiFi.begin(ssid, password);
        Serial.println(ssid);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}

void setup()
{
    Serial.begin(9600);
    connection_state = WiFiConnect();
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop()
{

  long now = millis();
  if (digitalRead(pirPin) == HIGH)
  {
    if (now > (lastSend + minSecsBetweenEmails * 1000l))
    {
      count++;
      Serial.println("MOVEMENT");
      Firebase.setString("Pir Status","Movement Detected");
      Firebase.setInt("Count",count);
      if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
  }
  String c =String(count);
  delay(1000);
  lastSend = now;
  sendEmail();
    }
    else
    {
      Serial.println("Too soon");
    }
  }
  delay(500);
}
byte sendEmail()
{
Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "Intrusion!";
    if(gsender->Subject(subject)->Send("pjcip1999@gmail.com", "Movement is detected!!!!")) {
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
}



