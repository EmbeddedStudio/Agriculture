#include "bsp_i2c.h"
#include "stm32f10x.h"
#include "bsp_usart.h"

/**
 * @brief I2C初始化函数
 * @param void
 * @note 设置 SCL 和 SDA 的 GPIO 模式
 */
void I2C_init (void)
{
        GPIO_InitTypeDef        GPIO_Initstuct;
        /* 开启 SCL 和 SDA 的时钟 */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 
        
        /* 配置 SCL 和 SDA 初始为输出模式 */
        GPIO_Initstuct.GPIO_Pin = I2C_SCL_Pin|I2C_SDA_Pin;
        GPIO_Initstuct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Initstuct.GPIO_Mode = GPIO_Mode_Out_OD;    //开漏输出
        GPIO_Init(I2C_PORT,&GPIO_Initstuct);
        SCL=1;
        SDA=1;
}
/**
 * @brief 设置 SDA 对应 GPIO 为输入模式
 */
void I2C_set_sda_input(void)
{
        GPIO_InitTypeDef        GPIO_Initstuct;
        GPIO_Initstuct.GPIO_Pin = I2C_SDA_Pin;
        GPIO_Initstuct.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(I2C_PORT,&GPIO_Initstuct);
}

/**
 * @brief 设置 SDA 对应 GPIO 为输出模式
 */
void I2C_set_sda_output(void)
{
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin = I2C_SDA_Pin;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(I2C_PORT,&GPIO_InitStruct);
}

/**
 * @brief 产生 I2C 起始信号
 * @note 在 SCL 为高电平的情况下, SDA 产生一个下降沿表示起始
 */
void I2C_Start(void)
{
        //I2C_set_sda_output();
        
        /* 这里需要注意顺序, SDA 先拉高, SCL 再拉高 */
        /* 因为 SCL 为高电平的情况下, SDA 的边沿动作是有意义的, 因此 SDA 要先拉高 */
        SCL=0;
        SDA=1;
        delay_us(1);
        SCL=1;
        delay_us(1);
        /* 在 SCL 为高电平的情况下, SDA 产生一个下降沿表示起始*/
        SDA=0;
        delay_us(1);
        /* 这里其实就是开始驱动传输的时钟了 */
        SCL=0;
}

/**
 * @brief 产生 I2C 停止信号
 * @note 在 SCL 为高电平的情况下, SDA 产生一个上升沿表示停止
 */
void I2C_Stop(void)
{
        //I2C_set_sda_output();
        
        /* 这里需要注意顺序, SCL 先拉低, SDA 再拉低 */
        /* 因为 SCL 为高电平的情况下, SDA 的边沿动作是有意义的, 因此 SCL 要先拉低 */
        SCL=0;
        SDA=0;
        delay_us(1);
        SCL=1;
        delay_us(1);
        
        SDA=1;
        delay_us(1);
        
}

/**
 * @brief 等待应答信号到来
 * @return u8 0,接收应答成功; -1,接收应答失败;
 * @note 主机发送完一帧数据之后, 需要等待从机给出响应才会继续发出下一帧数据
 * @note 这时候主机需要放出 SDA 的使用权, 由从机负责拉低 SDA (响应)
 * @note 主机在下一个 SCL 的上升沿(或说高电平)检测 SDA 是否为低电平, 低电平则表示有应答
 */

int I2C_WaitAsk(void)
{
        /* 定义一个变量, 作为超时的标记 */
        int ucErrTime = 0;
        /* 先默认的把 SDA 设置为高电平 */
        SDA=1;
        delay_us(1);
        /* 这里需要接收从机的应答信号, 因此需要设置为输入 */
        //I2C_set_sda_input();
        
        /* 下一个 SCL 时钟到了 */
        SCL=1;
        delay_us(1);
        /* 是时候去读取 SDA 看看有没有响应了 */
        /* 在没有超时之前只要读到了应答就可以自动跳出 while */
        __ASM("CPSID I");        //关中断  以免因中断误以为没有应答
        while(READSDA)
        {
                ucErrTime++;
                
                if(ucErrTime > 255)
                {
                        /* 主机就认为从机没有正确的接收, 就此作罢 */
                        I2C_Stop();
                        printf("[%s][%d]", __func__, __LINE__);
                        /* 函数返回接收应答失败 */
                        return -1;
                }
                
        }
        __ASM("CPSIE I"); //开中断
        /* 到这里, 读取应答信号的 SCL 结束了 */
        SCL=0;
        /* 能执行到这里, 说明读取到了应答的 */
        return 0;
}

/**
 * @brief HOST 产生应答
 * @note 此函数是在 HOST(STM32) 接收数据时才能使用
 * @note 如果只要接收4个字节, 前3次应答, 最后一次不应答就自动结束了。
 */
void I2C_Ask(void)
{
        SCL=0;
        delay_us(1);
        
        /* 要应答, 肯定要先获取SDA的使用权 */
       // I2C_set_sda_output();
        /* 在从机发送完本帧最后一个位的低电平期间, 把 SDA 拉低 */
        /* 千万不能在 SCL 高电平期间拉低, 那就变成起始信号了 */
        
        SDA=0;
        delay_us(1);
        
        /* 这里就是从机读取应答信号的一个 SCL 时钟周期了, SDA 不能动 */
        
        SCL=1;
        delay_us(1);
        SCL=0;
        
}

/**
 * @brief HOST 不产生应答
 * @note 此函数是在 HOST(STM32) 接收数据时才能使用
 * @note 如果只要接收4个字节, 前3次应答, 最后一次不应答就自动结束了。
 */

void I2C_NotAsk(void)
{
        SCL=0;
        delay_us(1);
        
        //I2C_set_sda_output();
        
        /* 既然拉低表示应答, 那我拉高不就行了 */
        SDA=1;
        delay_us(1);
        
         /* 下一个时钟一检测, 就发现, 没有应答 */
        SCL=1;
        delay_us(1);
        SCL=0;
}

/**
 * @brief I2C 发送一个字节
 * @param data 发送的数据
 */

void I2C_SentByte(u8 data)
{
        u8 i;
        //I2C_set_sda_output();
        
        /* 所有的数据的输出到 SDA 线上都是在 SCL 的低电平期间 */
        SCL=0;
        
        for(i=0;i<8;i++)
        {
                /* 写入数据的最高位 */
                SDA = (data & 0x80)>>7;
                 /* 发送完了最高位, 数据右移一个, 次低位变成了新的最高位 */
                data <<= 1;
                delay_us(1);
                 /* 在 SCL 的上升沿(或者高电平期间), 数据被从机接收读取 */
                SCL=1;
                delay_us(1);
                SCL=0;
        }
        /* 这函数结束的时候是不是 SCL = 0 */
        /* 一般这里接下来就会是 WaitResponse 操作了 */
        /* WaitResponse 的时候就是直接从 SCL = 0 开始的, 这样就不会多出来一个 SCL 的脉冲 */
}

/**
 * @brief I2C 读一个字节
 * @note 根据参数决定要不要应答, 如果只要接收4个字节, 前3次应答, 最后一次不应答就自动结束了。
 */
u8 I2C_ReadByte(void)
{
        u8 i, data = 0;
        //I2C_set_sda_input();
        for(i = 0; i < 8; i++)
        {
                SCL=0;
                delay_us(1);
                SCL=1;
                data <<= 1;
                if(READSDA)
                {
                        data++;
                }
                delay_us(1);
        }
        return data;
}









