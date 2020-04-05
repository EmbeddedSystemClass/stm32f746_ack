/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


/* Simple LED Task for ack based implementation */

/****************************************************************************/
/** **/
/** MODULES USED **/
/** **/
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"
#include "led.h"


/****************************************************************************/
/** **/
/** DEFINITIONS AND MACROS **/
/** **/
/****************************************************************************/

/****************************************************************************/
/** **/
/** TYPEDEFS AND STRUCTURES **/
/** **/
/****************************************************************************/

/****************************************************************************/
/** **/
/** PROTOTYPES OF LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/

/****************************************************************************/
/** **/
/** EXPORTED VARIABLES **/
/** **/
/****************************************************************************/

/****************************************************************************/
/** **/
/** GLOBAL VARIABLES **/
/** **/
/****************************************************************************/


/****************************************************************************/
/** **/
/** LOCAL VARIABLES **/
/** **/
/****************************************************************************/
static LED Led;

/****************************************************************************/
/** **/
/** EXPORTED FUNCTIONS **/
/** **/
/****************************************************************************/

/****************************************************************************/
/** **/
/** GLOBAL FUNCTIONS **/
/** **/
/****************************************************************************/


/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/


int ledGetAckIndex(LED_EVENT e) {
    configASSERT (e > LED_EVT_NONE || e <= LED_EVT_OFF_RSP)
    return (e / 3) + 1;
}


void ledSetAckInfo(osThreadId taskId, LED_EVENT e) {
    configASSERT(taskId);
    configASSERT (e > LED_EVT_NONE || e <= LED_EVT_OFF_RSP)
    
    Led.ack[ledGetAckIndex(e)].event = e;
    Led.ack[ledGetAckIndex(e)].ack = 1;
    Led.ack[ledGetAckIndex(e)].taskId = taskId;
}


void ledClearAckInfo(LED_EVENT e) {
    configASSERT (e > LED_EVT_NONE || e <= LED_EVT_OFF_RSP)
    
    Led.ack[ledGetAckIndex(e)].event = 0;
    Led.ack[ledGetAckIndex(e)].ack = 0;
    Led.ack[ledGetAckIndex(e)].taskId = NULL;
}

osThreadId ledGetClientTaskId(LED_EVENT e) {
    configASSERT (e > LED_EVT_NONE || e <= LED_EVT_OFF_RSP)
    
    return Led.ack[ledGetAckIndex(e)].taskId;
}


void ledTurnOn(unsigned char ackRequired) {
    /* led on*/

    /* response to origin */
    if (ackRequired) {
        ledSendSimpleEvent(LED_EVT_ON_RSP, 0, 0);
    }
}

void ledTurnOff(unsigned char ackRequired) {
    /* led off*/

    /* response to origin */
    if (ackRequired) {
        ledSendSimpleEvent(LED_EVT_OFF_RSP, 0, 0);
    }
}

static void ledHwInit(void) {
}

void ledInit(void) {
    ledHwInit();

    memset((void *)&Led, 0, sizeof(Led));
    Led.initialized = 1;
}