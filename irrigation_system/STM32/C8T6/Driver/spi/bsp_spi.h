#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "stm32f10x.h"

/************************** SPI Flash 连接引脚定义********************************/
#define      SPIx                                     SPI1
#define      SPI_APBxClock                            RCC_APB2PeriphClockCmd
#define      SPI_CLK                                  RCC_APB2Periph_SPI1

#define      SPI_CS1_APBxClock                         RCC_APB2PeriphClockCmd
#define      SPI_CS1_CLK                               RCC_APB2Periph_GPIOB   
#define      SPI_CS1_PORT                              GPIOB
#define      SPI_CS1_PIN                               GPIO_Pin_8

#define      SPI_CS2_APBxClock                         RCC_APB2PeriphClockCmd
#define      SPI_CS2_CLK                               RCC_APB2Periph_GPIOA   
#define      SPI_CS2_PORT                              GPIOA
#define      SPI_CS2_PIN                               GPIO_Pin_4

#define      SPI_SCK_GPIO_APBxClock                    RCC_APB2PeriphClockCmd
#define      SPI_GPIO_CLK                              RCC_APB2Periph_GPIOA   
#define      SPI_GPIO_PORT                             GPIOA 

#define      SPI_SCK_PORT                             GPIOA   
#define      SPI_SCK_PIN                              GPIO_Pin_5
  
#define      SPI_MISO_PORT                            GPIOA 
#define      SPI_MISO_PIN                             GPIO_Pin_6
   
#define      SPI_MOSI_PORT                            GPIOA 
#define      SPI_MOSI_PIN                             GPIO_Pin_7

#define      SPI_TIMEOUT                              (uint32_t)0x1000
#define      DUMMY                                      0x00
/************************** SPI Flash 函数宏定义********************************/
#define      SPI_CS1_LOW()                    GPIO_ResetBits(  SPI_CS1_PORT, SPI_CS1_PIN )
#define      SPI_CS1_HIGH()                   GPIO_SetBits(  SPI_CS1_PORT, SPI_CS1_PIN )

#define      SPI_CS2_LOW()                    GPIO_ResetBits(  SPI_CS2_PORT, SPI_CS2_PIN )
#define      SPI_CS2_HIGH()                   GPIO_SetBits(  SPI_CS2_PORT, SPI_CS2_PIN )

// SPI总线速度设置 
#define SPI_SPEED_2   0
#define SPI_SPEED_4   1
#define SPI_SPEED_8   2
#define SPI_SPEED_16  3
#define SPI_SPEED_256 4

/******************函数声明******************/ 
void spi_Init(void);
u8 SPI_SentByte(uint8_t data);
u8 SPI_ReadByte(void);
void SPI_SetSpeed1(u8 SpeedSet);

#endif  /*__BSP_SPI_H*/
