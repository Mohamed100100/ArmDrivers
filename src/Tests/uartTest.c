

#include <LIB/stdtypes.h>
#include "HAL/MCU_Driver/mcu.h"
#include "MCAL/UART_Driver/uart.h"
#include "MCAL/NVIC_Driver/nvic_stm32f401cc.h"
#include "test.h"

void Test_SynTransmitUART(void);
void Test_SynReceiveUART(void);
void Test_AsynTransmitUART(void);
void Test_AsynReceiveUART(void);
void uartTxCallBack(void);
void uartRxCallBack(void);
UART_AsynBuffer_t rxbuffer;
UART_AsynBuffer_t txbuffer;

/**
 * I found that if you use the odd or even parity with 8 bit length
 * you should use 9 bit length to accommodate the parity bit.
 * So in this test I will use 9 bit length with even parity.
 */

void uartTest(){
    UART_Config_t uartConfig;

    uartConfig.UART_Number = UART_1;
    uartConfig.UartEnabled = UART_ENABLE_TRANSMITE|UART_ENABLE_RECEIVE; // Enable both transmitter and receiver
    uartConfig.Parity = UART_PARITY_NONE; // No parity
    uartConfig.OverSampling = UART_OVERSAMPLING_8;
    uartConfig.StopBits = UART_STOPBITS_1;
    uartConfig.WordLength = UART_WORDLENGTH_8B; // 8 bits
    uartConfig.Sample = UART_THREE_SAMPLE; // 3 samples
    uartConfig.InterruptFlags = UART_INTERRUPT_RXNE; // No interrupts
    uartConfig.PeripheralClock = 16000000UL;
    uartConfig.BaudRate = 4800UL;
    
    MCU_enuInit(&MCU_Configs);

    NVIC_BP_SetPriority(NVIC_USART1_IRQ, 1);
    NVIC_BP_EnableIRQ(NVIC_USART1_IRQ);

    UART_enuInit(&uartConfig);

    Test_SynTransmitUART();
    // Test_SynReceiveUART();
    // Test_AsynTransmitUART();
    // Test_AsynReceiveUART();    
}

void Test_SynTransmitUART(void){
    const char* message = "Hello, UART!";

    while (1)
    {
        UART_enuSynTransmitBuffer(UART_1, (const uint8_t*)message, sizeof("Hello, UART!"));        
    }
}

void Test_SynReceiveUART(void){
    const char* message = "Hello, UART!";

    char receiveBuffer[20] = {0};
    while (1)
    {
        UART_enuSynReceiveBuffer(UART_1, (uint8_t*)receiveBuffer, 20);
        UART_enuSynTransmitBuffer(UART_1, (const uint8_t*)message, sizeof("Hello, UART!"));
        
    }
}

void Test_AsynTransmitUART(void){
    const char* message = "Hello, UART!";

    UART_AsynBuffer_t txbuffer;
    txbuffer.callback = uartTxCallBack;
    txbuffer.size = sizeof("Hello, UART!");
    txbuffer.index = 0;
    txbuffer.buffer = (uint8_t*)message;

    while (1)
    {
        UART_enuAsynTransmitBuffer(UART_1, &txbuffer);        
    }
}

void Test_AsynReceiveUART(void){
    const char* message = "Hello, UART!";

    char receiveBuffer[20] = {0};

    UART_AsynBuffer_t rxbuffer;
    rxbuffer.callback = uartRxCallBack;
    rxbuffer.size = 20;
    rxbuffer.index = 0;
    rxbuffer.buffer = (uint8_t*)receiveBuffer;

    UART_enuAsynReceiveBuffer(UART_1, &rxbuffer);        
}


void uartRxCallBack(void){
    int a = 0;
    UART_enuAsynTransmitBuffer(UART_1, &txbuffer);
}

void uartTxCallBack(void){
    int b = 0;
    UART_enuAsynReceiveBuffer(UART_1, &rxbuffer);
}