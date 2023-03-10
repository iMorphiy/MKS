/**
  ******************************************************************************
  * @file    03_Calibration/main.c 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    19-June-2015
  * @brief   This code example shows how to configure the RTC
  *          in order to have a calendar.
  *
 ===============================================================================
                    #####       MCU Resources     #####
 ===============================================================================
   - RCC
   - GPIO  PA9(USART1_TX),PA10(USART1_RX), PA0, PC8, PC9, PC13
   - RTC
   - EXTI

 ===============================================================================
                    ##### How to use this example #####
 ===============================================================================
   - this file must be inserted in a project containing  the following files :
      o system_stm32f0xx.c, startup_stm32f072xb.s
      o stm32f0xx.h to get the register definitions
      o CMSIS files

 ===============================================================================
                    ##### How to test this example #####
 ===============================================================================
   - Plug cable " USB to TTL 3V3 " (from FTDIChip)
   - Connect FTDI Rx to USART1 Tx(PA9)and FTDI Tx to USART1 Rx(PA10)
   - Launch serial communication SW
   - Launch the program
   - The time should be print on the interface
   - Press the user button to enter in the RTC init mode
   - Follow the step on the interface to change the RTC time
   - LED behaviour:
     1- Green LED is toggling to indicate that RTC is well configured and time 
        is displayed on PC interface
     2- Orange LED is ON to indicate that MCU enters in RTC init Mode
     3- Orange LED is OFF to indicate that RTC time is changed
   - The clock calibration output is selected on PC13 and can be check with an oscilloscope
   - a smooth calibration is set at around +20ppm
   - note: there is no LSE on board so the LSI is used instead, please keep in mind
     to have a precise time the LSE has to be used in a normal application
  *    
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "string.h"

/** @addtogroup STM32F0_Snippets
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SIZE_OF_PERIOD_TAB  (8)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t send = 0;
uint8_t stringtosend[20] = "\n00 : 00 : 00 ";
uint8_t RTC_InitializationMode = 0;
uint8_t CharToReceive = 0;
uint8_t CharReceived = 0;
uint8_t Alarm = 1; /* set to 1 for the first print */
volatile uint32_t DateToCompute = 0;

/* Private function prototypes -----------------------------------------------*/
void Configure_GPIO_LED(void);
void Configure_GPIO_USART1(void);
void Configure_USART1(void);
void Configure_RTC(void);
void Init_RTC(uint32_t Time);
void Configure_GPIO_Button(void);
void Configure_EXTI(void);
void Process(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{

  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f072xb.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f0xx.c file
     */

  Configure_GPIO_LED();
  Configure_GPIO_USART1();
  Configure_USART1();
  Configure_RTC();
  Init_RTC(0);
  Configure_GPIO_Button();
  Configure_EXTI();
  
  /* Infinite loop */
  while (1)
  {
    Process();
  }
}

/**
  * @brief  This function :
             - Enables GPIO clock
             - Configures the Green LED pin on GPIO PC9
             - Configures the Orange LED pin on GPIO PC8
  * @param  None
  * @retval None
  */
__INLINE void Configure_GPIO_LED(void)
{
  /* Enable the peripheral clock of GPIOC */
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	
  /* Select output mode (01) on PC8 and PC9 */
  GPIOC->MODER = (GPIOC->MODER & ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9)) \
                 | (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);
  
  /* lit green LED */
  GPIOC->BSRR = GPIO_BSRR_BS_9;
}

/**
  * @brief  This function :
             - Enables GPIO clock
             - Configures the USART1 pins on GPIO PA9 PA10
  * @param  None
  * @retval None
  */
__INLINE void Configure_GPIO_USART1(void)
{
  /* Enable the peripheral clock of GPIOA */
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	
  /* GPIO configuration for USART1 signals */
  /* (1) Select AF mode (10) on PA9 and PA10 */
  /* (2) AF1 for USART1 signals */
  GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10))\
                 | (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1); /* (1) */
  GPIOA->AFR[1] = (GPIOA->AFR[1] &~ (GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2))\
                  | (1 << (1 * 4)) | (1 << (2 * 4)); /* (2) */
}

/**
  * @brief  This function configures USART1.
  * @param  None
  * @retval None
  */
