
#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"
/*===============================================================*/
/*Write / Read Register Definition===============================*/
#define OV7670_WRITE_ADDR 0x42
#define OV7670_READ_ADDR  0x43


/*===============================================================*/
/*F103RB Pin Definition==========================================*/       

#define OV7670_SCL_GPIO_Port      GPIOB         //should be set Pull-up
#define OV7670_SCL_Pin            GPIO_PIN_6
#define OV7670_SDA_GPIO_Port      GPIOB         //should be set Pull-up
#define OV7670_SDA_Pin            GPIO_PIN_7

#define OV7670_VSYNC_GPIO_Port    GPIOC        //Indicating a start of a new frame
#define OV7670_VSYNC_Pin          GPIO_PIN_5
#define OV7670_HREF_GPIO_Port     GPIOB        //Indicating available pixel data in a line
#define OV7670_HREF_Pin           GPIO_PIN_12 
#define OV7670_PLCK_GPIO_Port     GPIOB        //Synchronizing data output
#define OV7670_PCLK_Pin           GPIO_PIN_13
#define OV7670_XCLK_GPIO_Port     GPIOA        //Requring 24MHz external clock signal
#define OV7670_XCLK_Pin           GPIO_PIN_1
#define OV7670_RESET_GPIO_Port
#define OV7670_RESET_Pin

/*===============================================================*/
/*F103RB Data Pin Definition=====================================*/ 
#define OV7670_D0_GPIO_Port       GPIOA
#define OV7670_D0_Pin             GPIO_PIN_4
#define OV7670_D1_GPIO_Port       GPIOB
#define OV7670_D1_Pin             GPIO_PIN_3
#define OV7670_D2_GPIO_Port       GPIOB
#define OV7670_D2_Pin             GPIO_PIN_0
#define OV7670_D3_GPIO_Port       GPIOA
#define OV7670_D3_Pin             GPIO_PIN_10
#define OV7670_D4_GPIO_Port       GPIOC
#define OV7670_D4_Pin             GPIO_PIN_1
#define OV7670_D5_GPIO_Port       GPIOA
#define OV7670_D5_Pin             GPIO_PIN_2
#define OV7670_D6_GPIO_Port       GPIOC
#define OV7670_D6_Pin             GPIO_PIN_0
#define OV7670_D7_GPIO_Port       GPIOA
#define OV7670_D7_Pin             GPIO_PIN_3
#define OV7670_PWDN_GPIO_Port     GPIOA
#define OV7670_PWDN_Pin           GPIO_PIN_15


