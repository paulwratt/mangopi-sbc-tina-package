/**
 * Copyright (c) 2012 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "app_timer_mesh.h"
#include "mesh_internal_api.h"
uint32_t app_timer_pass(app_timer_id_t *p_time_id)
{
    struct timeval cur_systime;
    timer_node_t * p_node     = p_time_id;
    uint32_t delt = 0;
    gettimeofday(&cur_systime,NULL);
    if(cur_systime.tv_sec >= p_node->last_systime.tv_sec)
    {
        delt = 1000*(cur_systime.tv_sec - p_node->last_systime.tv_sec);
        if(cur_systime.tv_usec > p_node->last_systime.tv_usec)
        {
            delt += (cur_systime.tv_usec - p_node->last_systime.tv_usec)/1000;
        }
        else
        {
            if(delt > 0)
            {
                delt -= (p_node->last_systime.tv_usec - cur_systime.tv_usec)/1000;
            }
        }
    }
    return delt;
}

ret_code_t app_timer_init(void)
{
    return NRF_SUCCESS;
}

ret_code_t app_timer_create(app_timer_id_t *      p_timer_id,
                            app_timer_mode_t            mode,
                            app_timer_timeout_handler_t timeout_handler)
{
    if (timeout_handler == NULL)
    {
        l_info("%s,timeout_handler fail\n",__FUNCTION__);
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_timer_id == NULL)
    {
        l_info("%s,p_timer_id fail\n",__FUNCTION__);
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_timer_id->is_running == true)
    {
        l_info("%s,is_running fail\n",__FUNCTION__);
        return NRF_ERROR_INVALID_STATE;
    }

    timer_node_t * p_node     = p_timer_id;
    p_node->is_running        = false;
    p_node->mode              = mode;
    p_node->p_timeout_handler = timeout_handler;
    p_node->p_timer_handler =  NULL;
    return NRF_SUCCESS;
}

ret_code_t app_timer_start(app_timer_id_t *p_timer_id, uint32_t timeout_ticks, void * p_context)
{
//    uint32_t timeout_periodic;
    uint32_t status = NRF_SUCCESS;
    struct timeval cur_systime;
    timer_node_t * p_node = (timer_node_t*)p_timer_id;

    gettimeofday(&cur_systime,NULL);
    l_info("%s",__FUNCTION__);
    if (p_timer_id == NULL)
    {
        l_info("%s,p_timer_id fail\n",__FUNCTION__);
        return NRF_ERROR_INVALID_STATE;
    }

    if (p_node->p_timeout_handler == NULL)
    {
        l_info("%s,p_timeout_handler fail\n",__FUNCTION__);

        return NRF_ERROR_INVALID_STATE;
    }

    if(p_node->p_timer_handler != NULL)
    {
        l_info("%s,p_timer_handler fail\n",__FUNCTION__);

        return NRF_ERROR_INVALID_STATE;
    }

    if (p_timer_id->is_running == true)
    {
        l_info("%s,is_running fail\n",__FUNCTION__);
        return NRF_ERROR_INVALID_STATE;
    }
    p_timer_id->last_systime = cur_systime;
    p_node->p_timer_handler = l_timeout_create_ms(timeout_ticks, p_node->p_timeout_handler, p_context, NULL);
    //l_timeout_create api provide secondes timer, and l_timeout_create_ms provides ms timer.
    l_info("%s,%d,%p,%p,%p,%p\n",__FUNCTION__,timeout_ticks,p_context,p_timer_id,p_node->p_timeout_handler,p_node->p_timer_handler);
    return status;
}


ret_code_t app_timer_stop(app_timer_id_t *p_timer_id)
{
    timer_node_t * p_node = (timer_node_t*)p_timer_id;

    l_info("%s,%p,%p,%p\n",__FUNCTION__,p_timer_id,p_node->p_timeout_handler,p_node->p_timer_handler);
    if ((p_timer_id == NULL) || (p_node->p_timeout_handler == NULL)||(p_node->p_timer_handler == NULL))
    {
        l_info("%s,fail\n",__FUNCTION__);
        return NRF_ERROR_INVALID_STATE;
    }

    l_timeout_remove(p_node->p_timer_handler);
    p_node->p_timer_handler = NULL;
    p_node->is_running = false;

    // Schedule timer stop operation
    return NRF_SUCCESS;
}


ret_code_t app_timer_stop_all(void)
{
    return NRF_SUCCESS;
}
