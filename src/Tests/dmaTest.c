
#include "LIB/stdtypes.h"

#include "HAL/MCU_Driver/mcu.h"
#include "MCAL/NVIC_Driver/nvic_stm32f401cc.h"

#include "MCAL/UART_Driver/uart.h"
#include "MCAL/DMA_Driver/dma.h"


#include "test.h"

void uartTcCallback(void);
void rxCallback(void);

void DMA_Test_Transmit(void){
    DMA_Config_t dmaConfig;

    DMA_Status_t dmaStatus;
    UART_Status_t uartStatus;

    MCU_enuInit(&MCU_Configs);

    NVIC_BP_SetPriority(NVIC_USART1_IRQ, 1);
    NVIC_BP_EnableIRQ(NVIC_USART1_IRQ);

    char dataBuffer[100] = "This is a test buffer for DMA transmission via UART.";

    // Configure DMA for UART transmission

    // 1. configure the DMA_Config_t structure
    // 2. initialize the DMA with DMA_enuInit
    // 3. Activate DMA in UART using UART_enuActivateDMA
    // 4. register callback for uart transmission complete
    // 5. Start the DMA transfer with DMA_enuStartTransfer
    // 6. activate UART_TC interrupt in UART using UART_enuEnableInterrupts
    
    dmaConfig.DMAx               = DMA2;
    dmaConfig.Streamx            = DMA_STREAM7;
    dmaConfig.Channel            = DMA_CHANNEL4;
    dmaConfig.MBurst             = DMA_MBurst_SINGLE;
    dmaConfig.PBurst             = DMA_PBurst_SINGLE;
    dmaConfig.DoubleBuffer       = DMA_DISABLE_DOUBLE_BUFFER;
    dmaConfig.Priority           = DMA_PRIORITY_HIGH;
    dmaConfig.MSize              = DMA_MSIZE_BYTE;
    dmaConfig.PSize              = DMA_PSIZE_BYTE;
    dmaConfig.MemoryInc          = DMA_MINC_AUTO_INCREMENT;
    dmaConfig.PeripheralInc      = DMA_PINC_FIXED;
    dmaConfig.CircularMode       = DMA_CIRCULAR_MODE_DISABLE;
    dmaConfig.Direction          = DMA_DIRECTION_M2P;
    dmaConfig.PeripheralFlowCtrl = DMA_FLOW_CONTROL_USING_DMA;
    dmaConfig.Mode               = DMA_MODE_DIRECT;
    dmaConfig.FifoThreshold      = DMA_FIFO_THRESHOLD_FULL; // Not important at direct mode  
    dmaConfig.NumberOfData       = sizeof("This is a test buffer for DMA transmission via UART.");
    dmaConfig.PeripheralAddress  = 0x40011004; // Example peripheral address
    dmaConfig.Memory0Address     = (uint32_t)dataBuffer;
    dmaConfig.Memory1Address     = 0; // Not used in normal mode
    dmaConfig.Interrupts         = 0;


    UART_Config_t uartConfig;

    uartConfig.UART_Number = UART_1;
    uartConfig.UartEnabled = UART_ENABLE_TRANSMITE; // Enable both transmitter and receiver
    uartConfig.Parity = UART_PARITY_NONE; // No parity
    uartConfig.OverSampling = UART_OVERSAMPLING_8;
    uartConfig.StopBits = UART_STOPBITS_1;
    uartConfig.WordLength = UART_WORDLENGTH_8B; // 8 bits
    uartConfig.Sample = UART_THREE_SAMPLE; // 3 samples
    uartConfig.InterruptFlags = 0; // No interrupts
    uartConfig.PeripheralClock = 16000000UL;
    uartConfig.BaudRate = 9600UL;
    uartStatus = UART_enuInit(&uartConfig);

    dmaStatus = DMA_enuInit(&dmaConfig);

    uartStatus = UART_enuActivateDMA(UART_1, UART_DMA_TRANSMIT_ENABLE);

    UART_Callbacks_t uartCallbacks;
    uartCallbacks.ParityErrorCallback = NULL;
    uartCallbacks.FramingErrorCallback = NULL;
    uartCallbacks.NoiseErrorCallback = NULL;
    uartCallbacks.OverrunErrorCallback = NULL;
    uartCallbacks.TC_Callback = uartTcCallback;

    uartStatus = UART_enuRegisterCallbacks(UART_1, &uartCallbacks);
    
    UART_enuClearFlags(UART_1,UART_FLAG_TC);
    dmaStatus = DMA_enuStartTransfer(dmaConfig.DMAx, dmaConfig.Streamx);
    uartStatus = UART_enuEnableInterrupts(UART_1, UART_INTERRUPT_TC);


    while (1)
    {
        /* code */
    }
    
}

