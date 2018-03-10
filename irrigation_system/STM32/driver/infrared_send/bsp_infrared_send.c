#include "bsp_infrared_send.h"

/* 生成红外协议的起始信号 */
static void Start_signal(void)
{
        /* 使能38kHz矩形波，使接收端电平拉低
           赋值 15 的值是根据自动重装载值确定的，
           大概为2/3，使输出38kHz波形 */
        TIM_x->TIM_CCR_x = 15;
        delay_ms(5);
        /* 失能矩形波，使接收端电平拉高 */
        TIM_x->TIM_CCR_x = 0;
        delay_ms(5);
}

/* 生成红外协议总定义为 1 的波形 */
static void Creat_bit_1(void)
{
        TIM_x->TIM_CCR_x = 15;
        delay_us(500);
        TIM_x->TIM_CCR_x = 0;
        delay_us(1500);
}

/* 生成红外协议总定义为 0 的波形 */
static void Creat_bit_0(void)
{
        TIM_x->TIM_CCR_x = 15;
        delay_us(500);
        TIM_x->TIM_CCR_x = 0;
        delay_us(500);
}

/* 生成红外协议的结束信号 */
static void End_signal(void)
{
        TIM_x->TIM_CCR_x = 15;
        delay_us(500);       
        TIM_x->TIM_CCR_x = 0;
        delay_ms(5);
}


/* 编码生成波形函数 */
void Send_infrared_code(const u8 * code, u8 code_len)
{
        char i = 0, j = 0, k = 0;
        const u8 * p = code;
        /* 执行一次，发送两次编码 */
        for(k = 0; k < 2; k++)
        {
                Start_signal();
                /* 循环编码数组的字节数 */
                for(j = 0; j < code_len; j++)
                {
                        /* 循环发送数组中每个字节的每一个bit */
                        for(i = 0; i < 8; i++)
                        {
                                if((*p)&(0x80>>i))
                                       Creat_bit_1(); 
                                else
                                       Creat_bit_0();
                        }
                        p++;
                }
                End_signal();
                p = code;
                
        }        
}








