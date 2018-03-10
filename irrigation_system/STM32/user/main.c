#include "stm32f10x.h"
#include "bsp_systick.h"
#include "bsp_usart.h"
#include "bsp_usart2.h"
#include "bsp_dht11.h"


extern u8 USart8266_temp[200];

char cStr [ 1500 ] = { 0 };
char dStr [ 1500 ] = { 0 };
int main(void)
{
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        
        systick_init();
        usart_init();
        DHT11_Init();
        DHT11_Data_TypeDef   DHT11_Data;
        USART_Config();
        while(1)
        {
                  if( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
                {
                        printf("\r\nHumidity%d.%d,Temperature%d.%d\r\n",\
                        DHT11_Data.humi_int,DHT11_Data.humi_deci,DHT11_Data.temp_int,DHT11_Data.temp_deci);
                }
                
                sprintf ( cStr,"{\"Temperature\":\"%d.%d\",\"Humidity\":\"%d.%d\",\"CO2\":\"21.6\",\"PH\":\"4.6\",\"Illumination\":\"6.5\"}",DHT11_Data.temp_int,DHT11_Data.temp_deci,DHT11_Data.humi_int,DHT11_Data.humi_deci);
                
                printf("%s",cStr);
                
                Usart2_SendString(USART2,cStr);
                
                delay_ms(2500);
                
                //                printf("[%s][%d]", __func__, __LINE__);
                
        }
}
/*  json 格式封装形式  */
//{"Temperature":"23.00","Humidity":"59.00","CO2":"21.6","PH":"4.6","Illumination":"6.5"}
//Message: {"method":"setMotor1","params":false}
//Message: {"method":"setCarPos","params":"73"}
//Message: {"method":"setHydrovalve1","params":false}


