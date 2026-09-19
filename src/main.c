/******************************************************************************
 * @file    main.c
 * @author  Eng.Gemy
 * @brief   Main Application Entry — Test Harness for ArmDrivers
 *          This main does NOT contain application logic; it is a minimal
 *          test harness that calls one of the driver test functions declared
 *          in test.h / implemented in src/Tests/*.c. Each test verifies a
 *          single driver (GPIO, SPI, UART, DMA, FLASH, etc.) or a HAL
 *          integration (HSERIAL, LCD). Uncomment the desired test to run it
 *          after MCU/RCC init inside the test itself, then upload via ST-Link.
 * @date    2024
 * @version 1.0
 * @note    All tests configure their own MCU clocks/peripherals via
 *          MCU_enuInit(&MCU_Configs) inside each test function. Only one
 *          test should be active at a time (others remain commented).
 *          The infinite while(1) after the test keeps the MCU alive for
 *          debugger/observer; async tests rely on interrupts/scheduler.
 ******************************************************************************/

#include "test.h"  /* Declarations: gpioTest(), nvicTest(), uartTest(), test_SPI_*, Test_Hserial_*, Example_Program* etc. */



/******************************************************************************
 * @brief Main entry — runs a single driver test then idles
 * @details See test.h for full list. To test a different driver:
 *          1) Comment current active call (e.g., test_SPI_PollingTransmitReceive)
 *          2) Uncomment desired test (e.g., Test_Hserial_Dma_Uart, Example_ProgramData)
 *          3) Build & upload: pio run -e genericSTM32F401CC -t upload
 * @param None
 * @return int 0 (never reached due to while(1))
 ******************************************************************************/
int main(){
    
    // test_SPI_PollingTransmitReceive();  /* MCAL SPI — polling TX/RX single byte, see testSpi.c */
    // Test_Hserial_Sync_Uart();           /* HAL HSERIAL — UART sync blocking TX/RX, see testHserial.c */
    // Test_Hserial_Dma_Uart();            /* HAL HSERIAL — UART DMA (DMA1 Stream6 + USART2), non-blocking */
    // Test_Hserial_ASync_Uart();          /* HAL HSERIAL — UART async interrupt (TXE/RXNE + NVIC) */

    // test_SPI_AsyncTransmitReceive();    /* MCAL SPI — async interrupt with callbacks, see testSpi.c */



    // Example_ProgramData();              /* MCAL FLASH — unlock, erase sector 5, program word, see flashTest.c */
    // Example_MassErase();                /* MCAL FLASH — mass erase all sectors (WARNING: erases app!) */
    // Example_ProgramBuffer();            /* MCAL FLASH — program byte buffer with alignment handling */

    test_SPI_PollingTransmitReceive();    /* ACTIVE: MCAL SPI polling demo (SPI1 master SW NSS) */
    // test_SPI_AsyncTransmitReceive();   /* ALT: SPI async demo (keep one active at a time) */

    
    while(1){
        /* Idle loop — keeps MCU running after test. Async/DMA/interrupt tests
           continue via ISRs and scheduler (if SCHED_enuStart was called inside
           the test). For polling tests, execution already completed before here. */
    }

    return 0;

    
}
