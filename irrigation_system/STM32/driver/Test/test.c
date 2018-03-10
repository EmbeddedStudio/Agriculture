#include "test.h"
#include "bsp_esp8266.h"
#include "bsp_SysTick.h"
#include <stdio.h>  
#include <string.h>  
#include <stdbool.h>
#include "bsp_dht11.h"
#include <string.h>
#include "bsp_led.h"

volatile uint8_t ucTcpClosedFlag = 0;

char cStr [ 1500 ] = { 0 };
char dStr [ 1500 ] = { 0 };
extern u8 order;
extern char tem;





/**
  * @brief  ESP8266 （Sta Tcp Client）透传
  * @param  无
  * @retval 无
  */
void ESP8266_StaTcpClient_UnvarnishTest ( void )
{
        uint8_t ucStatus;
        
        DHT11_Data_TypeDef   DHT11_Data;
        
        
        printf ( "\r\n正在配置 ESP8266 ......\r\n" );

        macESP8266_CH_ENABLE();  //使能模块的配置
        printf("[%s][%d]", __func__, __LINE__);
        ESP8266_AT_Test ();     //测试AT指令
        printf("[%s][%d]", __func__, __LINE__);
        ESP8266_Net_Mode_Choose ( STA );  //选择工作模式
        //连接WiFi     WiFi名和密码
        while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) ); 
        printf("WiFi已连接");
        //不配置多连接
        ESP8266_Enable_MultipleId ( DISABLE );
        //连接服务器       服务器协议     服务器IP     服务器端口     模块IP
        while ( ! ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
        printf("服务器已连接");
        
        ESP8266_Cmd("AT+CIFSR","OK",NULL,500);

        //服务器透传
        while ( ! ESP8266_UnvarnishSend () );

        printf ( "\r\n配置 ESP8266 完毕\r\n" );


        while ( 1 )
        {       
                char Str[]="{\"M\":\"checkin\",\"ID\":\"3115\",\"K\":\"52e38a188\"}\n";
                sprintf ( cStr,"{\"M\":\"update\",\"ID\":\"3115\",\"V\":{\"2948\":\"%d.%d\"}}\n",DHT11_Data.temp_int,DHT11_Data.temp_deci);
                sprintf ( dStr,"{\"M\":\"update\",\"ID\":\"3115\",\"V\":{\"2950\":\"%d.%d\"}}\n",DHT11_Data.humi_int,DHT11_Data.humi_deci);
                
                
                ESP8266_SendString ( ENABLE, Str, strlen(Str), Single_ID_0 );               //发送数据
                ESP8266_SendString ( ENABLE, cStr, strlen(cStr),Single_ID_0 );
                ESP8266_SendString ( ENABLE, dStr, strlen(cStr),Single_ID_0 );
                
                /*调用DHT11_Read_TempAndHumidity读取温湿度，若成功则输出该信息*/
                if( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
                {
                        printf("\r\n读取DHT11成功!\r\n\r\n湿度为%d.%d ％RH ，温度为 %d.%d℃ \r\n",\
                        DHT11_Data.humi_int,DHT11_Data.humi_deci,DHT11_Data.temp_int,DHT11_Data.temp_deci);
                }
                
                else
                {
                        printf("Read DHT11 ERROR!\r\n");
                }
                
                
                switch(order)
                {
                        case 1 :   
                                printf("灯翻转\r\n");
                                LED2_TOGGLE;
                                break;
                }
                
                
                
                delay_ms ( 1000 );
                
                
                
                if ( ucTcpClosedFlag )              //检测是否失去连接
                {
                        ESP8266_ExitUnvarnishSend ();            //退出透传模式
                        
                        do ucStatus = ESP8266_Get_LinkStatus ();     //获取连接状态
                        while ( ! ucStatus );
                        
                        if ( ucStatus == 4 )                 //确认失去连接后重连
                        {
                                printf ( "\r\n正在重连热点和服务器 ......\r\n" );
                                
                                while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );
                                
                                while ( ! ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
                                
                                printf ( "\r\n重连热点和服务器成功\r\n" );

                        }
                        
                        while ( ! ESP8266_UnvarnishSend () );
                        
                }
        }


}


