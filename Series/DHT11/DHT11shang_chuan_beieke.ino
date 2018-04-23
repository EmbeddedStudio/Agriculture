/////////////////////////
// Written by Wang Kai  //
//     2018-3-18       //
/////////////////////////

// arduino快捷键
// ctrl+T  制动化归档代码
// ctrl+R 编译
// ctrl+U 上传

#include <ESP8266WiFi.h>
#include <dht11.h>
dht11 DHT;
#define DHT11_PIN 2
int chk;

// 以下4行位配置信息,需要修改,具体跟路由器,服务器有关
const char *ssid     = "s008-flying";//这里是我的wifi，你使用时修改为你要连接的wifi ssid
const char *password = "dianxin132";//你要连接的wifi密码
const char *host = "121.42.180.30";//修改为Server服务端的IP地址，见https://www.bigiot.net/help/1.html
const int tcpPort = 8181;//修改为Server服务端的端口号


WiFiClient client; // 实例化客户端对象

void setup() //  串口的初始化和连上WiFi的基本思路如下
{
  Serial.begin(115200); // 初始化串口,设置波特率为115200
  delay(10); // 延迟10ms,串口开启需要一点时间
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); // 连接WiFi
 
  Serial.println("DHT TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");

  // 直道成功连上WiFi为止跳出while
  while (WiFi.status() != WL_CONNECTED)//WiFi.status() ，这个函数是wifi连接状态，返回wifi链接状态
  {
    delay(500); // 每5ms发起一次连接WiFi请求
    Serial.print(".");//如果没有连通向串口发送.....
  }

  Serial.println(""); // 换行作用。println()相当于print('\n')
  Serial.println("WiFi connected"); //串口打印, 连接WiFi成功
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());//WiFi.localIP()返回8266获得的ip地址,由路由器分配,以下几乎不会使用,只做显示
}


void loop()
{
  while (!client.connected())//几个非连接的异常处理,当未连接上服务器,或者异常掉线,贝壳40秒未上传数据会掉线,所以当掉线需要在代码中自动连上
  {
    if (!client.connect(host, tcpPort)) // 连接服务器,并判断是否连接成功
    {
      Serial.print("disconnect"); // 连接服务器失败
      delay(500); // 等待5ms后继续连接
    } else { // 连接成功
      Serial.print("chickin");
      // client.print()为客户端对象向服务器发送数据
      // 具体数据内容及格式与服务器有关,贝壳为json,具体键值对见以上网址
      // 这里发送的内容为登录自己在服务器创建的"智能设备",每次创建会分配唯一的ID和APIKEY,告诉服务器你将要说话的"对象",毕竟一个账号可以创建多个"对象"
      // ID、K对应的值需要改为自己的
      client.print("{\"M\":\"checkin\",\"ID\":\"5055\",\"K\":\"227b7792e\"}\n"); // '\n' 一般为网络传输的结束符,"\r\n"也可以
      delay(100); // 发送数据的等待时间,不要太长,也不要太短,可以修改,有比没有好

     
  }
  // 程序能运行到这里,说明成功连上服务器和对象

  while (client.available())//，无线读取到的数据,注意这里是client,功能与Serial.available()相同
  {
    String s = client.readStringUntil('\n'); //将服务器的数据读到s中,readStringUntil()执行后,会自动清除client数据通道中的数据,以跳出while
   Serial.print(s);
    delay(10);
  }




 
  //switch (chk){
    //case DHTLIB_OK:  
                //Serial.print("OK,\t"); 
               // break;
   // case DHTLIB_ERROR_CHECKSUM: 
             //   Serial.print("Checksum error,\t"); 
              //  break;
  //  case DHTLIB_ERROR_TIMEOUT: 
            //    Serial.print("Time out error,\t"); 
             //   break;
  // default: 
             //   Serial.print("Unknown error,\t"); 
             //   break;
 // }
 // DISPLAT DATA

  
    }

  
  // 当串口中有数据,可以在串口输入框内输入,在此输入的数据不用转义,但需要在末尾加'\n'
  // 贝壳示例:{"M":"update","ID":"xx1","V":{"id1":"value1",...}}\n,修改后就可以在串口输入框内输入,然后网址会有你上传的数据,调试成功后可以定时上传传感器的数据
  // 具体格式见以上网址

    
  Serial.print("DHT11, \t");
  chk = DHT.read(DHT11_PIN);    // READ DATA
    Serial.print(DHT.humidity,1);
  Serial.print(",\t");
  Serial.println(DHT.temperature,1);

 //void update2(String inputid1, int value1, String inputid2, int value2){
  client.print("{\"M\":\"update\",\"ID\":\"");
  client.print(5055);
  client.print("\",\"V\":{\"");
  client.print(4533);
  client.print("\":\"");
  client.print(DHT.humidity);
    delay(100);
  client.print("\",\"");
  client.print(4563);
  client.print("\":\"");
  client.print(DHT.temperature);
  client.println("\"}}");
  //lastCheckInTime = millis();
 // lastUpdateTime= millis();
 // Serial.print("update:");   
  //Serial.print(4533);   
  //Serial.print("->"); 
 // Serial.print(DHT.humidity);  
 // Serial.print("："); 
 // Serial.print(4563);   
 // Serial.print("->"); 
  //Serial.println(DHT.temperature);   
Serial.println(millis()); 


  delay(1000);
  if (Serial.available())
  {
    size_t counti = Serial.available(); // 读取串口通道数据中的数据长度
    uint8_t sbuf[counti]; // 创建改长度的数据,注意:arduino可以动态创建数组的长度,和标准C不同
    Serial.readBytes(sbuf, counti); // 将串口通道中counti长度的数据以字节形式读入sbuf
    client.write(sbuf, counti); // 将sbuf发送到服务器,注意:这里是write,write为发送字节数据,print为发送字符数据,不能互换,write速度快,print用法简单
    delay(100);
  }

}

