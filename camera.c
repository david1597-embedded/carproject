
#include "camera.h"

/*Initializing OV7670============================================*/
void OV7670_init(void)
{ 
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*Activate GPIO Clock*/
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    /*Configuration of I2C Pins*/
    GPIO_InitStruct.Pin = OV7670_SCL_Pin | OV7670_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OV7670_SCL_GPIO_Port, &GPIO_InitStruct);


    /*Configurating VSYNC, HREF, PCLK Pin to Input*/
    GPIO_InitStruct.Pin = OV7670_VSYNC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(OV7670_VSYNC_GPIO_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = OV7670_HREF_Pin | OV7670_PCLK_Pin;
    HAL_GPIO_Init(OV7670_HREF_GPIO_Port, &GPIO_InitStruct);

    /*Configurating Data Pins D0~D7*/
    /*GPIOA*/
    GPIO_InitStruct.Pin = OV7670_D0_Pin | OV7670_D3_Pin | OV7670_D5_Pin | OV7670_D7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(OV7670_D0_GPIO_Port, &GPIO_InitStruct);
    /*GPIOB*/
    GPIO_InitStruct.Pin = OV7670_D1_Pin | OV7670_D2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(OV7670_D1_GPIO_Port, &GPIO_InitStruct);
    /*GPIOC*/
    GPIO_InitStruct.Pin = OV7670_D4_Pin | OV7670_D6_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(OV7670_D4_GPIO_Port, &GPIO_InitStruct);



}
/*Reading data in buffer=========================================*/
void OV7670_ReadData(uint8_t *buffer, uint32_t length)
{

}
/*Reading Register in OV7670=====================================*/
uint8_t OV7670_ReadReg(uint8_t reg)
{

}
/*Wrigin value in register in OV7670=============================*/
uint8_t OV7670_WriteReg(uint8_t reg, uint8_t value)
{

}
/*Checking if OV7670 connected correctly=========================*/
uint8_t OV7670_CheckID(void)
{

}
/*Setting video format===========================================*/
void OV7670_SetFormat(uint8_t format)
{

}
/*Serring resolution of Window===================================*/
void OV7670_SetResolution(uint8_t res)
{

} 