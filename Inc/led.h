/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   led.h
 * Author: seongjinoh
 *
 * Created on Apri 04, 2020, 3:05 PM
 */

#ifndef __LED_H__
#define __LED_H__

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
    #define LED_CMD_Q_SIG                   (1 << 10)

    /* Task signal but notify to client task */
    #define LED_ACK_SIG                     (1 << 28)
    #define LED_NOTIFY_SIG                  (1 << 29)

    #define LED_MAX_Q_SIZE                 10


    /****************************************************************************/
    /** **/
    /** TYPEDEFS AND STRUCTURES **/
    /** **/
    /****************************************************************************/
    /* Event IDs */
    typedef enum {
        LED_EVT_NONE = 0,

        LED_EVT_ON_REQ,
        LED_EVT_ON_ACK,
        LED_EVT_ON_RSP,

        LED_EVT_OFF_REQ,
        LED_EVT_OFF_ACK,
        LED_EVT_OFF_RSP,

        LED_EVT_TIMER_EXPIRED_NOTI,
        LED_EVT_MAX,
    } LED_EVENT;

    #define MAX_ACK_SIZE    10
    typedef struct ack_t {
        osThreadId      taskId;
        LED_EVENT       event;
        unsigned char   ack;
        /* time to life */
    } LED_ACK;

    typedef struct LedMsgType {
        unsigned char   cmd;
        unsigned char   len;
        unsigned char   ack;
        int             nData;
        unsigned char   *pData;
        osThreadId      taskId;
    } LED_MSG;


    typedef struct led_t {
        unsigned char   initialized;
        LED_ACK         ack[MAX_ACK_SIZE];
    } LED;


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
    extern void ledInit(void);
    extern osThreadId ledGetClientTaskId(LED_EVENT e);
    extern void ledSetAckInfo(osThreadId taskId, LED_EVENT e);
    extern void ledClearAckInfo(LED_EVENT e);
    extern void ledTurnOn(unsigned char ackRequired);
    extern void ledTurnOff(unsigned char ackRequired);

    extern void ledSendSimpleEvent(LED_EVENT Evt, int data, unsigned char ack);
#ifdef __cplusplus
}
#endif

#endif /* __LED_H__ */