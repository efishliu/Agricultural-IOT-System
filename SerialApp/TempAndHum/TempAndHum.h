#ifndef TEMPANDHUM_H
#define TEMPANDHUM_H

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
#define TEMPANDHUM_READ_EVT          0x0004

// OTA Flow Control Delays
#define SERIALAPP_ACK_DELAY          1
#define SERIALAPP_NAK_DELAY          16

// OTA Flow Control Status
#define OTA_SUCCESS                  ZSuccess
#define OTA_DUP_MSG                 (ZSuccess+1)
#define OTA_SER_BUSY                (ZSuccess+2)

/*********************************************************************
 * MACROS
 */

#define SHT10_DATA                   P0_7
#define SHT10_SCK                    P0_6
  
#define HIGH                         1
#define LOW                          0
  
#define TEMPERATURE                  0x00
#define HUMIDITY                     0x01

#define DATA_OUTPUT                  P0DIR |=  0x80
#define DATA_HIGH                    SHT10_DATA = HIGH
#define DATA_LOW                     SHT10_DATA = LOW
#define DATA_INPUT                   P0DIR &= ~0x80
#define SCK_OUTPUT                   P0DIR |=  0x40
#define SCK_LOW                      SHT10_SCK = LOW
#define SCK_HIGH                     SHT10_SCK = HIGH
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
