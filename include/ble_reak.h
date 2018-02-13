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
 * ble_reak.h
 * - RSL10 Express Adaptation Kit
 * ----------------------------------------------------------------------------
 * $Revision: 1.8 $
 * $Date: 2017/06/12 15:55:37 $
 * ------------------------------------------------------------------------- */

#ifndef BLE_REAK_H
#define BLE_REAK_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* Simple helper functions */
#define MIN(a,b) (((a)<(b))?(a):(b))

/* Standard declaration/description UUIDs in 16-byte format */
#define REAK_ATT_SERVICE_128            {0x00,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define REAK_ATT_CHARACTERISTIC_128     {0x03,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define REAK_ATT_CLIENT_CHAR_CFG_128    {0x02,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define REAK_ATT_CHAR_USER_DESC_128     {0x01,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}

/* Macros to define reak_att_desc structures for custom services
	struct reak_att_desc {
    	struct gattm_att_desc att;
    	bool is_service;
    	uint16_t length;
    	void *data;
    	void *fct;
	}; */

/* Macros to declare a (custom) service with 16, 32 and 128 bit UUID
 *   - uuid: Service UUID */
#define REAK_SERVICE_UUID_16(uuid) \
            {{uuid, PERM(SVC_UUID_LEN, UUID_16), 0, 0}, true, 0, NULL, NULL}
#define REAK_SERVICE_UUID_32(uuid) \
            {{uuid, PERM(SVC_UUID_LEN, UUID_32), 0, 0}, true, 0, NULL, NULL}
#define REAK_SERVICE_UUID_128(uuid) \
            {{uuid, PERM(SVC_UUID_LEN, UUID_128), 0, 0}, true, 0, NULL, NULL}

/* Macros to define characteristics with 16, 32 and 128 bit UUID
 *   - uuid: UUID
 *   - perm: Permissions (see gattm_att_desc)
 *   - length: Value max length (in bytes)
 *   - data: Pointer to the data structure in the application
 *   - callback: Function to transfer the data between the application and the GATTM */
#define REAK_CHAR_UUID_16(uuid, perm, length, data, callback) \
            {{REAK_ATT_CHARACTERISTIC_128, PERM(RD, ENABLE), 0, 0}, false, 0, NULL, NULL}, \
            {{uuid, perm, length, PERM(RI,ENABLE) | PERM(UUID_LEN,UUID_16)}, false, length, data, callback}
#define REAK_CHAR_UUID_32(uuid, perm, length, data, callback) \
            {{REAK_ATT_CHARACTERISTIC_128, PERM(RD, ENABLE), 0, 0}, false, 0, NULL, NULL}, \
            {{uuid, perm, length, PERM(RI,ENABLE) | PERM(UUID_LEN,UUID_32)}, false, length, data, callback}
#define REAK_CHAR_UUID_128(uuid, perm, length, data, callback) \
            {{REAK_ATT_CHARACTERISTIC_128, PERM(RD, ENABLE), 0, 0}, false, 0, NULL, NULL}, \
            {{uuid, perm, length, PERM(RI,ENABLE) | PERM(UUID_LEN,UUID_128)}, false, length, data, callback}

/* Macro to add to the characteristic a CCC
 *   - data: Pointer to the 2-byte CCC data value in the application
 *   - callback: Function to transfer the CCC data between the application and the GATTM */
#define REAK_CHAR_CCC(data, callback) \
            {{REAK_ATT_CLIENT_CHAR_CFG_128, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, PERM(RI, ENABLE)}, false, 2, data, callback}

/* Macro to add to the characteristic a user description
 *   - length: Description length (in bytes)
 *   - data: Pointer to the description string (constant)
 *   - callback: Function to transfer the description string to the GATTM */
#define REAK_CHAR_USER_DESC(length, data, callback) \
            {{REAK_ATT_CHAR_USER_DESC_128, PERM(RD, ENABLE), length, PERM(RI, ENABLE)}, false, length, data, callback}

/* Custom service call back access (read or write) */
enum reak_cb_access {
    /* Read callback */
    reak_cb_read,
    /* Write callback */
    reak_cb_write
};

/* List of message handlers that are used by the custom service application manager */
#define REAK_MESSAGE_HANDLER_LIST \
        DEFINE_MESSAGE_HANDLER(GATTC_READ_REQ_IND, GATTC_ReadReqInd),\
        DEFINE_MESSAGE_HANDLER(GATTC_WRITE_REQ_IND, GATTC_WriteReqInd),\
        DEFINE_MESSAGE_HANDLER(GATTM_ADD_SVC_RSP, GATTM_AddSvcRsp)\

/* Define the available custom service states */
enum reak_state
{
    REAK_INIT,
    REAK_SERVICE_DISCOVERD,
    REAK_ALL_ATTS_DISCOVERED,
    REAK_STATE_MAX
};

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

/* Custom service environment */
struct reak_env_tag
{
    /* The value of service handle in the database of attributes in the stack */
    uint16_t start_hdl;

    /* Number of defined custom service attributes (all services together) */
    uint16_t nb_att;

    /* Number of service attribute definitions  */
    const uint8_t reak_max_idx;

    /* The state machine for service discovery, it is not used for server role */
    uint8_t state;
};

extern struct reak_env_tag    reak_env;


/* REAK custom service attribute definitions */
struct reak_att_desc {
    struct gattm_att_desc att;
    bool is_service;
    uint16_t length;
    void *data;
    void *fct;
};

extern const struct reak_att_desc    reak_att[];

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/
extern void REAK_Env_Initialize(void);
extern bool REAK_ServiceAdd(void);
extern void REAK_GenericDataAccess(void *gattm_data, void *data, uint16_t length, uint8_t access);
extern int GATTM_AddSvcRsp(ke_msg_id_t const msgid,
                           struct gattm_add_svc_rsp const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);
extern int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
                     struct gattc_read_req_ind const *param,
                     ke_task_id_t const dest_id,
                     ke_task_id_t const src_id);
extern int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
                      struct gattc_write_req_ind const *param,
                      ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern void REAK_SendNotification(void *data);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* BLE_REAK_H */
