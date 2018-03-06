#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include<DHT.h>
#define DHTTYPE DHT11
#define WIFI_AP "s008-flying"
#define WIFI_PASSWORD "dianxin132"

#define LEDTOKEN "MO3ZWweCvculYL83cPm7"
#define DHTOKEN "wedsfsdfsdfddfeefedf"

#define LEDID "a6aa0e70-a5f6-11e7-8d02-353f63eeab61"
#define DHTID "8cb59020-a5f1-11e7-b8a4-353f63eeab61"
#define DHTPIN 4
DHT dht(DHTPIN, DHTTYPE);
float dhtHum = 0.0; //温度
float dhtTem = 0.0;//湿度

char  ServerAddr[] = "117.21.178.99";
char StateTopicAddr[] = "v1/devices/me/telemetry";
char ControllTopicAddr[] = "v1/devices/me/rpc/request/+";


WiFiClient wifiClient;

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  InitWiFi();

  client.setServer( ServerAddr, 1883 );

  client.setCallback(on_message);
  dht.begin();
}

void loop()
{
  // put your main code here, to run repeatedly:
  if ( !client.connected() ) 
  {
    reconnect();
  }

  client.loop();

  dht11Func();
}

unsigned long lastSend = 0;
void dht11Func() 
{
  if (lastSend == 0 || millis() - lastSend >= 3000) 
  {
    lastSend = millis();
    //    int chk = DHT11.read(DHTPIN);
    //    Serial.print("Read sensor: ");
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
    //    dhtHum = (float)DHT11.humidity;
    //    Serial.print("Humidity(%):");
    //    Serial.println(dhtHum);
    //
    //    dhtTem = (float)DHT11.temperature;
    //    Serial.print("Temperature(℃):");
    //    Serial.println(dhtTem);
    //
    //    Serial.println("Sending current DHT11 status ...");
    dhtHum = dht.readHumidity();
    dhtTem = dht.readTemperature();
    //  float f = dht.readTemperature(true);
    if ( isnan(dhtHum) || isnan(dhtTem) ) 
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.print("Humidity: ");
    Serial.print(dhtHum);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(dhtTem);
    Serial.print(" *C ");
    client.publish(StateTopicAddr, get_dht11_status().c_str());


  }
}
void on_message(const char* topic, byte* payload, unsigned int length) 
{
  Serial.println("On_message......");
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
}
void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
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
    if ( client.connect(DHTID, DHTOKEN, NULL) )
    {
      Serial.println( "[DONE]" );
      // Subscribing to recec
      //client.subscribe(ControllTopicAddr);
      Serial.println("Sending current DHT11 status ...");
      client.publish(StateTopicAddr, get_dht11_status().c_str());
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
String get_dht11_status() 
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject & data = jsonBuffer.createObject();
  data["temperature"] = String(dhtTem);
  data["humi"] = String(dhtHum);
  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.print("Get dht11 status: ");
  Serial.println(strPayload);
  return strPayload;
}
