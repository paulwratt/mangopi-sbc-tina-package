/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     object_transfer.c
  * @brief    Source file for object transfer server model.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2018-5-21
  * @version  v1.0
  * *************************************************************************************
  */

#define MM_ID MM_MODEL

/* Add Includes here */
#include <string.h>
#include "mesh_api.h"
#include "object_transfer.h"

#define OBJ_TRANSFER_MAX_CHUNK_NUM      20

mesh_model_info_t obj_transfer_server;
struct
{
    obj_transfer_phase_t phase;
    obj_block_check_algo_t check_algo;
    uint8_t object_id[8];
    uint32_t object_size;
    uint32_t block_size;
    uint16_t block_num;
    //uint32_t current_block_size;
    uint16_t chunk_size; //!< maybe different in different blocks
    uint8_t chunk_flag[(OBJ_TRANSFER_MAX_CHUNK_NUM + 7) / 8]; //!< todo
    uint32_t max_object_size; //!< supported max size
    uint8_t min_block_size_log;
    uint8_t max_block_size_log;
    //uint16_t max_chunk_num;
} obj_transfer_server_ctx;

static mesh_msg_send_cause_t obj_transfer_server_send(mesh_msg_p pmesh_msg, uint8_t *pmsg,
                                                      uint16_t len)
{
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = pmesh_msg->pmodel_info;
    access_cfg(&mesh_msg);
    mesh_msg.pbuffer = pmsg;
    mesh_msg.msg_len = len;
    mesh_msg.dst = pmesh_msg->src;
    mesh_msg.app_key_index = pmesh_msg->app_key_index;
    return access_send(&mesh_msg);
}

mesh_msg_send_cause_t obj_transfer_phase_stat(mesh_msg_p pmesh_msg, obj_transfer_phase_t phase,
                                              uint8_t object_id[8])
{
    obj_transfer_phase_stat_t msg;
    msg.phase = phase;
    if (object_id)
    {
        memcpy(msg.object_id, object_id, sizeof(msg.object_id));
    }
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_OBJ_TRANSFER_PHASE_STAT);
    return obj_transfer_server_send(pmesh_msg, (uint8_t *)&msg,
                                    sizeof(obj_transfer_phase_stat_t) - (object_id != NULL ? 0 : 8));
}

mesh_msg_send_cause_t obj_transfer_stat(mesh_msg_p pmesh_msg, obj_transfer_stat_stat_t stat,
                                        uint8_t object_id[8], uint32_t object_size, uint8_t block_size_log)
{
    obj_transfer_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_OBJ_TRANSFER_STAT);
    msg.stat = stat;
    memcpy(msg.object_id, object_id, sizeof(msg.object_id));
    msg.object_size = object_size;
    msg.block_size_log = block_size_log;
    return obj_transfer_server_send(pmesh_msg, (uint8_t *)&msg, sizeof(obj_transfer_stat_t));
}

mesh_msg_send_cause_t obj_block_transfer_stat(mesh_msg_p pmesh_msg,
                                              obj_block_transfer_stat_stat_t stat)
{
    obj_block_transfer_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_OBJ_BLOCK_TRANSFER_STAT);
    msg.stat = stat;
    return obj_transfer_server_send(pmesh_msg, (uint8_t *)&msg, sizeof(obj_block_transfer_stat_t));
}

mesh_msg_send_cause_t obj_block_stat(mesh_msg_p pmesh_msg, obj_block_transfer_stat_stat_t stat,
                                     uint16_t chunk_list[], uint16_t chunck_count)
{
    mesh_msg_send_cause_t ret;
    obj_block_stat_t *pmsg = (obj_block_stat_t *)plt_malloc(MEMBER_OFFSET(obj_block_stat_t,
                                                                          chunk_list) + chunck_count * 2, RAM_TYPE_DATA_OFF);
    if (pmsg == NULL)
    {
        return MESH_MSG_SEND_CAUSE_NO_MEMORY;
    }
    ACCESS_OPCODE_BYTE(pmsg->opcode, MESH_MSG_OBJ_BLOCK_STAT);
    pmsg->stat = stat;
    if (chunck_count && chunk_list)
    {
        memcpy(pmsg->chunk_list, chunk_list, chunck_count * 2);
    }
    ret = obj_transfer_server_send(pmesh_msg, (uint8_t *)pmsg, MEMBER_OFFSET(obj_block_stat_t,
                                                                             chunk_list) + chunck_count * 2);
    plt_free(pmsg, RAM_TYPE_DATA_OFF);
    return ret;
}

