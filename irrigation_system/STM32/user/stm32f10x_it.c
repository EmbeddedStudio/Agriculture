#include "stm32f10x_it.h"
#include "bsp_usart.h"
#include "bsp_usart2.h"
#include <string.h>
#include "bsp_led.h"
#include "bsp_TiMbase.h" 

char USart8266_temp[200];

static u8 count = 0;

u8 USART2_IT_Flag=0;
u16 time=0;

void USART2_IRQHandler ( void )
{ 
        if ( USART_GetITStatus ( USART2, USART_IT_RXNE ) != RESET )
        {
                USart8266_temp[count]  = USART_ReceiveData( USART2 );
                count++;
                if(USART_ReceiveData( USART2 ) == '*')
                {
                        count = 0;
                        USART2_IT_Flag=1;
                }
        }
}

void  BASIC_TIM_IRQHandler (void)
{
        if ( TIM_GetITStatus( BASIC_TIM, TIM_IT_Update) != RESET ) 
        { 
                time++;
                TIM_ClearITPendingBit(BASIC_TIM , TIM_FLAG_Update); 
        } 
}










/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
        printf("[%s][%d]", __func__, __LINE__);
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
        /* Go to infinite loop when Hard Fault exception occurs */
        printf("[%s][%d]", __func__, __LINE__);
        
        while (1)
        {
          
        }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
        /* Go to infinite loop when Memory Manage exception occurs */
        
        printf("[%s][%d]", __func__, __LINE__);
        
        while (1)
        {
                
        }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
        /* Go to infinite loop when Bus Fault exception occurs */
        
        printf("[%s][%d]", __func__, __LINE__);
        
        while (1)
        {
                
        }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
        /* Go to infinite loop when Usage Fault exception occurs */
        
        printf("[%s][%d]", __func__, __LINE__);
        
        while (1)
        {
                
        }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
        printf("[%s][%d]", __func__, __LINE__);
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
        printf("[%s][%d]", __func__, __LINE__);
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
        printf("[%s][%d]", __func__, __LINE__);
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
        
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
