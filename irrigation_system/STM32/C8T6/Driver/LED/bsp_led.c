#include "bsp_led.h"

void LED_GPIO_Config(void)
{       
        /*定义一个GPIO_InitTypeDef类型的结构体*/
        GPIO_InitTypeDef GPIO_InitStructure;

        /*开启LED相关的GPIO外设时钟*/
        RCC_APB2PeriphClockCmd( Water_GPIO_CLK | TemDown_GPIO_CLK | LED_GPIO_CLK, ENABLE);
        /*选择要控制的GPIO引脚*/
        GPIO_InitStructure.GPIO_Pin = Water_GPIO_PIN | TemDown_GPIO_PIN | LED_GPIO_PIN; 

        /*设置引脚模式为通用推挽输出*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

        /*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

        /*调用库函数，初始化GPIO*/
        GPIO_Init(Water_GPIO_PORT , &GPIO_InitStructure); 
        GPIO_Init(TemDown_GPIO_PORT , &GPIO_InitStructure);
        GPIO_Init(LED_GPIO_PORT , &GPIO_InitStructure);
        
        
        /*初始默认配置为0*/
        GPIO_ResetBits(Water_GPIO_PORT, Water_GPIO_PIN);
        GPIO_ResetBits(TemDown_GPIO_PORT, TemDown_GPIO_PIN); 
        GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN);
}
