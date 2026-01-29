

#include "LIB/stdtypes.h"
#include "MCAL/UART_Driver/uart.h"
#include "MCAL/NVIC_Driver/nvic_stm32f401cc.h"
#include "MCAL/DMA_Driver/dma.h"
#include "MCAL/SPI_Driver/spi.h"

#include "HAL/HSERIAL_Driver/hserial.h"
#include "HAL/HSERIAL_Driver/hserial_cfg.h"
typedef struct {
    HSERIAL_Spi_Mode_t         HSERIAL_SpiMode;
    uint16_t* Buffer;
    uint16_t  Size;
    uint16_t  index;
}HSERIAL_SpiBuffer_t;

typedef struct {
    DMA_Controller_t    DMA_Controller;
    DMA_Stream_t        DMA_Stream;
    DMA_Channel_t       DMA_Channel;
}H_Dma_info_t;

static HSERIAL_SpiBuffer_t HSerialSpiBuffers[4] ={
    {HSERIAL_SPI_MASTER, NULL, 0, 0},
    {HSERIAL_SPI_MASTER, NULL, 0, 0},
    {HSERIAL_SPI_MASTER, NULL, 0, 0},
    {HSERIAL_SPI_MASTER, NULL, 0, 0}
};

uint32_t UART_ADDRESSS[3] = {
    0x40011000UL, // USART1_BASE_ADDRESS
    0x40004400UL, // USART2_BASE_ADDRESS
    0x40011400UL  // USART6_BASE_ADDRESS
};

static HSERIAL_Callback_t HSerialSpiCallbacks[4][2] = {
    {NULL,NULL},
    {NULL,NULL},
    {NULL,NULL},
    {NULL,NULL}
};


static const H_Dma_info_t HSERIAL_UART_TX_DMA_Map[] = {
    [HSERIAL_UART_1] = {
        .DMA_Controller = DMA2,
        .DMA_Stream     = DMA_STREAM7,
        .DMA_Channel    = DMA_CHANNEL4
    },
    [HSERIAL_UART_2] = {
        .DMA_Controller = DMA1,
        .DMA_Stream     = DMA_STREAM6,
        .DMA_Channel    = DMA_CHANNEL4
    },
    [HSERIAL_UART_6] = {
        .DMA_Controller = DMA2,
        .DMA_Stream     = DMA_STREAM1,
        .DMA_Channel    = DMA_CHANNEL5
    }
};
static const H_Dma_info_t HSERIAL_UART_RX_DMA_Map[] = {
    [HSERIAL_UART_1] = {
        .DMA_Controller = DMA2,
        .DMA_Stream     = DMA_STREAM5,
        .DMA_Channel    = DMA_CHANNEL4
    },
    [HSERIAL_UART_2] = {
        .DMA_Controller = DMA1,
        .DMA_Stream     = DMA_STREAM5,
        .DMA_Channel    = DMA_CHANNEL4
    },
    [HSERIAL_UART_6] = {
        .DMA_Controller = DMA2,
        .DMA_Stream     = DMA_STREAM2,
        .DMA_Channel    = DMA_CHANNEL5
    }
};

static const HSERIAL_Uart_Number_t HSERIAL_UART_NVIC_IRQ_Map[] = {
    NVIC_USART1_IRQ,
    NVIC_USART2_IRQ,
    NVIC_USART6_IRQ
};

static const HSERIAL_Spi_Number_t HSERIAL_SPI_NVIC_IRQ_Map[] = {
    NVIC_SPI1_IRQ,
    NVIC_SPI2_IRQ,
    NVIC_SPI3_IRQ,
    NVIC_SPI4_IRQ
};

static const HSERIAL_Uart_Number_t HSERIAL_DMA_NVIC_IRQ_Map[] = {
    NVIC_DMA2_STREAM5_IRQ, // for USART1 RX
    NVIC_DMA1_STREAM5_IRQ, // for USART2 RX
    NVIC_DMA2_STREAM1_IRQ  // for USART6 RX
};
extern const HSERIAL_Config_t HSERIAL_Configurations[HSERIAL_CHANNEL_LENGTH];

static HSERIAL_Status_t HSERIAL_enuSyncInitUart(HSERIAL_Channel_t channel);
static HSERIAL_Status_t HSERIAL_enuAsyncInitUart(HSERIAL_Channel_t channel);
static HSERIAL_Status_t HSERIAL_enuDmaInitUart(HSERIAL_Channel_t channel);
static HSERIAL_Status_t HSERIAL_enuSyncInitSpi(HSERIAL_Channel_t channel);
static HSERIAL_Status_t HSERIAL_enuAsyncInitSpi(HSERIAL_Channel_t channel);


static HSERIAL_Status_t HSERIAL_enuUARTSyncTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size);
static HSERIAL_Status_t HSERIAL_enuUARTAsyncTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size);
static HSERIAL_Status_t HSERIAL_enuUARTDmaTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size);
static HSERIAL_Status_t HSERIAL_enuSPISyncTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size);
static HSERIAL_Status_t HSERIAL_enuSPIAsyncTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size);

