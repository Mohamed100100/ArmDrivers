
#ifndef TEST_H_
#define TEST_H_

void LcdTest();
int gpioTest(void);
void SwitchTest();
void sevsegTest();
void nvicTest();
void testLinkerScript();
void AsynchLcdTest();
void uartTest();
void DMA_Test_Transmit(void);
void DMA_Test_Receive(void);

void test_SPI_PollingTransmitReceive(void);
void test_SPI_AsyncTransmitReceive(void);

void Test_Hserial_Sync_Uart(void);
void Test_Hserial_ASync_Uart(void);
void Test_Hserial_Dma_Uart(void);

#endif