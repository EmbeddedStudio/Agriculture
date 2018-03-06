#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include<DHT.h>

#define WIFI_AP "s008-flying"
#define WIFI_PASSWORD "dianxin132"

#define LEDTOKEN "MO3ZWweCvculYL83cPm7"
#define DHTOKEN "wedsfsdfsdfddfeefedf"

#define LEDID "a6aa0e70-a5f6-11e7-8d02-353f63eeab61"
#define DHTID "8cb59020-a5f1-11e7-b8a4-353f63eeab61"

//#define  greenhouseID  "8e27efe0-eb81-11e7-8341-353f63eeab61"
//#define  greenhouseKEY "i8j8gFS00lwaNzSOxrEe"

#define GPIO0 0
#define GPIO2 4
#define DHTPIN 2

#define GPIO0_PIN 3
#define GPIO2_PIN 5

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//dht11 DHT11;
float dhtHum = 0.0;
float dhtTem = 0.0;

char  ServerAddr[] = "117.21.178.99";
char StateTopicAddr[] = "v1/devices/me/telemetry";
char ControllTopicAddr[] = "v1/devices/me/rpc/request/+";

WiFiClient wifiClient;
WiFiClient wifiClient2;
PubSubClient client(wifiClient);
PubSubClient client2(wifiClient2);

int status = WL_IDLE_STATUS;

// We assume that all GPIOs are LOW
boolean gpioState[] = {false, false};

void setup()
{
  Serial.begin(115200);
  // Set output mode for all GPIO pins
  pinMode(GPIO0, OUTPUT);
  pinMode(GPIO2, OUTPUT);
  delay(10);
  InitWiFi();
  dht.begin();
  client.setServer( ServerAddr, 1883 );
  client2.setServer( ServerAddr, 1883 );
  client.setCallback(on_message);

}

unsigned long lastSend = 0;
void loop() 
{
  if ( !client.connected() || !client2.connected()) 
  {
    reconnect();
  }
  client2.loop();
  client.loop();

  dht11Func();

}

void dht11Func() 
{
  if (lastSend == 0 || millis() - lastSend >= 3000) 
  {
    lastSend = millis();
    //   int chk = DHT11.read(DHTPIN);

    Serial.print("Read sensor: ");
    //    switch (chk)
    //    {
    //      case DHTLIB_OK:
    //        Serial.println("OK");
    //        break;
    //      case DHTLIB_ERROR_CHECKSUM:
    //        Serial.println("Checksum error");
    //        break;
    //      case DHTLIB_ERROR_TIMEOUT:
    //        Serial.println("Time out error");
    //        break;
    //      default:
    //        Serial.println("Unknown error");
    //        break;
    //    }
    dhtHum = dht.readHumidity();
    Serial.print("Humidity(%):");
    Serial.println(dhtHum);

    dhtTem = dht.readTemperature();
    Serial.print("Temperature(℃):");
    Serial.println(dhtTem);


    Serial.println("Sending current DHT11 status ...");
    //    if ( isnan(dhtHum) || isnan(dhtTem) )
    //    {
    //      Serial.println("Failed to read from DHT sensor!");
    //      return;
    //    }
    client2.publish(StateTopicAddr, get_dht11_status().c_str());
  }
}
// The callback for when a PUBLISH message is received from the server.
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
    set_gpio_status(GPIO2_PIN, data["params"]); //data["params"]["pin"], data["params"]["enabled"]
    Serial.println("params:" + String((const char*)data["params"]));
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status().c_str());
    client.publish(StateTopicAddr, get_gpio_status().c_str());
  }
}

String get_dht11_status() 
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["temperature"] = String(dhtTem);
  data["humi"] = String(dhtHum);
  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.print("Get dht11 status: ");
  Serial.println(strPayload);
  return strPayload;
}

String get_gpio_status() 
{
  // Prepare gpios JSON payload string
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["params"] = gpioState[0] ? true : false;
  data["temperature"] = String(dhtTem);
  data["humi"] = String(dhtHum);
  //data[String(GPIO2_PIN)] = gpioState[1] ? true : false;
  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.print("Get gpio status: ");
  Serial.println(strPayload);
  return strPayload;
}

void set_gpio_status(int pin, boolean enabled) 
{
  if (pin == GPIO0_PIN) 
  {
    // Output GPIOs state
    digitalWrite(GPIO0, enabled ? HIGH : LOW);
    // Update GPIOs state
    gpioState[0] = enabled;
  }
  else if (pin == GPIO2_PIN) 
  {
    // Output GPIOs state
    digitalWrite(GPIO2, enabled ? HIGH : LOW);
    // Update GPIOs state
    gpioState[1] = enabled;
  }
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());//WiFi.localIP()返回8266获得的ip地址
  Serial.println("mac address");
  Serial.println(WiFi.macAddress());
}


void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected() || !client2.connected()) 
  {
    status = WiFi.status();
    if ( status != WL_CONNECTED)
    {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to Server node ...");

    // Attempt to connect (clientId, username, password)
    if ( client.connect("a6aa0e70-a5f6-11e7-8d02-353f63eeab61", "MO3ZWweCvculYL83cPm7", NULL) && client2.connect(DHTID, DHTOKEN, NULL) ) 
    {
      Serial.println( "[DONE]" );
      // Subscribing to recec
      client.subscribe(ControllTopicAddr);
      Serial.println("Sending current GPIO status ...");
      client.publish(StateTopicAddr, get_gpio_status().c_str());
      client2.publish(StateTopicAddr, get_dht11_status().c_str());
    }
    else
    {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

