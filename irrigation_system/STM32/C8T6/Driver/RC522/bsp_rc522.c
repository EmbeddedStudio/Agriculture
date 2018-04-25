#include "bsp_rc522.h"
#include "bsp_usart.h"

void RC522_GPIO_Config(void)
{
        RC522_APBxClock(IRQ_GPIO_CLK|RST_GPIO_CLK,ENABLE);
        GPIO_InitTypeDef  GPIO_Strtuce;
        
        GPIO_Strtuce.GPIO_Pin = RST_GPIO_PIN;
        GPIO_Strtuce.GPIO_Mode = GPIO_Mode_Out_PP ;
        GPIO_Strtuce.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(RST_GPIO_PORT,&GPIO_Strtuce);
        
        GPIO_Strtuce.GPIO_Pin = IRQ_GPIO_PIN;
        GPIO_Strtuce.GPIO_Mode =GPIO_Mode_IPU ;
        GPIO_Init(IRQ_GPIO_PORT,&GPIO_Strtuce);
}



void RC522_Init ( void )
{
        RC522_GPIO_Config();
        RC522_Rst_Disable();
        RC522_CS_HIGH();
        RC522_Rst();
        PcdAntennaOff();
        delay_ms(2);
        PcdAntennaOn();
        M500PcdConfigISOType( 'A' );
}
/**
 * @brief 写 RC522 寄存器
 * @param Address :寄存器的地址
 * @param value :需要写的数据
 * @note null
 */
void WriteRawRC(u8 Address, u8 value)
{  
        u8   ucAddr;

        RC522_CS_LOW();
        ucAddr = ((Address<<1)&0x7E);

        SPI_SentByte(ucAddr);
        SPI_SentByte(value);
        RC522_CS_HIGH();
}


/**
 *  @brief 读 RC522 寄存器
 *  @param ucAddress，寄存器地址
 */
u8 ReadRawRC ( u8 ucAddress )
{
        u8 ucAddr, ucReturn;
        ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;
        RC522_CS_LOW();
        SPI_SentByte ( ucAddr );
        ucReturn = SPI_ReadByte ();
        RC522_CS_HIGH();
        return ucReturn;
}
/**
 * @brief 天线复位
 */
void RC522AntennaRst(void)
{
          RC522_Rst();
          PcdAntennaOff();
          delay_ms(2);  
          PcdAntennaOn();
}  
/**
 * @brief 复位 RC522 
 * @param 无
 */
void RC522_Rst(void)
{
        RC522_Rst_Disable();   //低电平使能
        delay_us(1);
        RC522_Rst_Enable() ;  //高电平
        delay_us(1);
        RC522_Rst_Disable();
        delay_us(1);
        WriteRawRC(CommandReg,PCD_RESETPHASE);
        while ( ReadRawRC ( CommandReg ) & 0x10 );
        delay_us(1);

        WriteRawRC(ModeReg,0x3D);     //定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363
        WriteRawRC(TReloadRegL,30);    //16位定时器低位      
        WriteRawRC(TReloadRegH,0);              //16位定时器高位
        WriteRawRC(TModeReg,0x8D);              //定义内部定时器的设置
        WriteRawRC(TPrescalerReg,0x3E);         //设置定时器分频系数

        WriteRawRC(TxAutoReg,0x40);//必须要 //调制发送信号为100%ASK 

}

/**
 * @brief 设置RC522的工作方式
 * @param ucType，工作方式
 */
static void M500PcdConfigISOType ( u8 ucType )
{
        if ( ucType == 'A')                     //ISO14443_A
        {
                ClearBitMask ( Status2Reg, 0x08 );
                WriteRawRC ( ModeReg, 0x3D );//3F
                WriteRawRC ( RxSelReg, 0x86 );//84
                WriteRawRC( RFCfgReg, 0x7F );   //4F
                WriteRawRC( TReloadRegL, 30 );//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
                WriteRawRC ( TReloadRegH, 0 );
                WriteRawRC ( TModeReg, 0x8D );
                WriteRawRC ( TPrescalerReg, 0x3E );
                delay_us ( 2 );
                PcdAntennaOn ();//开天线
        }
}

/**
 * @brief 对RC522寄存器置位
 * @param ucReg，寄存器地址
 * @param ucMask，置位值
 */
void SetBitMask ( u8 ucReg, u8 ucMask )  
{
    u8 ucTemp;
    ucTemp = ReadRawRC ( ucReg );
    WriteRawRC ( ucReg, ucTemp | ucMask );         // set bit mask
}

/**
 * @brief 对RC522寄存器清位
 * @param   ucReg，寄存器地址
 * @param   ucMask，清位值
 */