void uartTcCallback(void){
    // Transmission complete callback
    int a = 0;
    UART_enuDisableInterrupts(UART_1, UART_INTERRUPT_TC);
}


void DMA_Test_Receive(void){
    DMA_Config_t dmaConfig;

    DMA_Status_t dmaStatus;
    UART_Status_t uartStatus;

    MCU_enuInit(&MCU_Configs);

    NVIC_BP_SetPriority(NVIC_DMA2_STREAM5_IRQ, 1);
    NVIC_BP_EnableIRQ(NVIC_DMA2_STREAM5_IRQ);

    char dataBuffer[20] = {0};
    // Configure DMA for UART transmission

    // 1. configure the DMA_Config_t structure
    // 2. initialize the DMA with DMA_enuInit
    // 3. Activate DMA in UART using UART_enuActivateDMA
    // 4. register callback for uart transmission complete
    // 5. Start the DMA transfer with DMA_enuStartTransfer
    // 6. activate UART_TC interrupt in UART using UART_enuEnableInterrupts
    
    dmaConfig.DMAx               = DMA2;
    dmaConfig.Streamx            = DMA_STREAM5;
    dmaConfig.Channel            = DMA_CHANNEL4;
    dmaConfig.MBurst             = DMA_MBurst_SINGLE;
    dmaConfig.PBurst             = DMA_PBurst_SINGLE;
    dmaConfig.DoubleBuffer       = DMA_DISABLE_DOUBLE_BUFFER;
    dmaConfig.Priority           = DMA_PRIORITY_HIGH;
    dmaConfig.MSize              = DMA_MSIZE_BYTE;
    dmaConfig.PSize              = DMA_PSIZE_BYTE;
    dmaConfig.MemoryInc          = DMA_MINC_AUTO_INCREMENT;
    dmaConfig.PeripheralInc      = DMA_PINC_FIXED;
    dmaConfig.CircularMode       = DMA_CIRCULAR_MODE_DISABLE;
    dmaConfig.Direction          = DMA_DIRECTION_P2M;
    dmaConfig.PeripheralFlowCtrl = DMA_FLOW_CONTROL_USING_DMA;
    dmaConfig.Mode               = DMA_MODE_DIRECT;
    dmaConfig.FifoThreshold      = DMA_FIFO_THRESHOLD_FULL; // Not important at direct mode  
    dmaConfig.NumberOfData       = sizeof(dataBuffer);
    dmaConfig.PeripheralAddress  = 0x40011004; // Example peripheral address
    dmaConfig.Memory0Address     = (uint32_t)dataBuffer;
    dmaConfig.Memory1Address     = 0; // Not used in normal mode
    dmaConfig.Interrupts         = DMA_INTERRUPT_TRANSFER_COMPLETE_ENABLE;


    UART_Config_t uartConfig;

    uartConfig.UART_Number = UART_1;
    uartConfig.UartEnabled = UART_ENABLE_RECEIVE; // Enable both transmitter and receiver
    uartConfig.Parity = UART_PARITY_NONE; // No parity
    uartConfig.OverSampling = UART_OVERSAMPLING_8;
    uartConfig.StopBits = UART_STOPBITS_1;
    uartConfig.WordLength = UART_WORDLENGTH_8B; // 8 bits
    uartConfig.Sample = UART_THREE_SAMPLE; // 3 samples
    uartConfig.InterruptFlags = 0; // No interrupts
    uartConfig.PeripheralClock = 16000000UL;
    uartConfig.BaudRate = 9600UL;
    uartStatus = UART_enuInit(&uartConfig);

    dmaStatus = DMA_enuInit(&dmaConfig);

    uartStatus = UART_enuActivateDMA(UART_1, UART_DMA_RECEIVE_ENABLE);

    dmaStatus = DMA_enuRegisterCallback(dmaConfig.DMAx, dmaConfig.Streamx, DMA_INTERRUPT_TRANSMISSION_COMPLETE, rxCallback);
    
    dmaStatus = DMA_enuStartTransfer(dmaConfig.DMAx, dmaConfig.Streamx);


    while (1)
    {
        /* code */
    }
}

void rxCallback(void){
    // Reception complete callback
    int a = 0;
}