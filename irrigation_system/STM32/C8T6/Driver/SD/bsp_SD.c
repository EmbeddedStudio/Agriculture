#include "bsp_SD.h"
u8  SD_Type=0;//SD卡的类型

/**
  * @brief  等待SD卡Ready
  * @retval   0：成功       1：失败
  * @note     SD卡准备好会返回0XFF
  * */
u8 SD_WaitReady(void)
{
        u8 r1;
        u16 retry;
        retry = 0;
        do
        {
                r1 = SPI_SentByte(0xFF);
                if(retry==0xfffe)
                {
                    return 1;
                }
        }while(r1!=0xFF);

        return 0;
} 


/**
  * @brief  等待SD卡回应
  * @param  Response 要得到的回应值
  * @retval   0：成功       1：失败
  * */
u8 SD_GetResponse(u8 Response)
{
        u16 Count=0xFFF;//等待次数  
        while ((SPI_SentByte(0XFF)!=Response)&&Count)Count--;//等待得到准确的回应  
        if (Count==0)return MSD_RESPONSE_FAILURE;//得到回应失败   
        else return MSD_RESPONSE_NO_ERROR;//正确回应
}

/**
 * @brief    等待SD卡写入完成
 * @retval   0,成功;
 */
u8 SD_WaitDataReady(void)
{
        u8 r1=MSD_DATA_OTHER_ERROR;
        u32 retry;
        retry=0;
        do
        {
                r1=SPI_SentByte(0xFF)&0X1F;//读到回应
                if(retry==0xfffe)return 1; 
                retry++;
                switch (r1)
                {       
                        case MSD_DATA_OK://数据接收正确了      
                                r1=MSD_DATA_OK;
                        break;  
                        case MSD_DATA_CRC_ERROR:  //CRC校验错误
                                return MSD_DATA_CRC_ERROR;  
                        case MSD_DATA_WRITE_ERROR://数据写入错误
                                return MSD_DATA_WRITE_ERROR;  
                        default://未知错误    
                                r1=MSD_DATA_OTHER_ERROR;
                        break;  
                }   
        }while(r1==MSD_DATA_OTHER_ERROR); //数据错误时一直等待
        retry=0;
        while(SPI_SentByte(0XFF)==0)//读到数据为0,则数据还未写完成
        {
        retry++;
        //delay_us(10);//SD卡写等待需要较长的时间
        if(retry>=0XFFFFFFFE)return 0XFF;//等待失败了
        };      
        return 0;//成功了
}       



/**
  * @brief  选择SD卡，并等待SD卡准备好
  * @retval   0：成功       -1：失败
  * @note     SD卡准备好会返回0XFF
  * */
u8 SD_Select(void)
{
        uint32_t t=0;
        u8 ack;
        SD_CS_LOW();  //片选SD，低电平使能
        do
        {
                if(SPI_SentByte(0XFF)==0XFF)
                {
                        ack=SPI_SentByte(0XFF);
                        printf("ack=%d",ack);
                        return 0;//OK
                }
                t++;
        }while(t<0XFFFF);//等待
//        SD_CS_HIGH();  //释放总线
        return 1;//等待失败
}

/** 
  * @brief   从sd卡读取一个数据包的内容
  * @param   buf：数据缓存数组    
  * @param   len:要读取的数据的长度
  * @retval   0：成功       其他：失败
  * @note   读取时需要等待SD卡发送数据起始令牌0XFE
  */
u8 SD_RecvData(u8*buf,u16 len)
{ 
        u16 Count=0xF000;//等待次数
        while ((SPI_SentByte(0XFF)!=0xFE)&&Count)Count--;//等待得到读取数据令牌0xfe
        if    (Count==0)   return MSD_RESPONSE_FAILURE;//获取令牌失败,返回0XFF      
        while(len--)//开始接收数据
        {
                *buf=SPI_SentByte(0xFF);
                buf++;
        }
        //下面是2个伪CRC（dummy CRC），假装接收了2个CRC
        SPI_SentByte(0xFF);
        SPI_SentByte(0xFF);                     
        return 0;//读取成功
}


/** 
  * @brief   向sd卡写入一个数据包的内容 512字节
  * @param   buf:数据缓存区    cmd:数据发送的令牌 
  * @retval  0：成功       其他：失败
  * @note    写数据时需要先发送数据起始令牌0XFE/0XFC/0XFD
  */
