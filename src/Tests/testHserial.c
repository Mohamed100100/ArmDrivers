

#include "LIB/stdtypes.h"
#include "HAL/MCU_Driver/mcu.h"
#include "HAL/HSERIAL_Driver/hserial.h"

#include "test.h"
void TxCallback(void);
void RxCallback(void);

void Test_Hserial_Sync_Uart(void){
    
    HSERIAL_Status_t HserialStatus = HSERIAL_NOT_OK;
    
    MCU_Status_t mcuStatus = MCU_enuInit(&MCU_Configs);
    HserialStatus = HSERIAL_enuInit();

    const uint8_t testData[] = "Hello, HSERIAL Sync UART!";
    const uint16_t dataSize = sizeof(testData) - 1; // Exclude null terminator

    uint8_t receiveBuffer[30] = {0};

    HserialStatus = HSERIAL_enuReceiveBuffer(HSERIAL_CHANNEL_1, receiveBuffer, 30);
    HserialStatus = HSERIAL_enuTransmitBuffer(HSERIAL_CHANNEL_1, receiveBuffer, 30);



    while (1);
    
}






void TxCallback(void){
    // Reception complete callback
    int a = 0;
}


HSERIAL_Status_t HserialStatus = HSERIAL_NOT_OK;
uint8_t receiveBuffer[30] = {0};
void Test_Hserial_Dma_Uart(void){
    
    MCU_Status_t mcuStatus = MCU_enuInit(&MCU_Configs);
    HserialStatus = HSERIAL_enuInit();

    const uint8_t testData[] = "Hello, HSERIAL ASync UART!";
    const uint16_t dataSize = sizeof(testData) - 1; // Exclude null terminator
    HserialStatus = HSERIAL_enuReceiveBuffer(HSERIAL_CHANNEL_1, receiveBuffer, 30);


    while (1);

}

void RxCallback(void){
    // Transmission complete callback
    int a = 0;
    HserialStatus = HSERIAL_enuTransmitBuffer(HSERIAL_CHANNEL_1, receiveBuffer, 30);

}

void Test_Hserial_ASync_Uart(void){
    
    MCU_Status_t mcuStatus = MCU_enuInit(&MCU_Configs);
    
    HserialStatus = HSERIAL_enuInit();

    HserialStatus = HSERIAL_enuReceiveBuffer(HSERIAL_CHANNEL_1, receiveBuffer, 30);


    while (1);
    
}


