#include "bsp_OLED.h"
#include "codetab.h"
#include "bsp_i2c.h"
#include "bsp_systick.h"
#include "bsp_usart.h"

static void WriteDat(unsigned char I2C_Data)//写数据
{
        I2C_Start();
        I2C_SentByte(OLED_ADDRESS);   //主机广播从机地址
        if( I2C_WaitAsk()!=0 )
        {
                I2C_Stop();
                return ;
        }
        I2C_SentByte(0x40);             //往这个地址写数据
        if( I2C_WaitAsk()!=0 )
        {
                I2C_Stop();
                return ;
        }
        I2C_SentByte(I2C_Data);
        I2C_Stop();
}
static void  WriteCmd(unsigned char I2C_Command)  //写命令
{
//        I2C_WriteByte(0x00, I2C_Command);
        I2C_Start();
        I2C_SentByte(OLED_ADDRESS);   //主机广播从机地址
        if( I2C_WaitAsk()!=0 )
        {
                I2C_Stop();
                return ;
        }
        I2C_SentByte(0x00);                     //往这个地址写命令
        if( I2C_WaitAsk()!=0 )
        {
                I2C_Stop();
                return ;
        }
        I2C_SentByte(I2C_Command);
        I2C_Stop();
}


void OLED_Init(void)
{
        delay_ms(10); //这里的延时很重要

        WriteCmd(0xAE); //display off
        WriteCmd(0x20); //Set Memory Addressing Mode    
        WriteCmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
        WriteCmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
        WriteCmd(0xc8); //Set COM Output Scan Direction
        WriteCmd(0x00); //---set low column address
        WriteCmd(0x10); //---set high column address
        WriteCmd(0x40); //--set start line address
        WriteCmd(0x81); //--set contrast control register
        WriteCmd(0xff); //亮度调节 0x00~0xff
        WriteCmd(0xa1); //--set segment re-map 0 to 127
        WriteCmd(0xa6); //--set normal display
        WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
        WriteCmd(0x3F); //
        WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
        WriteCmd(0xd3); //-set display offset
        WriteCmd(0x00); //-not offset
        WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
        WriteCmd(0xf0); //--set divide ratio
        WriteCmd(0xd9); //--set pre-charge period
        WriteCmd(0x22); //
        WriteCmd(0xda); //--set com pins hardware configuration
        WriteCmd(0x12);
        WriteCmd(0xdb); //--set vcomh
        WriteCmd(0x20); //0x20,0.77xVcc
        WriteCmd(0x8d); //--set DC-DC enable
        WriteCmd(0x14); //
        WriteCmd(0xaf); //--turn on oled panel
        OLED_Fill(0x00);
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
        WriteCmd(0xb0+y);
        WriteCmd(((x&0xf0)>>4)|0x10);
        WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
        unsigned char m,n;
        for(m=0;m<8;m++)
        {
                WriteCmd(0xb0+m);       //page0-page1
                WriteCmd(0x00);         //low column start address
                WriteCmd(0x10);         //high column start address
                for(n=0;n<128;n++)
                {
                        WriteDat(fill_Data);
                }
        }
}

void OLED_CLS(void)//清屏
{
        OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          : 
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
        WriteCmd(0X8D);  //设置电荷泵
        WriteCmd(0X14);  //开启电荷泵
        WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          : 
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
        WriteCmd(0X8D);  //设置电荷泵
        WriteCmd(0X10);  //关闭电荷泵
        WriteCmd(0XAE);  //OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y,char ch[], unsigned char TextSize)
{
        unsigned char c = 0,i = 0,j = 0;
        switch(TextSize)
        {
                case 1:
                {
                        while(ch[j] != '\0')
                        {
                                c = ch[j] - 32;
                                if(x > 126)
                                {
                                        x = 0;
                                        y++;
                                }
                                OLED_SetPos(x,y);
                                for(i=0;i<6;i++)
                                        WriteDat(F6x8[c][i]);
                                x += 6;
                                j++;
                        }
                }break;
                case 2:
                {
                        while(ch[j] != '\0')
                        {
                                c = ch[j] - 32;
                                if(x > 120)
                                {
                                        x = 0;
                                        y++;
                                }
                                OLED_SetPos(x,y);
                                for(i=0;i<8;i++)
                                        WriteDat(F8X16[c*16+i]);
                                OLED_SetPos(x,y+1);
                                for(i=0;i<8;i++)
                                        WriteDat(F8X16[c*16+i+8]);
                                x += 8;
                                j++;
                        }
                }break;
        }
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N ,unsigned char* F16x16)
{
        unsigned char wm=0;
        unsigned int  adder=0;
        unsigned int i=0;
        for(i=0;i<N;i++)
        {
                adder=32*i;
                OLED_SetPos(x+16*i,y);
                for(wm = 0;wm <16;wm++)
                {
                        WriteDat(F16x16[adder]);
                        adder += 1;
                }
                OLED_SetPos(x+16*i,y + 1);
                for(wm = 0;wm < 16;wm++)
                {
                        WriteDat(F16x16[adder]);
                        adder += 1;
                }
        }
}
