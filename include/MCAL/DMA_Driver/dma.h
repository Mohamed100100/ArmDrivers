
#ifndef DMA_H
#define DMA_H

#include "LIB/stdtypes.h"

//                                                0b10987654321098765432109876543210
#define DMA_INTERRUPT_TRANSFER_COMPLETE_ENABLE    0b00000000000000000000000000010000
#define DMA_INTERRUPT_HALF_TRANSFER_ENABLE        0b00000000000000000000000000001000
#define DMA_INTERRUPT_TRANSFER_ERROR_ENABLE       0b00000000000000000000000000000100
#define DMA_INTERRUPT_DIRECT_MODE_ERROR_ENABLE    0b00000000000000000000000000000010
#define DMA_INTERRUPT_FIFO_ERROR_ENABLE           0b00000000000000000000000010000000

typedef void (*DMA_CallBack_t)(void);

typedef enum {
    DMA_INTERRUPT_FIFO_ERROR            ,
    DMA_INTERRUPT_DIRECT_MODE_ERROR     ,
    DMA_INTERRUPT_TRANSFER_ERROR        ,
    DMA_INTERRUPT_HALF_TRANSFER         ,
    DMA_INTERRUPT_TRANSMISSION_COMPLETE ,
}DMA_Interrupts_t;



typedef enum {
    DMA1 = 0,
    DMA2
}DMA_Controller_t;


typedef enum {
    DMA_STREAM0 = 0,
    DMA_STREAM1,
    DMA_STREAM2,
    DMA_STREAM3,
    DMA_STREAM4,
    DMA_STREAM5,
    DMA_STREAM6,
    DMA_STREAM7
}DMA_Stream_t;

typedef enum {
//                       ***       
//                 0b10987654321098765432109876543210
    DMA_CHANNEL0 = 0b00000000000000000000000000000000,
    DMA_CHANNEL1 = 0b00000010000000000000000000000000,
    DMA_CHANNEL2 = 0b00000100000000000000000000000000,
    DMA_CHANNEL3 = 0b00000110000000000000000000000000,
    DMA_CHANNEL4 = 0b00001000000000000000000000000000,
    DMA_CHANNEL5 = 0b00001010000000000000000000000000,
    DMA_CHANNEL6 = 0b00001100000000000000000000000000,
    DMA_CHANNEL7 = 0b00001110000000000000000000000000
}DMA_Channel_t;


typedef enum {
//                              **
//                      0b10987654321098765432109876543210
    DMA_MBurst_SINGLE = 0b00000000000000000000000000000000,
    DMA_MBurst_INCR4  = 0b00000001000000000000000000000000,
    DMA_MBurst_INCR8  = 0b00000010000000000000000000000000,
    DMA_MBurst_INCR16 = 0b00000011000000000000000000000000
}DMA_MBurst_t;

typedef enum {
//                                 **
//                      0b10987654321098765432109876543210
    DMA_PBurst_SINGLE = 0b00000000000000000000000000000000,
    DMA_PBurst_INCR4  = 0b00000000001000000000000000000000,
    DMA_PBurst_INCR8  = 0b00000000010000000000000000000000,
    DMA_PBurst_INCR16 = 0b00000000011000000000000000000000
}DMA_PBurst_t;


typedef enum {
//                                             *
//                              0b10987654321098765432109876543210
    DMA_DISABLE_DOUBLE_BUFFER = 0b00000000000000000000000000000000,
    DMA_ENABLE_DOUBLE_BUFFER  = 0b00000000000001000000000000000100,
}DMA_DoubleBuffer_t;


typedef enum {
//                                           **
//                           0b10987654321098765432109876543210
    DMA_PRIORITY_LOW       = 0b00000000000000000000000000000000,
    DMA_PRIORITY_MEDIUM    = 0b00000000000000010000000000000000,
    DMA_PRIORITY_HIGH      = 0b00000000000000100000000000000000,
    DMA_PRIORITY_VERY_HIGH = 0b00000000000000110000000000000000
}DMA_Priority_t;


typedef enum {
//                                           **
//                        0b10987654321098765432109876543210
    DMA_MSIZE_BYTE      = 0b00000000000000000000000000000000,
    DMA_MSIZE_HALFWORD  = 0b00000000000000000010000000000000,
    DMA_MSIZE_WORD      = 0b00000000000000000100000000000000
}DMA_MSize_t;


typedef enum {
//                                             **
//                        0b10987654321098765432109876543210
    DMA_PSIZE_BYTE      = 0b00000000000000000000000000000000,
    DMA_PSIZE_HALFWORD  = 0b00000000000000000000100000000000,
    DMA_PSIZE_WORD      = 0b00000000000000000001000000000000
}DMA_PSize_t;

typedef enum {
//                                                    *
//                             0b10987654321098765432109876543210
    DMA_MINC_FIXED           = 0b00000000000000000000000000000000,
    DMA_MINC_AUTO_INCREMENT  = 0b00000000000000000000010000000000,
}DMA_MemoryInc_t;

