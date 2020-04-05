/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


/* Simple LED Task for ack based implementation 
 * I hope you to get a good idea from this project.
 * 
 * In order to refactoring command handler( like command dispatcher )
 * you can get a idea from another project
*/

/****************************************************************************/
/** **/
/** MODULES USED **/
/** **/
/****************************************************************************/
#include <stdio.h>
#include "cmsis_os.h"
#include "led.h"


/****************************************************************************/
/** **/
/** DEFINITIONS AND MACROS **/
/** **/
/****************************************************************************/
#define TASK_NOTIFY_ALL_BITS 0xFFFFFFFF

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
osThreadId lEDTaskHandle;
/* Queue */
QueueHandle_t lEDReceiveMsgQueue = NULL;
/* Mutex */
osMutexId LEDMutexHandle;
/* Timer */
osTimerId LEDTimerHandle;

/****************************************************************************/
/** **/
/** LOCAL VARIABLES **/
/** **/
/****************************************************************************/
static TickType_t xTicksToWait;

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

void StartLedTask(void const * argument);

/****************************************************************************/
/** **/
/** LOCAL FUNCTIONS **/
/** **/
/****************************************************************************/
void ledSendSimpleEvent(LED_EVENT Evt, int data, unsigned char ack);


BaseType_t ledSendEvent(LED_MSG message) {
    BaseType_t ret = 0;
    ret = xQueueSendToBack(lEDReceiveMsgQueue, &message, xTicksToWait);
    osSignalSet(lEDTaskHandle, LED_CMD_Q_SIG);
    return ret;
}

void ledSendSimpleEvent(LED_EVENT Evt, int data, unsigned char ack) {
    LED_MSG msg;
    msg.cmd = Evt;
    msg.ack = ack;
    msg.nData = data;
    msg.pData = NULL;
    msg.len = 0;
    ledSendEvent(msg);
}

/* 
 * Ack to client task.
 * If you know who is client and its queue you can send message.
 * 
*/
static void ledNotifyAckToClient(LED_EVENT e) {
    eTaskState taskState;
    osThreadId clientTask;

    if (osMutexWait(LEDMutexHandle, portMAX_DELAY) != osOK) {
        configASSERT(0);
    }

    clientTask = ledGetClientTaskId(e);
    taskState = eTaskGetState(clientTask);
    if (taskState >= eRunning && taskState < eDeleted ) {
        xTaskNotify(clientTask, LED_ACK_SIG, eSetBits);
    }

    if(osMutexRelease(LEDMutexHandle) != osOK) {
        configASSERT(0);
    }
}

static void ledNotifyToClient(LED_EVENT e) {
    eTaskState taskState;
    osThreadId clientTask;

    if (osMutexWait(LEDMutexHandle, portMAX_DELAY) != osOK) {
        configASSERT(0);
    }

    clientTask = ledGetClientTaskId(e);
    taskState = eTaskGetState(clientTask);
    if (taskState >= eRunning && taskState < eDeleted ) {
        xTaskNotify(clientTask, LED_NOTIFY_SIG, eSetBits);
    }

    if(osMutexRelease(LEDMutexHandle) != osOK) {
        configASSERT(0);
    }
}


/* 
 * step1 : Implement command handler
 * step2 : make command dispatcher
*/
static void ledCmdHandler(LED_MSG *msg) {
    switch (msg->cmd ) {
        case LED_EVT_ON_REQ:
            /* acknowledged */
            if (msg->ack) {
                ledSetAckInfo(msg->taskId, msg->cmd);
                ledSendSimpleEvent(LED_EVT_ON_ACK, 0, 0);
            }
            /* turn on led*/
            ledTurnOn(msg->ack);
            break;

        case LED_EVT_ON_ACK:
            break;

        case LED_EVT_ON_RSP:
            /* response to clinet */
            ledNotifyToClient(LED_EVT_ON_REQ);

            /* clear ack flag */
            ledClearAckInfo(msg->cmd);
            break;

        case LED_EVT_OFF_REQ:
            /* acknowledged */
            if (msg->ack) {
                ledSetAckInfo(msg->taskId, msg->cmd);
                ledSendSimpleEvent(LED_EVT_OFF_ACK, 0, 0);
            }
            /* turn on led*/
            ledTurnOff(msg->ack);
            break;
        
        case LED_EVT_OFF_ACK:
            break;

        case LED_EVT_OFF_RSP:
            /* response to clinet */
            ledNotifyToClient(LED_EVT_OFF_REQ);

            /* clear ack flag */
            ledClearAckInfo(msg->cmd);
            break;

        default:
            break;
    }
}

void ledTaskInit(void) {
    ledInit();

    /* create task queue */
    lEDReceiveMsgQueue = xQueueCreate(LED_MAX_Q_SIZE, sizeof(LED_MSG));
    configASSERT(lEDReceiveMsgQueue);
    
    /* mutex */
    osMutexDef(LedMutex);
    LEDMutexHandle = osMutexCreate(osMutex(LedMutex));
    configASSERT(LEDMutexHandle);
    
    /* create and start timer */

}


void StartLedTask(void const * argument)
{
    static BaseType_t xResult = 0;
    LED_MSG message;
    osEvent Event;
    
    for(;;)
    {
        Event = osSignalWait(TASK_NOTIFY_ALL_BITS, osWaitForever);

        if(Event.value.signals & LED_CMD_Q_SIG) {
            xResult = xQueueReceive(lEDReceiveMsgQueue, &(message),xTicksToWait);

            if (xResult == pdTRUE) {
                ledCmdHandler(&message);
            } else {
                printf("Error occured when getting data from rx queue\n");
            }
        }

        /* yield to other task */
        osThreadYield();
    }
}
