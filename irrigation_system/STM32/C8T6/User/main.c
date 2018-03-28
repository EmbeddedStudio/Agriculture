#include "stm32f10x.h"
#include "bsp_systick.h"
#include "bsp_usart.h"
#include "bsp_usart2.h"
#include "bsp_dht11.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h" 
#include <string.h>

extern u8 USart8266_temp[200];
extern u8 USART2_IT_Flag;
extern u16 time ;

char DataStr [ 500 ] = { 0 };

u8 *empty;

int main(void)
{
        char *p;
        char *name=NULL;
        char *status=NULL;
        
        u8 Temp_flag = 1 ;
        
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        
        systick_init();
        usart_init();
        DHT11_Init();
        USART_Config();
        LED_GPIO_Config();
        BASIC_TIM_Init();
        
        DHT11_Data_TypeDef   DHT11_Data;
         printf("[%s][%d]", __func__, __LINE__);
        while(1)
        {
                
                if(Temp_flag!=0)
                {
                        if( Temperature > temp_max )
                        {
                                         //关闭太阳灯并开启水阀进行降温
                                TemDown_ON;
                        }
                        else if ( Temperature < ((temp_max+temp_min)/2) && Temperature > temp_min )
                        {
                                TemDown_OFF;
                        }
                        else if ( Temperature > ((temp_max+temp_min)/2) && Temperature < temp_max )
                        {
                                LED_OFF ;
                        }
                        else if ( Temperature < temp_min )
                        {
                                      //关闭水阀并开启太阳灯进行升温
                                LED_ON;
                        }
                }
                
                if( time >= 2500 )    
                {
                        time = 0 ;
                        
                        /*装载 JSON 格式字符串*/
                        if( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
                        {
                                sprintf ( DataStr,"{\"Temperature\":\"%d.%d\",\"Humidity\":\"%d.%d\",\"CO2\":\"20.6\",\"PH\":\"5.6\",\"Illumination\":\"6.5\"}",\
                                DHT11_Data.temp_int,DHT11_Data.temp_deci,DHT11_Data.humi_int,DHT11_Data.humi_deci);
                        }
                        printf("%s\r\n",DataStr);
                        
                        Usart2_SendString(USART2,DataStr);
                }
                
                /* 解析串口接收的字符串  */
                if( USART2_IT_Flag != 0 )
                {
                        printf("receave %s",USart8266_temp);
                        p = strtok((char*)USart8266_temp, "_");
                        if(p)
                        {
                                name = p;
                        }
                        p = strtok(NULL, "_");
                        if(p)
                        {
                                status = p;
                        }
                        if(! (strcmp("setMotor1", name) || strcmp("true", status) ) ) 
                        {
                                //LED1_ON;   //添加需要的代码
                        }
                        if(! (strcmp("setMotor1", name) || strcmp("false", status) ) ) 
                        {
                                //LED1_OFF;  //添加需要的代码
                        }
                        if(! (strcmp("setHydrovalve1", name) || strcmp("true", status) ) ) 
                        {
                                //LED2_ON;  //添加需要的代码
                        }
                        if(! (strcmp("setHydrovalve1", name) || strcmp("false", status) ) ) 
                        {
                                //LED2_OFF;   //添加需要的代码
                        }
                        
                        /*
                         if()   //用户关闭升温系统
                        {
                                Temp_flag = 0 ;
                        }
                        if()   //用户开启升温系统
                        {
                                Temp_flag = 0 ;
                        }
                        if()   //接收来自云端设置的阈值
                        {
                                temp_max =  ;
                        }
                        if()    //接收来自云端设置的阈值
                        {
                                temp_min =  ;
                        }
                        
                        */
                        USART2_IT_Flag=0;
                }
        }
}

/*  json 格式封装形式  */
//{"Temperature":"23.00","Humidity":"59.00","CO2":"21.6","PH":"4.6","Illumination":"6.5"}
//Message: {"method":"setMotor1","params":false}
//Message: {"method":"setCarPos","params":"73"}
//Message: {"method":"setHydrovalve1","params":false}
//                printf("[%s][%d]", __func__, __LINE__);
//setMotor1_true
//setHydrovalve1_false