static HSERIAL_Status_t HSERIAL_enuUARTSyncReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size);
static HSERIAL_Status_t HSERIAL_enuUARTDmaReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size);
static HSERIAL_Status_t HSERIAL_enuUARTAsyncReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size);
static HSERIAL_Status_t HSERIAL_enuSPISyncReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size);
static HSERIAL_Status_t HSERIAL_enuSPIAsyncReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size);

static void HSERIAL_SpiCallBackHandler_1_Tx(void);
static void HSERIAL_SpiCallBackHandler_2_Tx(void);
static void HSERIAL_SpiCallBackHandler_3_Tx(void);
static void HSERIAL_SpiCallBackHandler_4_Tx(void);
static void HSERIAL_SpiCallBackHandler_1_Rx(void);
static void HSERIAL_SpiCallBackHandler_2_Rx(void);
static void HSERIAL_SpiCallBackHandler_3_Rx(void);
static void HSERIAL_SpiCallBackHandler_4_Rx(void);

static void LocalTxCallback(SPI_Number_t spiNumber);
static void LocalRxCallback(SPI_Number_t spiNumber);

static HSERIAL_Callback_t LocalSpiCallbacks[4][2] = {
    {HSERIAL_SpiCallBackHandler_1_Tx, HSERIAL_SpiCallBackHandler_1_Rx},
    {HSERIAL_SpiCallBackHandler_2_Tx, HSERIAL_SpiCallBackHandler_2_Rx},
    {HSERIAL_SpiCallBackHandler_3_Tx, HSERIAL_SpiCallBackHandler_3_Rx},
    {HSERIAL_SpiCallBackHandler_4_Tx, HSERIAL_SpiCallBackHandler_4_Rx}
};


HSERIAL_Status_t HSERIAL_enuInit(void){
    HSERIAL_Status_t status = HSERIAL_NOT_OK;

    for(uint8_t i = 0; i < HSERIAL_CHANNEL_LENGTH; i++){
        switch(HSERIAL_Configurations[i].HSERIAL_Mode){
            case HSERIAL_MODE_UART_ASYNC:
                status = HSERIAL_enuAsyncInitUart(i);
                break;
            case HSERIAL_MODE_UART_SYNC:
                status = HSERIAL_enuSyncInitUart(i);
                break;
            case HSERIAL_MODE_UART_DMA:
                status =  HSERIAL_enuDmaInitUart(i);
                break;
            case HSERIAL_MODE_SPI_SYNC:
                status = HSERIAL_enuSyncInitSpi(i);
                break;
            case HSERIAL_MODE_SPI_ASYNC:
                status = HSERIAL_enuAsyncInitSpi(i);
                break;
            case HSERIAL_MODE_SPI_DMA:
                // status = SPI_enuInitDMA(&HSERIAL_Configurations[i].SPI_Config);
                break;
            default:
                // Invalid mode
                status = HSERIAL_NOT_OK;
                break;
        }
    }
    return status;
}


HSERIAL_Status_t HSERIAL_enuTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;

    if(channel >= HSERIAL_CHANNEL_LENGTH){
        retStatus = HSERIAL_WRONG_CHANNEL;
    }else if (dataBuffer == NULL ){
        retStatus = HSERIAL_NULL_POINTER;
    }else if (size == 0){
        retStatus = HSERIAL_INVALID_SIZE;
    }else{
    
        const HSERIAL_Config_t* config = &HSERIAL_Configurations[channel];
        switch(config->HSERIAL_Mode){
            case HSERIAL_MODE_UART_ASYNC:
                retStatus = HSERIAL_enuUARTAsyncTransmitBuffer(channel, dataBuffer, size);
                break;
            case HSERIAL_MODE_UART_SYNC:
                retStatus = HSERIAL_enuUARTSyncTransmitBuffer(channel, dataBuffer, size);
                break;
            case HSERIAL_MODE_UART_DMA:
                retStatus = HSERIAL_enuUARTDmaTransmitBuffer(channel, dataBuffer, size);
                break;
            case HSERIAL_MODE_SPI_SYNC:
                retStatus = HSERIAL_enuSPISyncTransmitBuffer(channel, dataBuffer, size);
                break;
            case HSERIAL_MODE_SPI_ASYNC:
                retStatus = HSERIAL_enuSPIAsyncTransmitBuffer(channel, dataBuffer, size);
                break;
            default:
                retStatus = HSERIAL_NOT_OK;
                break;
        }
    }
    return retStatus;
}

HSERIAL_Status_t HSERIAL_enuReceiveBuffer(HSERIAL_Channel_t channel,uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;

    if(channel >= HSERIAL_CHANNEL_LENGTH){
        retStatus = HSERIAL_WRONG_CHANNEL;
    }else if (dataBuffer == NULL ){
        retStatus = HSERIAL_NULL_POINTER;
    }else if (size == 0){
        retStatus = HSERIAL_INVALID_SIZE;
    }else{
    
        const HSERIAL_Config_t* config = &HSERIAL_Configurations[channel];
        switch(config->HSERIAL_Mode){
            case HSERIAL_MODE_UART_ASYNC:
                retStatus = HSERIAL_enuUARTAsyncReceiveBuffer(channel, dataBuffer, size);
                break;
            case HSERIAL_MODE_UART_SYNC:
                retStatus = HSERIAL_enuUARTSyncReceiveBuffer(channel, dataBuffer, size);
                break;
            case HSERIAL_MODE_UART_DMA:
                retStatus = HSERIAL_enuUARTDmaReceiveBuffer(channel, dataBuffer, size);
                break;
            case HSERIAL_MODE_SPI_SYNC:
                retStatus = HSERIAL_enuSPISyncReceiveBuffer(channel, (uint8_t*)dataBuffer, size);
                break;
            case HSERIAL_MODE_SPI_ASYNC:
                retStatus = HSERIAL_enuSPIAsyncReceiveBuffer(channel, dataBuffer, size);
                break;
            default:
                retStatus = HSERIAL_NOT_OK;
                break;
        }
    }
    return retStatus;
}


