
#include "test.h"




int main(){

    // test_SPI_PollingTransmitReceive();
    // Test_Hserial_Sync_Uart();
    // Test_Hserial_Dma_Uart();
    // Test_Hserial_ASync_Uart();

    test_SPI_AsyncTransmitReceive();
    while(1){

    }

    return 0;

    
}