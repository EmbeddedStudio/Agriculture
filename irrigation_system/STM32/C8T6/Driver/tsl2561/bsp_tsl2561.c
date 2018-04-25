#include "bsp_tsl2561.h"
#include "bsp_i2c.h"
#include "bsp_usart.h"

float light_Min=50.0;
float light_Max=150.0;
 void TSL2561_Write(u8 addr,u8 cmd) 
{
        I2C_Start();
        I2C_SentByte( SLAVE_ADDR_WR ); //写器件
        if( I2C_WaitAsk()!=0 )
        {
                I2C_Stop();
                return ;
        }
        I2C_SentByte(addr);
        if( I2C_WaitAsk()!=0 )
        {
                I2C_Stop();
                return ;
        }
        I2C_SentByte(cmd);
        I2C_Stop();
}

 u8 TSL2561_Read(u8 addr)
{
        u8 data;
        I2C_Start();
        I2C_SentByte( SLAVE_ADDR_WR ); //写器件
        if( I2C_WaitAsk()!=0 )
        {
                I2C_Stop();
                return 0;
        }
        I2C_SentByte(addr);
        if( I2C_WaitAsk()!=0 )
        {
                I2C_Stop();
                return 0;
        }
        I2C_Start();
        I2C_SentByte( SLAVE_ADDR_RD );
        if( I2C_WaitAsk()!=0 )
        {
                I2C_Stop();
                return 0;
        }
        data=I2C_ReadByte();
        I2C_Stop();
        return data;
}

float Read_Light(void )
{
        u16 DataHigh0=0, DataHigh1=0,ch0=0,ch1=0;
        float lx=0;
        u8   DataLow0=0,DataLow1=0,lxp=0;
        DataLow0=TSL2561_Read(TSL2561_DATA0_LOW);
        DataHigh0=TSL2561_Read(TSL2561_DATA0_HIGH);
        ch0=(DataHigh0<<8|DataLow0);
         
        DataLow1=TSL2561_Read(TSL2561_DATA1_LOW);
        DataHigh1=TSL2561_Read(TSL2561_DATA1_HIGH);
        ch1=(DataHigh1<<8|DataLow1);
        lxp=(ch1*100/ch0);
        
        if((lxp>0)&&(lxp<=50))
        {
                lx=0.0304*ch0-0.062*ch0*((ch1/ch0)/4);
        }
        else if((50<lxp)&&(lxp<=61)) 
        {  
                lx=0.0224*ch0-0.031*ch1;  
        }   
        else if((61<lxp)&&(lxp<=80))
        {   
                lx=0.0128*ch0-0.0153*ch1; 

        }   
        else if((80<lxp)&&(lxp<=130)) 
        {   
                lx=0.00146*ch0-0.00112*ch1; 
        } 
        else if(130<lxp)
        {  
                lx=0;  
        }  
//        return lx;              //流明单位
        return lx*4*3.14;     //坎德拉单位
} 


void TSL2561_Init(void)
{
        TSL2561_Write(TSL2561_CMD,TSL2561_ON); //设置工作模式
        TSL2561_Write(TSL2561_TIMING,0x02); //设置积分时间
        TSL2561_Write(INTERRUPT,0x30);
}



