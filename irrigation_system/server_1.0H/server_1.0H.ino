#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include<DHT.h>

char ssid[] = "s008-flying";
char pass[] = "dianxin132";

#define  greenhouseID  "8e27efe0-eb81-11e7-8341-353f63eeab61"
#define  greenhouseKEY "i8j8gFS00lwaNzSOxrEe"

#define GPIO0 0
#define GPIO2 2
#define GPIO4 4

unsigned int httpPort = 8181;

const char *host = "121.42.180.30";      //贝壳物联服务器地址

char  ServerAddr[] = "117.21.178.99";    //MQTT服务器地址
char StateTopicAddr[] = "v1/devices/me/telemetry";
char ControllTopicAddr[] = "v1/devices/me/rpc/request/+";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

/*
   json关键字
   Temperature   //温度
   Humidity　　　//湿度　
   CO2　　        //二氧化碳
   PH            //PH值
   Illumination　//光照

   水阀控制
   getHydrovalve1
   setHydrovalve1
   电机控制
   getMotor1
   setMotor1
   小车位置控制
   getCarPos
   setCarPos
*/


int status = WL_IDLE_STATUS;

boolean gpioState[] = {false, false};

long last_time = 0;

float temp = 30.5;

void setup()
{
  Serial.begin(115200);
  delay(10);
  pinMode(GPIO2, OUTPUT);
  pinMode(GPIO0, OUTPUT);
  pinMode(GPIO4, OUTPUT);
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)//WiFi.status() ，这个函数是wifi连接状态，返回wifi链接状态
  {
    delay(500);
    Serial.print(".");
  }//如果没有连通向串口发送.....
  client.setServer( ServerAddr, 1883 ); //链接服务器及端口
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());//WiFi.localIP()返回8266获得的ip地址
  Serial.println("mac address");
  Serial.println(WiFi.macAddress());
  client.setCallback(message);
}

void loop()
{
  while (!client.connected())//几个非连接的异常处理
  {
    status = WiFi.status();
    if ( status != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    else
    {
      client.setServer( ServerAddr, 1883 );
      client.connect( greenhouseID, greenhouseKEY , NULL);
      Serial.println("Connecting to Server node ...");
    }
  }
  //  if ( client.connect( greenhouseID, greenhouseKEY , NULL) )
  //  {
  Serial.println( "[DONE]" );
  client.subscribe(ControllTopicAddr);
  Serial.println("Sending current GPIO status ...");
  client.publish(StateTopicAddr, get_gpio_status().c_str());
  client.publish(StateTopicAddr, getMotor1().c_str());
  delay(3000);
  //  }
  //  else
  //  {
  //    Serial.print( "[FAILED] [ rc = " );
  //    Serial.print( client.state() );
  //    Serial.println( " : retrying in 5 seconds]" );
  //    // Wait 5 seconds before retrying
  //    delay( 5000 );
  //  }

}

String getMotor1()
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["params"] = gpioState[0] ? true : false;
  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.print("Get Motor status: ");
  Serial.println(strPayload);
  return strPayload;
}


String get_gpio_status()
{
  // Prepare gpios JSON payload string
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
 // data["params"] = gpioState[0] ? true : false;
  data["Illumination"] = "56.5";
  data["Temperature"] = "20.5";
  data["Humidity"] = "20.6%";
  data["CO2"] = "23.6";
  data["PH"] = "2.6";
  //data["temperature"] = String(dhtTem);
  //data["humi"] = String(dhtHum);
  //data[String(GPIO2_PIN)] = gpioState[1] ? true : false;
  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.print("Get gpio status: ");
  Serial.println(strPayload);
  return strPayload;
}

void setMotor1(int GPIO)
{
  toggle(GPIO);
}

void set_gpio_status(int pin, boolean enabled)
{
  if (pin == GPIO0)
  {
    // Output GPIOs state
    digitalWrite(GPIO0, enabled ? HIGH : LOW);
    // Update GPIOs state
    gpioState[0] = enabled;
  }
  else if (pin == GPIO2)
  {
    // Output GPIOs state
    digitalWrite(GPIO2, enabled ? HIGH : LOW);
    // Update GPIOs state
    gpioState[1] = enabled;
  }
}


void toggle(int GPIO)   //以灯的反转作为测试代码
{
  digitalWrite(GPIO, digitalRead(GPIO) ? LOW : HIGH);
  //  if ( HIGH == digitalRead(GPIO) )
  //  {
  //    digitalWrite(GPIO, LOW);
  //  }
  //  else
  //  {
  //    digitalWrite(GPIO, HIGH);
  //  }
}

void message(const char* topic, byte* payload, unsigned int length)
{
  char json[length + 1];
  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(json);

  // Decode JSON request
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject((char*)json);

  if (!data.success())
  {
    Serial.println("parseObject() failed");
    return;
  }
  String methodName = String((const char*)data["method"]);
  Serial.println(methodName);
  if (methodName.equals("getValue"))
  {
    // Reply with GPIO status
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status().c_str());
  }
  else if (methodName.equals("setValue"))
  {
    // Update GPIO status and reply
    set_gpio_status(GPIO4, data["params"]); //data["params"]["pin"], data["params"]["enabled"]
    Serial.println("params:" + String((const char*)data["params"]));
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status().c_str());
    client.publish(StateTopicAddr, get_gpio_status().c_str());
  }

}