static HSERIAL_Status_t HSERIAL_enuSyncInitUart(HSERIAL_Channel_t channel){
    HSERIAL_Status_t status = HSERIAL_NOT_OK;

    const H_UART_Sync_Config_t *H_uartConfig = &(HSERIAL_Configurations[channel].UART_Sync_Config);

    UART_Config_t uartCfg;
    uartCfg.UART_Number         = H_uartConfig->HSERIAL_UartChannel;
    uartCfg.PeripheralClock     = H_uartConfig->HSERIAL_UartPeripheralClock;
    uartCfg.BaudRate            = H_uartConfig->HSERIAL_UartBaudRate;
    uartCfg.Parity              = H_uartConfig->HSERIAL_UartParity;
    uartCfg.OverSampling        = H_uartConfig->HSERIAL_UartOverSampling;
    uartCfg.StopBits            = H_uartConfig->HSERIAL_UartStopBits;
    uartCfg.WordLength          = H_uartConfig->HSERIAL_UartWordLength;
    uartCfg.Sample              = H_uartConfig->HSERIAL_UartSample;
    uartCfg.UartEnabled         = H_uartConfig->HSERIAL_UartEnable;
    uartCfg.InterruptFlags      = 0;
    
    UART_Status_t uartStatus = UART_enuInit(&uartCfg);

    if(uartStatus == UART_OK){
        status = HSERIAL_OK;
    }else{
        status = HSERIAL_ERROR_INIT_UART;
    }
    return status;
}


static HSERIAL_Status_t HSERIAL_enuAsyncInitUart(HSERIAL_Channel_t channel){
    HSERIAL_Status_t status = HSERIAL_NOT_OK;

    const H_UART_Async_Config_t *H_uartConfig = &(HSERIAL_Configurations[channel].UART_Async_Config);

    UART_Config_t uartCfg;
    uartCfg.UART_Number         = H_uartConfig->HSERIAL_UartChannel;
    uartCfg.PeripheralClock     = H_uartConfig->HSERIAL_UartPeripheralClock;
    uartCfg.BaudRate            = H_uartConfig->HSERIAL_UartBaudRate;
    uartCfg.Parity              = H_uartConfig->HSERIAL_UartParity;
    uartCfg.OverSampling        = H_uartConfig->HSERIAL_UartOverSampling;
    uartCfg.StopBits            = H_uartConfig->HSERIAL_UartStopBits;
    uartCfg.WordLength          = H_uartConfig->HSERIAL_UartWordLength;
    uartCfg.Sample              = H_uartConfig->HSERIAL_UartSample;
    uartCfg.UartEnabled         = H_uartConfig->HSERIAL_UartEnable;
    uartCfg.InterruptFlags      = 0;

    UART_Status_t uartStatus = UART_enuInit(&uartCfg);

    if(uartStatus == UART_OK){
        NVIC_BP_Status_t nvicStatus = NVIC_BP_EnableIRQ(HSERIAL_UART_NVIC_IRQ_Map[H_uartConfig->HSERIAL_UartChannel]);
        if( nvicStatus != NVIC_BP_OK ){
            status = HSERIAL_ERROR_NVIC;
        }else{
            nvicStatus = NVIC_BP_SetPriority(HSERIAL_UART_NVIC_IRQ_Map[H_uartConfig->HSERIAL_UartChannel], H_uartConfig->HSERIAL_UartInterruptPriority);
            if( nvicStatus != NVIC_BP_OK ){
                status = HSERIAL_ERROR_NVIC;
            }else{
                status = HSERIAL_OK;
            }
        }
    }else{
        status = HSERIAL_ERROR_INIT_UART;
    }

    return status;
}


