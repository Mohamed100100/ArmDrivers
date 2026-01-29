
#include "LIB/stdtypes.h"
// #include "MCAL/UART_Driver/uart.h"
// #include "MCAL/DMA_Driver/dma.h"

#include "HAL/HSERIAL_Driver/hserial.h"
#include "HAL/HSERIAL_Driver/hserial_cfg.h"

extern void TxCallback(void);
extern void RxCallback(void);

const HSERIAL_Config_t HSERIAL_Configurations[HSERIAL_CHANNEL_LENGTH] = {
    // [HSERIAL_CHANNEL_1] ={
    //     .HSERIAL_Mode = HSERIAL_MODE_UART_DMA,
    //     .UART_Dma_Config = {
    //         .HSERIAL_UartChannel         = HSERIAL_UART_1,
    //         .HSERIAL_UartPeripheralClock = 16000000UL,
    //         .HSERIAL_UartBaudRate        = 9600UL,
    //         .HSERIAL_UartParity          = HSERIAL_UART_PARITY_NONE,
    //         .HSERIAL_UartOverSampling    = HSERIAL_UART_OVERSAMPLING_16,
    //         .HSERIAL_UartStopBits        = HSERIAL_UART_STOPBITS_1,
    //         .HSERIAL_UartWordLength      = HSERIAL_UART_WORDLENGTH_8B,
    //         .HSERIAL_UartSample          = HSERIAL_UART_ONE_SAMPLE,
    //         .HSERIAL_UartEnable          = HSERIAL_ENABLE_UART_BOTH,
    //         .HSERIAL_UartTxCompleteCallback = TxCallback,
    //         .HSERIAL_UartRxCompleteCallback = RxCallback,
    //         .HSERIAL_UartInterruptPriority  = HSERIAL_PRIORITY_2
    //     }
    // },
    [HSERIAL_CHANNEL_1] = {
        .HSERIAL_Mode = HSERIAL_MODE_UART_ASYNC,
        .UART_Async_Config = {
            .HSERIAL_UartPeripheralClock    = 16000000UL,
            .HSERIAL_UartChannel            = HSERIAL_UART_1,
            .HSERIAL_UartBaudRate           = 9600UL,
            .HSERIAL_UartParity             = HSERIAL_UART_PARITY_NONE,
            .HSERIAL_UartOverSampling       = HSERIAL_UART_OVERSAMPLING_16,
            .HSERIAL_UartStopBits           = HSERIAL_UART_STOPBITS_1,
            .HSERIAL_UartWordLength         = HSERIAL_UART_WORDLENGTH_8B,
            .HSERIAL_UartSample             = HSERIAL_UART_ONE_SAMPLE,
            .HSERIAL_UartEnable             = HSERIAL_ENABLE_UART_BOTH,
            .HSERIAL_UartTxCompleteCallback = TxCallback,
            .HSERIAL_UartRxCompleteCallback = RxCallback,
            .HSERIAL_UartInterruptPriority  = HSERIAL_PRIORITY_2
        },
    },

    // [HSERIAL_CHANNEL_2] ={
    //     .HSERIAL_Mode = HSERIAL_MODE_UART_ASYNC,
    //     .UART_Async_Config = {
    //         .HSERIAL_UartPeripheralClock = 8000000UL,
    //         .HSERIAL_UartChannel = HSERIAL_UART_2,
    //         .HSERIAL_UartBaudRate = 115200UL,
    //         .HSERIAL_UartParity = HSERIAL_UART_PARITY_NONE,
    //         .HSERIAL_UartOverSampling = HSERIAL_UART_OVERSAMPLING_16,
    //         .HSERIAL_UartStopBits = HSERIAL_UART_STOPBITS_1,
    //         .HSERIAL_UartWordLength = HSERIAL_UART_WORDLENGTH_8B,
    //         .HSERIAL_UartSample = HSERIAL_UART_ONE_SAMPLE,
    //         .HSERIAL_UartEnable = HSERIAL_ENABLE_UART_BOTH
    //     }
    // },
    // [HSERIAL_SENSOR_TEMP] ={
    //     .HSERIAL_Mode = HSERIAL_MODE_UART_ASYNC,
    //     .UART_Async_Config = {
    //         .HSERIAL_UartPeripheralClock = 8000000UL,
    //         .HSERIAL_UartChannel = HSERIAL_UART_6,
    //         .HSERIAL_UartBaudRate = 4800UL,
    //         .HSERIAL_UartParity          = HSERIAL_UART_PARITY_NONE,
    //         .HSERIAL_UartOverSampling = HSERIAL_UART_OVERSAMPLING_16,
    //         .HSERIAL_UartStopBits = HSERIAL_UART_STOPBITS_1,
    //         .HSERIAL_UartWordLength = HSERIAL_UART_WORDLENGTH_8B,
    //         .HSERIAL_UartSample = HSERIAL_UART_ONE_SAMPLE,
    //         .HSERIAL_UartEnable = HSERIAL_ENABLE_UART_BOTH
    //     }
    // }
};