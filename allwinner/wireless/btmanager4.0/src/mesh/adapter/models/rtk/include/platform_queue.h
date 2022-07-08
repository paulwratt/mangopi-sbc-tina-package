/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     platform_queue.h
  * @brief    Head file for platform queue related.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2017-9-28
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _PLATFORM_QUEUE_
#define _PLATFORM_QUEUE_

#if defined (__cplusplus)
extern "C" {
#endif

#include "platform_misc.h"

/** @addtogroup Mesh_Platform_Queue Mesh Platform Queue
  * @brief queue related
  * @{
  */

/** @defgroup Mesh_Platform_Queue_Exported_Types Mesh Platform Queue Exported Types
  * @brief
  * @{
  */

/** @brief general queue element type of link list */
typedef struct _plt_queue_member_t
{
    struct _plt_queue_member_t *pnext;
    uint8_t data[4];
} plt_queue_member_t, *plt_queue_member_p;

/** @brief general queue type of link list */
typedef struct
{
    plt_queue_member_p pfirst;
    plt_queue_member_p plast;
    uint32_t count;
} plt_queue_t, *plt_queue_p;

/** @} */

/** @defgroup Mesh_Platform_Queue_Exported_Functions Mesh Platform Queue Exported Functions
  * @brief
  * @{
  */

/**
  * @brief push in the member to the end of the queue
  * @param[in] pqueue: the queue pointer
  * @param[in] pqueue_member: the element pointer
  * @return none
  */
void  plt_queue_in(plt_queue_p pqueue, void *pqueue_member);

/**
  * @brief pull out the first member of the queue
  * @param[in] pqueue: the queue pointer
  * @return the queue member
  */
void *plt_queue_out(plt_queue_p pqueue);

/**
  * @brief insert the member to any position of the queue
  * @param[in] pqueue: the queue pointer
  * @param[in] pqueue_member: the ahead element pointer
  * @param[in] pqueue_member_new: the new element pointer
  * @return none
  */
void  plt_queue_insert(plt_queue_p pqueue, void *pqueue_member, void *pqueue_member_new);

/**
  * @brief delete the member at any position of the queue
  * @param[in] pqueue: the queue pointer
  * @param[in] pqueue_member_previous: the ahead element pointer
  * @param[in] pqueue_member: the deleted element pointer
  * @return none
  */
void  plt_queue_delete(plt_queue_p pqueue, void *pqueue_member_previous, void *pqueue_member);

/** @} */

/** @} End of group Mesh_Platform_Queue */

#if defined (__cplusplus)
}
#endif

#endif /* _PLATFORM_QUEUE_ */