static HSERIAL_Status_t HSERIAL_enuDmaInitUart(HSERIAL_Channel_t channel){
    HSERIAL_Status_t status = HSERIAL_OK;

    const H_UART_Dma_Config_t *H_uartConfig = &(HSERIAL_Configurations[channel].UART_Dma_Config);

    UART_Config_t uartCfg;
    uartCfg.UART_Number         = H_uartConfig->HSERIAL_UartChannel;
    uartCfg.PeripheralClock     = H_uartConfig->HSERIAL_UartPeripheralClock;
    uartCfg.BaudRate            = H_uartConfig->HSERIAL_UartBaudRate;
    uartCfg.Parity              = H_uartConfig->HSERIAL_UartParity;
    uartCfg.OverSampling        = H_uartConfig->HSERIAL_UartOverSampling;
    uartCfg.StopBits            = H_uartConfig->HSERIAL_UartStopBits;
    uartCfg.WordLength          = H_uartConfig->HSERIAL_UartWordLength;
    uartCfg.Sample              = H_uartConfig->HSERIAL_UartSample;
    uartCfg.UartEnabled         = H_uartConfig->HSERIAL_UartEnable;
    uartCfg.InterruptFlags      = 0;

    UART_Status_t uartStatus = UART_enuInit(&uartCfg);

    if(uartStatus == UART_OK){
        if((H_uartConfig->HSERIAL_UartEnable & HSERIAL_ENABLE_UART_TRANSMITE) == HSERIAL_ENABLE_UART_TRANSMITE){
            
            NVIC_BP_Status_t nvicStatus = NVIC_BP_EnableIRQ(HSERIAL_UART_NVIC_IRQ_Map[H_uartConfig->HSERIAL_UartChannel]);
            if( nvicStatus != NVIC_BP_OK ){
                status = HSERIAL_ERROR_NVIC;
            }else{
                nvicStatus = NVIC_BP_SetPriority(HSERIAL_UART_NVIC_IRQ_Map[H_uartConfig->HSERIAL_UartChannel], H_uartConfig->HSERIAL_UartInterruptPriority);
                if( nvicStatus != NVIC_BP_OK ){
                    status = HSERIAL_ERROR_NVIC;
                }else{
                    DMA_Config_t dmaConfig;
                        dmaConfig.DMAx               = HSERIAL_UART_TX_DMA_Map[H_uartConfig->HSERIAL_UartChannel].DMA_Controller;
                        dmaConfig.Streamx            = HSERIAL_UART_TX_DMA_Map[H_uartConfig->HSERIAL_UartChannel].DMA_Stream;
                        dmaConfig.Channel            = HSERIAL_UART_TX_DMA_Map[H_uartConfig->HSERIAL_UartChannel].DMA_Channel;
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
                        dmaConfig.PeripheralAddress  = UART_ADDRESSS[H_uartConfig->HSERIAL_UartChannel] + 0x04; // Example peripheral address
                        dmaConfig.Memory1Address     = 0; // Not used in normal mode
                        dmaConfig.Interrupts         = 0;

                    DMA_Status_t dmaStatus = DMA_enuInit(&dmaConfig);
                    if(dmaStatus != DMA_OK){
                        status = HSERIAL_ERROR_INIT_DMA;
                    }else{
                        UART_Status_t uartStatus = UART_enuActivateDMA(H_uartConfig->HSERIAL_UartChannel, UART_DMA_TRANSMIT_ENABLE);
                        if(uartStatus != UART_OK){
                            status = HSERIAL_ERROR_INIT_DMA;
                        }else{

                            UART_Callbacks_t uartCallbacks;
                            uartCallbacks.ParityErrorCallback = NULL;
                            uartCallbacks.FramingErrorCallback = NULL;
                            uartCallbacks.NoiseErrorCallback = NULL;
                            uartCallbacks.OverrunErrorCallback = NULL;
                            uartCallbacks.TC_Callback = H_uartConfig->HSERIAL_UartTxCompleteCallback;

                            uartStatus = UART_enuRegisterCallbacks(H_uartConfig->HSERIAL_UartChannel, &uartCallbacks);
                            if(uartStatus != UART_OK){
                                status = HSERIAL_ERROR_INIT_DMA;
                            }else{
                                status = HSERIAL_OK;
                            }
                        }
                    }
                }
            }
        }else{
            // Do nothing
        }

        if(status == HSERIAL_OK ){
            if((H_uartConfig->HSERIAL_UartEnable & HSERIAL_ENABLE_UART_RECEIVE) == HSERIAL_ENABLE_UART_RECEIVE){

                NVIC_BP_Status_t nvicStatus = NVIC_BP_EnableIRQ(HSERIAL_DMA_NVIC_IRQ_Map[H_uartConfig->HSERIAL_UartChannel]);
                if( nvicStatus != NVIC_BP_OK ){
                    status = HSERIAL_ERROR_NVIC;
                }else{
                    nvicStatus = NVIC_BP_SetPriority(HSERIAL_DMA_NVIC_IRQ_Map[H_uartConfig->HSERIAL_UartChannel], H_uartConfig->HSERIAL_UartInterruptPriority);
                    if( nvicStatus != NVIC_BP_OK ){
                        status = HSERIAL_ERROR_NVIC;
                    }else{
                        DMA_Config_t dmaConfig;
                        dmaConfig.DMAx               = HSERIAL_UART_RX_DMA_Map[H_uartConfig->HSERIAL_UartChannel].DMA_Controller;
                        dmaConfig.Streamx            = HSERIAL_UART_RX_DMA_Map[H_uartConfig->HSERIAL_UartChannel].DMA_Stream;
                        dmaConfig.Channel            = HSERIAL_UART_RX_DMA_Map[H_uartConfig->HSERIAL_UartChannel].DMA_Channel;
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
                        dmaConfig.PeripheralAddress  = UART_ADDRESSS[H_uartConfig->HSERIAL_UartChannel] + 0x04; // Example peripheral address
                        dmaConfig.Memory1Address     = 0; // Not used in normal mode
                        dmaConfig.Interrupts         = DMA_INTERRUPT_TRANSFER_COMPLETE_ENABLE;

                        DMA_Status_t dmaStatus =  DMA_enuInit(&dmaConfig);
                        if(dmaStatus != DMA_OK){
                            status = HSERIAL_ERROR_INIT_DMA;
                        }else{
                            UART_Status_t uartStatus = UART_enuActivateDMA(H_uartConfig->HSERIAL_UartChannel, UART_DMA_RECEIVE_ENABLE);
                            if(uartStatus != UART_OK){
                                status = HSERIAL_ERROR_INIT_DMA;
                            }else{
                                
                                dmaStatus = DMA_enuRegisterCallback(dmaConfig.DMAx, dmaConfig.Streamx, DMA_INTERRUPT_TRANSMISSION_COMPLETE, H_uartConfig->HSERIAL_UartRxCompleteCallback);
                                if(dmaStatus != DMA_OK){
                                    status = HSERIAL_ERROR_INIT_DMA;
                                }else{
                                    status = HSERIAL_OK;
                                }
                            }
                        }
                    }
                }
            }else{
                // Do nothing
            }
        }else{
            // Do nothing
        }
    }else{
        status = HSERIAL_ERROR_INIT_UART;
    }

    return status;
}