mesh_msg_send_cause_t obj_info_stat(mesh_msg_p pmesh_msg, uint8_t min_block_size_log,
                                    uint8_t max_block_size_log, uint16_t max_chunk_num)
{
    obj_info_stat_t msg;
    ACCESS_OPCODE_BYTE(msg.opcode, MESH_MSG_OBJ_INFO_STAT);
    msg.min_block_size_log = min_block_size_log;
    msg.max_block_size_log = max_block_size_log;
    msg.max_chunk_num = max_chunk_num;
    return obj_transfer_server_send(pmesh_msg, (uint8_t *)&msg, sizeof(obj_info_stat_t));
}

void obj_transfer_server_handle_obj_block_transfer_start(mesh_msg_p pmesh_msg)
{
    obj_block_transfer_stat_stat_t stat;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    obj_block_transfer_start_t *pmsg = (obj_block_transfer_start_t *)pbuffer;

    if (obj_transfer_server_ctx.phase == OBJ_TRANSFER_PHASE_IDLE)
    {
        stat = OBJ_BLOCK_TRANSFER_STAT_INVALID_STATE;
        goto end;
    }

    if (0 != memcmp(pmsg->object_id, obj_transfer_server_ctx.object_id,
                    sizeof(obj_transfer_server_ctx.object_id)))
    {
        stat = OBJ_BLOCK_TRANSFER_STAT_WRONG_OBJECT_ID;
        goto end;
    }

    if (obj_transfer_server_ctx.block_num < pmsg->block_num)
    {
        stat = OBJ_BLOCK_TRANSFER_STAT_INVALID_BLOCK_NUM;
        goto end;
    }
    else if (obj_transfer_server_ctx.block_num > pmsg->block_num)
    {
        stat = OBJ_BLOCK_TRANSFER_STAT_DUPLICATE_BLOCK;
        goto end;
    }

    if (!pmsg->chunk_size)
    {
        stat = OBJ_BLOCK_TRANSFER_STAT_INVALID_PARAMETER;
        goto end;
    }

    uint16_t block_num, chunk_num;
    uint32_t block_size;
    block_num = (obj_transfer_server_ctx.object_size + obj_transfer_server_ctx.block_size - 1) /
                obj_transfer_server_ctx.block_size;
    if (obj_transfer_server_ctx.block_num == block_num - 1)
    {
        block_size = obj_transfer_server_ctx.object_size % obj_transfer_server_ctx.block_size;
        if (!block_size)
        {
            block_size = obj_transfer_server_ctx.block_size;
        }
    }
    else
    {
        block_size = obj_transfer_server_ctx.block_size;
    }
    chunk_num = (block_size + pmsg->chunk_size - 1) / pmsg->chunk_size;
    if (chunk_num > OBJ_TRANSFER_MAX_CHUNK_NUM)
    {
        stat = OBJ_BLOCK_TRANSFER_STAT_WRONG_CHUNK_SIZE;
        goto end;
    }

    if (pmsg->check_algo > OBJ_BLOCK_CHECK_ALGO_CRC32)
    {
        stat = OBJ_BLOCK_TRANSFER_STAT_UNKNOWN_CHECK_ALGO;
        goto end;
    }

    stat = OBJ_BLOCK_TRANSFER_STAT_ACCEPTED;
    obj_transfer_server_ctx.phase = OBJ_TRANSFER_PHASE_WAITING_CHUNK;
    //obj_transfer_server_ctx.block_num = pmsg->block_num;
    obj_transfer_server_ctx.chunk_size = pmsg->chunk_size;
    obj_transfer_server_ctx.check_algo = pmsg->check_algo;
    memset(obj_transfer_server_ctx.chunk_flag, 0, sizeof(obj_transfer_server_ctx.chunk_flag));
    printi("obj_transfer_server_handle_obj_block_transfer_start: block num %d, chunk size %d",
           pmsg->block_num, pmsg->chunk_size);
    // TODO: save check value

end:
    if (stat != OBJ_BLOCK_TRANSFER_STAT_ACCEPTED)
    {
        printw("obj_transfer_server_handle_obj_block_transfer_start: stat = %d", stat);
    }
    obj_block_transfer_stat(pmesh_msg, stat);
}

