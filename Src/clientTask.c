/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


/* client Task for testing Led Task
 * I hope you to get a good idea from this project.
 * 
 /

/****************************************************************************/
/** **/
/** MODULES USED **/
/** **/
/****************************************************************************/
#include <stdio.h>
#include "cmsis_os.h"
#include "client.h"
#include "led.h"


/****************************************************************************/
/** **/
/** DEFINITIONS AND MACROS **/
/** **/
/****************************************************************************/
#define TASK_NOTIFY_ALL_BITS 0xFFFFFFFF


#define CLIENT_ACK_EXPIRED        10
#define CLIENT_RESP_EXPIRED       20
#define CLIENT_LED_SVC              2000

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
/* Task Handle*/
osThreadId ClientaskHandle;
/* Queue */
QueueHandle_t ClientReceiveMsgQueue = NULL;
/* Mutex */
osMutexId ClientMutexHandle;

/* Timer */
// T1 Timer : timer that waiting ack from LED task
osTimerId ClientWaitingAckTimerHandle;
// T2 Timer : timer that waiting resp from LED task
osTimerId ClientWaitingRespTimerHandle;
// Test Timer
osTimerId ClientLedSvcTimerHandle;

/****************************************************************************/
/** **/
/** LOCAL VARIABLES **/
/** **/
/****************************************************************************/
static TickType_t xTicksToWait;
static int ledSvc = 0;

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

void StartClientTask(void const * argument);

/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/
void clientSendSimpleEvent(CLIENT_EVENT Evt, int data, unsigned char ack);

/* Timers */
// ack timer
static void clientWaitingAckTimerHandleCallback(void const *arg) {
    clientSendSimpleEvent(CLIENT_EVT_ACK_TIMER_EXPIRED_NOTI, 0, 0);
}
static void clientCreateAckTimer(void) {
    osTimerDef(ClientAckTimer, clientWaitingAckTimerHandleCallback);
    ClientWaitingAckTimerHandle = osTimerCreate(osTimer(ClientAckTimer), osTimerPeriodic,  NULL);
}
static void clientDeleteAckTimer(void) {
    osTimerDelete(ClientWaitingAckTimerHandle);
}
static void clientStartAckTimer(void) {
    osTimerStart(ClientWaitingAckTimerHandle, CLIENT_ACK_EXPIRED);
}
static void clientStopAckTimer(void) {
    osTimerStop(ClientWaitingAckTimerHandle);
}
static BaseType_t clientIsAckTimerActive(void) {
    return xTimerIsTimerActive(ClientWaitingAckTimerHandle);
}
// response
static void clientWaitingRespTimerHandleCallback(void const *arg) {
    clientSendSimpleEvent(CLIENT_EVT_RESP_TIMER_EXPIRED_NOTI, 0, 0);
}
static void clientCreateRespTimer(void) {
    osTimerDef(ClientRespTimer, clientWaitingRespTimerHandleCallback);
    ClientWaitingAckTimerHandle = osTimerCreate(osTimer(ClientRespTimer), osTimerPeriodic,  NULL);
}
static void clientDeleteRespTimer(void) {
    osTimerDelete(ClientWaitingRespTimerHandle);
}
static void clientStartRespTimer(void) {
    osTimerStart(ClientWaitingRespTimerHandle, CLIENT_RESP_EXPIRED);
}
static void clientStopRespTimer(void) {
    osTimerStop(ClientWaitingRespTimerHandle);
}
static BaseType_t clientIsRespTimerActive(void) {
    return xTimerIsTimerActive(ClientWaitingRespTimerHandle);
}
// test 
static void clientLedServiceHandleCallback(void const *arg) {
    clientSendSimpleEvent(CLIENT_EVT_REQ_LED_SVC, 0, 0);
}
static void clientCreateLedSvcTimer(void) {
    osTimerDef(ClientLedSvcTimer, clientLedServiceHandleCallback);
    ClientLedSvcTimerHandle = osTimerCreate(osTimer(ClientLedSvcTimer), osTimerPeriodic,  NULL);
}
static void clientDeleteLedSvcTimer(void) {
    osTimerDelete(ClientLedSvcTimerHandle);
}
static void clientStartLedSvcTimer(void) {
    osTimerStart(ClientLedSvcTimerHandle, CLIENT_LED_SVC);
}
static void clientStopLedSvcTimer(void) {
    osTimerStop(ClientLedSvcTimerHandle);
}
static BaseType_t clientIsLedSvcTimerActive(void) {
    return xTimerIsTimerActive(ClientLedSvcTimerHandle);
}

