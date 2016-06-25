/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "dma.h"
#include "i2c.h"
#include "sai.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "codec.h"
#include "math.h"
#include "arm_math.h"

//[[Includes]]
#include <arm_math.h>
//[[/Includes]]
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define AUDIO_I2C_ADDRESS           ((uint16_t)0x34)
extern AUDIO_DrvTypeDef wm8994_drv;

#define BUFFER_SIZE				2
static uint16_t Buffer[BUFFER_SIZE] = { 0,0 };

// 0x00 = -57 dB
// 0x27 = -18dB
// 0x2D = -12 dB
// 0x33 =  -6 dB
// 0x39 =   0 db
// 0x3F =  +6 db
static uint32_t OutputVolume = 0x2D;

// 0x00 = -71.625 dB
// 0x90 = -18.000 dB
// 0xA0 = -12.000 dB
// 0xB0 =  -6.000 dB
// 0xC0 =   0.000 dB
// 0cFF = +17.625 dB

static uint32_t InputVolume = 0xA0;


//[[Init Code]]
float32_t Oscillators[6];


struct Level {
    float32_t Bypass;
    float32_t GainType;
    float32_t Output;
    float32_t Offset;
    float32_t Gain;
    float32_t Input;
    void set_gainType(float value) {
        GainType = value;

    }
    void set_gain(float value) {
        Gain = value;

    }
    void set_offset(float value) {
        Offset = value;

    }
    void set_bypass(float value) {
        Bypass = value;

    }
    Level() {
        GainType = 0;
        Bypass = 0;
        Input = 0;
        Gain = 0;
        Offset = 0;
        Output = 0;

    }
    void process(float32_t Input, float32_t &Output) {
        // Starting stream 00 -------------------------
        {
            Bypass = GainType;

        } // Stream End 00
// Starting stream 01 -------------------------
        {
            Output = ((Input * Gain) + Offset);

        } // Stream End 01

    }
};


struct Select {
    float32_t Output;
    float32_t SelectBlock;
    float32_t WhenOff;
    float32_t WhenOn;
    bool Input;
    void set_whenOn(float value) {
        WhenOn = value;

    }
    void set_whenOff(float value) {
        WhenOff = value;

    }
    Select() {
        Input = true;
        WhenOn = 0;
        WhenOff = 0;
        SelectBlock = 0;
        Output = 0;

    }
    void process(bool Input, float32_t &Output) {
        // Starting stream 00 -------------------------
        {
            Output = Input ? WhenOn : WhenOff;

        } // Stream End 00

    }
};


struct Oscillator {
    float32_t PhaseInc;
    float32_t Frequency;
    float32_t Output;
    float32_t Amplitude;
    float32_t SineBlock;
    float32_t Phase;
    Level Level_177;
    float32_t _Level_177_out;
    float32_t NewPhase;
    Select Select_193;
    float32_t _Select_193_out;
    float32_t StartPhase;
    bool Reset;
    Select Select_199;
    float32_t _Select_199_out;
    void set_frequency(float value) {
        Frequency = value;

    }
    void set_amplitude(float value) {
        Amplitude = value;

    }
    void set_phase(float value) {
        StartPhase = value;

    }
    void set_reset(bool value) {
        Reset = value;

    }
    Oscillator() {
        Frequency = 0;
        PhaseInc = 0;
        Phase = 0;
        SineBlock = 0;
        Amplitude = 1;
        Output = 0;
        NewPhase = 0;
        Reset = false;
        StartPhase = 0;

    }
    void process(float32_t &Output) {
        // Starting stream 00 -------------------------
        {
            PhaseInc = (((Frequency * 2.0000000000f) * 3.1415926536f) / 48000.0000000000f);

        } // Stream End 00
// Starting stream 01 -------------------------
        {
            Level_177.set_gain(Amplitude);
            Level_177.process(arm_sin_f32(Phase), _Level_177_out);

            Output = _Level_177_out;

        } // Stream End 01
// Starting stream 02 -------------------------
        {
            Phase = (Phase + PhaseInc);

        } // Stream End 02
// Starting stream 03 -------------------------
        {
            NewPhase = (Phase - 6.2831853072f);

        } // Stream End 03
// Starting stream 04 -------------------------
        {
            Select_193.set_whenOff(Phase);
            Select_193.set_whenOn(NewPhase);
            Select_193.process((Phase > 6.2831853072f), _Select_193_out);

            Phase = _Select_193_out;

        } // Stream End 04
// Starting stream 05 -------------------------
        {
            Select_199.set_whenOff(Phase);
            Select_199.set_whenOn(StartPhase);
            Select_199.process(Reset, _Select_199_out);

            Phase = _Select_199_out;

        } // Stream End 05

    }
};
Oscillator Oscillator_000;
float32_t _Oscillator_000_out;
Oscillator Oscillator_034;
float32_t _Oscillator_034_out;
Oscillator Oscillator_068;
float32_t _Oscillator_068_out;
Oscillator Oscillator_102;
float32_t _Oscillator_102_out;
Oscillator Oscillator_136;
float32_t _Oscillator_136_out;
Oscillator Oscillator_170;
float32_t _Oscillator_170_out;
float32_t AudioOut[2];
Level Level_212;
float32_t _Level_212_out;
static q15_t Sample_q15[2];
//[[/Init Code]]


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
static void Codec_Init(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

    /* USER CODE BEGIN 1 */

//[[Config Code]]
    Oscillators[0] = 0.0;
    Oscillators[1] = 0.0;
    Oscillators[2] = 0.0;
    Oscillators[3] = 0.0;
    Oscillators[4] = 0.0;
    Oscillators[5] = 0.0;
    AudioOut[0] = 0.0;
    AudioOut[1] = 0.0;
// %%token%% = 0;
//[[/Config Code]]

    /* USER CODE END 1 */

    /* Enable I-Cache-------------------------------------------------------------*/
    SCB_EnableICache();

    /* Enable D-Cache-------------------------------------------------------------*/
    SCB_EnableDCache();

    /* MCU Configuration----------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_I2C3_Init();
    MX_SAI2_Init();

    /* USER CODE BEGIN 2 */

    __HAL_SAI_ENABLE(&hsai_BlockA2);

    Codec_Init();

    HAL_SAI_Transmit_IT(&hsai_BlockA2,(uint8_t *) Buffer, BUFFER_SIZE );

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

    }
    /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 432;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    HAL_PWREx_EnableOverDrive();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2|RCC_PERIPHCLK_I2C3;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 344;
    PeriphClkInitStruct.PLLI2S.PLLI2SP = 1;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
    PeriphClkInitStruct.PLLI2S.PLLI2SQ = 7;
    PeriphClkInitStruct.PLLI2SDivQ = 1;
    PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLI2S;
    PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
