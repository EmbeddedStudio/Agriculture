//#include <ESP8266WiFi.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

char ssid[]= "s008-flying";
char pass[]= "dianxin132";

int keyIndex = 0;

#define GPIO0 0
#define GPIO2 2

unsigned int localPort = 8181;

//const char* host = "121.42.180.30";

IPAddress timeServer(121, 42, 180, 30);

WiFiClient wifiClient;
//PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;

void setup()
{
  Serial.begin(115200);
  Serial.println("succe");
  //WiFi.begin(ssid, pass);
  Serial.println("successful");
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

}

void loop()
{
  Serial.print("ddd");
  delay(1000);
}







