/******************************************************************************
 * @file    lcd_queue.c
 * @author  Eng.Gemy
 * @brief   LCD Async Queue Implementation (FIFO circular buffer)
 *          Backs the asynchronous LCD driver (lcd.c). Queues LCD_DataBuffer_t
 *          requests (string/coord) for state-machine consumption by
 *          scheduler runnable lcdRunnableCBF every 5ms.
 * @date    2024
 * @version 1.0
 * @note    Size QUEUE_SIZE defined in lcd_queue.h. NOT ISR-safe for concurrent
 *          push from ISR + main without critical section; scheduler only
 *          context is safe.
 ******************************************************************************/

#include <string.h>
#include "./HAL/LCD_Driver/lcd_queue.h"

/** @brief Internal circular buffer storage */
static LCD_DataBuffer_t queue[QUEUE_SIZE];
/** @brief Index of front (oldest) element */
static uint8_t front;
/** @brief Index of rear (next write) position */
static uint8_t rear;
/** @brief Current occupancy count (0..QUEUE_SIZE) */
static uint8_t count;

/******************************************************************************
 * @brief Initialize queue to empty
 * @details Sets front=rear=count=0. Call once before any Push/Pop (e.g., in
 *          LCD_enuAsynInit).
 * @param None
 * @return None
 ******************************************************************************/
void Queue_Init(void)
{
    front = 0;
    rear = 0;
    count = 0;
}

/******************************************************************************
 * @brief Push a data buffer into the queue (copy)
 * @details Validates NULL, checks full (count>=QUEUE_SIZE), memcpy's
 *          row/col/buff[MAX_DATA_LEN], advances rear circularly.
 * @param[in] data Pointer to LCD_DataBuffer_t to enqueue (row/col + buff)
 * @return Queue_Status_t QUEUE_OK on success,
 *         QUEUE_NULL_PTR if data==NULL,
 *         QUEUE_FULL if no free slot.
 * @note  Caller retains ownership of *data; content is copied.
 ******************************************************************************/
Queue_Status_t Queue_Push(const LCD_DataBuffer_t *data)
{
    /* Check for NULL pointer */
    if (data == NULL)
    {
        return QUEUE_NULL_PTR;
    }
    
    /* Check if queue is full */
    if (count >= QUEUE_SIZE)
    {
        return QUEUE_FULL;
    }
    
    /* Copy data to queue (row/col + payload) */
    queue[rear].row = data->row;
    queue[rear].col = data->col;
    memcpy(queue[rear].buff, data->buff, MAX_DATA_LEN);
    
    /* Update rear index (circular wrap) */
    rear = (rear + 1) % QUEUE_SIZE;
    count++;
    
    return QUEUE_OK;
}

/******************************************************************************
 * @brief Peek front element without removal
 * @details Returns pointer into internal static queue[front] if not empty.
 * @param None
 * @return LCD_DataBuffer_t* Pointer to front entry, or NULL if queue empty
 * @note  Do not free returned pointer; it aliases internal storage.
 ******************************************************************************/
LCD_DataBuffer_t* Queue_Top(void)
{
    /* Check if queue is empty */
    if (count == 0)
    {
        return NULL;
    }
    
    return &queue[front];
}

/******************************************************************************
 * @brief Pop front element (discard)
 * @details Advances front circularly and decrements count if not empty.
 * @param None
 * @return None
 ******************************************************************************/
void Queue_Pop(void)
{
    /* Check if queue is empty */
    if (count == 0)
    {
        return;
    }
    
    /* Update front index (circular) */
    front = (front + 1) % QUEUE_SIZE;
    count--;
}

/******************************************************************************
 * @brief Check if queue is empty
 * @param None
 * @return bool TRUE (1) if empty, FALSE (0) otherwise
 ******************************************************************************/
bool Queue_IsEmpty(void)
{
    return (count == 0);
}

/******************************************************************************
 * @brief Check if queue is full
 * @param None
 * @return bool TRUE if count>=QUEUE_SIZE (no push without pop), else FALSE
 ******************************************************************************/
bool Queue_IsFull(void)
{
    return (count >= QUEUE_SIZE);
}
