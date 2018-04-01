#include "stm32f10x.h"
#include "bsp_systick.h"
#include "bsp_usart.h"
#include "bsp_usart2.h"
#include "bsp_dht11.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h" 
#include <string.h>
#include "bsp_i2c.h"
#include "bsp_OLED.h"
#include "bsp_tsl2561.h"
#include "stm32f10x_it.h"
#include "codetab.h"
#include "bsp_spi.h"
#include "bsp_rc522.h"

extern u8 USart8266_temp[200];
extern u8 USART2_IT_Flag;
extern u8 Time_Flag;

void  Temperature_System(void);
void Network_System(void);
void Receave_System(void);
void System_Init(void);
void RFID ( void );

char DataStr [ 500 ] = { 0 };

u8 Temp_flag = 1 ;
u8 Show_flag = 0;
u8 Irrigation_flag=0;   //喷灌标志
u8 Exhaust_flag=0;      //排风标志
int main(void)
{
        
        System_Init();
        printf("[%s][%d]\r\n", __func__, __LINE__);
        
        while(1)
        {
                RFID();
                if(Temp_flag!=0)
                {
                        Temperature_System();   //调用恒温系统执行函数
                }
                
                if( Time_Flag!=0 )    
                {
                        Network_System();       //调用网络函数，把传感器的值传上去
                        Time_Flag=0;
                }
                
                /* 解析串口接收的字符串  */
                if( USART2_IT_Flag != 0 )
                {
                        Receave_System();
                        USART2_IT_Flag=0;
                }
        }
}


void Receave_System(void)
{
        char *p;
        char *name=NULL;
        char *status=NULL;
        
        
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
                //Exhaust_flag=1;
        }
        if(! (strcmp("setMotor1", name) || strcmp("false", status) ) ) 
        {
                //Water_OFF;  //添加需要的代码
                //Exhaust_flag=0;
        }
        if(! (strcmp("setHydrovalve1", name) || strcmp("true", status) ) ) 
        {
                Water_ON;  //添加需要的代码
                Irrigation_flag=1;
        }
        if(! (strcmp("setHydrovalve1", name) || strcmp("false", status) ) ) 
        {
                Water_OFF;   //添加需要的代码
                Irrigation_flag=0;
        }
        
        /*
         if()   //用户关闭升温系统
        {
                Temp_flag = 0 ;
        }
        if()   //用户开启升温系统
        {
                Temp_flag = 1 ;
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
}

void Network_System(void)
{
        /*装载 JSON 格式字符串*/
        DHT11_Data_TypeDef   DHT11_Data;
        float Illumination=0;
        char temp[10];
        char Humi[10];
        char lighting[10];
        
        Illumination=Read_Light();
        if( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
        {
                sprintf ( DataStr,"{\"Temperature\":\"%d.%d\",\"Humidity\":\"%d.%d\",\"CO2\":\"20.6\",\"PH\":\"5.6\",\"Illumination\":\"%0.2f\"}",\
                DHT11_Data.temp_int,DHT11_Data.temp_deci,DHT11_Data.humi_int,DHT11_Data.humi_deci,Illumination);
        }
        printf("%s\r\n",DataStr);
        Usart2_SendString(USART2,DataStr);
        if(Show_flag==0)
        {
                sprintf(temp,"%d.%d",DHT11_Data.temp_int,DHT11_Data.temp_deci);
                sprintf(Humi,"%d.%d",DHT11_Data.humi_int,DHT11_Data.humi_deci);
                sprintf(lighting,"%0.2f",Illumination);
                OLED_Fill(0x00);
                OLED_ShowCN(32,0,4,title);      //显示标题----智慧农业
                OLED_ShowCN(0,2,3,temp_code);   //显示温度
                OLED_ShowCN(0,4,3,Humi_code);   //显示湿度
                OLED_ShowCN(0,6,3,light);       //显示光强
                OLED_ShowStr(49,2,temp,2);
                OLED_ShowStr(49,4,Humi,2);
                OLED_ShowStr(49,6,lighting,2);
                Show_flag=~Show_flag;
        }
        else
        {
//                sprintf();
                OLED_Fill(0x00);
                OLED_ShowCN(32,0,4,title);      //显示标题----智慧农业
                OLED_ShowCN(0,2,2,Air_Temp);
                OLED_ShowCN(0,4,2,Irrigation);
                OLED_ShowCN(0,6,2,Exhaust);
                OLED_ShowCN(33,2,3,System);
                OLED_ShowCN(33,4,3,System);
                OLED_ShowCN(33,6,3,System);
                if(Temp_flag!=0)
                {
                        OLED_ShowCN(97,2,2,Open);
                }
                else
                {
                        OLED_ShowCN(97,2,2,Close);
                }
                if(Irrigation_flag!=0)
                {
                        OLED_ShowCN(97,4,2,Open);
                }
                else
                {
                        OLED_ShowCN(97,4,2,Close);
                }
                if(Exhaust_flag!=0)
                {
                        OLED_ShowCN(97,6,2,Open);
                }
                else
                {
                        OLED_ShowCN(97,6,2,Close);
                }
                Show_flag=~Show_flag;
        }
        
}

void  Temperature_System(void)
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
void System_Init(void)
{
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        systick_init();
        usart_init();
        DHT11_Init();
        USART_Config();
        LED_GPIO_Config();
        BASIC_TIM_Init();
        I2C_init();
        OLED_Init();
        TSL2561_Init();
        spi_Init();
        RC522_Init();
}



void RFID ( void )
{
        char cStr [ 30 ];
        u8 Array_ID [ 4 ];                  //先后存放IC卡的类型和UID(IC卡序列号)
        u8 Status,s_flag=0,f_flag=0;   //定义刷卡成功失败的标志位        //返回状态
        
        if ( ( Status = PcdRequest ( PICC_REQALL, Array_ID ) ) == MI_OK )                                    //寻卡
        {       
                s_flag=1;
                Status=PcdAnticoll(Array_ID);
                
        }
        if ( Status == MI_OK  )
        {
                f_flag=1;
               Status=MI_ERR;
                sprintf ( cStr, "The Card ID is: %02X%02X%02X%02X", Array_ID [ 0 ], Array_ID [ 1 ], Array_ID [ 2 ], Array_ID [ 3 ] );
                printf ( "%s\r\n",cStr );
//                OLED_Fill(0x00);
//                OLED_ShowCN(48,3,2,successful);
        }
        if(s_flag!=0)
        {
                if(f_flag!=0)
                {
                        OLED_Fill(0x00);
                        OLED_ShowCN(48,3,2,successful);
                }
                else
                {
                        OLED_Fill(0x00);
                        OLED_ShowCN(48,3,2,fail);
                }
                
                s_flag=0;
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
