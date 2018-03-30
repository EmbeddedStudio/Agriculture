#include "bsp_usart.h"

/**
 * USART_RX_STA: 软件虚拟的寄存器, 用于控制字节流的接收
 * USART_RX_STA[15]: 接收完成标志
 * USART_RX_STA[14]: 接收到0x0d
 * USART_RX_STA[13:0]: 接收到的有效字节数目
 */
u16 USART_RX_STA = 0;

/* 接收缓冲数组, 最大接收USART_REC_LEN个字节 */
u8 USART_RX_BUF[USART_REC_LEN];

/**
 * @brief 初始化串口
 */
void usart_init(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;

        /* 第一步: 使能外设时钟 */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

        /* 第二步: 配置串口对应的GPIO, 设置为复用推挽输出 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        /* 第三步: 初始化设置 */
        USART_InitStructure.USART_BaudRate = 115200;                    // 根据宏设置波特率
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;     // 设置传输字长为8位
        USART_InitStructure.USART_StopBits = USART_StopBits_1;          // 设置一位停止位
        USART_InitStructure.USART_Parity = USART_Parity_No;             // 设置不进行数据校验
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 设置无硬件数据流控制
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 设置为收发模式
        USART_Init(USART1, &USART_InitStructure);                       // 初始化串口

        /* 第四步: 配置串口中断 */
        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        /* 第五步: 开启串口中断 */
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

        /* 第六步: 使能串口 */
        USART_Cmd(USART1, ENABLE);
}

/*****************  发送一个字符 **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
        /* 发送一个字节数据到USART */
        USART_SendData(USART1,ch);
                
        /* 等待发送数据寄存器为空 */
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	
}

/*****************  发送字符串 **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
        unsigned int k=0;
        do 
        {
        Usart_SendByte( USART1, *(str + k) );
        k++;
        } while(*(str + k)!='\0');

        /* 等待发送完成 */
        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET)
        {}
}

/**
 * @brief 串口中断服务函数, 每有接收一个字节, 申请一次中断, 中断函数里面接收, 直到接收到换行停止接收
 */
void USART1_IRQHandler(void)
{
        /* 用于保存本次串口接收到的字节 */
        u8 Res;

        /* 判断是不是接收中断, 如果是接收中断, 执行 if 里面的代码 */
        if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
        {
                /* 读取串口1接收到的数据 */
                Res = USART_ReceiveData(USART1);

                /* 接收未完成, 这里接收的是一大串字节 */
                if ((USART_RX_STA & 0x8000) == 0)
                {
                        /* 判断前一个接收的是不是 0x0d */
                        if (USART_RX_STA & 0x4000)
                        {
                                /* 如果前一个字节收到0x0d的话, 再接收一个字节 */
                                if (Res != 0x0a)
                                {
                                        /* 如果在接收的不是到0x0a, 接收错误, 重新开始 */
                                        USART_RX_STA = 0;
                                }
                                else
                                {
                                        /* 如果再接收到0x0a, 就把USART_RX_STA中接收完成标志置位, 表示接收完成了 */
                                        USART_RX_STA |= 0x8000;
                                }
                        }
                        else
                        {
                                /* 如果前一个字节还没收到0x0d的话, 就接着接收下一个字节 */
                                if (Res == 0x0d)
                                {
                                        /* 如果当前接收到的是0x0d的话 */
                                        /* 则置位接收到0x0d的标志位 USART_RX_STA[14] */
                                        USART_RX_STA |= 0x4000;
                                }
                                else
                                {
                                        /* 将当前接收到的值存入缓存数组中 */
                                        USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;

                                        /* 接收到的字节个数加一 */
                                        USART_RX_STA++;

                                        /* 判断接收到的数据有没有超过最开始设置的缓存区长度 */
                                        if (USART_RX_STA > (USART_REC_LEN - 1))
                                        {
                                                /* 如果超出范围, 表明接收数据错误, 重新开始接收 */
                                                USART_RX_STA = 0;
                                        }
                                }
                        }
                }
        }
}

/* 加入以下代码, 支持printf函数, 而不需要选择 use MicroLIB */
#pragma import(__use_no_semihosting)

/* 标准库需要的支持函数 */
struct __FILE
{
        int handle;
};
FILE __stdout;

/**
 * @brief 定义 _sys_exit() 以避免使用半主机模式
 */
void _sys_exit(int x)
{
        x = x;
}

/**
 * @brief 重定义 fputc 函数
 */
int fputc(int ch, FILE *f)
{
        /* 循环发送, 直到发送完毕 */
        while ((USART1->SR & 0x40) == 0)
        {

        }

        /* 装载要发送的数据 */
        USART1->DR = (u8) ch;

        return ch;
}