void ClearBitMask ( u8 ucReg, u8 ucMask )  
{
        u8 ucTemp;
        ucTemp = ReadRawRC ( ucReg );
        WriteRawRC ( ucReg, ucTemp & ( ~ ucMask) );  // clear bit mask
}
/**
 * @brief 开天线
 */
void PcdAntennaOn ( void )
{
        u8 uc;
        uc = ReadRawRC ( TxControlReg );
        if ( ! ( uc & 0x03 ) )
        SetBitMask(TxControlReg, 0x03);
}
/**
 * @brief 关天线
 */
void PcdAntennaOff(void)
{
        ClearBitMask(TxControlReg, 0x03);
}



/**
 * @brief 寻卡
 * @param   ucReq_code，寻卡方式
 *                     = 0x52，寻感应区内所有符合14443A标准的卡
 *                     = 0x26，寻未进入休眠状态的卡
 * @param     pTagType，卡片类型代码
 *                   = 0x4400，Mifare_UltraLight
 *                   = 0x0400，Mifare_One(S50)
 *                   = 0x0200，Mifare_One(S70)
 *                   = 0x0800，Mifare_Pro(X))
 *                   = 0x4403，Mifare_DESFire
 * @return  状态值 = MI_OK，成功
 */
char PcdRequest ( u8 ucReq_code, u8 * pTagType )
{
        char cStatus;  
        u8 ucComMF522Buf [ MAXRLEN ]; 
        u32 ulLen;
        ClearBitMask ( Status2Reg, 0x08 );      //清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况
        WriteRawRC ( BitFramingReg, 0x07 );     //      发送的最后一个字节的 七位
        SetBitMask ( TxControlReg, 0x03 );      //TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号
        ucComMF522Buf [ 0 ] = ucReq_code;           //存入 卡片命令字
        cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, & ulLen );     //寻卡  
        if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )        //寻卡成功返回卡类型 
        {    
                * pTagType = ucComMF522Buf [ 0 ];
                * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
        }
        else
        cStatus = MI_ERR;
        return cStatus;
}


/**
 * @brief  通过RC522和ISO14443卡通讯
 * @param  ucCommand:RC522命令字
 *         pInData，通过RC522发送到卡片的数据
 *         ucInLenByte，发送数据的字节长度
 *         pOutData，接收到的卡片返回数据
 *         pOutLenBit，返回数据的位长度
 * @return 状态值= MI_OK，成功
 */
char PcdComMF522 ( u8 ucCommand, u8 * pInData, u8 ucInLenByte, u8 * pOutData, u32 * pOutLenBit ) 
{
        char cStatus = MI_ERR;
        u8 ucIrqEn   = 0x00;
        u8 ucWaitFor = 0x00;
        u8 ucLastBits;
        u8 ucN;
        u32 ul;
        switch ( ucCommand )
        {
                case PCD_AUTHENT:         //Mifare认证
                        ucIrqEn   = 0x12;        //允许错误中断请求ErrIEn  允许空闲中断IdleIEn
                        ucWaitFor = 0x10;       //认证寻卡等待时候 查询空闲中断标志位
                break;
                case PCD_TRANSCEIVE:            //接收发送 发送接收
                        ucIrqEn   = 0x77;               //允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
                        ucWaitFor = 0x30;               //寻卡等待时候 查询接收中断标志位与 空闲中断标志位
                break;
                default:
                break;
        }
        WriteRawRC ( ComIEnReg, ucIrqEn | 0x80 );               //IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反 
        ClearBitMask ( ComIrqReg, 0x80 );               //Set1该位清零时，CommIRqReg的屏蔽位清零
        WriteRawRC ( CommandReg, PCD_IDLE );            //写空闲命令
        SetBitMask ( FIFOLevelReg, 0x80 );              //置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除
        for ( ul = 0; ul < ucInLenByte; ul ++ )
        WriteRawRC ( FIFODataReg, pInData [ ul ] );           //写数据进FIFOdata
        WriteRawRC ( CommandReg, ucCommand );           //写命令
        if ( ucCommand == PCD_TRANSCEIVE )
        SetBitMask(BitFramingReg,0x80);         //StartSend置位启动数据发送 该位与收发命令使用时才有效
        ul = 1000;//根据时钟频率调整，操作M1卡最大等待时间25ms
        do      //认证 与寻卡等待时间 
        {
                ucN = ReadRawRC ( ComIrqReg ); //查询事件中断
                ul --;
        } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );   //退出条件i=0,定时器中断，与写空闲命令

        ClearBitMask ( BitFramingReg, 0x80 );   //清理允许StartSend位

        if ( ul != 0 )
        {
                if ( ! ( ReadRawRC ( ErrorReg ) & 0x1B ) )      //读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
                {
                        cStatus = MI_OK;

                        if ( ucN & ucIrqEn & 0x01 )     //是否发生定时器中断
                        cStatus = MI_NOTAGERR;   

                        if ( ucCommand == PCD_TRANSCEIVE )
                        {
                                ucN = ReadRawRC ( FIFOLevelReg );       //读FIFO中保存的字节数

                                ucLastBits = ReadRawRC ( ControlReg ) & 0x07;   //最后接收到得字节的有效位数

                                if ( ucLastBits )
                                * pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	//N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
                                else
                                * pOutLenBit = ucN * 8; //最后接收到的字节整个字节有效

                                if ( ucN == 0 ) 
                                ucN = 1;    

                                if ( ucN > MAXRLEN )
                                ucN = MAXRLEN;   

                                for ( ul = 0; ul < ucN; ul ++ )
                                pOutData [ ul ] = ReadRawRC ( FIFODataReg );   
                        }
                }
                else
                cStatus = MI_ERR;   
        }
        SetBitMask ( ControlReg, 0x80 );           // stop timer now
        WriteRawRC ( CommandReg, PCD_IDLE ); 
        return cStatus;
}