u8 SD_SendBlock(u8*buf,u8 cmd)
{ 
        u32 t,Count=0XFFFF; 
        while ((SPI_SentByte(0XFF)!=0xFF)&&Count)Count--;//等待SD卡准备好
        if    (Count==0)   return MSD_RESPONSE_FAILURE;//SD卡未准备好，失败，返回
        SPI_SentByte(cmd); //发送数据起始或停止令牌
        if(cmd!=0XFD)//在不是结束令牌的情况下，开始发送数据
        {
                for(t=0;t<512;t++) SPI_SentByte(buf[t]);//提高速度,减少函数传参时间
                SPI_SentByte(0xFF);//发送2字节的CRC
                SPI_SentByte(0xFF);
                t=SPI_SentByte(0xFF);//紧跟在CRC之后接收数据写的状态
                if((t&0x1F)!=0x05)return MSD_DATA_WRITE_ERROR;//写入错误                    
        }                           
        return 0;//写入成功
}

/**
 * @brief 读SD卡的一个block
 * @param u32 sector 取地址（sector值，非物理地址）
 * @param u8 *buffer 数据存储地址（大小至少512byte）   
 * @retval 0： 成功
 *       other：失败       
 */
u8 SD_ReadSingleBlock(u32 sector, u8 *buffer)
{
        u8 r1; 
        //设置为高速模式
        SPI_SetSpeed1(SPI_SPEED_4);     
        //如果不是SDHC，给定的是sector地址，将其转换成byte地址
        if(SD_Type!=SD_TYPE_V2HC)
        {
                sector = sector<<9;
        } 
        r1 = SD_SendCmd(CMD17, sector, 0);//读命令     
        if(r1 != 0x00)return r1;        
        r1 = SD_RecvData(buffer, 512);      
        if(r1 != 0)return r1;   //读数据出错！
        else return 0; 
}

/** 
 *  @brief  向sd卡写入一个数据包的内容 512字节
 *  @param  cmd：命令  arg：命令参数  crc：crc校验值及停止位
 *  @retval 返回值:SD卡返回的对应相应命令的响应
 *  @note   响应为R1-R7，见SD协议手册V2.0版（2006）
 */
//u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
//{
//        u8 r1; 
//        u8 Retry=0; 
//        SD_CS_HIGH();//取消上次片选释放总线
//        SPI_SentByte(0xff);  //发命令前先发送8个时钟
//        SD_CS_LOW();
//        //发送
//        SPI_SentByte(cmd | 0x40);//分别写入命令
//        SPI_SentByte(arg >> 24);
//        SPI_SentByte(arg >> 16);
//        SPI_SentByte(arg >> 8);
//        SPI_SentByte(arg);   
//        SPI_SentByte(crc); 
//        //等待响应，或超时退出
//        do
//        {
//               r1 = SPI_SentByte(0xff);
//               Retry++;
//               //根据实验测得，最好重试次数多点
//                printf("r1=%d",r1);
//         } while((r1!=0xff) && Retry<100 );
//        //关闭片选
//        SD_CS_HIGH();
//        //在总线上额外增加8个时钟，让SD卡完成剩下的工作
//        SPI_SentByte(0xff);
//        printf("[%s][%d]\r\n", __func__, __LINE__);
//        //返回状态值
//        return r1;
//}




u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
        u8 r1; 
        u8 Retry=0; 
        SD_CS_HIGH();
        SPI_SentByte(0x00);//高速写命令延时
        SPI_SentByte(0x00);
        SPI_SentByte(0x00); 
        //片选端置低，选中SD卡
        SD_CS_LOW();
        //发送
        SPI_SentByte(cmd | 0x40);//分别写入命令
        SPI_SentByte(arg >> 24);
        SPI_SentByte(arg >> 16);
        SPI_SentByte(arg >> 8);
        SPI_SentByte(arg);
        SPI_SentByte(crc|0x01); 
        //等待响应，或超时退出
        while((r1=SPI_SentByte(0xFF))==0xFF)
        {
                Retry++; 
                if(Retry>100)break; 
        }   
        //关闭片选
        SD_CS_HIGH();
        //在总线上额外增加8个时钟，让SD卡完成剩下的工作
        SPI_SentByte(0x00);
        //返回状态值
        return r1;
}