__INLINE void Configure_USART1(void)
{
  /* Enable the peripheral clock USART1 */
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

  /* Configure USART1 */
  /* (1) oversampling by 16, 9600 baud */
  /* (2) 8 data bit, 1 start bit, 1 stop bit, no parity */
  USART1->BRR = 480000 / 96; /* (1) */
  USART1->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE; /* (2) */
  
  /* polling idle frame Transmission */
  while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC)
  { 
    /* add time out here for a robust application */
  }
  USART1->ICR |= USART_ICR_TCCF; /* clear TC flag */
  USART1->CR1 |= USART_CR1_TCIE; /* enable TC interrupt */
  
  /* Configure IT */
  /* (3) Set priority for USART1_IRQn */
  /* (4) Enable USART1_IRQn */
  NVIC_SetPriority(USART1_IRQn, 0); /* (3) */
  NVIC_EnableIRQ(USART1_IRQn); /* (4) */
}

/**
  * @brief  This function configures RTC.
  * @param  None
  * @retval None
  */
__INLINE void Configure_RTC(void)
{
  /* Enable the peripheral clock RTC */
  /* (1) Enable the LSI */
  /* (2) Wait while it is not ready */
  /* (3) Enable PWR clock */
  /* (4) Enable write in RTC domain control register */
  /* (5) LSI for RTC clock */
  /* (6) Disable PWR clock */
  RCC->CSR |= RCC_CSR_LSION; /* (1) */
  while((RCC->CSR & RCC_CSR_LSIRDY)!=RCC_CSR_LSIRDY) /* (2) */
  { 
    /* add time out here for a robust application */
  }
  RCC->APB1ENR |= RCC_APB1ENR_PWREN; /* (3) */
  PWR->CR |= PWR_CR_DBP; /* (4) */
  RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_RTCSEL) | RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_1; /* (5) */
  RCC->APB1ENR &=~ RCC_APB1ENR_PWREN; /* (7) */

  /* Configure RTC */
  /* (7) Write access for RTC regsiters */
  /* (8) Disable alarm A to modify it */
  /* (9) Wait until it is allow to modify alarm A value */
  /* (10) Modify alarm A mask to have an interrupt each 1Hz */
  /* (11) Enable alarm A and alarm A interrupt, calibration output (1Hz) enable */
  /* (12) Disable write access */
  RTC->WPR = 0xCA; /* (7) */
  RTC->WPR = 0x53; /* (7) */
  RTC->CR &=~ RTC_CR_ALRAE; /* (8) */
  while((RTC->ISR & RTC_ISR_ALRAWF) != RTC_ISR_ALRAWF) /* (9) */
  { 
    /* add time out here for a robust application */
  }
  RTC->ALRMAR = RTC_ALRMAR_MSK4 | RTC_ALRMAR_MSK3 | RTC_ALRMAR_MSK2 | RTC_ALRMAR_MSK1; /* (10) */
  RTC->CR = RTC_CR_ALRAIE | RTC_CR_ALRAE | RTC_CR_COE | RTC_CR_COSEL; /* (11) */ 
  RTC->WPR = 0xFE; /* (12) */
  RTC->WPR = 0x64; /* (12) */
  
  /* Configure exti and nvic for RTC IT */
  /* (13) unmask line 17 */
  /* (14) Rising edge for line 17 */
  /* (15) Set priority */
  /* (16) Enable RTC_IRQn */
  EXTI->IMR |= EXTI_IMR_MR17; /* (13) */ 
  EXTI->RTSR |= EXTI_RTSR_TR17; /* (14) */ 
  NVIC_SetPriority(RTC_IRQn, 0); /* (15) */ 
  NVIC_EnableIRQ(RTC_IRQn); /* (16) */ 
}

/**
  * @brief  This function configures RTC.
  * @param  uint32_t New time
  * @retval None
  */