void obj_transfer_server_handle_obj_chunk_transfer(mesh_msg_p pmesh_msg)
{
    uint32_t ret = 0;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    obj_chunk_transfer_t *pmsg = (obj_chunk_transfer_t *)pbuffer;
    if (obj_transfer_server_ctx.phase != OBJ_TRANSFER_PHASE_WAITING_CHUNK)
    {
        ret = __LINE__;
        goto end;
    }

    uint16_t block_num, chunk_num, chunk_size;
    uint32_t block_size;
    block_num = (obj_transfer_server_ctx.object_size + obj_transfer_server_ctx.block_size - 1) /
                obj_transfer_server_ctx.block_size;
    if (obj_transfer_server_ctx.block_num == block_num - 1)
    {
        block_size = obj_transfer_server_ctx.object_size % obj_transfer_server_ctx.block_size;
        if (!block_size)
        {
            block_size = obj_transfer_server_ctx.block_size;
        }
    }
    else
    {
        block_size = obj_transfer_server_ctx.block_size;
    }
    chunk_num = (block_size + obj_transfer_server_ctx.chunk_size - 1) /
                obj_transfer_server_ctx.chunk_size;
    chunk_size = pmesh_msg->msg_len - MEMBER_OFFSET(obj_chunk_transfer_t, data);
    printi("obj_transfer_server_handle_obj_chunk_transfer: block num %d/%d size %d, chunk num %d/%d size %d",
           obj_transfer_server_ctx.block_num, block_num, block_size,
           pmsg->chunk_num, chunk_num, chunk_size);

    if (pmsg->chunk_num >= chunk_num)
    {
        ret = __LINE__;
        goto end;
    }
    else
    {
        if (pmsg->chunk_num < chunk_num - 1)
        {
            if (chunk_size != obj_transfer_server_ctx.chunk_size)
            {
                ret = __LINE__;
                goto end;
            }
        }
        else
        {
            uint16_t last_chunk_size = block_size % obj_transfer_server_ctx.chunk_size;
            if (!last_chunk_size)
            {
                last_chunk_size = obj_transfer_server_ctx.chunk_size;
            }
            if (chunk_size != last_chunk_size)
            {
                ret = __LINE__;
                goto end;
            }
        }
    }

    if (plt_bit_pool_get(obj_transfer_server_ctx.chunk_flag, pmsg->chunk_num))
    {
        // duplicate
        ret = __LINE__;
        goto end;
    }

    plt_bit_pool_set(obj_transfer_server_ctx.chunk_flag, pmsg->chunk_num, TRUE);

    // check block complete
    uint16_t loop;
    for (loop = 0; loop < chunk_num; loop++)
    {
        if (!plt_bit_pool_get(obj_transfer_server_ctx.chunk_flag, loop))
        {
            break;
        }
    }
    if (loop < chunk_num)
    {
        ret = __LINE__;
        goto end;
    }

    // TODO: validate checksum?
    obj_transfer_server_ctx.phase = OBJ_TRANSFER_PHASE_WAITING_BLOCK;
    obj_transfer_server_ctx.block_num++;

    // check object complete
    if (obj_transfer_server_ctx.block_num != block_num)
    {
        ret = __LINE__;
        goto end;
    }

    obj_transfer_server_ctx.phase = OBJ_TRANSFER_PHASE_IDLE;
    printi("obj_transfer_server_handle_obj_chunk_transfer: done!");
    // TODO: inform the upper layer

    return;

end:
    printi("obj_transfer_server_handle_obj_chunk_transfer: ret = %d", ret);
}