/**
 * @brief  向SD卡发送一个命令(结束是不失能片选，还有后续数据传来）
 * @param  u8 cmd   命令 
 * @param  u32 arg  命令参数
 * @param  8 crc   crc校验值  
 * @retval SD卡返回的响应  
 */
u8 SD_SendCommand_NoDeassert(u8 cmd, u32 arg, u8 crc)
{
        u8 Retry=0;     
        u8 r1;  
        SPI_SentByte(0xff);//高速写命令延时
        SPI_SentByte(0xff);       
        SPI_CS1_LOW();//片选端置低，选中SD卡 
        //发送
        SPI_SentByte(cmd | 0x40); //分别写入命令
        SPI_SentByte(arg >> 24);
        SPI_SentByte(arg >> 16);
        SPI_SentByte(arg >> 8);
        SPI_SentByte(arg);
        SPI_SentByte(crc);   
        //等待响应，或超时退出
        while((r1=SPI_SentByte(0xFF))==0xFF)
        {
                Retry++; 
                if(Retry>200)break; 
        } 
        //返回响应值
        return r1;
}
/**
 * @brief  把SD卡设置到挂起模式
 * @retval 0,成功设置
 *         1,设置失败
 */
u8 SD_Idle_Sta(void)
{
        u16 i;
        u8 retry;       
        //for(i=0;i<0xf00;i++);//纯延时，等待SD卡上电完成  
        //先产生>74个脉冲，让SD卡自己初始化完成
        //delay_ms(2000);
        printf("[%s][%d]\r\n", __func__, __LINE__);
        for(i=0;i<10;i++)SPI_SentByte(0xFF); 
        //-----------------SD卡复位到idle开始-----------------
        //循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态
        //超时则直接退出
        retry = 0;
        do
        {       
        //发送CMD0，让SD卡进入IDLE状态
        i = SD_SendCmd(CMD0, 0, 0x95);
        retry++;
        }while((i!=0x01)&&(retry<200));
        printf("The SD init ret %d\r\n",i);
        //跳出循环后，检查原因：初始化成功？or 重试超时？
        if(retry==200)return 1; //失败
        return 0;//成功   
}


/** 
 * @brief   获取SD卡的CID信息，包括制造商信息
 * @param   cid_data(存放CID的内存，至少16Byte
 * @retval  0：成功       其他：失败
 * @note    CID寄存器内容详见SD协议手册V2.0版（2006）
 */
u8 SD_GetCID(u8 *cid_data)
{
        printf("[%s][%d]\r\n", __func__, __LINE__);
        u8 r1;    
        //发CMD10命令，读CID
        r1=SD_SendCmd(CMD10,0,0x01);
        if(r1==0x00)
        {
                r1=SD_RecvData(cid_data,16);//接收16个字节的数据  
        }
        SD_CS_HIGH();//取消片选
        if(r1)return 1;
        else return 0;
}

/** 
 * @brief   获取SD卡的CSD信息，包括容量和速度信息
 * @param   cid_data(存放CID的内存，至少16Byte
 * @retval  0：成功       其他：失败
 * @note    CID寄存器内容详见SD协议手册V2.0版（2006）
 */
u8 SD_GetCSD(u8 *csd_data)
{
        u8 r1;  
        r1=SD_SendCmd(CMD9,0,0x01);//发CMD9命令，读CSD 
        if(r1==0)
        {
                r1=SD_RecvData(csd_data,16);//接收16个字节的数据 
        }
        SD_CS_HIGH();//取消片选
        if(r1)return 1;
        else return 0;
} 

/** 
 * @brief   获取SD卡的总扇区数（扇区数
 * @param   cid_data(存放CSD的内存，至少16Byte
 * @retval  0：获取容量出错       其他：SD卡的扇区数量值
 * @note    CID寄存器内容详见SD协议手册V2.0版（2006）
 */
u32 SD_GetSectorCount(void)
{
        u8 csd[16];
        u32 Capacity;  
        u8 n;
        u16 csize;           
        //取CSD信息，如果期间出错，返回0 
        if(SD_GetCSD(csd)!=0) return 0;  //获取容量失败   
        //如果为SDHC卡，按照下面方式计算
        if((csd[0]&0xC0)==0x40)  //V2.00的卡
        { 
                csize = csd[9] + ((u16)csd[8] << 8) + 1;
                Capacity = (u32)csize << 10;//得到扇区数       
        }
        else//V1.XX的卡
        { 
                n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
                Capacity= (u32)csize << (n - 9);//得到扇区数   
        }
        return Capacity;
}

