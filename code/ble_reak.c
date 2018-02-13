/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * ble_reak.c
 * - RSL10 Express Adaptation Kit
 * ----------------------------------------------------------------------------
 * $Revision: 1.8 $
 * $Date: 2017/06/09 17:07:33 $
 * ------------------------------------------------------------------------- */

#include "app.h"

/* Global variable definition */
struct reak_env_tag   reak_env;

/* ----------------------------------------------------------------------------
 * Function      : void REAK_Env_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize custom service environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void REAK_Env_Initialize(void)
{
    /* Reset the application manager environment */
    memset(&reak_env, 0, sizeof(reak_env));
}

/* ----------------------------------------------------------------------------
 * Function      : bool REAK_ServiceAdd(void)
 * ----------------------------------------------------------------------------
 * Description   : Send request to add custom profile into the attribute database.
 *                 Defines the different access functions (setter/getter commands
 *                 to access the different characteristic attributes).
 * Inputs        : None
 * Outputs       : Returns true Batt_ServiceAdd_Server has no additional
 *                 service to add, otherwise false.
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
bool REAK_ServiceAdd(void)
{
    uint8_t reak_max_idx = reak_att_desc_max_idx();
    struct gattm_add_svc_req *req;
    uint8_t nb_att;

    uint8_t att_idx = reak_env.nb_att + 1;
    while (att_idx < reak_max_idx && !reak_att[att_idx].is_service)
    {
        att_idx++;
    }
    nb_att = att_idx - reak_env.nb_att - 1;

    req = KE_MSG_ALLOC_DYN(GATTM_ADD_SVC_REQ,
            TASK_GATTM, TASK_APP, gattm_add_svc_req,
            nb_att * sizeof(struct gattm_att_desc));

    /* Fill the add custom service message */
    req->svc_desc.start_hdl = 0;
    req->svc_desc.task_id = TASK_APP;
    req->svc_desc.perm = reak_att[reak_env.nb_att].att.perm; // PERM(SVC_UUID_LEN, UUID_128);
    req->svc_desc.nb_att = nb_att;
    memcpy(&req->svc_desc.uuid[0], &reak_att[reak_env.nb_att++].att.uuid, ATT_UUID_128_LEN);

    for(att_idx = 0; att_idx < nb_att; att_idx++)
    {
        memcpy(&req->svc_desc.atts[att_idx], &reak_att[reak_env.nb_att++].att,
               sizeof(struct gattm_att_desc));
    }

    /* Send the message */
    ke_msg_send(req);

    /* Returns true if no additional custom services have to be added */
    return (reak_env.nb_att == reak_max_idx);
}