void obj_transfer_server_handle_obj_block_get(mesh_msg_p pmesh_msg)
{
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    obj_block_get_t *pmsg = (obj_block_get_t *)pbuffer;
    // delete this check?
    /*
    if (obj_transfer_server_ctx.phase == OBJ_TRANSFER_PHASE_IDLE)
    {
        obj_block_stat(pmesh_msg, OBJ_BLOCK_TRANSFER_STAT_INVALID_STATE, NULL, 0);
        return;
    }
    */

    if (0 != memcmp(pmsg->object_id, obj_transfer_server_ctx.object_id,
                    sizeof(obj_transfer_server_ctx.object_id)))
    {
        obj_block_stat(pmesh_msg, OBJ_BLOCK_TRANSFER_STAT_WRONG_OBJECT_ID, NULL, 0);
        return;
    }

    if (pmsg->block_num < obj_transfer_server_ctx.block_num)
    {
        obj_block_stat(pmesh_msg, OBJ_BLOCK_TRANSFER_STAT_ACCEPTED, NULL, 0);
    }
    else
    {
        // TODO:
        uint16_t *chunk_list = plt_malloc(OBJ_TRANSFER_MAX_CHUNK_NUM << 1, RAM_TYPE_DATA_OFF);
        uint16_t chunk_list_len = 0;
        if (chunk_list)
        {
            uint16_t block_num, chunk_num;
            uint32_t block_size;
            block_num = (obj_transfer_server_ctx.object_size + obj_transfer_server_ctx.block_size - 1) /
                        obj_transfer_server_ctx.block_size;
            if (obj_transfer_server_ctx.block_num == block_num - 1)
            {
                block_size = obj_transfer_server_ctx.object_size % obj_transfer_server_ctx.block_size;
                if (!block_size)
                {
                    block_size = obj_transfer_server_ctx.block_size;
                }
            }
            else
            {
                block_size = obj_transfer_server_ctx.block_size;
            }
            chunk_num = (block_size + obj_transfer_server_ctx.chunk_size - 1) /
                        obj_transfer_server_ctx.chunk_size;
            for (uint16_t loop = 0; loop < chunk_num; loop++)
            {
                if (!plt_bit_pool_get(obj_transfer_server_ctx.chunk_flag, loop))
                {
                    chunk_list[chunk_list_len] = loop;
                    chunk_list_len++;
                }
            }
        }
        obj_block_stat(pmesh_msg, chunk_list_len ? OBJ_BLOCK_TRANSFER_STAT_NOT_ALL_CHUNCK_RECEIVED :
                       OBJ_BLOCK_TRANSFER_STAT_ACCEPTED, chunk_list, chunk_list_len);
        plt_free(chunk_list, RAM_TYPE_DATA_OFF);
    }
}

