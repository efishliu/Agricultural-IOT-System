#ifndef MOTOR_H
#define MOTOR_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"

/*********************************************************************
 * CONSTANTS
 */

// These constants are only for example and should be changed to the
// device's needs
#define SERIALAPP_ENDPOINT           11

#define SERIALAPP_PROFID             0x0F05
#define SERIALAPP_DEVICEID           0x0001
#define SERIALAPP_DEVICE_VERSION     0
#define SERIALAPP_FLAGS              0

#define SERIALAPP_MAX_CLUSTERS       2
#define SERIALAPP_CLUSTERID1         1
#define SERIALAPP_CLUSTERID2         2

#define SERIALAPP_SEND_EVT           0x0001
#define PERIOD_EVT                   0x0002

// OTA Flow Control Delays
#define SERIALAPP_ACK_DELAY          1
#define SERIALAPP_NAK_DELAY          16

// OTA Flow Control Status
#define OTA_SUCCESS                  ZSuccess
#define OTA_DUP_MSG                 (ZSuccess+1)
#define OTA_SER_BUSY                (ZSuccess+2)

//LED控制接口 
#define LED_1                       P0_4
#define LED_2                       P0_5
#define LED_3                       P0_6
#define LED_4                       P0_7
  
#define ON                          0
#define OFF                         1

//电机控制接口
#define LG9110_MR                   P0_0
#define LG9110_MF                   P0_1
#define HIGH                        1
#define LOW                         0

#define LED_1_ON                    1
#define LED_1_OFF                   2
#define LED_2_ON                    3
#define LED_2_OFF                   4
#define LED_3_ON                    5
#define LED_3_OFF                   6
#define LED_4_ON                    7
#define LED_4_OFF                   8
#define FRONT                       9
#define BACK                        10
#define HALT                        11
#define LED_ALL_ON                  12
#define LED_ALL_OFF                 13

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern byte SerialApp_TaskID;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the Serial Transfer Application
 */
extern void SerialApp_Init( byte task_id );

/*
 * Task Event Processor for the Serial Transfer Application
 */
extern UINT16 SerialApp_ProcessEvent( byte task_id, UINT16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 
