#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

char ssid[] = "360WiFi-48681F";
char pass[] = "dianxin151";

#define  greenhouseID  "8e27efe0-eb81-11e7-8341-353f63eeab61"
#define  greenhouseKEY "i8j8gFS00lwaNzSOxrEe"

char  ServerAddr[] = "117.21.178.99";    //MQTT服务器地址
char StateTopicAddr[] = "v1/devices/me/telemetry";          //client.publish(StateTopicAddr, get_gpio_status().c_str());上传地址
char ControllTopicAddr[] = "v1/devices/me/rpc/request/+";   //client.subscribe(ControllTopicAddr);连接下发地址

WiFiClient wifiClient;
PubSubClient client(wifiClient);
int status = WL_IDLE_STATUS;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)//WiFi.status() ，这个函数是wifi连接状态，返回wifi链接状态
  {
    delay(500);
    //    Serial.print(".");
  }
  client.setServer( ServerAddr, 1883 );//链接服务器及端口
  client.setCallback(on_message);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected())//几个非连接的异常处理
  {
    reconnect();
  }
  while (Serial.available()) { // 来自stm32的数据
      String s = Serial.readString();
      client.publish(StateTopicAddr, s.c_str());
      delay(100);
  }
  client.loop();
}

void reconnect() {
  while (!client.connected())
  {
    status = WiFi.status();
    if ( status != WL_CONNECTED)
    {
//      Serial.print("error wifi");
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        //        Serial.print(".");
      }
      //      Serial.println("Connected to AP");
    }
    else
    {
      client.setServer( ServerAddr, 1883 );
      client.connect( greenhouseID, greenhouseKEY , NULL);
    }
    client.subscribe(ControllTopicAddr);//连接下发的地址
//    Serial.print("begin");  // 回应stm32已经连接服务器
  }
}

void on_message(const char* topic, byte* payload, unsigned int length)
{
  char json[length + 1];
  strncpy (json, (char*)payload, length);
  json[length] = '\0';
    StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject((char*)json);

  if (!data.success())
  {
//    Serial.print("error json data");
    return;
  }
  String methodName = String((const char*)data["method"]);
  String params = String((const char*)data["params"]);
  Serial.print(methodName+"_"+params+"_"+"*");
  delay(100);
}