static HSERIAL_Status_t HSERIAL_enuSyncInitSpi(HSERIAL_Channel_t channel){
    HSERIAL_Status_t status = HSERIAL_NOT_OK;

    const H_SPI_Sync_Config_t *H_spiConfig = &(HSERIAL_Configurations[channel].SPI_Sync_Config);

    SPI_Config_t spiCfg;
    spiCfg.spiNumber            = H_spiConfig->HSERIAL_SpiChannel;
    spiCfg.communicationMode    = H_spiConfig->HSERIAL_SpiCommunicationMode;
    spiCfg.mode                 = H_spiConfig->HSERIAL_SpiMode;
    spiCfg.frameFormat          = H_spiConfig->HSERIAL_SpiFrameFormat;
    spiCfg.polarityPhase        = H_spiConfig->HSERIAL_SpiPolarityPhase;
    spiCfg.nssManagement        = H_spiConfig->HSERIAL_SpiNss;
    spiCfg.baudRate             = H_spiConfig->HSERIAL_SpiBaudRate;
    spiCfg.dataOrder            = H_spiConfig->HSERIAL_SpiDataOrder;
    spiCfg.crcState             = H_spiConfig->HSERIAL_SpiCrc; 
    spiCfg.dataLength           = H_spiConfig->HSERIAL_SpiDataLength;
    spiCfg.dmaState             = SPI_DISABLE_DMA;
    spiCfg.crcPolynomial        = 0;
    spiCfg.slavesConfig.numberOfSlaves = 0;

    SPI_Status_t spiStatus = SPI_enuInit(&spiCfg);

    if(spiStatus == SPI_OK){
        status = HSERIAL_OK;
    }else{
        status = HSERIAL_ERROR_INIT_SPI;
    }
    return status;
}



static HSERIAL_Status_t HSERIAL_enuAsyncInitSpi(HSERIAL_Channel_t channel){
    HSERIAL_Status_t status = HSERIAL_NOT_OK;

    const H_SPI_Async_Config_t *H_spiConfig = &(HSERIAL_Configurations[channel].SPI_Async_Config);

    SPI_Config_t spiCfg;
    spiCfg.spiNumber            = H_spiConfig->HSERIAL_SpiChannel;
    spiCfg.communicationMode    = H_spiConfig->HSERIAL_SpiCommunicationMode;
    spiCfg.mode                 = H_spiConfig->HSERIAL_SpiMode;
    spiCfg.frameFormat          = H_spiConfig->HSERIAL_SpiFrameFormat;
    spiCfg.polarityPhase        = H_spiConfig->HSERIAL_SpiPolarityPhase;
    spiCfg.nssManagement        = H_spiConfig->HSERIAL_SpiNss;
    spiCfg.baudRate             = H_spiConfig->HSERIAL_SpiBaudRate;
    spiCfg.dataOrder            = H_spiConfig->HSERIAL_SpiDataOrder;
    spiCfg.crcState             = H_spiConfig->HSERIAL_SpiCrc; 
    spiCfg.dataLength           = H_spiConfig->HSERIAL_SpiDataLength;
    spiCfg.dmaState             = SPI_DISABLE_DMA;
    spiCfg.crcPolynomial        = 0;
    spiCfg.slavesConfig.numberOfSlaves = 0;

    SPI_Status_t spiStatus = SPI_enuInit(&spiCfg);

    if(spiStatus == SPI_OK){
        NVIC_BP_Status_t nvicStatus = NVIC_BP_EnableIRQ(HSERIAL_SPI_NVIC_IRQ_Map[H_spiConfig->HSERIAL_SpiChannel]);
        if( nvicStatus != NVIC_BP_OK ){
            status = HSERIAL_ERROR_NVIC;
        }else{
            nvicStatus = NVIC_BP_SetPriority(HSERIAL_SPI_NVIC_IRQ_Map[H_spiConfig->HSERIAL_SpiChannel], H_spiConfig->HSERIAL_SpiInterruptPriority);
            if( nvicStatus != NVIC_BP_OK ){
                status = HSERIAL_ERROR_NVIC;
            }else{
                status = HSERIAL_OK;
            }
        }
    }else{
        status = HSERIAL_ERROR_INIT_SPI;
    }
    return status;
}

