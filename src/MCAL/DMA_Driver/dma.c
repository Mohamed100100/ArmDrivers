

#include "MCAL/DMA_Driver/dma_priv.h"
#include "MCAL/DMA_Driver/dma.h"

static void DMA_Local_Handler(DMA_Controller_t dmaController, DMA_Stream_t stream);

static DMA_Register_t* dmaRegisters[] = {
    DMA1_BASE_ADDR,
    DMA2_BASE_ADDR
};
const uint8_t flagsOffsetTable[] = {
    0,          // Stream 0,4
    6,          // Stream 1,5
    16,         // Stream 2,6
    22,         // Stream 3,7
};

const uint8_t flagsPositions[] = {
    0,          // FIFO Error
    2,          // Direct Mode Error
    3,          // Transfer Error
    4,          // Half Transfer
    5           // Transmission Complete
};


static DMA_CallBack_t dmaCallbacks[2][8][5] = { { {0} } };

DMA_Status_t DMA_enuInit(const DMA_Config_t* ConfigPtr){
    DMA_Status_t retStatus = DMA_NOT_OK;
    if(NULL == ConfigPtr){
        retStatus = DMA_NULL_PTR;
    }else{
        if(ConfigPtr->DMAx > DMA2){
            retStatus = DMA_WRONG_DMA_CONTROLLER;
        }else if((ConfigPtr->Streamx > DMA_STREAM7)){
            retStatus = DMA_WRONG_STREAM;
        }else{
            if((ConfigPtr->Channel & DMA_CHANNEL_MASK) != 0){
                retStatus = DMA_WRONG_CHANNEL;
            }else{
                if((ConfigPtr->MBurst & DMA_MBurst_MASK) != 0){
                    retStatus = DMA_WRONG_MBURST;
                }else{
                    if((ConfigPtr->PBurst & DMA_PBurst_MASK) != 0){
                        retStatus = DMA_WRONG_PBURST;
                    }else{
                        if((ConfigPtr->DoubleBuffer & DMA_DOUBLE_BUFFER_MASK) != 0){
                            retStatus = DMA_WRONG_DOUBLE_BUFFER;
                        }else{
                            if((ConfigPtr->Priority & DMA_PRIORITY_MASK) != 0){
                                retStatus = DMA_WRONG_PRIORITY;
                            }else{
                                if((ConfigPtr->MSize & DMA_MSIZE_MASK) != 0){
                                    retStatus = DMA_WRONG_MSIZE;
                                }else{  
                                    if((ConfigPtr->PSize & DMA_PSIZE_MASK) != 0){
                                        retStatus = DMA_WRONG_PSIZE;
                                    }else{
                                        if((ConfigPtr->MemoryInc & DMA_MINC_MASK) != 0){
                                            retStatus = DMA_WRONG_MINC;
                                        }else{
                                            if((ConfigPtr->PeripheralInc & DMA_PINC_MASK) != 0){
                                                retStatus = DMA_WRONG_PINC;
                                            }else{
                                                if((ConfigPtr->CircularMode & DMA_CIRCULAR_MODE_MASK) != 0){
                                                    retStatus = DMA_WRONG_CIRCULAR_MODE;
                                                }else{
                                                    if((ConfigPtr->Direction & DMA_DIRECTION_MASK) != 0){
                                                        retStatus = DMA_WRONG_DIRECTION;
                                                    }else{
                                                        if((ConfigPtr->PeripheralFlowCtrl & DMA_FLOW_CONTROL_MASK) != 0){
                                                            retStatus = DMA_WRONG_FLOW_CONTROL;
                                                        }else{
                                                            if((ConfigPtr->Mode & DMA_MODE_MASK) != 0){
                                                                retStatus = DMA_WRONG_MODE;
                                                            }else{
                                                                if((ConfigPtr->FifoThreshold & DMA_FIFO_THRESHOLD_MASK) != 0){
                                                                    retStatus = DMA_WRONG_FIFO_THRESHOLD;
                                                                }else{
                                                                    if((ConfigPtr->Interrupts & DMA_INTERRUPT_MASK) != 0){
                                                                        retStatus = DMA_WRONG_INTERRUPTS;
                                                                    }else{
                                                                        if(ConfigPtr->NumberOfData == 0){
                                                                            retStatus = DMA_WRONG_ZERO_NUMBER_OF_DATA;
                                                                        }else{
                                                                            // All parameters are valid, proceed with configuration
                                                                            DMA_StreamRegs_t* streamRegs = &dmaRegisters[ConfigPtr->DMAx]->STREAM[ConfigPtr->Streamx];

                                                                            // Disable the stream before configuration
                                                                            streamRegs->SCR &= DMA_DISABLE;

                                                                            // Configure the stream
                                                                            uint32_t scrValue = 0;
                                                                            scrValue |= ConfigPtr->Channel;
                                                                            scrValue |= ConfigPtr->MBurst;
                                                                            scrValue |= ConfigPtr->PBurst;
                                                                            scrValue |= ConfigPtr->DoubleBuffer;
                                                                            scrValue |= ConfigPtr->Priority;
                                                                            scrValue |= ConfigPtr->MSize;
                                                                            scrValue |= ConfigPtr->PSize;
                                                                            scrValue |= ConfigPtr->MemoryInc;
                                                                            scrValue |= ConfigPtr->PeripheralInc;
                                                                            scrValue |= ConfigPtr->CircularMode;
                                                                            scrValue |= ConfigPtr->Direction;
                                                                            scrValue |= ConfigPtr->PeripheralFlowCtrl;                                                                            
                                                                            scrValue |= (ConfigPtr->Interrupts & DMA_INTERRUPT_SCR_REG);
                                                                            streamRegs->SCR |= scrValue;

                                                                            streamRegs->SNDTR = ConfigPtr->NumberOfData;
                                                                            streamRegs->SPAR = ConfigPtr->PeripheralAddress;
                                                                            streamRegs->SM0AR = ConfigPtr->Memory0Address;
                                                                            if(ConfigPtr->DoubleBuffer == DMA_ENABLE_DOUBLE_BUFFER){
                                                                                streamRegs->SM1AR = ConfigPtr->Memory1Address;
                                                                            }else{
                                                                                // If double buffer is not enabled, SM1AR is not used
                                                                            }
                                                                        
                                                                            streamRegs->SFCR |= ConfigPtr->Mode;
                                                                            streamRegs->SFCR |= ConfigPtr->FifoThreshold;
                                                                            streamRegs->SFCR |= (ConfigPtr->Interrupts & DMA_INTERRUPT_SFCR_REG);
                                                                        
                                                                            retStatus = DMA_OK;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return retStatus;
}


DMA_Status_t DMA_enuStartTransfer(DMA_Controller_t DMAx, DMA_Stream_t Streamx){
    DMA_Status_t retStatus = DMA_NOT_OK;
    if(DMAx > DMA2){
        retStatus = DMA_WRONG_DMA_CONTROLLER;
    }else if((Streamx > DMA_STREAM7)){
        retStatus = DMA_WRONG_STREAM;
    }else{
        DMA_StreamRegs_t* streamRegs = &dmaRegisters[DMAx]->STREAM[Streamx];
        // Enable the stream to start the transfer
        streamRegs->SCR |= DMA_ENABLE;
        retStatus = DMA_OK;
    }
    return retStatus;
}

DMA_Status_t DMA_enuStopTransfer(DMA_Controller_t DMAx, DMA_Stream_t Streamx){
    DMA_Status_t retStatus = DMA_NOT_OK;
    if(DMAx > DMA2){
        retStatus = DMA_WRONG_DMA_CONTROLLER;
    }else if((Streamx > DMA_STREAM7)){
        retStatus = DMA_WRONG_STREAM;
    }else{
        DMA_StreamRegs_t* streamRegs = &dmaRegisters[DMAx]->STREAM[Streamx];
        // Disable the stream to stop the transfer
        streamRegs->SCR &= DMA_DISABLE;
        retStatus = DMA_OK;
    }
    return retStatus;
}

DMA_Status_t DMA_enuRegisterCallback(DMA_Controller_t DMAx, DMA_Stream_t Streamx, DMA_Interrupts_t Interrupt, DMA_CallBack_t callback){
    DMA_Status_t retStatus = DMA_NOT_OK;
    if(DMAx > DMA2){
        retStatus = DMA_WRONG_DMA_CONTROLLER;
    }else if((Streamx > DMA_STREAM7)){
        retStatus = DMA_WRONG_STREAM;
    }else{
        if(Interrupt > DMA_INTERRUPT_TRANSMISSION_COMPLETE){
            retStatus = DMA_WRONG_INTERRUPTS;
        }else{
            // Register the callback for the specified interrupt
            dmaCallbacks[DMAx][Streamx][Interrupt] = callback;
            retStatus = DMA_OK;
        }
    }
    return retStatus;
}


uint8_t DMA_u8ReadFlag(DMA_Controller_t DMAx, DMA_Stream_t Streamx, DMA_Interrupts_t Interrupt){
    uint8_t flagStatus = 0;
    if(DMAx > DMA2){
        // Invalid DMA controller
        flagStatus = 0xFF; // Indicate error
    }else if((Streamx > DMA_STREAM7)){
        // Invalid stream
        flagStatus = 0xFF; // Indicate error
    }else{
        if(Interrupt > DMA_INTERRUPT_TRANSMISSION_COMPLETE){
            // Invalid interrupt
            flagStatus = 0xFF; // Indicate error
        }else{
            DMA_Register_t* dmaReg = dmaRegisters[DMAx];
            uint8_t flagindex;
            flagindex = flagsPositions[Interrupt] + (flagsOffsetTable[Streamx % 4]);
            if(Streamx < DMA_STREAM4){
                // Low interrupt status register
                flagStatus = (dmaReg->LISR >> flagindex) & 0x1;
            }else{
                // High interrupt status register
                flagStatus = (dmaReg->HISR >> flagindex) & 0x1;
            }
        }
    }
    return flagStatus;
}

DMA_Status_t DMA_enuClearFlag(DMA_Controller_t DMAx, DMA_Stream_t Streamx, DMA_Interrupts_t Interrupt){
    DMA_Status_t retStatus = DMA_NOT_OK;
    if(DMAx > DMA2){
        retStatus = DMA_WRONG_DMA_CONTROLLER;
    }else if((Streamx > DMA_STREAM7)){
        retStatus = DMA_WRONG_STREAM;
    }else{
        if(Interrupt > DMA_INTERRUPT_TRANSMISSION_COMPLETE){
            retStatus = DMA_WRONG_INTERRUPTS;
        }else{
            DMA_Register_t* dmaReg = dmaRegisters[DMAx];
            uint8_t flagindex;
            flagindex = flagsPositions[Interrupt] + (flagsOffsetTable[Streamx % 4]);
            if(Streamx < DMA_STREAM4){
                // Clear flag in Low interrupt flag clear register
                dmaReg->LIFCR |= (1 << flagindex);
            }else{
                // Clear flag in High interrupt flag clear register
                dmaReg->HIFCR |= (1 << flagindex);
            }
            retStatus = DMA_OK;
        }
    }
    return retStatus;
}

static void DMA_Local_Handler(DMA_Controller_t dmaController, DMA_Stream_t stream) {
    // Check which interrupt occurred and call the corresponding callback
    if(DMA_u8ReadFlag(dmaController, stream, DMA_INTERRUPT_TRANSMISSION_COMPLETE) == 1){
        // Clear the transmission complete flag
        DMA_enuClearFlag(dmaController, stream, DMA_INTERRUPT_TRANSMISSION_COMPLETE);

        // Call the registered callback function
        if(dmaCallbacks[dmaController][stream][DMA_INTERRUPT_TRANSMISSION_COMPLETE] != 0){
            dmaCallbacks[dmaController][stream][DMA_INTERRUPT_TRANSMISSION_COMPLETE]();
        }
    }

    if(DMA_u8ReadFlag(dmaController, stream, DMA_INTERRUPT_HALF_TRANSFER) == 1){
        // Clear the half transfer flag
        DMA_enuClearFlag(dmaController, stream, DMA_INTERRUPT_HALF_TRANSFER);

        // Call the registered callback function
        if(dmaCallbacks[dmaController][stream][DMA_INTERRUPT_HALF_TRANSFER] != 0){
            dmaCallbacks[dmaController][stream][DMA_INTERRUPT_HALF_TRANSFER]();
        }
    }

    if(DMA_u8ReadFlag(dmaController, stream, DMA_INTERRUPT_TRANSFER_ERROR) == 1){
        // Clear the transfer error flag
        DMA_enuClearFlag(dmaController, stream, DMA_INTERRUPT_TRANSFER_ERROR);

        // Call the registered callback function
        if(dmaCallbacks[dmaController][stream][DMA_INTERRUPT_TRANSFER_ERROR] != 0){
            dmaCallbacks[dmaController][stream][DMA_INTERRUPT_TRANSFER_ERROR]();
        }
    }

    if(DMA_u8ReadFlag(dmaController, stream, DMA_INTERRUPT_DIRECT_MODE_ERROR) == 1){
        // Clear the direct mode error flag
        DMA_enuClearFlag(dmaController, stream, DMA_INTERRUPT_DIRECT_MODE_ERROR);

        // Call the registered callback function
        if(dmaCallbacks[dmaController][stream][DMA_INTERRUPT_DIRECT_MODE_ERROR] != 0){
            dmaCallbacks[dmaController][stream][DMA_INTERRUPT_DIRECT_MODE_ERROR]();
        }
    }

    if(DMA_u8ReadFlag(dmaController, stream, DMA_INTERRUPT_FIFO_ERROR) == 1){
        // Clear the FIFO error flag
        DMA_enuClearFlag(dmaController, stream, DMA_INTERRUPT_FIFO_ERROR);

        // Call the registered callback function
        if(dmaCallbacks[dmaController][stream][DMA_INTERRUPT_FIFO_ERROR] != 0){
            dmaCallbacks[dmaController][stream][DMA_INTERRUPT_FIFO_ERROR]();
        }
    }
}

void DMA1_Stream0_IRQHandler(void) {
    DMA_Local_Handler(DMA1, DMA_STREAM0);
}


void DMA2_Stream0_IRQHandler(void) {
    DMA_Local_Handler(DMA2, DMA_STREAM0);
}

void DMA1_Stream1_IRQHandler(void) {
    DMA_Local_Handler(DMA1, DMA_STREAM1);
}

void DMA2_Stream1_IRQHandler(void) {
    DMA_Local_Handler(DMA2, DMA_STREAM1);
}

void DMA1_Stream2_IRQHandler(void) {
    DMA_Local_Handler(DMA1, DMA_STREAM2);
}


void DMA2_Stream2_IRQHandler(void) {
    DMA_Local_Handler(DMA2, DMA_STREAM2);
}


void DMA1_Stream3_IRQHandler(void) {
    DMA_Local_Handler(DMA1, DMA_STREAM3);
}


void DMA2_Stream3_IRQHandler(void) {
    DMA_Local_Handler(DMA2, DMA_STREAM3);
}


void DMA1_Stream4_IRQHandler(void) {
    DMA_Local_Handler(DMA1, DMA_STREAM4);
}


void DMA2_Stream4_IRQHandler(void) {
    DMA_Local_Handler(DMA2, DMA_STREAM4);
}


void DMA1_Stream5_IRQHandler(void) {
    DMA_Local_Handler(DMA1, DMA_STREAM5);
}


void DMA2_Stream5_IRQHandler(void) {
    DMA_Local_Handler(DMA2, DMA_STREAM5);
}


void DMA1_Stream6_IRQHandler(void) {
    DMA_Local_Handler(DMA1, DMA_STREAM6);
}

void DMA2_Stream6_IRQHandler(void) {
    DMA_Local_Handler(DMA2, DMA_STREAM6);
}

void DMA1_Stream7_IRQHandler(void) {
    DMA_Local_Handler(DMA1, DMA_STREAM7);
}

void DMA2_Stream7_IRQHandler(void) {
    DMA_Local_Handler(DMA2, DMA_STREAM7);
}


