#include <ESP8266WiFi.h>

#define relay1 2
const char *ssid     = "s008-flying";//这里是我的wifi，你使用时修改为你要连接的wifi ssid
const char *password = "dianxin132";//你要连接的wifi密码
const char *host = "121.42.180.30";//修改为手机的的tcpServer服务端的IP地址，即手机在路由器上的ip
WiFiClient client;
const int tcpPort = 8181;//修改为你建立的Server服务端的端口号


void setup()
{ //pinMode(relay1,OUTPUT);
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)//WiFi.status() ，这个函数是wifi连接状态，返回wifi链接状态
    //这里就不一一赘述它返回的数据了，有兴趣的到ESP8266WiFi.cpp中查看
  {
    delay(500);
    Serial.print(".");
  }//如果没有连通向串口发送.....

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());//WiFi.localIP()返回8266获得的ip地址
}


void loop()
{
  while (!client.connected())//几个非连接的异常处理
  {
    if (!client.connect(host, tcpPort))
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
  while (client.available())    //无线读取到的数据转发到到串口
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
      //client.write(sbuf, counti);
    delay(500);
  }
  client.print("{\"M\":\"update\",\"ID\":\"3115\",\"V\":{\"2948\":\"33\"}}\r\n");
  delay(500);
}

