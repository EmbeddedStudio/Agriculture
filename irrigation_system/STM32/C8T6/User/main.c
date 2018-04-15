#include "main.h"

const uint32_t Administrator_ID = 0x9276381B;           //管理员卡号
const uint32_t Initial=0x00000000;                      //默认没刷到卡的返回值

u8 Temp_flag = 1 ;                      //是否开启自动调温系统
u8 Show_flag = 0;
u8 BEEP_Flag = 0 ;
u8 Irrigation_flag=0;   //喷灌标志
u8 Exhaust_flag=0;      //排风标志
u8 Administ_Flag=0;            //管理员标志，用于开始计时三秒后如果还是管理员才进入录入模式
u8 Administ_Entering=0;        //真正确定是否进入录入模式的标志
float Illumination=0;                   //光强值
u8 anomaly = 0;           //异常种类 0未关门 1未知卡刷卡  2温度过高
char temp[20];                  //装载字符串用于屏幕显示
char Humi[20];
char lighting[20];
char Blank[16]="               ";   //15个空格 不能少
u32 ID_Card[256];                       //用于存储录入的卡号
u8 Step=0;                      //录卡的步骤
u8 Mode;
u8 Door_Flag=2;         //0默认  不给方波   1开门  2关门


int main(void)
{
        System_Init();                  //系统初始化
        printf("[%s][%d]\r\n", __func__, __LINE__);
        
        /*************************局部变量的定义**************************/
        char cCard_str[20];
        u8 Entering_Flag=1;
        uint32_t New_Card =0x22222222;      //新的卡号，用于临时存储将要录入的卡号
        uint32_t New_Card1=0x88888888;      //新的卡号，用于临时存储将要录入的卡号
        uint32_t New_Card2=0x66666666;      //新的卡号，用于临时存储将要录入的卡号
//              The_System_First_Run();                //函数慎用
        uint32_t *p_Pos= (uint32_t *)(Pos_Address);
        uint32_t  Card_Pos=*p_Pos;
        uint32_t *p_Card;
        u8 i;           //用于加载钥匙库的循环变量以及遍历的循环变量

        u8 Temperature_Max=25.00;          //高温预警的阈值
        
        //把 flash 里面的 ID 号加载到内存中来
        printf("\r\n现在有%d个卡号\r\n",*p_Pos);
        for(i=0;i<Card_Pos;i++)
        {
                p_Card = (uint32_t *)(Card_Address+i*4);
                ID_Card[i]=*p_Card;
                printf("ID为%8X\r\n",ID_Card[i]);
        }
        
        Mode = General_Mode;
        while(1)
        {
                if(Temp_int>Temperature_Max)
                {
                        Mode=Abnormal_Mode;
                        anomaly=2;
                }
                New_Card=RFID_Number();
                //从内存里查询是否有这个卡号
                for(i=0;i<Card_Pos;i++)
                {
                        if(New_Card==ID_Card[i])
                        {
                                Door_Flag=1;
                                Door_Time=0;
                                if(Mode!=Entering_Mode)
                                {
                                        BEEP_OFF;
                                        Mode=General_Mode;
                                        BEEP_Flag=0;
                                }
                                break;
                        }
                        if(i==Card_Pos-1 && New_Card!=Initial && Mode!=Entering_Mode)
                        {
                                printf("[%s][%d]\r\n", __func__, __LINE__);
                                //查询后未找到卡确定为未知卡刷卡则报警
                                Mode=Abnormal_Mode;
                                Door_Flag=2;
                                Door_Time=0;
                                BEEP_Flag=1;
                                BEEP_Time=0;
                                anomaly=1;
                        }
                }
                //普通模式下的管理员开门并且开始计时
                
                if( (New_Card == Administrator_ID) && (Mode==General_Mode) && Administ_Entering==0 )
                {
                        Administ_Flag=1;             //标志位置1让定时器开始计时
                        
                }
                //5秒时间到   并且步骤卡号模式都正确那么将进入录入模式
                if( (Administ_Entering!=0) && Step==0 &&  (Administrator_ID == New_Card)  && (Mode==General_Mode) )
                {
                        Administ_Entering=0;
                        Administ_Time=0;                 //进入录入模式之后将Administ_Entering关闭
                        Step=1;                          //同时计时清零  狡兔死走狗烹
                        Mode=Entering_Mode;
                        delay_ms(1000);
                        printf("开始录入新的卡号\r\n");
                        OLED_ShowStr(0,0,Blank,2);         //16个空格
                        OLED_ShowStr(0,1,Blank,2);
                        OLED_ShowStr(0,2,"    ",2);
                        OLED_ShowCN(32,2,4,begin_card1);
                        OLED_ShowStr(97,2,"    ",2);
                        OLED_ShowStr(0,4,"    ",2);
                        OLED_ShowCN(32,4,4,begin_card2);
                        OLED_ShowStr(97,4,"    ",2);
                        OLED_ShowStr(0,6,Blank,2);
                        OLED_ShowStr(0,7,Blank,2);
                }
                switch(Mode)
                {
                        case Abnormal_Mode:             //异常模式
                        
                        if(BEEP_Flag!=0)        //蜂鸣器响30秒后停止
                        {                       
                                BEEP_ON;
                        }
                        else 
                        {
                                BEEP_OFF;
                        }
                                //屏幕持续显示异常原因直到正常为止
                        
                        OLED_ShowStr(0,0,Blank,2);         //16个空格
                        OLED_ShowStr(0,1,Blank,2);
//                        OLED_ShowStr(0,2,Blank,2);
//                        OLED_ShowStr(0,3,Blank,2);
//                        OLED_ShowStr(0,4,Blank,2);
                        OLED_ShowStr(0,5,Blank,2);
                        OLED_ShowStr(0,6,Blank,2);
                        OLED_ShowStr(0,7,Blank,2);
//                        OLED_ShowStr(0,3,"      ",2);
//                        OLED_ShowCN(48,3,2,abnormal);           //异常前面后面写空格
//                        OLED_ShowStr(81,3,"      ",2);
                        switch(anomaly)
                        {
                                case 0  :       // 未关门
                                        OLED_ShowStr(0,3,"    ",2);
                                        OLED_ShowStr(97,3,"   ",2);
                                        OLED_ShowCN(32,3,4,Nodoor);
                                        break;
                                case 1  :       // 未知卡刷卡
                                        OLED_ShowStr(0,3,"    ",2);
                                        OLED_ShowStr(97,3,"   ",2);
                                        OLED_ShowCN(32,3,2,NFC);
                                        OLED_ShowCN(64,3,2,fail);
                                        break;
                                case 2  :       // 高温预警
                                        OLED_ShowStr(0,3,"    ",2);
                                        OLED_ShowStr(97,3,"   ",2);
                                        OLED_ShowCN(32,3,4,High_Temperature);
                                        break;
                                default :
                                        break;
                        }
                        break ; 
                        
                        case Entering_Mode:             //录入模式
                                        
                                        if(Step==1 && New_Card!=Administrator_ID && New_Card!=Initial  )
                                        {
                                                New_Card1=New_Card;
                                                Step=2;
                                                printf("New_Card1:%08X  ",New_Card1);
                                                sprintf(cCard_str,"    %08X    ",New_Card1);
                                                OLED_ShowCN(32,1,2,NFC);
                                                OLED_ShowCN(64,1,2,successful);
                                                OLED_ShowStr(0,3,cCard_str,2);
                                                OLED_ShowStr(0,5,"     ",2);
                                                OLED_ShowStr(89,5,"    ",2);
                                                OLED_ShowCN(40,5,3,please);
                                                
                                        }
                                        if(New_Card==Administrator_ID && Step==2)
                                        {
                                                Step=3;
                                                OLED_ShowStr(0,0,Blank,2);
                                                OLED_ShowStr(0,1,Blank,2);
                                                OLED_ShowStr(0,2,Blank,2);
                                                OLED_ShowStr(0,5,Blank,2);
                                                OLED_ShowStr(0,6,Blank,2);
                                                OLED_ShowCN(24,3,5,pleaseagain);
                                                
                                        }
                                        if( Step==3 && New_Card!=Administrator_ID && New_Card!=Initial )
                                        {
                                                New_Card2=New_Card;
                                                Step=4;
                                        }

                                        if( Step==4 && New_Card1==New_Card2 )
                                        {
                                               
                                                for (i=0;i<Card_Pos;i++)
                                                {
                                                        if(New_Card2==ID_Card[i])
                                                        {
                                                                printf("此卡已录入\r\n");
                                                                OLED_ShowCN(24,3,5,card_already);
                                                                delay_ms(500);
                                                                Entering_Flag=0;
                                                                break;
                                                        }
                                                }
                                                if(Entering_Flag!=0)
                                                {
                                                        ID_Card[Card_Pos]=New_Card2;
                                                        FLASH_Unlock();
                                                        FLASH_ProgramWord(Card_Address+Card_Pos*4,New_Card2);
                                                        FLASH_ErasePage(Pos_Address);
                                                        FLASH_ProgramWord(Pos_Address,Card_Pos+1);
                                                        FLASH_Lock();
                                                        Card_Pos=Card_Pos+1;
                                                        printf("录入成功\r\n");
                                                        printf("新的卡号为：%08X\r\n",New_Card2);
//                                                        OLED_ShowStr(0,3,Blank,2);
                                                        OLED_ShowStr(0,2,"    ",2);
                                                        OLED_ShowStr(0,97,"   ",2);
                                                        OLED_ShowCN(32,2,2,enter);
                                                        OLED_ShowCN(64,2,2,successful);
                                                        OLED_ShowStr(0,4,cCard_str,2);
                                                        
                                                }
                                                Entering_Flag=1;
                                                Step=0;
                                                Mode= General_Mode;
                                        }
                                        if(New_Card1!=New_Card2 && Step==4)
                                        {
                                                printf("两张卡不一致\r\n");
                                                OLED_ShowCN(16,3,6,inconformity);
                                                delay_ms(500);
                                                Step=0;
                                                Mode= General_Mode;
                                        }
                        break ;
                        
                        case General_Mode:              //普通模式
                                
                                if(Temp_flag!=0)
                                {
                                        Temperature_System();   //调用恒温系统执行函数
                                }
                                
                                switch(Door_Flag)   //0默认  不给方波   1开门  2关门
                                {
                                        case 1:
                                                TIM3->CCR3=15;    //90°开门
                                                break;
                                        case 2:
                                                TIM3->CCR3=5;    //0°关门
                                                delay_ms(1000);  //给舵机回位留一定的时间
                                                Door_Flag=0;
                                                break;
                                        default:
                                                TIM3->CCR3=0;    //防止抖动
                                                break;
                                }
                                //调光   如果光强小于一定的时候就开灯
                                if( Illumination < light_Min)
                                {
                                        Water_ON;
                                }
                                if( Illumination > light_Max)
                                {
                                        Water_OFF;
                                }
                                
                                if(Show_flag!=0)
                                {
//                                        OLED_Fill(0x00);
                                        sprintf(temp,"%d.%d     ",Temp_int,Temp_deci);
                                        //DHT11的湿度
//                                     sprintf(Humi,"%d.%d",Humi_int,Humi_deci);
                                        //土壤的湿度
                                        sprintf(Humi,"%0.2f     ",Soil_Humidity);  //5个空格不能少
                                        sprintf(lighting,"%0.2f     ",Illumination);
                                        OLED_ShowStr(0,0,"    ",2);     //4个空格
                                        OLED_ShowStr(97,0,"   ",2);     //3个空格
                                        
                                        OLED_ShowCN(32,0,4,title);      //显示标题----智慧农业
                                        OLED_ShowCN(0,2,3,temp_code);   //显示温度
                                        OLED_ShowCN(0,4,3,Humi_code);   //显示湿度
                                        OLED_ShowCN(0,6,3,light);       //显示光强
                                        OLED_ShowStr(49,2,temp,2);
//                                        OLED_ShowStr(113,2,"  ",2);
//                                        OLED_ShowStr(113,3,"  ",2);
                                        OLED_ShowStr(49,4,Humi,2);
                                        OLED_ShowStr(49,6,lighting,2);
                                }
                                else
                                {
                                        
                                        OLED_ShowStr(0,0,"    ",2);     //4个空格
                                        OLED_ShowStr(97,0,"   ",2);     //3个空格
                                        OLED_ShowCN(32,0,4,title);      //显示标题----智慧农业
                                        OLED_ShowCN(0,2,2,Air_Temp);
                                        OLED_ShowCN(0,4,2,Irrigation);
//                                        OLED_ShowStr(113,2,"  ",2);
//                                        OLED_ShowStr(113,3,"  ",2);
                                        OLED_ShowCN(0,6,2,Exhaust);
                                        OLED_ShowCN(33,2,3,System);
                                        OLED_ShowCN(33,4,3,System);
                                        OLED_ShowCN(33,6,3,System);
                                        if(Temp_flag!=0)
                                        {
                                                OLED_ShowCN(81,2,2,Open);
                                        }
                                        else
                                        {
                                                OLED_ShowCN(81,2,2,Close);
                                        }
                                        if(Irrigation_flag!=0)
                                        {
                                                OLED_ShowCN(81,4,2,Open);
                                        }
                                        else
                                        {
                                                OLED_ShowCN(81,4,2,Close);
                                        }
                                        if(Exhaust_flag!=0)
                                        {
                                                OLED_ShowCN(81,6,2,Open);
                                        }
                                        else
                                        {
                                                OLED_ShowCN(81,6,2,Close);
                                        }
                                
                                }
                        break ;
                        default :
                                break;
                }
        }
}



