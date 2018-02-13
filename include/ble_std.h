/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * This module is derived in part from example code provided by RivieraWaves
 * and as such the underlying code is the property of RivieraWaves [a member
 * of the CEVA, Inc. group of companies], together with additional code which
 * is the property of ON Semiconductor. The code (in whole or any part) may not
 * be redistributed in any form without prior written permission from
 * ON Semiconductor.
 *
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * ble_std.h
 * - Bluetooth standard header
 * ----------------------------------------------------------------------------
 * $Revision: 1.10 $
 * $Date: 2017/06/12 15:55:37 $
 * ------------------------------------------------------------------------- */

#ifndef BLE_STD_H
#define BLE_STD_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* Number of APP Task Instances */
#define APP_IDX_MAX                     1

/* Define the available application states */
enum appm_state
{
    /* Initialization state */
    APPM_INIT,
    /* Database create state */
    APPM_CREATE_DB,
    /* Ready State */
    APPM_READY,
    /* Advertising state */
    APPM_ADVERTISING,
    /* Connecting state */
    APPM_CONNECTING,
    /* Connected state */
    APPM_CONNECTED,
    /* Number of defined states */
    APPM_STATE_MAX
};

/* List of message handlers that are used by the Bluetooth application manager */
#define BLE_MESSAGE_HANDLER_LIST \
        DEFINE_MESSAGE_HANDLER(GAPM_CMP_EVT, GAPM_CmpEvt),\
        DEFINE_MESSAGE_HANDLER(GAPM_PROFILE_ADDED_IND, GAPM_ProfileAddedInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_CONNECTION_REQ_IND, GAPC_ConnectionReqInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_CMP_EVT, GAPC_CmpEvt),\
        DEFINE_MESSAGE_HANDLER(GAPC_DISCONNECT_IND, GAPC_DisconnectInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_GET_DEV_INFO_REQ_IND, GAPC_GetDevInfoReqInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_PARAM_UPDATED_IND, GAPC_ParamUpdatedInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_PARAM_UPDATE_REQ_IND, GAPC_ParamUpdateReqInd)\

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

/* Support for the application manager and the application environment */
extern const struct ke_state_handler    appm_default_handler;
extern ke_state_t                       appm_state[APP_IDX_MAX];

struct ble_env_tag {
    /* Connection handle */
    uint16_t conhdl;

    /* Connection index */
    uint8_t conidx;

    /* Next service to initialize */
    uint8_t next_svc;

    /* Bond status */
    bool bonded;

    /* Application state */
    uint8_t state;

    /* Connection parameters */
    uint16_t con_interval;
    uint16_t time_out;
    uint16_t updated_con_interval;
    uint16_t updated_latency;
    uint16_t updated_suo_to;
};

/* Support for the application manager and the application environment */
extern struct ble_env_tag ble_env;

/* Bluetooth Device Address */
extern uint8_t                          bdaddr[];

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/

/* Bluetooth baseband application support functions */
extern void BLE_Initialize(void);
extern bool Service_Add(void);
extern void Advertising_Start(void);
extern void Advertising_Stop(void);
extern void Connection_ParamUpdate(struct gapc_conn_param *conn_param);
extern void Connection_Disconnect(void);
extern void BLE_SetStateEnable(void);
extern void BLE_SetServiceState(bool);

/* Bluetooth event and message handlers */
extern int GAPM_ProfileAddedInd(ke_msg_id_t const msgid,
                               struct gapm_profile_added_ind const *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);
extern int GAPM_CmpEvt(ke_msg_id_t const msgid,
                       struct gapm_cmp_evt const *param,
                       ke_task_id_t const dest_id,
                       ke_task_id_t const src_id);
extern int GAPC_CmpEvt(ke_msg_id_t const msgid,
                       struct gapc_cmp_evt const *param,
                       ke_task_id_t const dest_id,
                       ke_task_id_t const src_id);
extern int GAPC_GetDevInfoReqInd(ke_msg_id_t const msgid,
                                 struct gapc_get_dev_info_req_ind const *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id);
extern int Gapc_SetDevInfoReqInd(ke_msg_id_t const msgid,
                                 struct gapc_set_dev_info_req_ind const *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id);
extern int GAPC_DisconnectInd(ke_msg_id_t const msgid,
                              struct gapc_disconnect_ind const *param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id);
extern int GAPC_ParamUpdatedInd(ke_msg_id_t const msgid,
                                struct gapc_param_updated_ind const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id);
extern int GAPC_ParamUpdateReqInd(ke_msg_id_t const msg_id,
                           struct gapc_param_update_req_ind const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);
extern  int GAPC_ConnectionReqInd(ke_msg_id_t const msgid,
                                  struct gapc_connection_req_ind const *param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* BLE_STD_H */
