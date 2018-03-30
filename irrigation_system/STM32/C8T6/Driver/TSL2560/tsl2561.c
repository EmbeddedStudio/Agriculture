#include "tsl2561.h"
#include "delay.h"
#include "myiic.h"
#include "sys.h"
#include <stdio.h>
#include <math.h>
#include "string.h"
#include "tcp_demo.h"

uint16_t	DataLow0,DataHigh0,DataLow1,DataHigh1;
uint32_t	Channel_0,Channel_1;
extern u32 lux;
//u8 data1[2]={0x01,0x01};
//u8 data2[2]={0x01,0x01};
void tsl2561_Init(void)
{
	IIC_Init();
	TSL2561_Write(CONTROL,TSL2561_CONTROL_POWERON);
	delay_ms(100);
  TSL2561_Write(TIMING,0x02);
/*	
	TSL2561_Write(THRESHLOWLOW,0x010);                 //手册说不能用字节来写？？？
	TSL2561_Write(THRESHLOWHIGH,0x01); 
	TSL2561_Write(THRESHHIGHLOW,0x01); 
	TSL2561_Write(THRESHHIGHHIGH,0x01); 
	TSL2561_Write(INTERRUPT,0x15);              //电平中断，10个周期
*/
	TSL2561_Write(INTERRUPT,0x30);             // 直接进一次中断
//	TSL2561_Write(TIMING,0X02);
//	TSL2561_Write(THRESHLOWLOW,0X00);
//	TSL2561_Write(THRESHLOWHIGH,0X00);
//	TSL2561_Write(INTERRUPT ,0X00);
 //TSL2561_Write(TIMING,0x02); //设置时间寄存器
//	 TSL2561_Write(INTERRUPT,0x25); //，2s
	
}

void respons() 
{   
 uchar i;  
 IIC_SCL=1;  
 delay_us(4);   
 while((IIC_SDA==1)&&(i<250))   
 i++;  
 IIC_SCL=0;    
 delay_us(4); 
}

void TSL2561_Write(uchar command,uchar date)
{
 IIC_Start();   
 IIC_Send_Byte(0x72);  
IIC_Wait_Ack();	  
 IIC_Send_Byte(command);  
IIC_Wait_Ack();	
 IIC_Send_Byte(date);  
IIC_Wait_Ack();	
 IIC_Stop();
}
uchar TSL2561_Read(uchar command) 
{   
 uchar date;  
 IIC_Start();   
 IIC_Send_Byte(0x72);  
IIC_Wait_Ack();	
 IIC_Send_Byte(command);  
IIC_Wait_Ack();	  
 IIC_Start();   
 IIC_Send_Byte(0x73);  
IIC_Wait_Ack();	
 date = IIC_Read_Byte(0);  
 IIC_Stop();  
 return date; 
}

/*向寄存器写入多个字节
    dev:目标设备地址
    reg：寄存器
    length:字节数
    *data:数据首地址
*/
u8 IICwriteBytes(u8 dev, u8 reg, u8 length, u8* data)
{   
    u8 count = 0;  
    IIC_Start();  
    IIC_Send_Byte(dev);   
    IIC_Wait_Ack();  
    IIC_Send_Byte(reg);   
    IIC_Wait_Ack();     
    for(count=0;count<length;count++){  
        IIC_Send_Byte(data[count]);   
        IIC_Wait_Ack();   
     }  
    IIC_Stop();
    return 1; 
}  

