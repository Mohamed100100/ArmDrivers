#include <string.h>
#include "./HAL/LCD_Driver/lcd_queue.h"

static LCD_DataBuffer_t queue[QUEUE_SIZE];
static uint8_t front;
static uint8_t rear;
static uint8_t count;

void Queue_Init(void)
{
    front = 0;
    rear = 0;
    count = 0;
}

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
    
    /* Copy data to queue */
    queue[rear].row = data->row;
    queue[rear].col = data->col;
    memcpy(queue[rear].buff, data->buff, MAX_DATA_LEN);
    
    /* Update rear index (circular) */
    rear = (rear + 1) % QUEUE_SIZE;
    count++;
    
    return QUEUE_OK;
}

LCD_DataBuffer_t* Queue_Top(void)
{
    /* Check if queue is empty */
    if (count == 0)
    {
        return NULL;
    }
    
    return &queue[front];
}

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

bool Queue_IsEmpty(void)
{
    return (count == 0);
}

bool Queue_IsFull(void)
{
    return (count >= QUEUE_SIZE);
}