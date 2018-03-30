#ifndef TSL2561_H
#define TSL2561_H
#include "sys.h"
/*
#define uchar unsigned char
//ÃüÁî¿ØÖÆ¼Ä´æÆ÷ 
#define CONTROL  0x80  
#define TIMING  0x81 
#define THRESHLOWLOW  0x82 
#define THRESHLOWHIGH 0x83 
#define THRESHHIGHLOW  0x84 
#define THRESHHIGHHIGH  0x85 
#define INTERRUPT  0x86 
#define ID  0x8A 
#define DATA0LOW  0x8C 
#define DATA0HIGH 0x8D 
#define DATA1LOW  0x8E 
#define DATA1HIGH 0x8F
*/


#define uchar    unsigned char
#define uint8_t  unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long

//Éè±¸¶ÁÐ´µØÖ·
#define SLAVE_ADDR_WR 0x72
#define SLAVE_ADDR_RD 0x73

//ÃüÁî¿ØÖÆ¼Ä´æÆ÷
#define CONTROL 	0x80
#define TIMING 		0x81
#define THRESHLOWLOW  0xa2           //ÓÃ×ÖÐ´£¬¶ø²»ÓÃ×Ö½Ú
#define THRESHLOWHIGH 0xa3 
#define THRESHHIGHLOW  0xa4 
#define THRESHHIGHHIGH  0xa5 
#define INTERRUPT  0xC6 
#define ID  0x8A 
#define DATA0LOW 	0x8C
#define DATA0HIGH 	0x8D
#define DATA1LOW 	0x8E
#define DATA1HIGH 	0x8F


#define TSL2561_CONTROL_POWERON   0x03
#define TSL2561_CONTROL_POWEROFF  0x00

#define TSL2561_LUX_LUXSCALE      14      // Scale by 2^14
#define TSL2561_LUX_RATIOSCALE    9       // Scale ratio by 2^9
#define TSL2561_LUX_CHSCALE       10      // Scale channel values by 2^10
#define TSL2561_LUX_CHSCALE_TINT0 0x7517  // 322/11 * 2^TSL2561_LUX_CHSCALE
#define TSL2561_LUX_CHSCALE_TINT1 0x0FE7  // 322/81 * 2^TSL2561_LUX_CHSCALE


#define TSL2561_LUX_K1T           0x0040  // 0.125 * 2^RATIO_SCALE
#define TSL2561_LUX_B1T           0x01f2  // 0.0304 * 2^LUX_SCALE
#define TSL2561_LUX_M1T           0x01be  // 0.0272 * 2^LUX_SCALE
#define TSL2561_LUX_K2T           0x0080  // 0.250 * 2^RATIO_SCALE
#define TSL2561_LUX_B2T           0x0214  // 0.0325 * 2^LUX_SCALE
#define TSL2561_LUX_M2T           0x02d1  // 0.0440 * 2^LUX_SCALE
#define TSL2561_LUX_K3T           0x00c0  // 0.375 * 2^RATIO_SCALE
#define TSL2561_LUX_B3T           0x023f  // 0.0351 * 2^LUX_SCALE
#define TSL2561_LUX_M3T           0x037b  // 0.0544 * 2^LUX_SCALE
#define TSL2561_LUX_K4T           0x0100  // 0.50 * 2^RATIO_SCALE
#define TSL2561_LUX_B4T           0x0270  // 0.0381 * 2^LUX_SCALE
#define TSL2561_LUX_M4T           0x03fe  // 0.0624 * 2^LUX_SCALE
#define TSL2561_LUX_K5T           0x0138  // 0.61 * 2^RATIO_SCALE
#define TSL2561_LUX_B5T           0x016f  // 0.0224 * 2^LUX_SCALE
#define TSL2561_LUX_M5T           0x01fc  // 0.0310 * 2^LUX_SCALE
#define TSL2561_LUX_K6T           0x019a  // 0.80 * 2^RATIO_SCALE
#define TSL2561_LUX_B6T           0x00d2  // 0.0128 * 2^LUX_SCALE
#define TSL2561_LUX_M6T           0x00fb  // 0.0153 * 2^LUX_SCALE
#define TSL2561_LUX_K7T           0x029a  // 1.3 * 2^RATIO_SCALE
#define TSL2561_LUX_B7T           0x0018  // 0.00146 * 2^LUX_SCALE
#define TSL2561_LUX_M7T           0x0012  // 0.00112 * 2^LUX_SCALE
#define TSL2561_LUX_K8T           0x029a  // 1.3 * 2^RATIO_SCALE
#define TSL2561_LUX_B8T           0x0000  // 0.000 * 2^LUX_SCALE
#define TSL2561_LUX_M8T           0x0000  // 0.000 * 2^LUX_SCALE

#define  TSL2561_INTEGRATIONTIME_13MS    0x00    // 13.7ms
#define  TSL2561_INTEGRATIONTIME_101MS   0x01    // 101ms
#define  TSL2561_INTEGRATIONTIME_402MS   0x02    // 402ms

#define  TSL2561_GAIN_0X   0x00    // No gain
#define  TSL2561_GAIN_16X  0x10    // 16x gain





void tsl2561_Init(void);
void TSL2561_Write(uchar command,uchar date);
uchar TSL2561_Read(uchar command);
uint32_t Read_Light(void);
uint32_t calculateLux(uint16_t ch0, uint16_t ch1);
u8 IICwriteBytes(u8 dev, u8 reg, u8 length, u8* data);
void send_guangqiang(void);


#endif

