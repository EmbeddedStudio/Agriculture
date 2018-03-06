#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include<DHT.h>

char ssid[] = "s008-flying";
char pass[] = "dianxin132";

#define  greenhouseID  "8e27efe0-eb81-11e7-8341-353f63eeab61"
#define  greenhouseKEY "i8j8gFS00lwaNzSOxrEe"

#define HydrovalvePin 0   //水阀开关引脚   D3
#define DHTPIN        2   //DHT输入引脚   D4
#define MotorPin      4   //风扇开关引脚   D2
#define analogPin     10  //模拟输出测试引脚  SD3

//unsigned int httpPort = 8181;     //贝壳物联服务器地址
//const char *host = "121.42.180.30";      //贝壳物联服务器地址

char  ServerAddr[] = "117.21.178.99";    //MQTT服务器地址
char StateTopicAddr[] = "v1/devices/me/telemetry";          //client.publish(StateTopicAddr, get_gpio_status().c_str());上传地址
char ControllTopicAddr[] = "v1/devices/me/rpc/request/+";   //client.subscribe(ControllTopicAddr);连接下发地址

WiFiClient wifiClient;
PubSubClient client(wifiClient);

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float dhtHum = 0.0;
float dhtTem = 0.0;  //温湿度

float Value ;   //存储云端下发的阀值

/*
   上传json关键字
   Temperature   //温度
   Humidity　　　//湿度　
   CO2　　        //二氧化碳
   PH            //PH值
   Illumination　//光照

   下发json关键字
   水阀控制
   getHydrovalvePin1
   setHydrovalvePin1
   电机控制
   getMotorPin1
   setMotorPin1
   小车位置控制
   getCarPos
   setCarPos
*/


int status = WL_IDLE_STATUS;

boolean gpioState[] = {false, false, false, false, false};

long last_time = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(HydrovalvePin, OUTPUT);
  pinMode(MotorPin, OUTPUT);
  pinMode(analogPin, OUTPUT);
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)//WiFi.status() ，这个函数是wifi连接状态，返回wifi链接状态
  {
    delay(500);
    Serial.print(".");
  }//如果没有连通向串口发送.....
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());//WiFi.localIP()返回8266获得的ip地址
  Serial.println("mac address");
  Serial.println(WiFi.macAddress());
  client.setServer( ServerAddr, 1883 );//链接服务器及端口
  client.setCallback(on_message);
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
      Serial.println("Connecting to Server node ...");
      Serial.println( "[DONE]" );
      client.connect( greenhouseID, greenhouseKEY , NULL);
    }
    client.subscribe(ControllTopicAddr);//连接下发的地址
  }

  if (last_time == 0 || millis() - last_time >= 2000)
  {
    last_time = millis();
    Serial.println("Sending current GPIO status ...");
    dht11Func();
    client.publish(StateTopicAddr, Data().c_str());
  }
//  if(dhtTem>Value)
//  {
//    digitalWrite(HydrovalvePin,HIGH);
//    gpioState[HydrovalvePin]=true;
//  }
//  else
//  {
//    digitalWrite(HydrovalvePin,LOW);
//    gpioState[HydrovalvePin]=false;
//  }
  
  client.loop();
}

void dht11Func()
{
  Serial.print("dht11Func ");
  dhtHum = dht.readHumidity();
  dhtTem = dht.readTemperature();
}


String Data()
{
  // Prepare gpios JSON payload string
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  //加json的关键字和需要传送的数据
  
  data["Temperature"] = String(dhtTem);
  data["Humidity"] = String(dhtHum);
  data["CO2"] = "21.6";
  data["PH"] = "4.6";
  data["Illumination"] = "6.5";

  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.print("Get Data: ");
  Serial.println(strPayload);
  return strPayload;
}

String get_gpio_status( int PIN )
{
  // Prepare gpios JSON payload string
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  //获取gpio的状态
  data["params"] = gpioState[PIN] ? true : false;

  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.print("Get gpio status: ");
  Serial.println(strPayload);
  return strPayload;
}

void set_gpio_status(int PIN, boolean enabled)
{
  if (PIN == HydrovalvePin)
  {
    // Output GPIOs state
    digitalWrite(HydrovalvePin, enabled ? HIGH : LOW);
    // Update GPIOs state
    gpioState[HydrovalvePin] = enabled;
  }
  else if (PIN == MotorPin)
  {
    // Output GPIOs state
    digitalWrite(MotorPin, enabled ? HIGH : LOW);
    //Update GPIOs state
    gpioState[MotorPin] = enabled;
  }
  else if (PIN == analogPin)
  {
    // Output GPIOs state
    analogWrite(analogPin, int(float(enabled) * 2.55));
    // Update GPIOs state
    //gpioState[analogPin] = enabled;
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

void on_message(const char* topic, byte* payload, unsigned int length)
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

  // Check request method
  String methodName = String((const char*)data["method"]);
  Serial.println(methodName);
  if (methodName.equals("getMotor1"))
  {

    // Reply with GPIO status
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status(MotorPin).c_str());
  }
  else if (methodName.equals("setMotor1"))
  {
    // Update GPIO status and reply
    set_gpio_status(MotorPin, data["params"]); //data["params"]["pin"], data["params"]["enabled"]
    Serial.println("params:" + String((const char*)data["params"]));
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status(MotorPin).c_str());
    client.publish(StateTopicAddr, get_gpio_status(MotorPin).c_str());
  }

  else if (methodName.equals("getHydrovalve1"))
  {

    // Reply with GPIO status
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status(HydrovalvePin).c_str());
  }
  else if (methodName.equals("setHydrovalve1"))
  {

    // Update GPIO status and reply
    set_gpio_status(HydrovalvePin, data["params"]); //data["params"]["pin"], data["params"]["enabled"]
    Serial.println("params:" + String((const char*)data["params"]));
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status(HydrovalvePin).c_str());
    client.publish(StateTopicAddr, get_gpio_status(HydrovalvePin).c_str());
  }

  else if (methodName.equals("getCarPos"))
  {
    // Reply with GPIO status
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    //client.publish(responseTopic.c_str(), get_gpio_status(analogPin).c_str());
  }
  else if (methodName.equals("setCarPos"))
  {
    // Update GPIO status and reply
    set_gpio_status(analogPin, data["params"]); //data["params"]["pin"], data["params"]["enabled"]
    Value=float(data["params"]);
    Serial.println("Value=data");
    Serial.println("params:" + String((const char*)data["params"]));
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    //client.publish(responseTopic.c_str(), get_gpio_status(analogPin).c_str());
    //client.publish(StateTopicAddr, get_gpio_status(analogPin).c_str());
  }
}