void Codec_Init(void)
{
    uint32_t deviceid = 0x00;
    deviceid = wm8994_drv.ReadID(AUDIO_I2C_ADDRESS);

    if((deviceid) == WM8994_ID)
    {
        wm8994_drv.Reset(AUDIO_I2C_ADDRESS);
        wm8994_drv.Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, OutputVolume, InputVolume, AUDIO_FREQUENCY_48K);

        HAL_GPIO_TogglePin(GPIOI,GPIO_PIN_1);
    }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{


    for (uint8_t i = 0; i < 4 ; i++)
    {

//[[Dsp Code]]
// Starting stream 00 -------------------------
        {
            Oscillator_000.set_frequency(440);
            Oscillator_000.set_amplitude(0.1);
            Oscillator_034.set_frequency(443);
            Oscillator_034.set_amplitude(0.1);
            Oscillator_068.set_frequency(437);
            Oscillator_068.set_amplitude(0.1);
            Oscillator_102.set_frequency(880);
            Oscillator_102.set_amplitude(0.1);
            Oscillator_136.set_frequency(885);
            Oscillator_136.set_amplitude(0.1);
            Oscillator_170.set_frequency(887);
            Oscillator_170.set_amplitude(0.1);
            Oscillator_000.process(_Oscillator_000_out);
            Oscillator_034.process(_Oscillator_034_out);
            Oscillator_068.process(_Oscillator_068_out);
            Oscillator_102.process(_Oscillator_102_out);
            Oscillator_136.process(_Oscillator_136_out);
            Oscillator_170.process(_Oscillator_170_out);

            Oscillators[0] = _Oscillator_000_out;
            Oscillators[1] = _Oscillator_034_out;
            Oscillators[2] = _Oscillator_068_out;
            Oscillators[3] = _Oscillator_102_out;
            Oscillators[4] = _Oscillator_136_out;
            Oscillators[5] = _Oscillator_170_out;

        } // Stream End 00
// Starting stream 01 -------------------------
        {
            Level_212.set_gain(0.5);
            Level_212.process((((((Oscillators[0] + Oscillators[1]) + Oscillators[2]) + Oscillators[3]) + Oscillators[4]) + Oscillators[5]), _Level_212_out);


            arm_float_to_q15(&_Level_212_out,&Sample_q15[0],1);
            ;

            arm_float_to_q15(&_Level_212_out,&Sample_q15[1],1);
            ;


            hsai_BlockA2.Instance->DR = Sample_q15[0];
            hsai_BlockA2.Instance->DR = Sample_q15[1];


        } // Stream End 01
//[[/Dsp Code]]
    }

    __HAL_SAI_ENABLE_IT(&hsai_BlockA2,SAI_IT_FREQ);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_11 )
    {

    }
}


/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */

}

#endif

/**
  * @}
  */

/**
  * @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
