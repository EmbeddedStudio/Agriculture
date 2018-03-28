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
                if( time >= 2500)    
                {
                        time = 0 ;
                        if( DHT11_Read_TempAndHumidity ( & DHT11_Data ) == SUCCESS)
                        {
                                sprintf ( DataStr,"{\"Temperature\":\"%d.%d\",\"Humidity\":\"%d.%d\",\"CO2\":\"21.6\",\"PH\":\"4.6\",\"Illumination\":\"6.5\"}",\
                                DHT11_Data.temp_int,DHT11_Data.temp_deci,DHT11_Data.humi_int,DHT11_Data.humi_deci);
                        }
                        printf("%s\r\n",DataStr);
                        
                        Usart2_SendString(USART2,DataStr);
                }
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
                                LED1_ON;
                        }
                        if(! (strcmp("setMotor1", name) || strcmp("false", status) ) ) 
                        {
                                LED1_OFF;
                        }
                        if(! (strcmp("setHydrovalve1", name) || strcmp("true", status) ) ) 
                        {
                                LED2_ON;
                        }
                        if(! (strcmp("setHydrovalve1", name) || strcmp("false", status) ) ) 
                        {
                                LED2_OFF;
                        }
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
