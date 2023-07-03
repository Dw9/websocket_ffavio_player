//
// Created by 韦少 on 2023/6/21.
//

#ifndef WEBSCOKET_FFAVIO_PLAYER_RINGBUFFER_H
#define WEBSCOKET_FFAVIO_PLAYER_RINGBUFFER_H
#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

/* Header includes -----------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>




//#define  ring_buffer_malloc(size)      (size *)malloc(sizeof(size))
//#define  ring_buffer_free(block)	   free(block)

/* Type definitions ----------------------------------------------------------*/
typedef  struct
{
    uint8_t *buffer;
    uint32_t size;
    uint32_t in;
    uint32_t out;
}ringbuffer;


/* Variable declarations -----------------------------------------------------*/
/* Variable definitions ------------------------------------------------------*/
/* Function declarations -----------------------------------------------------*/
ringbuffer *ringbuffer_malloc(uint32_t size);
void ringBuffer_free(ringbuffer *fifo);



uint32_t ringbuffer_in(ringbuffer *fifo, const void *in, uint32_t len);
uint32_t ringbuffer_out(ringbuffer *fifo, void *out, uint32_t len);




/**
* @brief  Removes the entire FIFO contents.
* @param  [in] fifo: The fifo to be emptied.
* @return None.
**/


static inline uint32_t ringbuffer_inlocate(ringbuffer *fifo)
{
    return fifo->in;
}


static inline uint32_t ringbuffer_outlocate(ringbuffer *fifo)
{
    return fifo->out;
}

static inline void  ringbuffer_reset(ringbuffer *fifo)
{
    fifo->in = fifo->out = 0;
}


static inline void  ringbuffer_reset_in(ringbuffer *fifo)
{
    fifo->in = 0;
}

/**
* @brief  Returns the size of the FIFO in bytes.
* @param  [in] fifo: The fifo to be used.
* @return The size of the FIFO.
*/

static inline uint32_t ringbuffer_size(ringbuffer *fifo)
{
    return (fifo->size);
}

/**
* @brief  Returns the number of used bytes in the FIFO.
* @param  [in] fifo: The fifo to be used.
* @return The number of used bytes.
*/

static inline uint32_t ringbuffer_len(ringbuffer *fifo)
{
    return (fifo->in - fifo->out);
}

/**
* @brief  Returns the number of bytes available in the FIFO.
* @param  [in] fifo: The fifo to be used.
* @return The number of bytes available.
*/

static inline uint32_t ringbuffer_avail(ringbuffer *fifo)
{
    return (ringbuffer_size(fifo) - ringbuffer_len(fifo));
}

/**
* @brief  Is the FIFO empty?
* @param  [in] fifo: The fifo to be used.
* @retval true:      Yes.
* @retval false:     No.
*/

static inline bool ringbuffer_empty(ringbuffer *fifo)
{
    return (ringbuffer_len(fifo) == 0);
}

/**
* @brief  Is the FIFO full?
* @param  [in] fifo: The fifo to be used.
* @retval true:      Yes.
* @retval false:     No.
*/
static inline bool ringbuffer_full(ringbuffer *fifo)
{
    return (ringbuffer_avail(fifo) == 0);
}


#endif /* __RINGBUFFER_H */

#endif //WEBSCOKET_FFAVIO_PLAYER_RINGBUFFER_H
