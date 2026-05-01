/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  *
  * Pin assignment (Discovery board):
  * I2C1_SCL -> PB6
  * I2C1_SDA -> PB7
  * BME280 -> I2C1, addr 0x76
  * SSD1306 -> I2C1, addr 0x3C
  *
  * Clock: HSE 8 MHz → PLL → SYSCLK 168 MHz (standard Discovery config)
  */

#include "stm32f4xx_hal.h"

#include "AppConfig.h"
#include "BME280Driver.h"
#include "SSD1306Driver.h"
#include "SensorManager.h"
#include "DataStorage.h"
#include "WeatherAnalyzer.h"
#include "DisplayManager.h"
#include "WeatherStationApp.h"

// HAL handles
I2C_HandleTypeDef hi2c1;

// Forward declarations
static void SystemClock_Config();
static void MX_GPIO_Init();
static void MX_I2C1_Init();
static void Error_Handler();

// Application objects
static course_work::AppConfig appConfig;
static course_work::BME280Driver bme280(hi2c1, appConfig.bme280I2cAddr);
static course_work::SSD1306Driver ssd1306(hi2c1, appConfig.ssd1306I2cAddr, appConfig.displayBrightness);
static course_work::SensorManager sensorMgr(bme280, appConfig);
static course_work::DataStorage dataStorage(appConfig);
static course_work::WeatherAnalyzer analyzer(dataStorage, appConfig);
static course_work::DisplayManager displayMgr(ssd1306, appConfig);
static course_work::WeatherStationApp app(appConfig, sensorMgr, dataStorage, analyzer, displayMgr);

// Entry point
int main()
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    if (!app.init()) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        while (true) { __WFI(); }
    }

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);

    while (true) {
        app.run();
    }
}

// Clock configuration (168 MHz, HSE 8 MHz)
static void SystemClock_Config()
{
    RCC_OscInitTypeDef osc = {};
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM = 8;
    osc.PLL.PLLN = 336;
    osc.PLL.PLLP = RCC_PLLP_DIV2;
    osc.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();

    RCC_ClkInitTypeDef clk = {};
    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV4;
    clk.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_5) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init()
{
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {};
    gpio.Pin = GPIO_PIN_12 | GPIO_PIN_14;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &gpio);
}

static void MX_I2C1_Init()
{
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {};
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);

    hi2c1.Instance = I2C1;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

static void Error_Handler()
{
    __disable_irq();
    while (true) {}
}