static HSERIAL_Status_t HSERIAL_enuUARTSyncTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;
    UART_Status_t uartStatus;

    const HSERIAL_Config_t* config = &HSERIAL_Configurations[channel];
    uartStatus = UART_enuSynTransmitBuffer(config->UART_Sync_Config.HSERIAL_UartChannel, dataBuffer, size);
    if(uartStatus != UART_OK){
        retStatus = HSERIAL_FAILED_TRANSMIT;
    }else{
        retStatus = HSERIAL_OK;
    }
    return retStatus;
}

static HSERIAL_Status_t HSERIAL_enuSPISyncTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;

    const H_SPI_Sync_Config_t* config = &HSERIAL_Configurations[channel].SPI_Sync_Config;
    SPI_Status_t spiStatus;
    for(uint16_t i = 0; i < size; i++){
        if(config->HSERIAL_SpiMode == HSERIAL_SPI_MASTER){
            // use master functions in spi.c
            spiStatus = SPI_enuMasterSyncTransmit(config->HSERIAL_SpiChannel, dataBuffer[i]);
        }else{
            // use slave functions in
            spiStatus = SPI_enuSlaveSyncTransmit(config->HSERIAL_SpiChannel, dataBuffer[i]);
        }
        if(spiStatus != SPI_OK){
            retStatus = HSERIAL_FAILED_TRANSMIT;
            break;
        }else{
            retStatus = HSERIAL_OK;
        }
    }
    return retStatus;
}


static HSERIAL_Status_t HSERIAL_enuUARTAsyncTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;
    UART_Status_t uartStatus;
    
    const HSERIAL_Config_t* config = &HSERIAL_Configurations[channel];
    
    UART_AsynBuffer_t txBuffer;
    txBuffer.buffer = (uint8_t*)dataBuffer;
    txBuffer.size = size;
    txBuffer.index = 0;
    txBuffer.callback = config->UART_Async_Config.HSERIAL_UartTxCompleteCallback;
    uartStatus = UART_enuAsynTransmitBuffer(config->UART_Async_Config.HSERIAL_UartChannel, &txBuffer);
    if(uartStatus != UART_OK){
        retStatus = HSERIAL_FAILED_TRANSMIT;
    }else{
        retStatus = HSERIAL_OK;
    }
    
    return retStatus;
}




static HSERIAL_Status_t HSERIAL_enuUARTDmaTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;
    UART_Status_t uartStatus;
    
    const H_UART_Dma_Config_t* config = &HSERIAL_Configurations[channel].UART_Dma_Config;
    
    DMA_Status_t dmaStatus = DMA_enuSetMemoryAddress(HSERIAL_UART_TX_DMA_Map[config->HSERIAL_UartChannel].DMA_Controller,
                                                            HSERIAL_UART_TX_DMA_Map[config->HSERIAL_UartChannel].DMA_Stream,
                                                            (uint32_t)dataBuffer);
    if(dmaStatus != DMA_OK){
        retStatus = HSERIAL_FAILED_TRANSMIT;
    }else{
        dmaStatus = DMA_enuSetNumberOfData(HSERIAL_UART_TX_DMA_Map[config->HSERIAL_UartChannel].DMA_Controller,
                                                            HSERIAL_UART_TX_DMA_Map[config->HSERIAL_UartChannel].DMA_Stream,
                                                            size);
        if(dmaStatus != DMA_OK){
            retStatus = HSERIAL_FAILED_TRANSMIT;
        }else{

            uartStatus = UART_enuClearFlags(UART_1,UART_FLAG_TC);
            if(uartStatus != UART_OK){
                retStatus = HSERIAL_FAILED_TRANSMIT;
            }else{
                dmaStatus = DMA_enuStartTransfer(HSERIAL_UART_TX_DMA_Map[config->HSERIAL_UartChannel].DMA_Controller,
                                                                HSERIAL_UART_TX_DMA_Map[config->HSERIAL_UartChannel].DMA_Stream);
                if(dmaStatus != DMA_OK){
                    retStatus = HSERIAL_FAILED_TRANSMIT;
                }else{
                    uartStatus = UART_enuEnableInterrupts(UART_1, UART_INTERRUPT_TC);
                    if(uartStatus != UART_OK){
                        retStatus = HSERIAL_FAILED_TRANSMIT;
                    }else{
                        retStatus = HSERIAL_OK;
                    }
                }
            }
        }
    }
    return retStatus;
}

