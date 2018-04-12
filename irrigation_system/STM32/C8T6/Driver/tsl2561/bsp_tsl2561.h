#ifndef  __BSP_TSL2561_H
#define  __BSP_TSL2561_H

#include "bsp_systick.h"


//Timing Register Value.Set integration time  
                                      //最后两位设置积分时间  
#define  TIMING_13MS         0x00     //积分时间13.7毫秒  
#define  TIMING_101MS        0x01     //积分时间101毫秒  
#define  TIMING_402MS        0x02     //积分时间402毫秒  
#define  TIMING_GAIN_1X      0x10     //增益倍数与积分时间进行或运算  
#define  TIMING_GAIN_16X     0x00 

// TSL2561 registers  
  
#define TSL2561_CMD           0x80  
#define TSL2561_TIMING        0x81  
#define TSL2561_REG_ID        0x8A  
#define TSL2561_DATA0_LOW     0x8C  
#define TSL2561_DATA0_HIGH    0x8D  
#define TSL2561_DATA1_LOW     0x8E  
#define TSL2561_DATA1_HIGH    0x8F  

#define TSL2561_ON   0x03
#define TSL2561_OFF  0x00
#define INTERRUPT     0xC6 
#define TSL2561_ADDR0 0x29 // address with '0' shorted on board  
#define TSL2561_ADDR  0x39 // default address  
#define TSL2561_ADDR1 0x49 // address with '1' shorted on board

#define SLAVE_ADDR_WR 0x72 //数据手册给的是上面的地址，但是还要进行<<1位就是现在的地址了
#define SLAVE_ADDR_RD 0x73 

extern float light_Min;
extern float light_Max;
void TSL2561_Write(u8 addr,u8 cmd);
u8 TSL2561_Read(u8 addr);
void TSL2561_Init(void);
float Read_Light(void );

#endif  /*__BSP_TSL2561_H*/