/* ----------------------------------------------------------------------------
 * Function      : void REAK_GenericDataAccess(void *gattm_data, void *app_data,
 *                                         uint16_t length, uint8_t access)
 * ----------------------------------------------------------------------------
 * Description   : Generic function to transfer the data between the application
 *                 and the GATTM. Called by GATTC_ReadReqInd and GATTC_WriteReqInd.
 * Inputs        : - gattm_data : Pointer to the GATTM data structure
 *                 - app_data   : Pointer to the application data structure
 *                 - length     : Data length (in bytes)
 *                 - access     : Data access (reak_cb_read or reak_cb_write)
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void REAK_GenericDataAccess(void *gattm_data, void *app_data, uint16_t length, uint8_t access)
{
    if (access == reak_cb_read)
    {
        memcpy(gattm_data, app_data, length);
    }
    else
    {
        memcpy(app_data, gattm_data, length);
    }
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTM_AddSvcRsp(ke_msg_id_t const msg_id,
 *                                     struct gattm_add_svc_rsp
 *                                     const *param,
 *                                     ke_task_id_t const dest_id,
 *                                     ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the response from adding a service in the attribute
 *                 database from the GATT manager
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattm_add_svc_rsp
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTM_AddSvcRsp(ke_msg_id_t const msg_id,
                    struct gattm_add_svc_rsp const *param,
                    ke_task_id_t const dest_id,
                    ke_task_id_t const src_id)
{
    if(reak_env.start_hdl == 0)
    {
        reak_env.start_hdl = param->start_hdl;
    }

    /* Add the next requested service  */
    if(!Service_Add())
    {
        /* All services have been added, go to the ready state
         * and start advertising */
        ble_env.state = APPM_READY;
        Advertising_Start();
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
 *                                      struct gattc_read_req_ind
 *                                      const *param,
 *                                      ke_task_id_t const dest_id,
 *                                      ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received read request indication from a GATT
 *                 controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_read_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
                     struct gattc_read_req_ind const *param,
                     ke_task_id_t const dest_id,
                     ke_task_id_t const src_id)
{
    uint8_t length = 0;
    uint8_t status = GAP_ERR_NO_ERROR;
    uint16_t attnum;
    struct gattc_read_cfm *cfm;

    /* Verify the correctness of the read request. Set the attribute index and
     * data length if the request is valid */
    attnum = (param->handle - reak_env.start_hdl);

    if(param->handle <= reak_env.start_hdl)
    {
        status = ATT_ERR_INVALID_HANDLE;
    }
    else if ( (attnum >= reak_env.nb_att) ||
               !(reak_att[attnum].att.perm & (PERM(RD,ENABLE) | PERM(NTF,ENABLE))) )
       {
        status = ATT_ERR_READ_NOT_PERMITTED;
    }
    else
    {
        length = reak_att[attnum].length;
    }

    /* Allocate and build message */
    cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                KE_BUILD_ID(TASK_GATTC, ble_env.conidx), TASK_APP, gattc_read_cfm,
                length);

    /* If there is no error, copy the requested attribute value, using the
     * callback function */
    if(status == GAP_ERR_NO_ERROR)
    {
        ((void(*)())reak_att[attnum].fct)(cfm->value, reak_att[attnum].data,
                                        length, reak_cb_read);
    }

    cfm->handle = param->handle;
    cfm->length = length;
    cfm->status = status;

    /* Send the message */
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
 *                                       struct gattc_write_req_ind
 *                                       const *param,
 *                                       ke_task_id_t const dest_id,
 *                                       ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received write request indication
 *                 from a GATT Controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_write_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
                      struct gattc_write_req_ind const *param,
                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_write_cfm *cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM,
            KE_BUILD_ID(TASK_GATTC, ble_env.conidx), TASK_APP, gattc_write_cfm);

    uint8_t status = GAP_ERR_NO_ERROR;
    uint16_t attnum;

    /* Verify the correctness of the write request. Set the attribute index if
     * the request is valid */
    attnum = (param->handle - reak_env.start_hdl);
    if(param->offset)
    {
        status = ATT_ERR_INVALID_OFFSET;
    }
    else if(param->handle <= reak_env.start_hdl)
    {
        status = ATT_ERR_INVALID_HANDLE;
    }
    else if ( (attnum >= reak_env.nb_att) ||
               !(reak_att[attnum].att.perm & (PERM(WRITE_REQ,ENABLE) | PERM(WRITE_COMMAND,ENABLE))) )
    {
        status = ATT_ERR_WRITE_NOT_PERMITTED;
    }

    /* If there is no error, copy the requested attribute value, using the
     * callback function */
    if(status == GAP_ERR_NO_ERROR)
    {
           ((void(*)())reak_att[attnum].fct)(param->value, reak_att[attnum].data,
                                           MIN(param->length,reak_att[attnum].length),
                                           reak_cb_write);
    }
    cfm->handle = param->handle;
    cfm->status = status;

    /* Send the message */
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void REAK_SendNotification(uint8_t conidx,
 *                               uint8_t attidx, uint8_t *value, uint8_t length)
 * ----------------------------------------------------------------------------
 * Description   : Send a notification to the client device
 * Inputs        : - data  - Pointer to the data structure in the application
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void REAK_SendNotification(void *data)
{
    int attidx;

    /* Ignore the notification request if no connection is established */
    if (ble_env.state != APPM_CONNECTED)
    {
    	return;
    }

    /* Search the relevant attribute index. Ignore the notification request if
     * the data pointer doesn't match with a registered data structure */
    for(attidx = 0; attidx < reak_env.nb_att; attidx++)
    {
        if(data == reak_att[attidx].data) {
            break;
        }
    }
    if(attidx==reak_env.nb_att)
    {
        return;
    }

//    /* Ignore the notification request of notification is not enabled */
//    if (attidx==reak_env.nb_att-1 || reak_att[attidx+1].att)

    uint8_t conidx = ble_env.conidx;
    struct gattc_send_evt_cmd *cmd;
    uint16_t handle = (attidx + reak_env.start_hdl);
    uint16_t length = reak_att[attidx].length;

    /* Prepare a notification message for the specified attribute */
    cmd = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                           KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                           gattc_send_evt_cmd,
                           length * sizeof(uint8_t));
    cmd->handle = handle;
    cmd->length = length;
    cmd->operation = GATTC_NOTIFY;
    cmd->seq_num = 0;
    memcpy(cmd->value, data, length);

    /* Send the message */
    ke_msg_send(cmd);
}