void  Temperature_System (void)
{
        if(Temp_flag!=0)
        {
                if( Temperature > temp_max )
                {
                                 //关闭太阳灯并开启水阀进行降温
                        TemDown_ON;
                }
                else if ( Temperature < ((temp_max+temp_min)/2)  )
                {
                        TemDown_OFF;
                }
                else if ( Temperature > ((temp_max+temp_min)/2)  )
                {
                        LED_OFF ;
                }
                else if ( Temperature < temp_min )
                {
                              //关闭水阀并开启太阳灯进行升温
                        LED_ON;
                }
        }
}
void System_Init(void)
{
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        systick_init();
        usart_init();     //usart1
        DHT11_Init();
        USART_Config();    //usart2
        LED_GPIO_Config();
        BASIC_TIM_Init();
        GENERAL_TIM_Init();
        I2C_init();
        OLED_Init();
        TSL2561_Init();
        spi_Init();
        ADC1_Init();
        RC522_Init();
}

uint32_t RFID_Number ( void )
{
        u8 Array_ID [ 4 ];                  //先后存放IC卡的类型和UID(IC卡序列号)
        u8 Status,i;
        u32 Card_Number=Initial;
        if ( ( Status = PcdRequest ( PICC_REQALL, Array_ID ) ) != MI_OK )       //寻卡
        {
                Status = PcdRequest ( PICC_REQALL, Array_ID );
        }
        if(Status==MI_OK)
        {
                if ( PcdAnticoll ( Array_ID ) == MI_OK )
                {
                        for(i=0;i<4;i++)
                        Card_Number=(Card_Number<<8)|Array_ID[i];
                        printf("card:%08X\r\n",Card_Number);
                }
        }
        return Card_Number;
}

void The_System_First_Run( void )  //芯片首次运行时需要先屏蔽所有的代码先执行此函数，在flash 里面写入相应的数据
{
        uint32_t * P;
        FLASH_Unlock();                 //此函数一旦运用原来录入的钥匙号将会被删除   慎用
        FLASH_ErasePage(Card_Address);
        FLASH_ErasePage(Pos_Address);
        FLASH_ProgramWord(Card_Address,Administrator_ID);
        FLASH_ProgramWord(Pos_Address,0x00000001);
        FLASH_Lock();
        P = (uint32_t *)(Card_Address);
        printf("\r\n管理员卡号为:%8X",*P);
        P = (uint32_t *)(Pos_Address);
        printf("\r\n当前第一个空地址为:%d",*P);
}









/*  json 格式封装形式  */
//{"Temperature":"23.00","Humidity":"59.00","CO2":"21.6","PH":"4.6","Illumination":"6.5"}
//Message: {"method":"setMotor1","params":false}
//Message: {"method":"setCarPos","params":"73"}
//Message: {"method":"setHydrovalve1","params":false}
//                printf("[%s][%d]", __func__, __LINE__);
//setMotor1_true
//setHydrovalve1_false