typedef enum {
//                                                     *
//                             0b10987654321098765432109876543210
    DMA_PINC_FIXED           = 0b00000000000000000000000000000000,
    DMA_PINC_AUTO_INCREMENT  = 0b00000000000000000000001000000000,
}DMA_PerihperalInc_t;


typedef enum {
//                                                       *
//                              0b10987654321098765432109876543210
    DMA_CIRCULAR_MODE_DISABLE = 0b00000000000000000000000000000000,
    DMA_CIRCULAR_MODE_ENABLE  = 0b00000000000000000000000100000000,
}DMA_CircularMode_t;

typedef enum {
//                                                **
//                      0b10987654321098765432109876543210
    DMA_DIRECTION_P2M = 0b00000000000000000000000000000000,
    DMA_DIRECTION_M2P = 0b00000000000000000000000001000000,
    DMA_DIRECTION_M2M = 0b00000000000000000000000010000000,
}DMA_Direction_t;


typedef enum {
//                                                                  *
//                                      0b10987654321098765432109876543210
    DMA_FLOW_CONTROL_USING_DMA        = 0b00000000000000000000000000000000,
    DMA_FLOW_CONTROL_USING_PERIPHERAL = 0b00000000000000000000000000100000,
}DMA_PeripheralFlowCtrl_t;


typedef enum {
// 
//                    0b10987654321098765432109876543210
    DMA_MODE_DIRECT = 0b00000000000000000000000000000000,
    DMA_MODE_FIFO   = 0b00000000000000000000000000000100,
}DMA_Mode_t;


typedef enum {
//                                                                     **
//                                     0b10987654321098765432109876543210  
    DMA_FIFO_THRESHOLD_QUARTER       = 0b00000000000000000000000000000000,
    DMA_FIFO_THRESHOLD_HALF          = 0b00000000000000000000000000000001,
    DMA_FIFO_THRESHOLD_THREEQUARTERS = 0b00000000000000000000000000000010,
    DMA_FIFO_THRESHOLD_FULL          = 0b00000000000000000000000000000011,
}DMA_FifoThreshold_t;




typedef struct {
    DMA_Controller_t          DMAx;
    DMA_Stream_t              Streamx;
    DMA_Channel_t             Channel;
    DMA_Direction_t           Direction;
    DMA_PeripheralFlowCtrl_t  PeripheralFlowCtrl;
    DMA_Mode_t                Mode;
    DMA_Priority_t            Priority;
    DMA_MSize_t               MSize;
    DMA_PSize_t               PSize;
    DMA_MemoryInc_t           MemoryInc;
    DMA_PerihperalInc_t       PeripheralInc;
    DMA_CircularMode_t        CircularMode;
    DMA_MBurst_t              MBurst;
    DMA_PBurst_t              PBurst;
    DMA_DoubleBuffer_t        DoubleBuffer;
    DMA_FifoThreshold_t       FifoThreshold;
    uint32_t                  PeripheralAddress;
    uint32_t                  Memory0Address; 
    uint32_t                  Memory1Address; // Used in Double Buffer mode
    uint32_t                  Interrupts;     // Bitwise OR of interrupt enables
    uint16_t                  NumberOfData;
}DMA_Config_t;



typedef enum {
    DMA_NOT_OK,
    DMA_OK,
    DMA_NULL_PTR,
    DMA_WRONG_DMA_CONTROLLER,
    DMA_WRONG_STREAM,
    DMA_WRONG_CHANNEL,
    DMA_WRONG_MBURST,
    DMA_WRONG_PBURST,
    DMA_WRONG_DOUBLE_BUFFER,
    DMA_WRONG_PRIORITY,
    DMA_WRONG_MSIZE,
    DMA_WRONG_PSIZE,
    DMA_WRONG_MINC,
    DMA_WRONG_PINC,
    DMA_WRONG_CIRCULAR_MODE,
    DMA_WRONG_DIRECTION,
    DMA_WRONG_FLOW_CONTROL,
    DMA_WRONG_MODE,
    DMA_WRONG_FIFO_THRESHOLD,
    DMA_WRONG_INTERRUPTS,
    DMA_WRONG_ZERO_NUMBER_OF_DATA
}DMA_Status_t;


DMA_Status_t DMA_enuInit(const DMA_Config_t* ConfigPtr);
DMA_Status_t DMA_enuStartTransfer(DMA_Controller_t DMAx, DMA_Stream_t Streamx);
DMA_Status_t DMA_enuStopTransfer(DMA_Controller_t DMAx, DMA_Stream_t Streamx);
DMA_Status_t DMA_enuRegisterCallback(DMA_Controller_t DMAx, DMA_Stream_t Streamx,DMA_Interrupts_t Interrupt, DMA_CallBack_t callback);
uint8_t DMA_u8ReadFlag(DMA_Controller_t DMAx, DMA_Stream_t Streamx, DMA_Interrupts_t Interrupt);
DMA_Status_t DMA_enuClearFlag(DMA_Controller_t DMAx, DMA_Stream_t Streamx, DMA_Interrupts_t Interrupt);






#endif // DMA_H