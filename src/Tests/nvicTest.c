
#include "LIB/stdtypes.h"
#include "MCAL/NVIC_Driver/nvic_stm32f401cc.h"


#include "test.h"

void nvicTest(){

    NVIC_BP_EnableIRQ(NVIC_EXTI0_IRQ);
    NVIC_BP_SetPriority(NVIC_EXTI0_IRQ,NVIC_PRIORITY_12);
    NVIC_BP_Priority_t priority;
    NVIC_BP_GetPriority(NVIC_EXTI0_IRQ,&priority);
    NVIC_BP_SetPendingIRQ(NVIC_SPI1_IRQ);
    NVIC_BP_Pending_t pending;
    NVIC_BP_GetPendingIRQ(NVIC_SPI1_IRQ,&pending);
}