#ifndef LCD_QUEUE_H
#define LCD_QUEUE_H

#include "LIB/stdtypes.h"
#include <stdbool.h>

#define MAX_DATA_LEN    32
#define QUEUE_SIZE      10

typedef struct{
    uint8_t  row;
    uint8_t  col;
    uint8_t  buff[MAX_DATA_LEN];
}LCD_DataBuffer_t;

typedef enum{
    QUEUE_OK,
    QUEUE_FULL,
    QUEUE_EMPTY,
    QUEUE_NULL_PTR
}Queue_Status_t;

/**
 * @brief Initialize the queue
 */
void Queue_Init(void);

/**
 * @brief Push data to the queue
 * @param data Pointer to LCD_DataBuffer_t to copy into queue
 * @return Queue_Status_t 
 */
Queue_Status_t Queue_Push(const LCD_DataBuffer_t *data);

/**
 * @brief Get pointer to front element without removing it
 * @return Pointer to front LCD_DataBuffer_t, NULL if empty
 */
LCD_DataBuffer_t* Queue_Top(void);

/**
 * @brief Remove front element from queue
 */
void Queue_Pop(void);

/**
 * @brief Check if queue is empty
 * @return true if empty, false otherwise
 */
bool Queue_IsEmpty(void);

/**
 * @brief Check if queue is full
 * @return true if full, false otherwise
 */
bool Queue_IsFull(void);

#endif /* LCD_QUEUE_H */