#define ENABLE  1
#define DISABLE 0
/*===============================================================*/ 
/* OV7670 Register Definition====================================*/                                   
/* For all Registers Enable/Disable, ENABLE=1 DISABLE=0==========*/
/* Device slave addresses are 0x42 for write 0x43 for read=======*/
/* RSVD can be ignored because it means That register is reserved*/
#define REG_GAIN                0x00 //default 00 AGC-Gain control gain setting
#define REG_BLUE                0x01 //default 80 AWB-Blue channel gain setting
#define REG_RED                 0x02 //default 80 AWB-Red channel gain setting
#define REG_VREF                0x03 //default 00 Vertical Frame Control
#define REG_COM1                0x04 //default 00 Common Control 1
#define REG_BAVE                0x05 //default 00 U/B Average Level
#define REG_GbAVE               0x06 //default 00 Y/Gb Average Level
#define REG_AECHH               0x07 //default 00 Exposure Value - AEC MSB 5bits
#define REG_RAVE                0x08 //default 00 V/R Average Level
#define REG_COM2                0x09 //default 01 Common Control 2
#define REG_PID                 0x0A //default 76 Product ID Number MSB
#define REG_VER                 0x0B //default 73 Product ID Number LSB
#define REG_COM3                0x0C //default 00 Common Control 3
#define REG_COM4                0x0D //default 00 Common Control 4
#define REG_COM5                0x0E //default 01 Common Control 5
#define REG_COM6                0x0F //default 43 Common Control 6
#define REG_AECH                0x10 //default 40 Exposure Value
#define REG_CLKRC               0x11 //default 80 Internal Clock
#define REG_COM7                0x12 //default 00 Common Control 7
#define REG_COM8                0x13 //default 8F Common Control 8
#define REG_COM9                0x14 //default 4A Common Control 9
#define REG_COM10               0x15 //default 00 Common Control 10
#define REG_HSTART              0x17 //default 11 Output Format - Horizontal Frame start high 8-bit
#define REG_HSTOP               0x18 //default 61 output Format - Horizontal Frame end high 8-bit
#define REG_VSTART              0x19 //default 03 Output Format - Vertical Frame start high 8-bit
#define REG_VSTOP               0x1A //default 7B Output Format - Vertical Frame end high 8-bit
#define REG_PSHFT               0x1B //default 00 Data Format - Pixel Delay Select
#define REG_MIDH                0x1C //default 7F Manufacturer ID Byte - High
#define REG_MIDL                0x1D //default A2 Manufacturer ID Byte - Low
#define REG_MVFP                0x1E //default 01 Mirror/VFlip Enable
#define REG_LAEC                0x1F //default 00 Reserved
#define REG_ADCCTR0             0x20 //default 04 ADC Control
#define REG_ADCCTR1             0x21 //default 02 Reserved
#define REG_ADCCTR2             0x22 //default 01 Reserved
#define REG_ADCCTR3             0x23 //default 00 Reserved
#define REG_AEW                 0x24 //default 75 AGC/AEC - Stable Operating Region (Upper limit)
#define REG_AEB                 0x25 //default 63 AGC/AEC - Stable Operating Region (Lower limit)
#define REG_VPT                 0x26 //default D4 AGC/AEC Fast Mode Operating Region
#define REG_BBIAS               0x27 //default 80 B Channel Signal Output bias (effective only when COM6[3] = 1)
#define REG_GbBIAS              0x28 //default 80 Gb Channel Signal Output bias (effective only when COM6[3] = 1)
#define REG_EXHCH               0x2A //default 00 Dummy Pixel Insert MSB 
#define REG_EXHCL               0x2B //default 00 Dummy Pixel Insert LSB
#define REG_RBIAS               0x2C //default 80 R Channel Signal Output bias (effective only when COM6[3] = 1)
#define REG_ADVFL               0x2D //default 00 LSB of Insert Dummy Lines in Vertical Direction
#define REG_ADVFH               0x2E //default 00 MSB of Insert Dummy Lines in Vertical Direction
#define REG_YAVE                0x2F //default 00 Y/G Channel Average Value
#define REG_HSYST               0x30 //default 08 HSYNC Rising Edge Delay
#define REG_HSYEN               0x31 //default 30 HSYNC Falling Edge Delay
#define REG_HREF                0x32 //default 80 HREF Control
#define REG_CHLF                0x33 //default 08 Array Current Control
#define REG_ARBLM               0x34 //default 11 Array Reference Control
#define REG_ADC                 0x37 //default 3F ADC Control
#define REG_ACOM                0x38 //default 01 ADC and Analog Common Mode Control
#define REG_OFON                0x39 //default 00 ADC Offset Control
#define REG_TSLB                0x3A //default 0D Line Buffer Test Option
#define REG_COM11               0x3B //default 00 Common Control 11
#define REG_COM12               0x3C //default 68 Common Control 12
#define REG_COM13               0x3D //default 88 Common Control 13
#define REG_COM14               0x3E //default 00 Common Control 14
#define REG_EDGE                0x3F //default 00 Edge Enhancement Adjustment
#define REG_COM15               0x40 //default C0 Common Control 15
#define REG_COM16               0x41 //default 08 Common Control 16
#define REG_COM17               0x42 //default 00 Common Control 17
#define REG_AWBC1               0x43 //default 14 Reserved
#define REG_AWBC2               0x44 //default F0 Reserved
#define REG_AWBC3               0x45 //default 45 Reserved
#define REG_AWBC4               0x46 //default 61 Reserved
#define REG_AWBC5               0x47 //default 51 Reserved
#define REG_AWBC6               0x48 //default 79 Reserved
#define REG_REG4B               0x4B //default 00 Register 4B
#define REG_DNSTH               0x4C //default 00 De-noise Strength
#define REG_MTX1                0x4F //default 40 Matrix Coefficient 1
#define REG_MTX2                0x50 //default 34 Matrix Coefficient 2
#define REG_MTX3                0x51 //default 0C Matrix Coefficient 3
#define REG_MTX4                0x52 //default 17 Matrix Coefficient 4
#define REG_MTX5                0x53 //default 29 Matrix Coefficient 5
#define REG_MTX6                0x54 //default 40 Matrix Coefficient 6
#define REG_BRIGHT              0x55 //default 00 Brightness Control
#define REG_CONTRAS             0x56 //default 40 Contrast Control
#define REG_CONTRAS_CENTER      0x57 //default 80 Contrast Center
#define REG_MTXS                0x58 //default 1E Matrix Coefficient Sign for Coefficient 5 to 0
#define REG_LCC1                0x62 //default 00 Lens Correction Option 1 - X Coordinate of Lens Correction Center
#define REG_LCC2                0x63 //default 00 Lens Correction Option 2 - Y Coordinate of Lens Correction Center
#define REG_LCC3                0x64 //default 50 Lens Correction Option 3
#define REG_LCC4                0x65 //default 30 Lens Correction Option 4
#define REG_LCC5                0x66 //default 00 Lens Correction Control 5
#define REG_MANU                0x67 //default 80 Manual U Value (effective only when register TSLB[4] is high)
#define REG_MANV                0x68 //default 80 Manual V Value (effective only when register TSLB[4] is high)
#define REG_GFIX                0x69 //default 00 Fix Gain Control
#define REG_GGAIN               0x6A //default 00 G channel AWB Gain
#define REG_DBLV                0x6B //default 0A PLL, Regulator control
#define REG_AWBCTR3             0x6C //default 02 AWB Control 3
#define REG_AWBCTR2             0x6D //default 55 AWB Control 2
#define REG_AWBCTR1             0x6E //default C0 AWB Control 1
#define REG_AWBCTR0             0x6F //default 9A AWB Control 0
#define REG_SCALING_XSC         0x70 //default 3A 
#define REG_SCALING_YSC         0x71 //default 35
#define REG_SCALING_DCWCTR      0x72 //default 11 DCW Control
#define REG_SCALING_PCLK_DIV    0x73 //default 00 Prescaler 
#define REG_REG74               0x74 //default 00 Register 74
#define REG_REG75               0x75 //default 0F Register 75
#define REG_REG76               0x76 //default 01 Register 76
#define REG_REG77               0x77 //default 10 Register 77
#define REG_SLOP                0x7A //default 24 Gamma Curve Highest Segment Slope - calculated as follows
#define REG_GAM1                0x7B //default 04 Gamma Curve 1st Segment Input end Point 0x04 Output Value
#define REG_GAM2                0x7C //default 07 Gamma Curve 2nd Segment Input end Point 0x08 Output Value
#define REG_GAM3                0x7D //default 10 Gamma Curve 3rd Segment Input end Point 0x10 Output Value
#define REG_GAM4                0x7E //default 28 Gamma Curve 4th Segment Input end Point 0x20 Output Value
#define REG_GAM5                0x7F //default 36 Gamma Curve 5th Segment Input end Point 0x28 Output Value
#define REG_GAM6                0x80 //default 44 Gamma Curve 6th Segment Input end Point 0x30 Output Value
#define REG_GAM7                0x81 //default 52 Gamma Curve 7th Segment Input end Point 0x38 Output Value
#define REG_GAM8                0x82 //default 60 Gamma Curve 8th Segment Input end Point 0x40 Output Value
#define REG_GAM9                0x83 //default 6C Gamma Curve 9th Segment Input end Point 0x48 Output Value 
#define REG_GAM10               0x84 //default 78 Gamma Curve 10th Segment Input end Point 0x50 Output Value
#define REG_GAM11               0x85 //default 8C Gamma Curve 11th Segment Input end Point 0x60 Output Value
#define REG_GAM12               0x86 //default 9E Gamma Curve 12th Segment Input end Point 0x70 Output Value
#define REG_GAM13               0x87 //default BB Gamma Curve 13th Segment Input end Point 0x90 Output Value
#define REG_GAM14               0x88 //default D2 Gamma Curve 14th Segment Input end Point 0xB0 Output Value
#define REG_GAM15               0x89 //default E5 Gamma Curve 15th Segment Input end Point 0xD0 Output Value
#define REG_RGB444              0x8C //default 00 RGB444 enable
#define REG_DM_LNL              0x92 //default 00 Dummy Line low 8 bits
#define REG_DM_LNH              0x93 //default 00 Dummy Line high 8 bits
#define REG_LCC6                0x94 //default 50 Lens Correction Option 6
#define REG_LCC7                0x95 //default 50 Lens Correction Option 7
#define REG_BD50ST              0x9D //default 99 50Hz Banding Filter Value
#define REG_BD60ST              0x9E //default 7F 60Hz Banding Filter Value
#define REG_HAECC1              0x9F //default C0 Histogram-based AEC/AGC Control 1
#define REG_HAECC2              0xA0 //default 90 Histogram-based AEC/AGC Control 2
#define REG_SCALING_PLCK_DELAY  0xA2 //default 02 Pixel Clock Delay
#define REG_NT_CTRL             0xA4 //default 00 NT_CTRL
#define REG_BD50MAX             0xA5 //default 0F 50Hz Banding Setp Limit
#define REG_HAECC3              0xA6 //default F0 Histogram-based AEC/AGC Control 3
#define REG_HAECC4              0xA7 //default C1 Histogram-based AEC/AGC Control 4
#define REG_HAECC5              0xA8 //default F0 Histogram-based AEC/AGC Control 5
#define REG_HAECC6              0xA9 //default C1 Histogram-based AEC/AGC Control 6
#define REG_HAECC7              0xAA //default 14 Bit[7] AEC algorithm selection
#define REG_BD60MAX             0xAB //default 0F 60Hz Banding Step Limit
#define REG_STR_OPT             0xAC //default 00 Register AC
#define REG_STR_R               0xAD //default 80 R Gain for LED Output Frame
#define REG_STR_G               0xAE //default 80 G Gain for LED Output Frame
#define REG_STR_B               0xAF //default 80 B Gain for LED Output Frame
#define REG_ABLC1               0xB1 //default 00 
#define REG_THL_ST              0xB3 //default 80 ABLC Target
#define REG_THL_DLT             0xB5 //default 04 ABLC Stabl Range
#define REG_AD_CHB              0xBE //default 00 Blue Channel Black Level Compensation
#define REG_AD_CHR              0xBF //default 00 Red Channel Black Level Compensation
#define REG_AD_CHGb             0xC0 //default 00 Gb Channel Black Level Compensation
#define REG_AD_CHGr             0xC1 //default 00 Gr Channel Black Level Compensation
#define REG_SATCTR              0xC9 //defulat C0 Saturation Control

/*===============================================================*/
/*OV7670 register value macro*/
#define
#define
#define
#define
#define
#define
#define
#define
#define
#define
#define


/*===============================================================*/ 
/*OV7670 Function Prototype Declaration==========================*/
/*Initializing OV7670============================================*/
void    OV7670_init(void); 
/*Reading data in buffer=========================================*/
void    OV7670_ReadData(uint8_t *buffer, uint32_t length);
/*Reading Register in OV7670=====================================*/
uint8_t OV7670_ReadReg(uint8_t reg);
/*Wrigin value in register in OV7670=============================*/
uint8_t OV7670_WriteReg(uint8_t reg, uint8_t value);
/*Checking if OV7670 connected correctly=========================*/
uint8_t OV7670_CheckID(void);
/*Setting video format===========================================*/
void    OV7670_SetFormat(uint8_t format); 
/*Serring resolution of Window===================================*/
void    OV7670_SetResolution(uint8_t res); 