static HSERIAL_Status_t HSERIAL_enuSPIAsyncTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;

    const H_SPI_Async_Config_t* config = &HSERIAL_Configurations[channel].SPI_Async_Config;
    
    SPI_Status_t spiStatus;

    HSerialSpiCallbacks[config->HSERIAL_SpiChannel][0] = HSERIAL_Configurations[channel].SPI_Async_Config.HSERIAL_SpiTxCompleteCallback;
    HSerialSpiBuffers[config->HSERIAL_SpiChannel].Buffer = (uint16_t*)dataBuffer;
    HSerialSpiBuffers[config->HSERIAL_SpiChannel].Size = size;
    HSerialSpiBuffers[config->HSERIAL_SpiChannel].HSERIAL_SpiMode = config->HSERIAL_SpiMode;
    HSerialSpiBuffers[config->HSERIAL_SpiChannel].index = 1;
    
    if(config->HSERIAL_SpiMode == HSERIAL_SPI_MASTER){
        // use master functions in spi.c
        spiStatus = SPI_enuMasterAsynTransmit(config->HSERIAL_SpiChannel, dataBuffer[0],LocalSpiCallbacks[config->HSERIAL_SpiChannel][0]);
    }else{
        // use slave functions in
        spiStatus = SPI_enuSlaveAsynTransmit(config->HSERIAL_SpiChannel, dataBuffer[0],LocalSpiCallbacks[config->HSERIAL_SpiChannel][0]);
    }
    if(spiStatus != SPI_OK){
        retStatus = HSERIAL_FAILED_TRANSMIT;    
    }else{
        retStatus = HSERIAL_OK;
    }
    
    return retStatus;
}


static HSERIAL_Status_t HSERIAL_enuUARTSyncReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;
    UART_Status_t uartStatus;

    const HSERIAL_Config_t* config = &HSERIAL_Configurations[channel];
    uartStatus = UART_enuSynReceiveBuffer(config->UART_Sync_Config.HSERIAL_UartChannel, dataBuffer, size);
    if(uartStatus != UART_OK){
        retStatus = HSERIAL_FAILED_TRANSMIT;
    }else{
        retStatus = HSERIAL_OK;
    }
    return retStatus;
}

static HSERIAL_Status_t HSERIAL_enuUARTAsyncReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;
    UART_Status_t uartStatus;
    
    const HSERIAL_Config_t* config = &HSERIAL_Configurations[channel];
    
    UART_AsynBuffer_t rxBuffer;
    rxBuffer.buffer = (uint8_t*)dataBuffer;
    rxBuffer.size = size;
    rxBuffer.index = 0;
    rxBuffer.callback = config->UART_Async_Config.HSERIAL_UartRxCompleteCallback;
    uartStatus = UART_enuAsynReceiveBuffer(config->UART_Async_Config.HSERIAL_UartChannel, &rxBuffer);
    if(uartStatus != UART_OK){
        retStatus = HSERIAL_FAILED_TRANSMIT;
    }else{
        retStatus = HSERIAL_OK;
    }
    
    return retStatus;
}


static HSERIAL_Status_t HSERIAL_enuUARTDmaReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;
    const H_UART_Dma_Config_t* config = &HSERIAL_Configurations[channel].UART_Dma_Config;

    DMA_Status_t dmaStatus = DMA_enuSetMemoryAddress(HSERIAL_UART_RX_DMA_Map[config->HSERIAL_UartChannel].DMA_Controller,
                                                  HSERIAL_UART_RX_DMA_Map[config->HSERIAL_UartChannel].DMA_Stream,
                                            (uint32_t)dataBuffer);
    if(dmaStatus != DMA_OK){
        retStatus = HSERIAL_FAILED_TRANSMIT;
    }else{
        dmaStatus = DMA_enuSetNumberOfData(HSERIAL_UART_RX_DMA_Map[config->HSERIAL_UartChannel].DMA_Controller,
                                        HSERIAL_UART_RX_DMA_Map[config->HSERIAL_UartChannel].DMA_Stream,
                                   size);
        if(dmaStatus != DMA_OK){
            retStatus = HSERIAL_FAILED_TRANSMIT;
        }else{
            dmaStatus = DMA_enuStartTransfer(HSERIAL_UART_RX_DMA_Map[config->HSERIAL_UartChannel].DMA_Controller,
                                          HSERIAL_UART_RX_DMA_Map[config->HSERIAL_UartChannel].DMA_Stream);
            if(dmaStatus != DMA_OK){
                retStatus = HSERIAL_FAILED_TRANSMIT;
            }else{
                retStatus = HSERIAL_OK;
            }
        }
    }
    return retStatus;
}


static HSERIAL_Status_t HSERIAL_enuSPISyncReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;

    const H_SPI_Sync_Config_t* config = &HSERIAL_Configurations[channel].SPI_Sync_Config;
    SPI_Status_t spiStatus;
    for(uint16_t i = 0; i < size; i++){
        if(config->HSERIAL_SpiMode == HSERIAL_SPI_MASTER){
            // use master functions
            spiStatus = SPI_enuMasterSyncReceive(config->HSERIAL_SpiChannel, (uint16_t *)&dataBuffer[i]);
        }else{
            // use slave functions
            spiStatus = SPI_enuSlaveSyncReceive(config->HSERIAL_SpiChannel, (uint16_t *)&dataBuffer[i]);
        }

        if(spiStatus != SPI_OK){
            retStatus = HSERIAL_FAILED_TRANSMIT;
            break;
        }else{
            retStatus = HSERIAL_OK;
        }
    }
    return retStatus;
}

