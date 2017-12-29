#include <ESP8266WiFi.h>

char ssid[] = "s008-flying";
char pass[] = "dianxin132";

#define GPIO0 0
#define GPIO2 2

#define LEDID "3115"
#define LEDTOKEN "52e38a188"  //数据接口

unsigned int httpPort = 8181;

const char *host = "121.42.180.30";

WiFiClient client;

int status = WL_IDLE_STATUS;

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());//WiFi.localIP()返回8266获得的ip地址
  Serial.println("success");
}
long last_time = 0;
void loop()
{
  while (!client.connected())//几个非连接的异常处理
  {
    if (!client.connect(host, httpPort))
    {
      Serial.print("disconnect");
      //client.stop();
      delay(500);
    }
    else
    {
      Serial.print("chickin");
      client.print("{\"M\":\"checkin\",\"ID\":\"3115\",\"K\":\"52e38a188\"}\r\n");
      delay(500);
    }

  }
  while (client.available())//，无线读取到的数据转发到到串口
  {
    String s = client.readString();
    Serial.print(s);
    delay(500);
  }
  if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
  {
    size_t counti = Serial.available();
    uint8_t sbuf[counti];
    Serial.readBytes(sbuf, counti);
    client.write(sbuf, counti);
    delay(500);
  }

  if(millis() - last_time > 3000){
    last_time=millis();
    String s = "{\"M\":\"update\",\"ID\":\"3115\",\"V\":{\"2948\":\"29.2\"}}\n";
    client.print(s);
    }
}







