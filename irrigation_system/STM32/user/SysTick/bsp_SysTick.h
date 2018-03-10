#ifndef __BSP_SYSTICK_H
#define __BSP_SYSTICK_H

#include "stm32f10x.h"

/* 由于 STM32F103 的 GPIO 位于 Cortex-M3 位带区, 可以通过下面的宏直接操作每一位 */
/* 具体原理请参考《Cortex-M3权威指南》*/
#define BITBAND(addr, bitnum)   ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5)+(bitnum << 2))
#define MEM_ADDR(addr)          *((volatile unsigned long * )(addr))
#define BIT_ADDR(addr, bitnum)  MEM_ADDR(BITBAND(addr, bitnum))

/* I/O地址映射 */
#define GPIOA_ODR_Addr  (GPIOA_BASE + 12)
#define GPIOB_ODR_Addr  (GPIOB_BASE + 12)
#define GPIOC_ODR_Addr  (GPIOC_BASE + 12)
#define GPIOD_ODR_Addr  (GPIOD_BASE + 12)
#define GPIOE_ODR_Addr  (GPIOE_BASE + 12)
#define GPIOF_ODR_Addr  (GPIOF_BASE + 12)
#define GPIOG_ODR_Addr  (GPIOG_BASE + 12)
#define GPIOH_ODR_Addr  (GPIOH_BASE + 12)
#define GPIOI_ODR_Addr  (GPIOI_BASE + 12)

#define GPIOA_IDR_Addr  (GPIOA_BASE + 8)
#define GPIOB_IDR_Addr  (GPIOB_BASE + 8)
#define GPIOC_IDR_Addr  (GPIOC_BASE + 8)
#define GPIOD_IDR_Addr  (GPIOD_BASE + 8)
#define GPIOE_IDR_Addr  (GPIOE_BASE + 8)
#define GPIOF_IDR_Addr  (GPIOF_BASE + 8)
#define GPIOG_IDR_Addr  (GPIOG_BASE + 8)
#define GPIOH_IDR_Addr  (GPIOH_BASE + 8)
#define GPIOI_IDR_Addr  (GPIOI_BASE + 8)

/*通过位带操作操作GPIO*/
#define PAout(n)        BIT_ADDR(GPIOA_ODR_Addr, n)
#define PAin(n)         BIT_ADDR(GPIOA_IDR_Addr, n)
#define PBout(n)        BIT_ADDR(GPIOB_ODR_Addr, n)
#define PBin(n)         BIT_ADDR(GPIOB_IDR_Addr, n)
#define PCout(n)        BIT_ADDR(GPIOC_ODR_Addr, n)
#define PCin(n)         BIT_ADDR(GPIOC_IDR_Addr, n)
#define PDout(n)        BIT_ADDR(GPIOD_ODR_Addr, n)
#define PDin(n)         BIT_ADDR(GPIOD_IDR_Addr, n)
#define PEout(n)        BIT_ADDR(GPIOE_ODR_Addr, n)
#define PEin(n)         BIT_ADDR(GPIOE_IDR_Addr, n)
#define PFout(n)        BIT_ADDR(GPIOF_ODR_Addr, n)
#define PFin(n)         BIT_ADDR(GPIOF_IDR_Addr, n)
#define PGout(n)        BIT_ADDR(GPIOG_ODR_Addr, n)
#define PGin(n)         BIT_ADDR(GPIOG_IDR_Addr, n)
#define PHout(n)        BIT_ADDR(GPIOH_ODR_Addr, n)
#define PHin(n)         BIT_ADDR(GPIOH_IDR_Addr, n)
#define PIout(n)        BIT_ADDR(GPIOI_ODR_Addr, n)
#define PIin(n)         BIT_ADDR(GPIOI_IDR_Addr, n)

#define SYSTEM_CLOCK 72                                 // 定义系统时钟, 单位MHz

void systick_init(void);                                // 初始化延时函数
void delay_us(u32 nus);                                 // 延时 nus 微秒, 最大延时 1864135us
void delay_ms(u16 nms);                                 // 延时 nms 毫秒, 最大延时 65536ms

#endif /* __BSP_SYSTICK_H */