/** 
 * @brief    读SD卡
 * @param    buf:数据缓存区        sector:欲读取的地址      cnt:欲读取的扇区数 
 * @retvsl   0：成功       其他：失败
 * @note     1.读取的地址必须是一个扇区的起始
 *          2.必须是SD2.0卡，其他的卡不处理
 */
u8 SD_ReadDisk(u8*buf,u32 sector,u8 cnt)
{
        u8 r1;
        if(cnt==1)
        {
                r1=SD_SendCmd(CMD17,sector,0X01);//读命令
                if(r1==0)//指令发送成功
                {
                        r1=SD_RecvData(buf,512);//接收512个字节    
                }
        }
        else
        {
                r1=SD_SendCmd(CMD18,sector,0X01);//连续读命令
                do
                {
                        r1=SD_RecvData(buf,512);//接收512个字节  
                        buf+=512;  
                }while(--cnt && r1==0);  
                SD_SendCmd(CMD12,0,0X01); //发送停止命令
        }   
        SD_CS_HIGH();//取消片选
        return r1;//
}

/** 
 * @brief       写SD卡
 * @param       buf:数据缓存区        sector:待写的地址      cnt:待写的扇区数 
 * @retvsl      0：成功       其他：失败
 * @note        1.写的地址必须是一个扇区的起始
 *             2.必须是SD2.0卡，其他的卡不处理
 */
u8 SD_WriteDisk(u8*buf,u32 sector,u8 cnt)
{
        u8 r1;
        if(cnt==1)
        {
                r1=SD_SendCmd(CMD24,sector,0X01);//单个扇区写命令
                if(r1==0)//指令发送成功
                {
                        r1=SD_SendBlock(buf,0xFE);//写512个字节    
                }
        }
        else
        {
                if(SD_Type!=SD_TYPE_MMC)
                {
                        SD_SendCmd(CMD55,0,0X01); 
                        SD_SendCmd(ACMD23,cnt,0X01);//发送待写入的扇区的数量，此命令用来预擦除所有待写入的扇区 
                }
                r1=SD_SendCmd(CMD25,sector,0X01);//连续写命令，发送起始地址
                if(r1==0)
                {
                        do
                        {
                                r1=SD_SendBlock(buf,0xFC);//发送512个字节  
                                buf+=512;  
                        }while(--cnt && r1==0);
                        r1=SD_SendBlock(0,0xFD);//发送停止位
                }
        }   
        SD_CS_HIGH();//取消片选
        return r1;//
}

