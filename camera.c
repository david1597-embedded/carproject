/*
 * camera.c
 *
 *  Created on: May 12, 2025
 *      Author: USER
 */

#include "camera.h"

/*===============================================================*/
/* Static Helper Functions ======================================*/

/**
 * @brief Applies a configuration array to OV7670 registers.
 * @param config Array of OV7670_Reg structs
 * @param size Number of elements in the array
 * @retval HAL_StatusTypeDef: HAL_OK if successful, else error code
 */
static HAL_statusTypeDef OV7670_ApplyConfig(const OV7670_Reg *config, uint32_t size)
{
   for(uint32_t i=0; i <size; i++)
   {
     if(OV7670_WriteReg(config[i].reg, config[i].val) != HAL_OK)
     {
       return HAL_ERROR;
     }
     HAL_Delay(10);
   }

   /*return OK if no error when reading register configurations*/
   return HAL_OK;
}

/**
 * @brief Resets the OV7670 module using the RESET pin.
 * @param None
 * @retval None
 */
static void OV7670_HardwareReset(void)
{
    HAL_GPIO_WritePin(OV7670_RESET_GPIO_Port, OV7670_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OV7670_RESET_GPIO_Port, OV7670_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

/**
 * @brief Powers on/off the OV7670 using the PWDN pin.
 * @param state 0 to power on, 1 to power down
 * @retval None
 */
static void OV7670_PowerState(uint8_t state)
{
    HAL_GPIO_WritePin(OV7670_PWDN_GPIO_Port, OV7670_PWDN_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_Delay(10);
}

/*===============================================================*/
/* Function Implementations =====================================*/
HAL_StatusTypeDef OV7670_MCO_Init(void)
{
  //Configure MC01 (PC9) to output 24MHz using HSE and PLL
  RCC_ClockConfigTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  // Enable HSE (assuming 8MHz external crystal)
  __HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
  while (!__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY));

  // Configure PLL for 24MHz output
  RCC_ClkInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_ClkInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_ClkInitStruct.PLL.PLLM = 8;  // 8MHz / 8 = 1MHz
  RCC_ClkInitStruct.PLL.PLLN = 168; // 1MHz * 168 = 168MHz
  RCC_ClkInitStruct.PLL.PLLP = RCC_PLLP_DIV7; // 168MHz / 7 = 24MHz
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
      return HAL_ERROR;
  }

  // Configure MCO1 to output PLL clock
   HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_1);

   // Configure PC9 as MCO1 output
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   GPIO_InitStruct.Pin = OV7670_XCLK_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
   HAL_GPIO_Init(OV7670_XCLK_GPIO_Port, &GPIO_InitStruct);

   return HAL_OK;
}

HAL_StatusTypeDef OV7670_DCMI_Init(void)
{
   hdcmi.Instance = DCMI;
   hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
   hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
   hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
   hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
   hdcmi.Init.CaptureRate = DMA_NORMAL;
   hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
   hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
   if (HAL_DCMI_Init(&hdcmi) != HAL_OK) {
     return HAL_ERROR;
   }

   hdma_dcmi.Instance = DMA2_Stream1;
   hdma_dcmi.Init.Channel = DMA_CHANNEL_1;
   hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
   hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
   hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
   hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
   hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
   hdma_dcmi.Init.Mode = DMA_NORMAL;
   hdma_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
   hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
   hdma_dcmi.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
   hdma_dcmi.Init.MemBurst = DMA_MBURST_SINGLE;
   hdma_dcmi.Init.PeriphBurst = DMA_PBURST_SINGLE;
   if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK) {
     return HAL_ERROR;
   }

   __HAL_LINKDMA(&hdcmi, DMA_Handle, hdma_dcmi);

   // Enable DMA interrupt
   HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

   return HAL_OK;
}

