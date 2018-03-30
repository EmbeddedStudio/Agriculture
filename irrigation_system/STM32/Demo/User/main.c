#include "stm32f10x.h"
#include "bsp_systick.h"
#include "bsp_usart.h"

int main(void)
{
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        
        systick_init();
        usart_init();
        
        while(1)
        {
                printf("[%s][%d]", __func__, __LINE__);
                delay_ms(1000);
        }
}
