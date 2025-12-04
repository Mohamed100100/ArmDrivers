#ifndef MCU_CFG_H
#define MCU_CFG_H


// this vlaue is used to check in the PLL user selection values (ex: PLL_M,PLL_N,...etc)
// be carful when you put wrong values it may cause wrong system behavior
#define MCU_HSI_CLOCK_SOURCE_VALUE          (16000000UL)                     // 16MHZ
#define MCU_HSE_CLOCK_SOURCE_VALUE          (25000000UL)                     // 25MHZ


/*  The available values for MCU_SYSCLK_SOURCE are 
    *   MCU_SYSCLK_HSI
    *   MCU_SYSCLK_HSE
    *   MCU_SYSCLK_PLL
*/
#define MCU_SYSCLK_SOURCE    MCU_SYSCLK_HSI

/*  The available values for RCC_PLL_SOURCE are 
    *   MCU_PLL_SOURCE_HSI
    *   MCU_PLL_SOURCE_HSE
*/
#define MCU_PLL_SOURCE     MCU_PLL_SOURCE_HSI

/*  PLL Configuration Parameters */
#define MCU_PLL_M        (16UL)
#define MCU_PLL_N        (336UL)
#define MCU_PLL_P        (4UL)
#define MCU_PLL_Q        (7UL)

/*  The available values for RCC_AHB_PRESCALER are 
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
#define MCU_AHB_PRESCALER   MCU_AHB_NO_DIVISION

#/*  The available values for MCU_APB1_PRESCALER are 
    *   MCU_APB1_NO_DIVISION
    *   MCU_APB1_DIVIDED_BY_2
    *   MCU_APB1_DIVIDED_BY_4
    *   MCU_APB1_DIVIDED_BY_8
    *   MCU_APB1_DIVIDED_BY_16
*/
#define MCU_APB1_PRESCALER   MCU_APB1_NO_DIVISION

/*  The available values for MCU_APB2_PRESCALER are 
    *   MCU_APB2_NO_DIVISION
    *   MCU_APB2_DIVIDED_BY_2
    *   MCU_APB2_DIVIDED_BY_4
    *   MCU_APB2_DIVIDED_BY_8
    *   MCU_APB2_DIVIDED_BY_16
*/
#define MCU_APB2_PRESCALER   MCU_APB2_NO_DIVISION


/*  The available values for MCU_AHB1_PERIPHERALS_ENABLE are 
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
#define MCU_AHB1_PERIPHERALS_ENABLE   \
    (                                 \
      MCU_AHB1_GPIOB_CLOCK |          \
      MCU_AHB1_GPIOA_CLOCK            \
    )

        
/*  The available values for MCU_AHB2_PERIPHERALS_ENABLE are 
  --->> YOU CAN or more than value as this example
  *  MCU_AHB2_NO_PERIPHERAL             // you can use it alone if
  *  MCU_AHB2_OTGFS_CLOCK
*/
#define MCU_AHB2_PERIPHERALS_ENABLE   MCU_AHB2_NO_PERIPHERAL

/*  The available values for MCU_APB1_PERIPHERALS_ENABLE are 
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
#define MCU_APB1_PERIPHERALS_ENABLE     \
    (                                   \
      MCU_APB1_NO_PERIPHERAL            \
    )


    /*  The available values for MCU_APB2_PERIPHERALS_ENABLE are 
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
#define MCU_APB2_PERIPHERALS_ENABLE     \
    (                                   \
      MCU_APB2_NO_PERIPHERAL            \
    )

#endif  // MCU_CFG_H - End of header guard