/**
    Sample
*/
bool obj_transfer_server_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_OBJ_TRANSFER_PHASE_GET:
        if (pmesh_msg->msg_len == sizeof(obj_transfer_phase_get_t))
        {
            obj_transfer_phase_stat(pmesh_msg, obj_transfer_server_ctx.phase,
                                    obj_transfer_server_ctx.phase != OBJ_TRANSFER_PHASE_IDLE ? obj_transfer_server_ctx.object_id :
                                    NULL);
        }
        break;
    case MESH_MSG_OBJ_TRANSFER_GET:
        if (pmesh_msg->msg_len == sizeof(obj_transfer_get_t))
        {
            obj_transfer_stat_stat_t stat;
            obj_transfer_get_t *pmsg = (obj_transfer_get_t *)pbuffer;
            if (obj_transfer_server_ctx.phase == OBJ_TRANSFER_PHASE_IDLE)
            {
                stat = OBJ_TRANSFER_STAT_READY;
            }
            else
            {
                if (0 == memcmp(pmsg->object_id, obj_transfer_server_ctx.object_id,
                                sizeof(obj_transfer_server_ctx.object_id)))
                {
                    stat = OBJ_TRANSFER_STAT_BUSY;
                }
                else
                {
                    stat = OBJ_TRANSFER_STAT_INVALID_PARAMETER;
                }
            }
            obj_transfer_stat(pmesh_msg, stat, obj_transfer_server_ctx.object_id,
                              obj_transfer_server_ctx.object_size, plt_log2(obj_transfer_server_ctx.block_size));
        }
        break;
    case MESH_MSG_OBJ_TRANSFER_START:
        if (pmesh_msg->msg_len == sizeof(obj_transfer_start_t))
        {
            obj_transfer_stat_stat_t stat;
            obj_transfer_start_t *pmsg = (obj_transfer_start_t *)pbuffer;
            if (obj_transfer_server_ctx.phase != OBJ_TRANSFER_PHASE_IDLE)
            {
                stat = OBJ_TRANSFER_STAT_BUSY;
            }
            else if (pmsg->curr_block_size_log < BLOCK_SIZE_LOG_MIN ||
                     pmsg->curr_block_size_log > BLOCK_SIZE_LOG_MAX)
            {
                stat = OBJ_TRANSFER_STAT_INVALID_PARAMETER;
            }
            else
            {
                uint8_t loop = 0;
                for (; loop < sizeof(obj_transfer_server_ctx.object_id); loop++)
                {
                    if (pmsg->object_id[loop])
                    {
                        break;
                    }
                }
                if (loop >= sizeof(obj_transfer_server_ctx.object_id))
                {
                    stat = OBJ_TRANSFER_STAT_INVALID_PARAMETER;
                }
                else
                {
                    if (pmsg->object_size > obj_transfer_server_ctx.max_object_size)
                    {
                        stat = OBJ_TRANSFER_STAT_NOT_SUPPORTED;
                    }
                    else
                    {
                        obj_transfer_server_ctx.phase = OBJ_TRANSFER_PHASE_WAITING_BLOCK;
                        memcpy(obj_transfer_server_ctx.object_id, pmsg->object_id,
                               sizeof(obj_transfer_server_ctx.object_id));
                        obj_transfer_server_ctx.object_size = pmsg->object_size;
                        obj_transfer_server_ctx.block_size = plt_exp2(pmsg->curr_block_size_log);
                        stat = OBJ_TRANSFER_STAT_BUSY;
                    }
                }
            }
            obj_transfer_stat(pmesh_msg, stat, obj_transfer_server_ctx.object_id,
                              obj_transfer_server_ctx.object_size, plt_log2(obj_transfer_server_ctx.block_size));
        }
        break;
    case MESH_MSG_OBJ_TRANSFER_ABORT:
        if (pmesh_msg->msg_len == sizeof(obj_transfer_abort_t))
        {
            obj_transfer_stat_stat_t stat;
            obj_transfer_abort_t *pmsg = (obj_transfer_abort_t *)pbuffer;
            if (obj_transfer_server_ctx.phase == OBJ_TRANSFER_PHASE_IDLE)
            {
                stat = OBJ_TRANSFER_STAT_CANNOT_ABORT;
            }
            else
            {
                if (0 == memcmp(pmsg->object_id, obj_transfer_server_ctx.object_id,
                                sizeof(obj_transfer_server_ctx.object_id)))
                {
                    obj_transfer_server_ctx.phase = OBJ_TRANSFER_PHASE_IDLE;
                    // TODO:  Clear rx data
                    stat = OBJ_TRANSFER_STAT_READY;
                }
                else
                {
                    stat = OBJ_TRANSFER_STAT_INVALID_PARAMETER;
                }
            }
            obj_transfer_stat(pmesh_msg, stat, obj_transfer_server_ctx.object_id,
                              obj_transfer_server_ctx.object_size, plt_log2(obj_transfer_server_ctx.block_size));
        }
        break;
    case MESH_MSG_OBJ_BLOCK_TRANSFER_START:
        obj_transfer_server_handle_obj_block_transfer_start(pmesh_msg);
        break;
    case MESH_MSG_OBJ_CHUNCK_TRANSFER:
        obj_transfer_server_handle_obj_chunk_transfer(pmesh_msg);
        break;
    case MESH_MSG_OBJ_BLOCK_GET:
        if (pmesh_msg->msg_len == sizeof(obj_block_get_t))
        {
            obj_transfer_server_handle_obj_block_get(pmesh_msg);
        }
        break;
    case MESH_MSG_OBJ_INFO_GET:
        if (pmesh_msg->msg_len == sizeof(obj_info_get_t))
        {
            obj_info_stat(pmesh_msg, obj_transfer_server_ctx.min_block_size_log,
                          obj_transfer_server_ctx.max_block_size_log, OBJ_TRANSFER_MAX_CHUNK_NUM);
        }
        break;
    default:
        ret = FALSE;
        break;
    }

    if (ret)
    {
        printi("obj_transfer_server_receive: opcode = 0x%x, phase = %d", pmesh_msg->access_opcode,
               obj_transfer_server_ctx.phase);
    }
    return ret;
}

void obj_transfer_server_reg(uint32_t max_object_size)
{
    obj_transfer_server.model_id = MESH_MODEL_OBJ_TRANSFER_SERVER;
    obj_transfer_server.model_receive = obj_transfer_server_receive;
    mesh_model_reg(0, &obj_transfer_server);
    obj_transfer_server_ctx.max_object_size = max_object_size;
    obj_transfer_server_ctx.max_block_size_log = 12;
    obj_transfer_server_ctx.min_block_size_log = 10;
}
