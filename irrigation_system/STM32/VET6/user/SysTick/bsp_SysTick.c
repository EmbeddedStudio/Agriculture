#include "bsp_systick.h"

/* 保存延时 1us/1ms 滴答定时器的计数个数 */
static u8 fac_us = 0;
static u16 fac_ms = 0;

/**
 * @brief 初始化滴答定时器
 */
void systick_init(void)
{
        /* 设置滴答定时器的时钟为系统时钟的 8 分频 */
        SysTick->CTRL &= SysTick_CLKSource_HCLK_Div8;

        /* 系统时钟为 SYSTEM_CLOCK, 滴答定时器的时钟为: ((SYSTEM_CLOCK * 10^6) / 8) */
        /* 可以算出, 一个计数值的时间是: (8 / (SYSTEM_CLOCK * 10^6)) */
        /* 那么定时 1us 的计数值就是: ((10^-6 * (SYSTEM_CLOCK * 10^6))/ 8) = (SYSTEM_CLOCK / 8) */
        fac_us = SYSTEM_CLOCK / 8;

        /* 定时 1ms 的计数值肯定就是 1us 的 1000 倍了 */
        fac_ms = (u16) fac_us * 1000;
}

/**
 * @brief 延时 nus 微秒
 * @param nus 延时时间 nus <= 1864135us (最大值即 2^24 / fac_us)
 */
void delay_us(u32 nus)
{
        /* 记录读取的滴答定时器控制寄存器 CTRL 数据的临时变量 */
        u32 temp;

        /* 设置定时器的重装载值为 nus * fac_us */
        /* 根据定时的时间给滴答定时器加载重装载值 */
        SysTick->LOAD = nus * fac_us;

        /* 设置滴答定时器的计数值为 0, 清空滴答定时器 */
        SysTick->VAL = 0;

        /* 开启滴答定时器, 此刻滴答定时器已经默默的在后台开始计时了 */
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

        /* 持续的读取滴答定时器 COUNTFLAG (CTRL的位16), 判断定时是否到达 */
        /* 定时时间没有到达之前程序一直停在这个死循环里面 */
        do
        {
                temp = SysTick->CTRL;
        } while ((temp & 0x01) && !(temp & (1 << 16)));

        /* 关闭滴答定时器 */
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

        /* 设置滴答定时器的计数值为 0, 清空滴答定时器 */
        SysTick->VAL = 0;
}

/**
 * @brief 延时 nms 毫秒
 * @param nms 延时时间 nms <= 1864ms 算法同上一个函数
 */
static void delay_xms(u16 nms)
{
        u32 temp;
        SysTick->LOAD = (u32) nms * fac_ms;
        SysTick->VAL = 0x00;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
        do
        {
                temp = SysTick->CTRL;
        } while ((temp & 0x01) && !(temp & (1 << 16)));
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        SysTick->VAL = 0X00;
}

/**
 * @brief 延时 nms 毫秒
 * @param nms 延时时间 nms <= 65535
 */
void delay_ms(u16 nms)
{
        /* 这个函数其实就是将 nms 分解开来, 分解成为多个 1500ms * repeat + remain */
        u8 repeat = nms / 1500;
        u16 remain = nms % 1500;

        while (repeat)
        {
                delay_xms(1500);
                repeat--;
        }

        if (remain)
        {
                delay_xms(remain);
        }
}