/* service functions */
BaseType_t clientSendEvent(CLIENT_MSG message) {
    BaseType_t ret = 0;
    ret = xQueueSendToBack(ClientReceiveMsgQueue, &message, xTicksToWait);
    osSignalSet(ClientaskHandle, CLIENT_CMD_Q_SIG);
    return ret;
}

void clientSendSimpleEvent(CLIENT_EVENT Evt, int data, unsigned char ack) {
    CLIENT_MSG msg;
    msg.cmd = Evt;
    msg.ack = ack;
    msg.nData = data;
    msg.pData = NULL;
    msg.len = 0;
    clientSendEvent(msg);
}


static void clientReqLedOnOff(int onoff) {
    /* send req.to led task */
    if (onoff)
        ledSendSimpleEvent(LED_EVT_ON_REQ, 0, 1);
    else
        ledSendSimpleEvent(LED_EVT_OFF_REQ, 0, 1);
    /* start timer for ack received or not */
    clientStartAckTimer();
}


static void clientCmdHandler(CLIENT_MSG *msg) {
    switch (msg->cmd ) {
        case CLIENT_EVT_REQ_LED_SVC:
            clientReqLedOnOff(!ledSvc);
            ledSvc = !ledSvc;
            break;

        case CLIENT_EVT_ACK_RECIEVED:
            /* stop ack timer */
            clientStopAckTimer();
            
            printf("ack received\n");
            
            /* start resp timer */
            clientStartRespTimer();
            break;

        case CLIENT_EVT_RESP_RECEIVED:
            /* stop resp timer*/
            clientStopRespTimer();

            printf("resp received\n");
            printf("req was succefully completed\n");
            break;

        case CLIENT_EVT_ACK_TIMER_EXPIRED_NOTI:
            /* never happend! */
            configASSERT(0);
            break;

        case CLIENT_EVT_RESP_TIMER_EXPIRED_NOTI:
            /* never happend! */
            configASSERT(0);
            break;

        default:
            break;
    }
}

void clientTaskInit(void) {

    /* create task queue */
    ClientReceiveMsgQueue = xQueueCreate(CLIENT_MAX_Q_SIZE, sizeof(CLIENT_MSG));
    configASSERT(ClientReceiveMsgQueue);
    
    /* mutex */
    osMutexDef(ClientMutex);
    ClientMutexHandle = osMutexCreate(osMutex(ClientMutex));
    configASSERT(ClientMutexHandle);
    
    /* create timer */
    clientCreateAckTimer();
    clientCreateRespTimer();
    clientCreateLedSvcTimer();
}


void clientTask(void const * argument)
{
    static BaseType_t xResult = 0;
    CLIENT_MSG message;
    osEvent Event;
    
    clientTaskInit();
    clientStartLedSvcTimer();

    for(;;)
    {
        Event = osSignalWait(TASK_NOTIFY_ALL_BITS, osWaitForever);

        if(Event.value.signals & CLIENT_CMD_Q_SIG) {
            xResult = xQueueReceive(ClientReceiveMsgQueue, &(message),xTicksToWait);

            if (xResult == pdTRUE) {
                clientCmdHandler(&message);
            } else {
                printf("Error occured when getting data from rx queue\n");
            }
        }

        /* yield to other task */
        osThreadYield();
    }
}