__INLINE void Init_RTC(uint32_t Time)
{
  /* RTC init mode */
  /* Configure RTC */
  /* (1) Write access for RTC registers */
  /* (2) Enable init phase */
  /* (3) Wait until it is allow to modify RTC register values */
  /* (4) set prescaler, 40kHz/125 => 320 Hz, 320Hz/320 => 1Hz */
  /* (5) New time in TR */
  /* (6) Disable init phase */
  /* (7) Wait until it's allow to modify calibartion register */
  /* (8) Set calibration to around +20ppm, which is a standard value @25?C */
  /*     Note: the calibration is relevant when LSE is selected for RTC clock */
  /* (9) Disable write access for RTC registers */
  RTC->WPR = 0xCA; /* (1) */ 
  RTC->WPR = 0x53; /* (1) */
  RTC->ISR |= RTC_ISR_INIT; /* (2) */
  while((RTC->ISR & RTC_ISR_INITF)!=RTC_ISR_INITF) /* (3) */
  { 
    /* add time out here for a robust application */
  }
  RTC->PRER = (124<<16) | 319; /* (4) */
  RTC->TR = RTC_TR_PM | Time; /* (5) */
  RTC->ISR &=~ RTC_ISR_INIT; /* (6) */
  while((RTC->ISR & RTC_ISR_RECALPF) == RTC_ISR_RECALPF) /* (7) */
  { 
    /* add time out here for a robust application */
  }
  RTC->CALR = RTC_CALR_CALP | 482; /* (8) */
  RTC->WPR = 0xFE; /* (9) */
  RTC->WPR = 0x64; /* (9) */
}

/**
  * @brief  This function :
             - Enables GPIO clock
             - Configures the Push Button GPIO PA0
  * @param  None
  * @retval None
  */
__INLINE void Configure_GPIO_Button(void)
{
  /* Enable the peripheral clock of GPIOA */
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	
  /* Select mode */
  /* Select input mode (00) on PA0 */
  GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER0));
}

/**
  * @brief  This function configures EXTI.
  * @param  None
  * @retval None
  */
__INLINE void Configure_EXTI(void)
{
  /* Configure Syscfg, exti and nvic for pushbutton PA0 */
  /* (1) PA0 as source input */
  /* (2) unmask port 0 */
  /* (3) Rising edge */
  /* (4) Set priority */
  /* (5) Enable EXTI0_1_IRQn */
  SYSCFG->EXTICR[0] = (SYSCFG->EXTICR[0] & ~SYSCFG_EXTICR1_EXTI0) | SYSCFG_EXTICR1_EXTI0_PA; /* (1) */ 
  EXTI->IMR |= EXTI_IMR_MR0; /* (2) */ 
  EXTI->RTSR |= EXTI_RTSR_TR0; /* (3) */ 
  NVIC_SetPriority(EXTI0_1_IRQn, 0); /* (4) */ 
  NVIC_EnableIRQ(EXTI0_1_IRQn); /* (5) */ 
}

/**
  * @brief  This function processes RTC with USART.
  * @param  None
  * @retval None
  */
