

#include "LIB/stdtypes.h"
#include "MCAL/RCC_Driver/rcc_int.h"

#include "HAL/MCU_Driver/mcu_cfg.h"
#include "HAL/MCU_Driver/mcu.h"

const MCU_Config_t MCU_Configs = {
    .MCU_AHB1_PrephralEnable = MCU_AHB1_GPIOA_CLOCK|MCU_AHB1_GPIOB_CLOCK|MCU_AHB1_GPIOC_CLOCK|MCU_AHB1_DMA2_CLOCK,
    .MCU_AHB2_PrephralEnable = MCU_AHB2_NO_PERIPHERAL,
    .MCU_APB1_PrephralEnable = MCU_APB1_USART2_CLOCK,
    .MCU_APB2_PrephralEnable = MCU_APB2_USART1_CLOCK|MCU_APB2_USART6_CLOCK,
    .MCU_SystemClockSource   = MCU_SYSCLK_HSI,
    .MCU_AHP_Prescaler       = MCU_AHB_NO_DIVISION,
    .MCU_APB1_Prescaler      = MCU_APB1_NO_DIVISION,
    .MCU_APB2_Prescaler      = MCU_APB2_NO_DIVISION,
    .MCU_HSI_ClockSource     = 16000000UL,              //16MHZ
    .MCU_HSE_ClockSource     = 25000000UL,              //25MHZ
    .MCU_PLLN                = 336,
    .MCU_PLLM                = 16,
    .MCU_PLLP                = 4,  
    .MCU_PLLQ                = 7  
};



// this vlaue is used to check in the PLL user selection values (ex: PLL_M,PLL_N,...etc)
// be carful when you put wrong values it may cause wrong system behavior
// .MCU_HSI_ClockSource = (16000000UL)   // 16MHZ
// .MCU_HSE_ClockSource = (25000000UL) s  // 25MHZ


/*  The available values for MCU_SystemClockSource are 
    *   MCU_SYSCLK_HSI
    *   MCU_SYSCLK_HSE
    *   MCU_SYSCLK_PLL
*/
// ex >> .MCU_SystemClockSource = MCU_SYSCLK_HSI

/*  The available values for RCC_PLL_SOURCE are 
    *   MCU_PLL_SOURCE_HSI
    *   MCU_PLL_SOURCE_HSE
*/
// ex >> .MCU_PLLClockSource = MCU_PLL_SOURCE_HSI


/*  PLL Configuration Parameters */
//  .MCU_PLLN = 16
//  .MCU_PLLM = 336
//  .MCU_PLLP = 4
//  .MCU_PLLQ = 7

/*  The available values for MCU_AHP_Prescaler are 
    *   MCU_AHB_NO_DIVISION
    *   MCU_AHB_DIVIDED_BY_2
    *   MCU_AHB_DIVIDED_BY_4
    *   MCU_AHB_DIVIDED_BY_8
    *   MCU_AHB_DIVIDED_BY_16
    *   MCU_AHB_DIVIDED_BY_64
    *   MCU_AHB_DIVIDED_BY_128
    *   MCU_AHB_DIVIDED_BY_256
    *   MCU_AHB_DIVIDED_BY_512
*/
// ex >> .MCU_AHP_Prescaler = MCU_AHB_NO_DIVISION

/*  The available values for MCU_APB1_Prescaler are 
    *   MCU_APB1_NO_DIVISION
    *   MCU_APB1_DIVIDED_BY_2
    *   MCU_APB1_DIVIDED_BY_4
    *   MCU_APB1_DIVIDED_BY_8
    *   MCU_APB1_DIVIDED_BY_16
*/
// ex >> .MCU_APB1_Prescaler = MCU_APB1_NO_DIVISION

/*  The available values for MCU_APB2_Prescaler are 
    *   MCU_APB2_NO_DIVISION
    *   MCU_APB2_DIVIDED_BY_2
    *   MCU_APB2_DIVIDED_BY_4
    *   MCU_APB2_DIVIDED_BY_8
    *   MCU_APB2_DIVIDED_BY_16
*/
// ex >> .MCU_APB2_Prescaler = MCU_APB2_NO_DIVISION


/*  The available values for MCU_AHB1_PrephralEnable are 
  --->> YOU CAN or more than value as this example
  *  MCU_AHB1_NO_PERIPHERAL       // you can use it alone if no select
  *  MCU_AHB1_GPIOA_CLOC
  *  MCU_AHB1_GPIOB_CLOC
  *  MCU_AHB1_GPIOC_CLOC
  *  MCU_AHB1_GPIOD_CLOC
  *  MCU_AHB1_GPIOE_CLOC
  *  MCU_AHB1_GPIOH_CLOC
  *  MCU_AHB1_CRC_CLOCK 
  *  MCU_AHB1_DMA1_CLOCK
  *  MCU_AHB1_DMA2_CLOCK 
*/
// ex >> .MCU_AHB1_PrephralEnable = MCU_AHB1_GPIOB_CLOCK | MCU_AHB1_GPIOA_CLOCK   

        
/*  The available values for MCU_AHB2_PrephralEnable are 
  --->> YOU CAN or more than value as this example
  *  MCU_AHB2_NO_PERIPHERAL             // you can use it alone if
  *  MCU_AHB2_OTGFS_CLOCK
*/
// ex >> .MCU_AHB2_PrephralEnable = MCU_AHB2_NO_PERIPHERAL

/*  The available values for MCU_APB1_PrephralEnable are 
  --->> YOU CAN or more than value as this example
  * MCU_APB1_NO_PERIPHERAL      // you can use it alone if no select
  * MCU_APB1_TIMER2_CLOCK
  * MCU_APB1_TIMER3_CLOCK
  * MCU_APB1_TIMER4_CLOCK
  * MCU_APB1_TIMER5_CLOCK
  * MCU_APB1_WWDG_CLOCK  
  * MCU_APB1_SPI2_CLOCK  
  * MCU_APB1_SPI3_CLOCK  
  * MCU_APB1_USART2_CLOCK
  * MCU_APB1_I2C1_CLOCK  
  * MCU_APB1_I2C2_CLOCK  
  * MCU_APB1_I2C3_CLOCK  
  * MCU_APB1_PWR_CLOCK   
*/
// ex >> .MCU_APB1_PrephralEnable = MCU_APB1_NO_PERIPHERAL

/*  The available values for MCU_APB2_PrephralEnable are 
  --->> YOU CAN or more than value as this example
  * MCU_APB2_NO_PERIPHERAL    // you can use it alone if no select
  * MCU_APB2_TIMER1_CLOCK 
  * MCU_APB2_USART1_CLOCK 
  * MCU_APB2_USART6_CLOCK 
  * MCU_APB2_ADC1_CLOCK   
  * MCU_APB2_SDIO_CLOCK   
  * MCU_APB2_SPI1_CLOCK   
  * MCU_APB2_SPI4_CLOCK   
  * MCU_APB2_SYSCFG_CLOCK 
  * MCU_APB2_TIMER9_CLOCK 
  * MCU_APB2_TIMER10_CLOCK
  * MCU_APB2_TIMER11_CLOCK
*/
// .MCU_APB2_PrephralEnable = MCU_APB2_NO_PERIPHERAL

