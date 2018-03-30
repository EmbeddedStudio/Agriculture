#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "stm32f10x.h"
#include "bsp_systick.h"

#define I2C_PORT         GPIOB
#define I2C_SCL_Pin      GPIO_Pin_6
#define I2C_SDA_Pin      GPIO_Pin_7
#define SCL              PBout(6)
#define SDA              PBout(7)
#define READSDA         ((I2C_PORT->IDR & I2C_SDA_Pin)!=0)
//1 Read    0 Write     R/!W 
/*********************************************/
void I2C_init (void);
void I2C_set_sda_input(void);
void I2C_set_sda_output(void);
void I2C_Start(void);
void I2C_Stop(void);
int I2C_WaitAsk(void);
void I2C_Ask(void);
void I2C_NotAsk(void);
void I2C_SentByte(u8 data);
u8 I2C_ReadByte(void);


#endif  /*__BSP_I2C_H*/


