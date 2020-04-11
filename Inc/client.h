/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   led.h
 * Author: seongjinoh
 *
 * Created on Apri 11, 2020,10:05 AM
 */

#ifndef __CLIENT_H__
#define __CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

    /****************************************************************************/
    /** **/
    /** MODULES USED **/
    /** **/
    /****************************************************************************/

    /****************************************************************************/
    /** **/
    /** DEFINITIONS AND MACROS **/
    /** **/
    /****************************************************************************/
    /* Task Signals */
    #define CLIENT_CMD_Q_SIG                   (1 << 10)

    /* Task signal but notify to client task */
    #define CLIENT_ACK_SIG                     (1 << 28)
    #define CLIENT_NOTIFY_SIG                  (1 << 29)

    #define CLIENT_MAX_Q_SIZE                 10


    /****************************************************************************/
    /** **/
    /** TYPEDEFS AND STRUCTURES **/
    /** **/
    /****************************************************************************/
    /* Event IDs */
    typedef enum {
        CLIENT_EVT_NONE = 0,

        CLIENT_EVT_REQ_LED_SVC,

        CLIENT_EVT_ACK_RECIEVED,
        CLIENT_EVT_RESP_RECEIVED,
        CLIENT_EVT_ACK_TIMER_EXPIRED_NOTI,
        CLIENT_EVT_RESP_TIMER_EXPIRED_NOTI,
        CLIENT_EVT_MAX,
    } CLIENT_EVENT;

    typedef struct ClientMsgType {
        unsigned char   cmd;
        unsigned char   len;
        unsigned char   ack;
        int             nData;
        unsigned char   *pData;
        osThreadId      taskId;
    } CLIENT_MSG;


    /****************************************************************************/
    /** **/
    /** EXPORTED VARIABLES **/
    /** **/
    /****************************************************************************/

    /****************************************************************************/
    /** **/
    /** EXPORTED FUNCTIONS **/
    /** **/
    /****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __CLIENT_H__ */