/**
 * @brief    写SD卡
 * @param    无
 * @retvsl   0：成功       其他：失败
 * @note     1.写的地址必须是一个扇区的起始
 *           2.必须是SD2.0卡，其他的卡不处理
*/
u8 SD_Initialize(void)
{
        u8 r1;      // 存放SD卡的返回值
        u16 retry;  // 用来进行超时计数
        u8 buf[4];  
        u16 i;

        //SPI_SD_Init();  //初始化IO
       // SD_SPI_SpeedLow(); //设置到低速模式 
        SPI_SetSpeed1(SPI_SPEED_256);        
        for(i=0;i<10;i++)SPI_SentByte(0XFF);//发送最少74个脉冲,此时保持片选线是高电平
        retry=20;
        do
        {
                r1=SD_SendCmd(CMD0,0,0x95);//进入复位，同时选中了SPI模式（发送CMD0时，CSN为低电平）
        }while((r1!=0X01) && retry--);
        
        SD_Type=0;//默认无卡
        if(r1==0X01)
        {
                if(SD_SendCmd(CMD8,0x1AA,0x87)==1)//利用V2.0版SD卡特有的命令CMD8检查是否为2.0卡
                {
                        for(i=0;i<4;i++)buf[i]=SPI_SentByte(0XFF); //Get trailing return value of R7 resp
                        if(buf[2]==0X01&&buf[3]==0XAA)//卡是否支持2.7~3.6V
                        {
                                retry=0XFFFE;
                                do
                                {
                                        SD_SendCmd(CMD55,0,0X01); //发送CMD55
                                        r1=SD_SendCmd(ACMD41,0x40000000,0X01);//发送CMD41
                                }while(r1&&retry--);
                                if(retry&&SD_SendCmd(CMD58,0,0X01)==0)//鉴别SD2.0卡版本,读取OCR的值
                                {
                                        for(i=0;i<4;i++)buf[i]=SPI_SentByte(0XFF);//得到OCR值
                                        if(buf[0]&0x40)SD_Type=SD_TYPE_V2HC;    //检查CCS (第30位)
                                        else SD_Type=SD_TYPE_V2;   
                                }
                        }
                }
                else//不是2.0卡的情况下，检查是否为1.0卡或者mmc卡
                {
                        SD_SendCmd(CMD55,0,0X01);  //发送CMD55
                        r1=SD_SendCmd(ACMD41,0,0X01); //发送CMD41
                        if(r1<=1)//发送CMD55和CMD41成功，表示这是1.0卡
                        {  
                                SD_Type=SD_TYPE_V1;
                                retry=0XFFFE;
                                do //等待退出IDLE模式
                                {
                                        SD_SendCmd(CMD55,0,0X01); //发送CMD55
                                        r1=SD_SendCmd(ACMD41,0,0X01);//发送CMD41 进行初始化
                                }while(r1&&retry--);
                        }
                        else //不是1.0卡，则考虑是MMC卡
                        {
                                SD_Type=SD_TYPE_MMC;//先假设是MMC卡
                                retry=0XFFFE;
                                do //等待退出IDLE模式
                                {               
                                        r1=SD_SendCmd(CMD1,0,0X01);//发送CMD1，利用复位功能判断是否为MMC卡
                                }while(r1&&retry--); //发送复位命令，超时则复位失败 
                        }
                if(retry==0||SD_SendCmd(CMD16,512,0X01)!=0) 
                        SD_Type=SD_TYPE_ERR;//MMC卡复位失败
                }
        }
        SD_CS_HIGH();//取消片选
        if(SD_Type)return 0; //初始化成功
        else if(r1)return r1; //初始化失败    
        return 0xaa;//其他错误
}


//获取SD卡的容量（字节）   
//返回值:0： 取容量出错 
//       其他:SD卡的容量(字节)  
u32 SD_GetCapacity(void)
{
        u8 csd[16];
        u32 Capacity;
        u8 r1;
        u16 i;
        u16 temp;       
        //取CSD信息，如果期间出错，返回0
        if(SD_GetCSD(csd)!=0) 
        {
                printf("[%s][%d]\r\n", __func__, __LINE__);
                return 0;
        }                
        //如果为SDHC卡，按照下面方式计算
        if((csd[0]&0xC0)==0x40)
        {       
                Capacity=((u32)csd[8])<<8;
                Capacity+=(u32)csd[9]+1;        
                Capacity = (Capacity)*1024;//得到扇区数
                Capacity*=512;//得到字节数   
        }
        else
        {       
                i = csd[6]&0x03;
                i<<=8;
                i += csd[7];
                i<<=2;
                i += ((csd[8]&0xc0)>>6);
                //C_SIZE_MULT
                r1 = csd[9]&0x03;
                r1<<=1;
                r1 += ((csd[10]&0x80)>>7);      
                r1+=2;//BLOCKNR
                temp = 1;
                while(r1)
                {
                        temp*=2;
                        r1--;
                }
                Capacity = ((u32)(i+1))*((u32)temp);    
                // READ_BL_LEN
                i = csd[5]&0x0f;
                //BLOCK_LEN
                temp = 1;
                while(i)
                {
                        temp*=2;
                        i--;
                }
                //The final result
                Capacity *= (u32)temp;//字节为单位   
        }
        return (u32)Capacity;
}

