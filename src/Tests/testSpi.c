


#include  "LIB/stdtypes.h"
#include "MCAL/SPI_Driver/spi.h"
#include "HAL/MCU_Driver/mcu.h"
#include "MCAL/NVIC_Driver/nvic_stm32f401cc.h"

#include "test.h"
void Tx_Callback(void);

void test_SPI_PollingTransmitReceive(void){
    SPI_Status_t status;

    SPI_Config_t spiConfig;
    spiConfig.spiNumber = SPI1;
    spiConfig.communicationMode = SPI_FULL_DUPLEX;
    spiConfig.mode = SPI_MASTER;
    spiConfig.crcState = SPI_CRC_DISABLED;
    spiConfig.dataLength = SPI_16_BIT_DATA;
    spiConfig.dataOrder = SPI_MSB_FIRST;
    spiConfig.baudRate = SPI_BAUDRATE_DIV8;
    spiConfig.polarityPhase = SPI_ONE_IDLE_FIRST_EDGE;
    spiConfig.frameFormat = SPI_MOTOROLA;
    spiConfig.dmaState = SPI_DISABLE_DMA;
    spiConfig.nssManagement = SPI_NSS_MASTER_SW;
    // spiConfig.crcPolynomial = 7;
    spiConfig.slavesConfig.numberOfSlaves = 0;
    // spiConfig.slavesConfig.slaves[0].port = GPIO_PORT_A;
    // spiConfig.slavesConfig.slaves[0].pin  = GPIO_PIN_4;


    MCU_Status_t mcuStatus = MCU_enuInit(&MCU_Configs);
    status = SPI_enuInit(&spiConfig);


    uint16_t txData = 0x5A;
    uint16_t rxData = 0x00;

    status = SPI_enuMasterSyncTransmitReceive(SPI1, txData, &rxData);
    
}

uint16_t rxSpiData = 0x00;

void test_SPI_AsyncTransmitReceive(void){
    SPI_Status_t status;

    NVIC_BP_Status_t nvicStatus = NVIC_BP_EnableIRQ(NVIC_SPI1_IRQ);
    nvicStatus = NVIC_BP_SetPriority(NVIC_SPI1_IRQ,NVIC_PRIORITY_2);

    SPI_Config_t spiConfig;
    spiConfig.spiNumber         = SPI1;
    spiConfig.communicationMode = SPI_FULL_DUPLEX;
    spiConfig.mode              = SPI_MASTER;
    spiConfig.crcState          = SPI_CRC_DISABLED;
    spiConfig.dataLength        = SPI_16_BIT_DATA;
    spiConfig.dataOrder         = SPI_MSB_FIRST;
    spiConfig.baudRate          = SPI_BAUDRATE_DIV8;
    spiConfig.polarityPhase     = SPI_ONE_IDLE_FIRST_EDGE;
    spiConfig.frameFormat       = SPI_MOTOROLA;
    spiConfig.dmaState          = SPI_DISABLE_DMA;
    spiConfig.nssManagement     = SPI_NSS_MASTER_SW;
    // spiConfig.crcPolynomial = 7;
    spiConfig.slavesConfig.numberOfSlaves = 0;
    // spiConfig.slavesConfig.slaves[0].port = GPIO_PORT_A;
    // spiConfig.slavesConfig.slaves[0].pin  = GPIO_PIN_4;

    MCU_Status_t mcuStatus = MCU_enuInit(&MCU_Configs);
    status = SPI_enuInit(&spiConfig);

    uint16_t txData = 0xA5;
    // status = SPI_enuMasterAsynTransmit(SPI1, txData, Tx_Callback);
    status = SPI_enuMasterAsynReceive(SPI1, &rxSpiData, Tx_Callback);
}

void Tx_Callback(void){
    // Transmission complete callback
    int a =0;
}