HAL_StatusTypeDef OV7670_DCMI_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    // Configure VSYNC, HREF, PCLK
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;

    GPIO_InitStruct.Pin = OV7670_VSYNC_Pin;
    HAL_GPIO_Init(OV7670_VSYNC_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OV7670_HREF_Pin;
    HAL_GPIO_Init(OV7670_HREF_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OV7670_PCLK_Pin;
    HAL_GPIO_Init(OV7670_PCLK_GPIO_Port, &GPIO_InitStruct);

    // Configure data pins (D0-D7)
    GPIO_InitStruct.Pin = OV7670_D0_Pin | OV7670_D1_Pin;
    HAL_GPIO_Init(OV7670_D0_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OV7670_D2_Pin | OV7670_D3_Pin | OV7670_D4_Pin | OV7670_D6_Pin | OV7670_D7_Pin;
    HAL_GPIO_Init(OV7670_D2_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OV7670_D5_Pin;
    HAL_GPIO_Init(OV7670_D5_GPIO_Port, &GPIO_InitStruct);

    // Configure RESET and PWDN as output
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = OV7670_RESET_Pin;
    HAL_GPIO_Init(OV7670_RESET_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OV7670_PWDN_Pin;
    HAL_GPIO_Init(OV7670_PWDN_GPIO_Port, &GPIO_InitStruct);

    return HAL_OK;
}

HAL_StatusTypeDef OV7670_ReadRow(uint8_t *buffer, uint32_t length)
{
    // 한 row(352 바이트) 캡처
    if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)buffer, length / 4) != HAL_OK) {
        return HAL_ERROR;
    }

    // DMA 전송 완료 대기
    uint32_t timeout = HAL_GetTick() + 100;
    while (HAL_DCMI_GetState(&hdcmi) != HAL_DCMI_STATE_READY) {
        if (HAL_GetTick() > timeout) {
            HAL_DCMI_Stop(&hdcmi);
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

HAL_StatusTypeDef OV7670_ReadData(uint8_t *buffer, uint32_t length)
{
    if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)buffer, length / 4) != HAL_OK) {
        return HAL_ERROR;
    }

    // Wait for DMA transfer to complete
    uint32_t timeout = HAL_GetTick() + 1000;
    while (HAL_DCMI_GetState(&hdcmi) != HAL_DCMI_STATE_READY) {
        if (HAL_GetTick() > timeout) {
            HAL_DCMI_Stop(&hdcmi);
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

uint8_t OV7670_ReadReg(uint8_t reg)
{
    uint8_t value = 0xFF;
    if (HAL_I2C_Mem_Read(&hi2c1, OV7670_READ_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100) != HAL_OK) {
        return 0xFF; // Error
    }
    return value;
}

HAL_StatusTypeDef OV7670_WriteReg(uint8_t reg, uint8_t value)
{
    if (HAL_I2C_Mem_Write(&hi2c1, OV7670_WRITE_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100) != HAL_OK) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef OV7670_CheckID(void)
{
    uint8_t pid = OV7670_ReadReg(REG_PID);
    uint8_t ver = OV7670_ReadReg(REG_VER);

    if (pid == 0x76 && ver == 0x73) { // OV7670 ID
        return HAL_OK;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef OV7670_Init(void)
{
    // Power on and reset
    OV7670_PowerState(0); // Power on
    OV7670_HardwareReset();

    // Initialize MCO for 24MHz XCLK
    if (OV7670_MCO_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    // Initialize DCMI and GPIO
    if (OV7670_DCMI_GPIO_Init() != HAL_OK || OV7670_DCMI_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    // Check OV7670 ID
    if (OV7670_CheckID() != HAL_OK) {
        return HAL_ERROR;
    }

    // Apply common settings
    if (OV7670_ApplyConfig(common_settings, sizeof(common_settings) / sizeof(OV7670_Reg)) != HAL_OK) {
        return HAL_ERROR;
    }

    // Apply VGA resolution (default)
    if (OV7670_ApplyConfig(res_qcif, sizeof(res_qcif) / sizeof(OV7670_Reg)) != HAL_OK) {
        return HAL_ERROR;
    }

    // Apply RGB565 format (default)
    if (OV7670_ApplyConfig(fmt_rgb565, sizeof(fmt_rgb565) / sizeof(OV7670_Reg)) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef OV7670_Test(void)
{
    // Test 1: Check OV7670 ID
    if (OV7670_CheckID() != HAL_OK) {
        return HAL_ERROR;
    }

    // Test 2: Write and read back a test register (REG_GAIN)
    uint8_t test_value = 0xAA;
    if (OV7670_WriteReg(REG_GAIN, test_value) != HAL_OK) {
        return HAL_ERROR;
    }
    if (OV7670_ReadReg(REG_GAIN) != test_value) {
        return HAL_ERROR;
    }

    // Test 3: Verify DCMI initialization
    if (OV7670_DCMI_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

