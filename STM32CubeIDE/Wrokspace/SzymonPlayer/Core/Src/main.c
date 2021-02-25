/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
uint8_t buf[12];
HAL_StatusTypeDef ret;
static const uint8_t TRIGGER_ADDR = 0x37 << 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
      if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T002.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }

      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T002.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }
      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T002.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }
      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T002.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }
      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T002.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }
      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T003.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }
      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T003.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }
      if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T003.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }
      if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T003.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }
      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T003.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }
      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7)){
        buf[0] = 0x01; //play track
        buf[1] = 0x02; //T003.mp3
        ret = HAL_I2C_Master_Transmit(&hi2c1, TRIGGER_ADDR, buf, 2, 5000);
      }

   }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */
  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */
  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 110;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */
  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC6 PC7 PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