static HSERIAL_Status_t HSERIAL_enuSPIAsyncReceiveBuffer(HSERIAL_Channel_t channel, uint8_t* dataBuffer, uint16_t size){
    HSERIAL_Status_t retStatus = HSERIAL_NOT_OK;

    const H_SPI_Async_Config_t* config = &HSERIAL_Configurations[channel].SPI_Async_Config;
    
    SPI_Status_t spiStatus;

    HSerialSpiCallbacks[config->HSERIAL_SpiChannel][1] = HSERIAL_Configurations[channel].SPI_Async_Config.HSERIAL_SpiRxCompleteCallback;
    HSerialSpiBuffers[config->HSERIAL_SpiChannel].Buffer = (uint16_t*)dataBuffer;
    HSerialSpiBuffers[config->HSERIAL_SpiChannel].Size = size;
    HSerialSpiBuffers[config->HSERIAL_SpiChannel].HSERIAL_SpiMode = config->HSERIAL_SpiMode;
    HSerialSpiBuffers[config->HSERIAL_SpiChannel].index = 1;
    
    if(config->HSERIAL_SpiMode == HSERIAL_SPI_MASTER){
        // use master functions in spi.c
        spiStatus = SPI_enuMasterAsynReceive(config->HSERIAL_SpiChannel, (uint16_t *)&dataBuffer[0],LocalSpiCallbacks[config->HSERIAL_SpiChannel][1]);
    }else{
        // use slave functions in
        spiStatus = SPI_enuSlaveAsynReceive(config->HSERIAL_SpiChannel, (uint16_t *)&dataBuffer[0],LocalSpiCallbacks[config->HSERIAL_SpiChannel][1]);
    }
    if(spiStatus != SPI_OK){
        retStatus = HSERIAL_FAILED_TRANSMIT;    
    }else{
        retStatus = HSERIAL_OK;
    }
    
    return retStatus;
}

static void LocalTxCallback(SPI_Number_t spiNumber){
    if(HSerialSpiBuffers[spiNumber].index > HSerialSpiBuffers[spiNumber].Size){
        if(HSerialSpiBuffers[spiNumber].HSERIAL_SpiMode == HSERIAL_SPI_MASTER){
            // use master functions
            SPI_enuMasterAsynTransmit(spiNumber, HSerialSpiBuffers[spiNumber].Buffer[HSerialSpiBuffers[spiNumber].index++], LocalSpiCallbacks[spiNumber][0]);
        }else{
            // use slave functions
            SPI_enuSlaveAsynTransmit(spiNumber, HSerialSpiBuffers[spiNumber].Buffer[HSerialSpiBuffers[spiNumber].index++], LocalSpiCallbacks[spiNumber][0]);
        }
        HSerialSpiBuffers[spiNumber].Size--;
    }

    if(HSerialSpiBuffers[spiNumber].index > HSerialSpiBuffers[spiNumber].Size){
        
        // Call user callback if registered
        if(HSerialSpiCallbacks[spiNumber][0] != NULL){
            HSerialSpiCallbacks[spiNumber][0]();
        }        
    }
}

static void LocalRxCallback(SPI_Number_t spiNumber){
    if(HSerialSpiBuffers[spiNumber].index > HSerialSpiBuffers[spiNumber].Size){
        if(HSerialSpiBuffers[spiNumber].HSERIAL_SpiMode == HSERIAL_SPI_MASTER){
            // use master functions
            SPI_enuMasterAsynReceive(spiNumber, &HSerialSpiBuffers[spiNumber].Buffer[HSerialSpiBuffers[spiNumber].index++], LocalSpiCallbacks[spiNumber][1]);
        }else{
            // use slave functions
            SPI_enuSlaveAsynReceive(spiNumber, &HSerialSpiBuffers[spiNumber].Buffer[HSerialSpiBuffers[spiNumber].index++], LocalSpiCallbacks[spiNumber][1]);
        }
    }

    if(HSerialSpiBuffers[spiNumber].index > HSerialSpiBuffers[spiNumber].Size){
        
        // Call user callback if registered
        if(HSerialSpiCallbacks[spiNumber][1] != NULL){
            HSerialSpiCallbacks[spiNumber][1]();
        }        
    }
}

static void HSERIAL_SpiCallBackHandler_1_Tx(void){
    LocalTxCallback(SPI1);
}

static void HSERIAL_SpiCallBackHandler_2_Tx(void){
    LocalTxCallback(SPI2);
}

static void HSERIAL_SpiCallBackHandler_3_Tx(void){
    LocalTxCallback(SPI3);
}

static void HSERIAL_SpiCallBackHandler_4_Tx(void){
    LocalTxCallback(SPI4);
}

static void HSERIAL_SpiCallBackHandler_1_Rx(void){
    LocalRxCallback(SPI1);
}

static void HSERIAL_SpiCallBackHandler_2_Rx(void){
    LocalRxCallback(SPI2);
}

static void HSERIAL_SpiCallBackHandler_3_Rx(void){
    LocalRxCallback(SPI3);
}

static void HSERIAL_SpiCallBackHandler_4_Rx(void){
    LocalRxCallback(SPI4);
}