/**
 * @brief 防冲撞
 * @param  pSnr，卡片序列号，4字节
 * @return 状态值 = MI_OK，成功
 */
char PcdAnticoll ( u8 * pSnr )
{
        char cStatus;
        u8 uc, ucSnr_check = 0;
        u8 ucComMF522Buf [ MAXRLEN ]; 
        u32 ulLen;

        ClearBitMask ( Status2Reg, 0x08 );              //清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
        WriteRawRC ( BitFramingReg, 0x00);              //清理寄存器 停止收发
        ClearBitMask ( CollReg, 0x80 );                 //清ValuesAfterColl所有接收的位在冲突后被清除

        ucComMF522Buf [ 0 ] = 0x93;                //卡片防冲突命令
        ucComMF522Buf [ 1 ] = 0x20;

        cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, & ulLen);//与卡片通信

        if ( cStatus == MI_OK)                  //通信成功
        {
                for ( uc = 0; uc < 4; uc ++ )
                {
                        * ( pSnr + uc )  = ucComMF522Buf [ uc ];        //读出UID
                        ucSnr_check ^= ucComMF522Buf [ uc ];
                }
                if ( ucSnr_check != ucComMF522Buf [ uc ] )
                cStatus = MI_ERR;    
        }
        SetBitMask ( CollReg, 0x80 );
        return cStatus;
}
/**
 * @brief  CalulateCRC用RC522计算CRC16
 * @param  pIndata:计算CRC16的数组
 * @param  ucLen:计算CRC16的数组字节长度
 * @param  pOutData:存放计算结果存放的首地址
 */
void CalulateCRC ( u8 * pIndata, u8 ucLen, u8 * pOutData )
{
        u8 uc, ucN;
        ClearBitMask(DivIrqReg,0x04);
        WriteRawRC(CommandReg,PCD_IDLE);
        SetBitMask(FIFOLevelReg,0x80);
        for ( uc = 0; uc < ucLen; uc ++)
        WriteRawRC ( FIFODataReg, * ( pIndata + uc ) );   
        WriteRawRC ( CommandReg, PCD_CALCCRC );
        uc = 0xFF;
        
        do 
        {
                ucN = ReadRawRC ( DivIrqReg );
                uc --;
        } while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );

        pOutData [ 0 ] = ReadRawRC ( CRCResultRegL );
        pOutData [ 1 ] = ReadRawRC ( CRCResultRegM );
}
/**
 * @brief   PcdSelect选定卡片
 * @param   pSnr，卡片序列号，4字节
 * @return 状态值= MI_OK，
 */
char PcdSelect ( u8 * pSnr )
{
        char ucN;
        u8 uc;
        u8 ucComMF522Buf [ MAXRLEN ]; 
        u32  ulLen;
        
        ucComMF522Buf [ 0 ] = PICC_ANTICOLL1;
        ucComMF522Buf [ 1 ] = 0x70;
        ucComMF522Buf [ 6 ] = 0;

        for ( uc = 0; uc < 4; uc ++ )
        {
                ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
                ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
        }
        CalulateCRC ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );
        ClearBitMask ( Status2Reg, 0x08 );
        ucN = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, & ulLen );
        if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
                ucN = MI_OK;  
        else
                ucN = MI_ERR;  
        return ucN;
}

/**
 * @brief  验证卡片密码
 * @param  ucAuth_mode，密码验证模式
 *                     = 0x60，验证A密钥
 *                     = 0x61，验证B密钥
 * @param  u8 ucAddr，块地址
 *         pKey，密码
 *         pSnr，卡片序列号，4字节
 * @return  : 状态值 = MI_OK，成功
 */
