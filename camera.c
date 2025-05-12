
#include "camera.h"

/*Initializing OV7670============================================*/
void OV7670_init(OV7670_Resolution res, OV7670_Format fmt)
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

    /*Configurating PWDN, RESET Pin to Output*/
    GPIO_InitStruct.Pin = OV7670_PWDN_Pin | OV7670_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(OV7670_PWDN_GPIO_Port, &GPIO_InitStruct);

    /*Configurating XCLK Pin with TIM2 CHANNEL2 (PA1)*/
    GPIO_InitStruct.Pin = OV7670_XCLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    HAL_GPIO_Init(OV7670_XCLK_GPIO_Port, &GPIO_InitStruct);

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.Period = 2; // 72MHz / 3 = 24MHz
    HAL_TIM_PWM_Init(&htim2);
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

    /*Initializing I2C*/
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    HAL_I2C_Init(&hi2c1);

    /*Power on seqeunce*/
    HAL_GPIO_WritePin(OV7670_PWDN_GPIO_Port, OV7670_PWDN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OV7670_RESET_GPIO_Port, OV7670_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OV7670_RESET_GPIO_Port, OV7670_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(10); // Stabilizing XCLK

    /*Applying Resolution Configuration*/
    switch (res) {
	case RES_VGA:
	    OV7670_ApplySettings(res_vga, sizeof(res_vga) / sizeof(res_vga[0]));
	    break;
	case RES_CIF:
	    OV7670_ApplySettings(res_cif, sizeof(res_cif) / sizeof(res_cif[0]));
	    break;
	case RES_QVGA:
	    OV7670_ApplySettings(res_qvga, sizeof(res_qvga) / sizeof(res_qvga[0]));
	    break;
	case RES_QCIF:
	    OV7670_ApplySettings(res_qcif, sizeof(res_qcif) / sizeof(res_qcif[0]));
	    break;
    }

    /*Applying Format Configuration*/
    switch (fmt) {
	case FORMAT_RGB565:
	    OV7670_ApplySettings(fmt_rgb565, sizeof(fmt_rgb565) / sizeof(fmt_rgb565[0]));
	    break;
	case FORMAT_RGB555:
	    OV7670_ApplySettings(fmt_rgb555, sizeof(fmt_rgb555) / sizeof(fmt_rgb555[0]));
	    break;
	case FORMAT_RGB444:
	    OV7670_ApplySettings(fmt_rgb444, sizeof(fmt_rgb444) / sizeof(fmt_rgb444[0]));
	    break;
	case FORMAT_YUV422:
	    OV7670_ApplySettings(fmt_yuv422, sizeof(fmt_yuv422) / sizeof(fmt_yuv422[0]));
	    break;
	case FORMAT_RAW_RGB:
	    OV7670_ApplySettings(fmt_raw_rgb, sizeof(fmt_raw_rgb) / sizeof(fmt_raw_rgb[0]));
	    break;
    }


}
/*Reading data in buffer=========================================*/
void OV7670_ReadData(uint8_t *buffer, uint32_t length)
{
  uint32_t buffer_index = 0;

      // VSYNC 신호 대기 (프레임 시작, VSYNC=0)
      while (HAL_GPIO_ReadPin(OV7670_VSYNC_GPIO_Port, OV7670_VSYNC_Pin) == GPIO_PIN_SET);
      while (HAL_GPIO_ReadPin(OV7670_VSYNC_GPIO_Port, OV7670_VSYNC_Pin) == GPIO_PIN_RESET);

      // 프레임 데이터 읽기
      while (buffer_index < length)
      {
          // HREF=1 (유효한 행) 대기
          if (HAL_GPIO_ReadPin(OV7670_HREF_GPIO_Port, OV7670_HREF_Pin) == GPIO_PIN_SET)
          {
              // PCLK 상승 에지에서 데이터 읽기
              while (HAL_GPIO_ReadPin(OV7670_PCLK_GPIO_Port, OV7670_PCLK_Pin) == GPIO_PIN_RESET);
              if (buffer_index < length)
              {
                  // D0~D7 비트 조합
                  uint8_t data = 0;
                  data |= (HAL_GPIO_ReadPin(OV7670_D0_GPIO_Port, OV7670_D0_Pin) << 0); // D0: PA4
                  data |= (HAL_GPIO_ReadPin(OV7670_D1_GPIO_Port, OV7670_D1_Pin) << 1); // D1: PB3
                  data |= (HAL_GPIO_ReadPin(OV7670_D2_GPIO_Port, OV7670_D2_Pin) << 2); // D2: PB0
                  data |= (HAL_GPIO_ReadPin(OV7670_D3_GPIO_Port, OV7670_D3_Pin) << 3); // D3: PA10
                  data |= (HAL_GPIO_ReadPin(OV7670_D4_GPIO_Port, OV7670_D4_Pin) << 4); // D4: PC1
                  data |= (HAL_GPIO_ReadPin(OV7670_D5_GPIO_Port, OV7670_D5_Pin) << 5); // D5: PA2
                  data |= (HAL_GPIO_ReadPin(OV7670_D6_GPIO_Port, OV7670_D6_Pin) << 6); // D6: PC0
                  data |= (HAL_GPIO_ReadPin(OV7670_D7_GPIO_Port, OV7670_D7_Pin) << 7); // D7: PA3
                  buffer[buffer_index++] = data;
              }
              // PCLK 하강 에지 대기
              while (HAL_GPIO_ReadPin(OV7670_PCLK_GPIO_Port, OV7670_PCLK_Pin) == GPIO_PIN_SET);
          }
      }
}
/*Reading Register in OV7670=====================================*/
uint8_t OV7670_ReadReg(uint8_t reg)
{
    uint8_t value;
    // 1. 레지스터 주소 전송 (쓰기 모드)
    if (HAL_I2C_Master_Transmit(&hi2c1, OV7670_WRITE_ADDR, &reg, 1, 100) != HAL_OK)
    {
        return 0xFF; // 오류 시 0xFF 반환
    }
    // 2. 레지스터 값 읽기 (읽기 모드)
    if (HAL_I2C_Master_Receive(&hi2c1, OV7670_READ_ADDR, &value, 1, 100) != HAL_OK)
    {
        return 0xFF; // 오류 시 0xFF 반환
    }
    return value;
}
/*Writing value in register in OV7670=============================*/
uint8_t OV7670_WriteReg(uint8_t reg, uint8_t value)
{
  uint8_t data[2] = {reg, value};
  if (HAL_I2C_Master_Transmit(&hi2c1, OV7670_WRITE_ADDR, data, 2, 100) == HAL_OK)
  {
      return 0; // Success
  }
  return 1; // Failed
}
/*Checking if OV7670 connected correctly=========================*/
uint8_t OV7670_CheckID(void)
{
  uint8_t pid = OV7670_ReadReg(REG_PID); // 0x0A
  uint8_t ver = OV7670_ReadReg(REG_VER); // 0x0B

  if (pid == 0x76 && ver == 0x73)
  {
      return 0; // Success
  }
  return 1; // Failed
}
/*Applying specific settings=====================================*/
void OV7670_ApplySettings(const OV7670_Reg *settings, uint32_t count)
{
  for (uint32_t i = 0; i < count; i++)
  {
      if (OV7670_WriteReg(settings[i].reg, settings[i].val) != HAL_OK)
      {

      }
      HAL_Delay(1);
  }
}



