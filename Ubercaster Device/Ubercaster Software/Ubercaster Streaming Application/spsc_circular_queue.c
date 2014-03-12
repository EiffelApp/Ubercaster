/*
 * spsc_circular_queue.c - lock-free implementation of Single Producer Single
 *                         Consumer Circular Queue
 *
 * Author: Artem Bagautdinov, <artem.bagautdinov@gmail.com>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spsc_circular_queue.h"

/* Element of a lock-free Single Producer Single Consumer (SPSC) circular queue */
typedef struct _SpscCircularQueueElement {
    bool contains_data;
    uint8_t *data;
    struct _SpscCircularQueueElement *next;
} SpscCircularQueueElement;

struct _SpscCircularQueue {
    int elements_num;
    size_t element_size;
    /* the memory is allocated in two big chunks; one is for array of
       SpscCircularQueueElements, another is for data; this approach makes memory
       allocation less sgmented, easier to understand and very easy to release
       all allocated memory */
    SpscCircularQueueElement *elements;     /* SpscCircularQueueElements chunk */
    uint8_t *data;                          /* data chunk */
    /* pointer on the current 'head' element of the queue; is used to put data
       into the queue */
    SpscCircularQueueElement *head;
    /* pointer on the current 'tail' element of the queue; is used to get data
       from the queue */
    SpscCircularQueueElement *tail;
};

bool spsc_circular_queue_push(SpscCircularQueue *queue, const uint8_t *data)
{
    assert(queue != NULL);
    assert(queue->head != NULL);
    assert(data != NULL);

    if (queue->head->contains_data)
        return false;

    memcpy(queue->head->data, data, queue->element_size);
    queue->head->contains_data = true;
    //__sync_bool_compare_and_swap(&(queue->head->contains_data), false, true);
    queue->head = queue->head->next;
    return true;
}

bool spsc_circular_queue_pop(SpscCircularQueue *queue, uint8_t *data)
{
    assert(queue != NULL);
    assert(queue->tail != NULL);
    assert(data != NULL);

    if (!queue->tail->contains_data)
        return false;

    memcpy(data, queue->tail->data, queue->element_size);
    queue->tail->contains_data = false;
    //__sync_bool_compare_and_swap(&(queue->tail->contains_data), true, false);
    queue->tail = queue->tail->next;
    return true;
}

SpscCircularQueue* spsc_circular_queue_alloc(int elements_num, size_t element_size)
{
    assert(elements_num > 0);
    assert(element_size > 0);

    SpscCircularQueue *queue = calloc(1, sizeof(SpscCircularQueue));
    if (!queue)
        return NULL;

    queue->elements = calloc(elements_num, sizeof(SpscCircularQueueElement));
    if (!queue->elements) {
        free(queue);
        return NULL;
    }

    queue->data = calloc(elements_num, element_size);
    if (!queue->data) {
        free(queue->elements);
        free(queue);
        return NULL;
    }

    queue->elements_num = elements_num;
    queue->element_size = element_size;

    for (int i = 0; i < elements_num; i++) {
        queue->elements[i].contains_data = false;
        queue->elements[i].data = queue->data + (i * element_size);

        if (i == (elements_num - 1))
            queue->elements[i].next = &(queue->elements[0]);
        else
            queue->elements[i].next = &(queue->elements[i + 1]);
    }

    queue->head = queue->tail = queue->elements;
    return queue;
}

void spsc_circular_queue_free(SpscCircularQueue *queue)
{
    if (!queue)
        return;

    if (queue->data)
        free(queue->data);

    if (queue->elements)
        free(queue->elements);

    free(queue);
}