char PcdAuthState ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr )
{
        char cStatus;
        u8 uc, ucComMF522Buf [ MAXRLEN ];
        u32 ulLen;

        ucComMF522Buf [ 0 ] = ucAuth_mode;
        ucComMF522Buf [ 1 ] = ucAddr;

        for ( uc = 0; uc < 6; uc ++ )
            ucComMF522Buf [ uc + 2 ] = * ( pKey + uc );   

        for ( uc = 0; uc < 6; uc ++ )
            ucComMF522Buf [ uc + 8 ] = * ( pSnr + uc );   

        cStatus = PcdComMF522 ( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, & ulLen );

        if ( ( cStatus != MI_OK ) || ( ! ( ReadRawRC ( Status2Reg ) & 0x08 ) ) )
       
        cStatus = MI_ERR;   
        return cStatus;
}


/**
 * @brief  写数据到M1卡一块
 * @param  u8 ucAddr，块地址
 *         pData，写入的数据，16字节
 * @return  状态值  = MI_OK，成功
 */
char PcdWrite ( u8 ucAddr, u8 * pData )
{
        char cStatus;
        u8 uc, ucComMF522Buf [ MAXRLEN ];
        u32 ulLen;
        
        ucComMF522Buf [ 0 ] = PICC_WRITE;
        ucComMF522Buf [ 1 ] = ucAddr;

        CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

        cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

        if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
        cStatus = MI_ERR;   

        if ( cStatus == MI_OK )
        {
                                //memcpy(ucComMF522Buf, pData, 16);
                for ( uc = 0; uc < 16; uc ++ )
                        ucComMF522Buf [ uc ] = * ( pData + uc );  
                                
                CalulateCRC ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );

                cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, & ulLen );
                                
                if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
                        cStatus = MI_ERR;   
        } 
        return cStatus;
}

/**
 * @brief  读取M1卡一块数据
 * @param  u8 ucAddr，块地址
 *         pData，读出的数据，16字节
 * @return 状态值
 *         = MI_OK，成功
 */
char PcdRead ( u8 ucAddr, u8 * pData )
{
        char cStatus;
        u8 uc, ucComMF522Buf [ MAXRLEN ]; 
        u32 ulLen;

        ucComMF522Buf [ 0 ] = PICC_READ;
        ucComMF522Buf [ 1 ] = ucAddr;

        CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

        cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

        if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
        {
                for ( uc = 0; uc < 16; uc ++ )
                * ( pData + uc ) = ucComMF522Buf [ uc ];   
        }
                
        else
        cStatus = MI_ERR;   

        return cStatus;
}


/**
 * @brief  命令卡片进入休眠状态
 * @return 状态值
 *         = MI_OK，成功
 */
char PcdHalt( void )
{
        u8 ucComMF522Buf [ MAXRLEN ]; 
        u32  ulLen;
        ucComMF522Buf [ 0 ] = PICC_HALT;
        ucComMF522Buf [ 1 ] = 0;
        CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
        PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );
        return MI_OK;

}


void IC_CMT ( u8 * UID, u8 * KEY, u8 RW, u8 * Dat )
{
        u8 ucArray_ID [ 4 ] = { 0 };//先后存放IC卡的类型和UID(IC卡序列号)
        PcdRequest ( 0x52, ucArray_ID );//寻卡
        PcdAnticoll ( ucArray_ID );//防冲撞
        PcdSelect ( UID );//选定卡
        PcdAuthState ( 0x60, 0x10, KEY, UID );//校验
        if ( RW )//读写选择，1是读，0是写
                PcdRead ( 0x10, Dat );
        else 
                PcdWrite ( 0x10, Dat );
        PcdHalt ();
}


/**
 *  @brief  扣款和充值
 *  @param  dd_mode[IN]：命令字
 *                 0xC0 = 扣款
 *                 0xC1 = 充值
 *  @param  addr[IN]：钱包地址
 *            pValue[IN]：4字节增(减)值，低位在前
 *  @return 成功返回MI_OK
 */
char PcdValue(u8 dd_mode,u8 addr,u8 *pValue)
{
        char status;
        u32  unLen;
        u8 ucComMF522Buf[MAXRLEN]; 
        //u8 i;

        ucComMF522Buf[0] = dd_mode;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }

        if (status == MI_OK)
        {
        memcpy(ucComMF522Buf, pValue, 4);
        //for (i=0; i<16; i++)
        //{    ucComMF522Buf[i] = *(pValue+i);   }
        CalulateCRC(ucComMF522Buf,4,&ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
                if (status != MI_ERR)
        {    status = MI_OK;    }
        }

        if (status == MI_OK)
        {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]); 

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] &  0x0F) != 0x0A))
        {   status = MI_ERR;   }
        }
        return status;
}