uint32_t Read_Light()
{
 uint32_t Channel;

		DataLow0=TSL2561_Read(DATA0LOW);
		DataHigh0=TSL2561_Read(DATA0HIGH);
		Channel_0=256*DataHigh0+DataLow0;

		DataLow1=TSL2561_Read(DATA1LOW);
		DataHigh1=TSL2561_Read(DATA1HIGH);
		Channel_1=256*DataHigh1+DataLow1;

		Channel_1<<=16;
		Channel_1|=Channel_0;

		Channel=Channel_1;
		return Channel;
	/*
	if(0.0<Channel1/Channel0&&Channel1/Channel0<=0.52)E=(0.0315*Channel0-0.0593*Channel0*pow(Channel1/Channel0, 1.4));
 if(0.52<Channel1/Channel0&&Channel1/Channel0<=0.65)E=(0.0229*Channel0-0.0291*Channel1);
 if(0.65<Channel1/Channel0&&Channel1/Channel0<=0.80)E=(0.0157*Channel0-0.0180*Channel1);
 if(0.80<Channel1/Channel0&&Channel1/Channel0<=1.30)E=(0.00338*Channel0-0.00260*Channel1);
 if(Channel1/Channel0>1.30)E=0;
	*/
 //delay_ms(200);
}

uint32_t calculateLux(uint16_t ch0, uint16_t ch1)
{
		uint32_t chScale;
		uint32_t channel1;
		uint32_t channel0;
		uint32_t temp;
		uint32_t ratio1 = 0;
		uint32_t ratio	;
		uint32_t lux_temp;
		uint16_t b, m;
//		chScale=TSL2561_LUX_CHSCALE_TINT1; 		       //这是时间寄存器为0x00的
//		chScale=TSL2561_LUX_CHSCALE_TINT1;        //这是时间寄存器为0x01的
	chScale=(1 <<TSL2561_LUX_CHSCALE);           //这是时间寄存器为0x02的
		chScale = chScale << 4;                      //这是增益为1的，增益为16不用写这一条
		
		// scale the channel values
		channel0 = (ch0 * chScale) >> TSL2561_LUX_CHSCALE;
		channel1 = (ch1 * chScale) >> TSL2561_LUX_CHSCALE;
		
		// find the ratio of the channel values (Channel1/Channel0)
		if (channel0 != 0) 
		ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;
		ratio = (ratio1 + 1) >> 1;	  									 // round the ratio value
		if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1T))
			{
				b=TSL2561_LUX_B1T;
				m=TSL2561_LUX_M1T;
			}
		else if (ratio <= TSL2561_LUX_K2T)
			{
				b=TSL2561_LUX_B2T;
				m=TSL2561_LUX_M2T;
			}
		else if (ratio <= TSL2561_LUX_K3T)
			{
				b=TSL2561_LUX_B3T;
				m=TSL2561_LUX_M3T;
			}
		else if (ratio <= TSL2561_LUX_K4T)
			{
				b=TSL2561_LUX_B4T;
				m=TSL2561_LUX_M4T;
			}
		else if (ratio <= TSL2561_LUX_K5T)
			{
				b=TSL2561_LUX_B5T; 
				m=TSL2561_LUX_M5T;
			}
		else if (ratio <= TSL2561_LUX_K6T)
			{
				b=TSL2561_LUX_B6T; 
				m=TSL2561_LUX_M6T;
			}
		else if (ratio <= TSL2561_LUX_K7T)
			{
				b=TSL2561_LUX_B7T; 
				m=TSL2561_LUX_M7T;
			}
		else if (ratio > TSL2561_LUX_K8T)
			{
				b=TSL2561_LUX_B8T; 
				m=TSL2561_LUX_M8T;
			}
						  
		temp = ((channel0 * b) - (channel1 * m));
		if (temp < 0)  temp = 0;							// do not allow negative lux value
		temp += (1 << (TSL2561_LUX_LUXSCALE-1));			// round lsb (2^(LUX_SCALE-1))
		lux_temp = temp >> TSL2561_LUX_LUXSCALE;			// strip off fractional portion
		return lux_temp;		  							// Signal I2C had no errors
}

void send_guangqiang(void)
{
	tcp_server_databuf[22]=lux/10000; //万位
	tcp_server_databuf[23]=lux%10000/1000;//千位
	tcp_server_databuf[24]=lux%1000/100;//百位
	tcp_server_databuf[25]=lux%100/10;//十位
	tcp_server_databuf[26]=lux%10;//个位
}

	