u8 SD_Init(void)
{
        printf("[%s][%d]\r\n", __func__, __LINE__);
        u8 i;
        u8 r1;      // 存放SD卡的返回值
        u16 retry;  // 用来进行超时计数
        u8 buff[6];
        SPI_SetSpeed1(SPI_SPEED_256);
        SD_CS_HIGH();
        
        delay_ms(100);
        for(i=0;i<10;i++)  //发送大于74个时钟周期
        {
                SPI_SentByte(0xFF); //80clks
        }
        /********************SD卡复位到idle状态******************/
        retry=0;
        do 
        {
                printf("[%s][%d]\r\n", __func__, __LINE__);
                r1=SD_SendCmd(CMD0,0,0x95);
                retry++;
                printf("r1=%d\n",r1);
                
        }while ( (r1!=0x01) && (retry<100) );
        if(retry==100)
        {
                printf("[%s][%d]\r\n", __func__, __LINE__);
                return 1;
        }
        /*****************SD卡复位到idle结束**********************/
        r1 = SD_SendCommand_NoDeassert(CMD8,0x1aa,0x87);
        if(r1==0x05)  
        {
                SD_Type = SD_TYPE_V1;
                //如果是V1.0卡，CMD8指令后没有后续数据
                //片选置高，结束本次命令
                SD_CS_HIGH();
                SPI_SentByte(0xff);
                retry = 0;
                do
                {
                        //先发CMD55，应返回0x01，否则出错
                       r1 = SD_SendCmd(CMD55,0,0);
                       if(r1 !=0x01)
                       return r1;
                       //得到正确响应后，发ACMD41，应得到返回值0x00，佛则重试400次
                       r1 = SD_SendCmd(ACMD41,0,0);
                       retry++;
                 }while((r1!=0x00)&&(retry<400));
                      //判断是超时还是得到正确回应
                      // 若有回应：是SD卡：没有回应：是MMC卡
                if(retry==400)
                {
                 retry =0;
                     //发送MMC卡初始化命令(没有测试)
                     do
                      {
                         r1=SD_SendCmd(CMD1,0,0);
                         retry++;
                       }while((r1!=0x00)&&(retry<400));
                       if(retry==400)
                       {
                                //写入卡类型
                                SD_Type=SD_TYPE_MMC;
                               return 1;//MMC卡初始化超时
                       }
                 }
                       //----------MMC卡额外初始化操作结束---------------
                       //设置SPI为高速模式
                       SPI_SetSpeed1(SPI_SPEED_4);
                       SPI_SentByte(0xff);

                       //禁止CRC校验
                       r1=SD_SendCmd(CMD59,0,0x95);
                       if(r1!=0x00)return r1;//命令错误，返回r1
                       //-------------SD卡、MMC卡初始化结束-------------

           }//SD卡为V1.0版本的初始化结束
                   //下面是V2.0卡的初始化
                       //其中需要读取OCR数据，判断是SD2.0还是SD2.0HC
        else if(r1==0x01)
        {
                //v2.0的卡，CMD8命令后会传回4字节的数据，要跳过在结束本命令
                buff[0]=SPI_SentByte(0xff);//shoule be 0x00
                buff[1]=SPI_SentByte(0xff);//shoule be 0x00
                buff[2]=SPI_SentByte(0xff);//shoule be 0x11
                SD_CS_HIGH();
                SPI_SentByte(0xff);
                if(buff[2]==0x01&&buff[3]==0xaa)
                {
                        retry = 0;
                        do
                        {
                                retry++;
                                r1=SD_SendCmd(CMD55,0,0);
                                if(r1!=0x01)return r1;
                                r1=SD_SendCmd(ACMD41,0x40000000,1);
                                if(retry>200)return r1;
                        }while(r1!=0);
                        r1=SD_SendCommand_NoDeassert(CMD58,0,0);
                        if(r1!=0x00)return r1;
                        //读OCR指令发出后，紧接着是4字节的OCR信息
                          buff[0]=SPI_SentByte(0xff);
                          buff[1]=SPI_SentByte(0xff);
                          buff[2]=SPI_SentByte(0xff);
                          buff[3]=SPI_SentByte(0xff);
                        //OCR接收完成，片选置高
                        SD_CS_HIGH();
                        SPI_SentByte(0xff);
                }
                        
                  if(buff[0]&0x40)
                          SD_Type = SD_TYPE_V2HC;//检查CCS
                  else 
                          SD_Type=SD_TYPE_V2;
                  //------------------鉴别SD2.0卡版本结束------------------
                  //设置SPI为高速模式
                  SPI_SetSpeed1(SPI_SPEED_4);
         }
                 return r1;
}
                
       
        
        
   
        

