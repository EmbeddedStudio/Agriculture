#include "bsp_spi.h"
#include "bsp_usart.h"
#include "bsp_systick.h"

//static void GPIO_SPI_Config (void)
//{
//        GPIO_InitTypeDef  GPIO_InitStruct;      //配置连接到的GPIO

//        FLASH_SPI_CS_APBxClock_FUN (FLASH_SPI_CS_CLK,ENABLE);
//        FLASH_SPI_SCK_APBxClock_FUN(FLASH_SPI_SCK_CLK,ENABLE);
//        FLASH_SPI_APBxClock_FUN(FLASH_SPI_CLK,ENABLE);

//        /*MISO MOSI  SCK*/
//        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//        GPIO_InitStruct.GPIO_Pin = FLASH_SPI_MISO_PIN;
//        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//        GPIO_Init(FLASH_SPI_MISO_PORT,&GPIO_InitStruct);

//        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
//        GPIO_InitStruct.GPIO_Pin = FLASH_SPI_MOSI_PIN;
//        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//        GPIO_Init(FLASH_SPI_MOSI_PORT,&GPIO_InitStruct);

//        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
//        GPIO_InitStruct.GPIO_Pin = FLASH_SPI_SCK_PIN;
//        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//        GPIO_Init(FLASH_SPI_SCK_PORT,&GPIO_InitStruct);

//        //配置CS片选引脚
//        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
//        GPIO_InitStruct.GPIO_Pin = FLASH_SPI_CS_PIN;
//        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//        GPIO_Init(FLASH_SPI_CS_PORT,&GPIO_InitStruct);
// 
//        SPI_FLASH_CS_HIGH();
//}


void spi_Init(void)
{
        SPI_InitTypeDef  SPI_InitStruct;
        GPIO_InitTypeDef GPIO_InitStruct;

        SPI_APBxClock(SPI_CLK,ENABLE);                           //开 SPI 时钟
        SPI_CS2_APBxClock(SPI_CS2_CLK|SPI_CS1_CLK|SPI_GPIO_CLK,ENABLE);              //开 GPIOA 时钟
//        SPI_SCK_GPIO_APBxClock(SPI_GPIO_CLK,ENABLE);    //开 SPI 复用功能引脚的时钟
        //*SPI 引脚结构体配置*/
        //片选线
        GPIO_InitStruct.GPIO_Pin = SPI_CS1_PIN|SPI_CS2_PIN;
        GPIO_InitStruct.GPIO_Mode =  GPIO_Mode_Out_PP;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(SPI_CS1_PORT,&GPIO_InitStruct);
        GPIO_Init(SPI_CS2_PORT,&GPIO_InitStruct);
        //设置初始化为高电平即失能
        SPI_CS1_HIGH();
        SPI_CS2_HIGH();
        
        //spi 时钟线
        GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Pin   = SPI_SCK_PIN;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(SPI_GPIO_PORT, &GPIO_InitStruct);
        //MISO
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Pin  = SPI_MISO_PIN;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(SPI_MISO_PORT,&GPIO_InitStruct);
        //MOSI
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Pin  = SPI_MOSI_PIN;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(SPI_MISO_PORT,&GPIO_InitStruct);
        
        //*****SPI结构体配置**********/
        SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//定义波特率预分频的值:波特率预分频值为256
        SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;//串行同步时钟的第一个跳变沿（上升或下降）数据被采样
        SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;  //串行同步时钟的空闲状态为低电平
        SPI_InitStruct.SPI_CRCPolynomial=7;     //CRC值计算的多项式
        SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;  //设置SPI的数据大小:SPI发送接收8位帧结构
        SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
        SPI_InitStruct.SPI_FirstBit =  SPI_FirstBit_MSB  ;//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
        SPI_InitStruct.SPI_Mode = SPI_Mode_Master ;//设置SPI工作模式:设置为主SPI
        SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;  //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
        
        SPI_Init(SPIx,&SPI_InitStruct);
        SPI_Cmd(SPIx, ENABLE);
        
}

u8 SPI_SentByte(uint8_t Data)
{
         
        
        u32  SPITIME = SPI_TIMEOUT;

        while(SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_TXE)==RESET)
        {
                if((SPITIME--) == 0) 
                {
                        printf ("SPI 等待超时!");
                        printf("[%s][%d]\r\n", __func__, __LINE__);
                        return 0;
                }
        }
//        SPI_I2S_SendData(SPIx,Data);
        SPI1->DR = Data; //寄存器版本，固件库用上面的函数，功能一样，为了相对高效一点点
        while(SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_RXNE)==RESET)
        {
                if( (SPITIME--)==0 ) 
                {
                        printf ("SPI 等待超时!");
                        printf("[%s][%d]\r\n", __func__, __LINE__);
                        return 0;
                }
        }

//        return SPI_I2S_ReceiveData(SPIx);
        return SPI1->DR;
}

u8 SPI_ReadByte(void)
{
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  
//                SPI_I2S_SendData(SPI1, DUMMY);    //发送与应用不相关的数据，为后续的读数据提供时钟信号  
                SPI1->DR = DUMMY;////寄存器版本，固件库用上面的函数，功能一样，为了相对高效一点点
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);   
//                return SPI_I2S_ReceiveData(SPI1); 
        return  SPI1->DR;
}

//u8 SPI_ReadByte(void)
//{
//        return SPI_SentByte(DUMMY);
//}