void Process(void)
{
  volatile uint32_t TimeToCompute = 0;
  static uint32_t NewTime=0;
  
  switch(RTC_InitializationMode)
  {
  case 0:
    /* check alarm and synchronisation flag */
    if((Alarm)&&((RTC->ISR & RTC_ISR_RSF) == RTC_ISR_RSF))
    {
      Alarm=0;
      
      TimeToCompute = RTC->TR; /* get time */
      DateToCompute = RTC->DR; /* need to read date also */
      
      stringtosend[1] = (uint8_t)(((TimeToCompute & RTC_TR_HT)>>20) + 48);/* hour tens */
      stringtosend[2] = (uint8_t)(((TimeToCompute & RTC_TR_HU)>>16) + 48);/* hour units */
      stringtosend[6] = (uint8_t)(((TimeToCompute & RTC_TR_MNT)>>12) + 48);/* minute tens */
      stringtosend[7] = (uint8_t)(((TimeToCompute & RTC_TR_MNU)>>8) + 48);/* minute units */
      stringtosend[11] = (uint8_t)(((TimeToCompute & RTC_TR_ST)>>4) + 48);/* second tens */
      stringtosend[12] = (uint8_t)((TimeToCompute & RTC_TR_SU) + 48);/* second units */
      
      /* start USART transmission */
      USART1->TDR = stringtosend[send++]; /* Will inititiate TC if TXE */
    }
  break;
  case 1:      
    {
      if(!send)
      {
        strcpy((char *)stringtosend,"\nSend hour tens\n");
        USART1->TDR = stringtosend[send++]; /* Will inititiate TC if TXE */
        RTC_InitializationMode=2;
      }
    }
    break;
  case 2: /* Hour tens */
    {
       if(CharReceived)
      {
        CharReceived=0;
        CharToReceive -= 48;
        NewTime = CharToReceive << 20;
        RTC_InitializationMode=3;
      }
    }
    break;
  case 3:      
    {
      if(!send)
      {
        strcpy((char *)stringtosend,"\nSend hour units\n");
        USART1->TDR = stringtosend[send++]; /* Will inititiate TC if TXE */
        RTC_InitializationMode=4;
      }
    }
    break;
  case 4: /* Hour units */
    {
       if(CharReceived)
      {
        CharReceived=0;
        CharToReceive -= 48;
        NewTime |= CharToReceive << 16;
        RTC_InitializationMode=5;
      }
    }
    break;
  case 5:      
    {
      if(!send)
      {
        strcpy((char *)stringtosend,"\nSend minute tens\n");
        USART1->TDR = stringtosend[send++]; /* Will inititiate TC if TXE */
        RTC_InitializationMode=6;
      }
    }
    break;
  case 6: /* Minute tens */
    {
      if(CharReceived)
      {
        CharReceived=0;
        CharToReceive -= 48;
        NewTime |= CharToReceive << 12;
        RTC_InitializationMode=7;
      }
    }
    break;
  case 7:      
    {
      if(!send)
      {
        strcpy((char *)stringtosend,"\nSend minute units\n");
        USART1->TDR = stringtosend[send++]; /* Will inititiate TC if TXE */
        RTC_InitializationMode=8;
      }
    }
    break;
  case 8: /* Minute units */
    {
      if(CharReceived)
      {
        CharReceived=0;
        CharToReceive -= 48;
        NewTime |= CharToReceive << 8;
        RTC_InitializationMode=9;
      }
    }
    break;
  case 9:      
    {
      if(!send)
      {
        strcpy((char *)stringtosend,"\nSend second tens\n");
        USART1->TDR = stringtosend[send++]; /* Will inititiate TC if TXE */
        RTC_InitializationMode=10;
      }
    }
    break;
  case 10: /* Second tens */
    {
       if(CharReceived)
      {
        CharReceived=0;
        CharToReceive -= 48;
        NewTime |= CharToReceive << 4;
        RTC_InitializationMode=11;
      }
    }
    break;
  case 11:      
    {
      if(!send)
      {
        strcpy((char *)stringtosend,"\nSend second units\n");
        USART1->TDR = stringtosend[send++]; /* Will inititiate TC if TXE */
        RTC_InitializationMode=12;
      }
    }
    break;
  case 12: /* Second Units */
    {
       if(CharReceived)
      {
        CharReceived=0;
        CharToReceive -= 48;
        NewTime |= CharToReceive;
        RTC_InitializationMode=13;
      }
    }
    break;
  case 13:
    {
      if (NewTime > (2<<20 | 3<<16 | 5<<12 | 9<<8 | 5<<4 | 9<<0))
      {
        NewTime = 0;
      }
      Init_RTC(NewTime);
      strcpy((char *)stringtosend,"\n-- : -- : --      ");
      RTC_InitializationMode=0;
      GPIOC->BSRR = GPIO_BSRR_BR_8 ; /* Off Orange LED */
    }
    break;
  }  
}

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}


/**
  * @brief  This function handles EXTI 0 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_1_IRQHandler(void)
{
  EXTI->PR |= EXTI_PR_PR0;
  
  GPIOC->BSRR = GPIO_BSRR_BS_8 ; /* Lit Orange LED */
  RTC_InitializationMode = 1;
}

/**
  * @brief  This function handles USART1 interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
  if((USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
  {
    CharToReceive = (uint8_t)(USART1->RDR); /* Receive data, clear flag */
    CharReceived = 1;      
  }
  else if((USART1->ISR & USART_ISR_TC) == USART_ISR_TC)
  {
    if(send == sizeof(stringtosend))
    {
      send=0;
      USART1->ICR |= USART_ICR_TCCF; /* Clear transfer complete flag */
    }
    else
    {
      /* clear transfer complete flag and fill TDR with a new char */
      USART1->TDR = stringtosend[send++];
    }
  }
  else
  {
      NVIC_DisableIRQ(USART1_IRQn); /* Disable USART1_IRQn */
  }
	
}

/**
  * @brief  This function handles RTC interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
  /* Check alarm A flag */
  if((RTC->ISR & (RTC_ISR_ALRAF)) == (RTC_ISR_ALRAF))
  {
    RTC->ISR &=~ RTC_ISR_ALRAF; /* clear flag */
    EXTI->PR |= EXTI_PR_PR17; /* clear exti line 17 flag */
    GPIOC->ODR ^= GPIO_ODR_9 ; /* Toggle Green LED */
    Alarm = 1;
  }
  else
  {
    NVIC_DisableIRQ(RTC_IRQn);/* Disable RTC_IRQn */
  }
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
