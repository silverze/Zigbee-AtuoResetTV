/**************************************************************************************************
  Filename:       AutoResetTV.c
  Revised:        $Date: 2009-03-18 15:56:27 -0700 (Wed, 18 Mar 2009) $
  Revision:       $Revision: 19453 $

  Description:    Sample Application (no Profile).


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends it's messages either as broadcast or
  broadcast filtered group messages.  The other (more normal)
  message addressing is unicast.  Most of the other sample
  applications are written to support the unicast message model.

  Key control:
    SW1:  Sends a flash command to all devices in Group 1.
    SW2:  Adds/Removes (toggles) this device in and out
          of Group 1.  This will enable and disable the
          reception of the flash command.
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"

#include "AutoResetTV.h"
#include "AutoResetTVHw.h"

#include "OnBoard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

/*MT ²ã*/
#include "MT_UART.h"
#include <string.h>
#include "AutoResetTV_App.h"
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// This list should be filled with Application specific Cluster IDs.
const cId_t AutoResetTV_ClusterList[AutoResetTV_MAX_CLUSTERS] =
{
  AutoResetTV_PERIODIC_CLUSTERID,
  AutoResetTV_RESET_OK_CLUSTERID
};

const SimpleDescriptionFormat_t AutoResetTV_SimpleDesc =
{
  AutoResetTV_ENDPOINT,              //  int Endpoint;
  AutoResetTV_PROFID,                //  uint16 AppProfId[2];
  AutoResetTV_DEVICEID,              //  uint16 AppDeviceId[2];
  AutoResetTV_DEVICE_VERSION,        //  int   AppDevVer:4;
  AutoResetTV_FLAGS,                 //  int   AppFlags:4;
  AutoResetTV_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)AutoResetTV_ClusterList,  //  uint8 *pAppInClusterList;
  AutoResetTV_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)AutoResetTV_ClusterList   //  uint8 *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in AutoResetTV_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t AutoResetTV_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern uint8 recStep;//´®¿Ú½ÓÊÕTV·µ»ØÖ¸Áî²½Öè
extern uint8 sendCmdCnt;//¼ÆÊýÖ¸Áî·¢ËÍ´ÎÊý
/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8 AutoResetTV_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // AutoResetTV_Init() is called.
devStates_t AutoResetTV_NwkState;

uint8 AutoResetTV_TransID;  // This is the unique message ID (counter)

afAddrType_t AutoResetTV_Periodic_DstAddr;
afAddrType_t AutoResetTV_Reset_DstAddr; //·¢ËÍ¸´Î»³É¹¦ÐÅÏ¢µÄÄ¿±êµØÖ·

aps_Group_t AutoResetTV_Group;

uint8 AutoResetTVPeriodicCounter = 0;
uint8 AutoResetTVFlashCounter = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void AutoResetTV_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void AutoResetTV_SendRestOKMessage( void );

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      AutoResetTV_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void AutoResetTV_Init( uint8 task_id )
{
  AutoResetTV_TaskID = task_id;
  AutoResetTV_NwkState = DEV_INIT;
  AutoResetTV_TransID = 0;

  MT_UartInit ();//³õÊ¼»¯´®¿Ú
  MT_UartRegisterTaskID(AutoResetTV_TaskID);//×¢²áÈÎÎñÊÂ¼þID
  
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

 #if defined ( BUILD_ALL_DEVICES )
  // The "Demo" target is setup to have BUILD_ALL_DEVICES and HOLD_AUTO_START
  // We are looking at a jumper (defined in AutoResetTVHw.c) to be jumpered
  // together - if they are - we will start up a coordinator. Otherwise,
  // the device will start as a router.
  if ( readCoordinatorJumper() )
    zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;
  else
    zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
#endif // BUILD_ALL_DEVICES

#if defined ( HOLD_AUTO_START )
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);
#endif

  // Setup for the periodic message's destination address
  // Broadcast to everyone
  AutoResetTV_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  AutoResetTV_Periodic_DstAddr.endPoint = AutoResetTV_ENDPOINT;
  AutoResetTV_Periodic_DstAddr.addr.shortAddr = 0xFFFF;

  // Setup for the Reset message's destination address 
  AutoResetTV_Reset_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  AutoResetTV_Reset_DstAddr.endPoint = AutoResetTV_ENDPOINT;
  AutoResetTV_Reset_DstAddr.addr.shortAddr = 0xFFFF;

  // Fill out the endpoint description.
  AutoResetTV_epDesc.endPoint = AutoResetTV_ENDPOINT;
  AutoResetTV_epDesc.task_id = &AutoResetTV_TaskID;
  AutoResetTV_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&AutoResetTV_SimpleDesc;
  AutoResetTV_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &AutoResetTV_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( AutoResetTV_TaskID );
  
#ifdef EndDeviceEB
  AutoResetTVApp_RegisterTaskID( AutoResetTV_TaskID );  
#endif 

  // By default, all devices start out in Group 1
  AutoResetTV_Group.ID = 0x0001;
  osal_memcpy( AutoResetTV_Group.name, "Group 1", 7  );
  aps_AddGroup( AutoResetTV_ENDPOINT, &AutoResetTV_Group );

#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "AutoResetTV", HAL_LCD_LINE_1 );
#endif
}

/*********************************************************************
 * @fn      AutoResetTV_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 AutoResetTV_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter
  //static uint8 cnt = 0;//¶¨Ê±ÖÜÆÚ¼ÆÊý

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( AutoResetTV_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        // Received when a key is pressed
//        case KEY_CHANGE:
//        AutoResetTV_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
//        break;

        // Received when a messages is received (OTA) for this endpoint
        case AF_INCOMING_MSG_CMD:
          AutoResetTV_MessageMSGCB( MSGpkt );
          break;

        // Received whenever the device changes state in the network
        case ZDO_STATE_CHANGE:
          AutoResetTV_NwkState = (devStates_t)(MSGpkt->hdr.status);
          
          //ÖÕ¶ËÉè±¸Á¬½Ó½øÈëZBÍøÂçºó£¬Æô¶¯¶¨Ê±·¢ËÍTV¸´Î»Ö¸Áî
          if ( AutoResetTV_NwkState == DEV_END_DEVICE ) 
          {
            // Start sending the periodic message in a regular interval.
            osal_start_timerEx( AutoResetTV_TaskID,
                                AutoResetTV_SEND_PERIODIC_MSG_EVT,
                                AutoResetTV_SEND_PERIODIC_MSG_TIMEOUT );
          }
          else
          {
            // Device is no longer in the network
          }
          break;
         
        //½ÓÊÕµ½´ËÏµÍ³ÏûÏ¢£¬±íÃ÷µçÊÓ»ú¸´Î»³É¹¦£¡  
        case TV_RESET_FLAG:
          //printf("µçÊÓ»ú¸´Î»³É¹¦£¡\n");
              AutoResetTV_SendRestOKMessage();
          break;
           
        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next - if one is available
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( AutoResetTV_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in AutoResetTV_Init()).
  if ( events & AutoResetTV_SEND_PERIODIC_MSG_EVT )
  {
    /*Ã¿ÌõÖ¸Áî×î¶àÖØ¸´·¢ËÍÈý´Î*/
    if(recStep == 1 && sendCmdCnt < 3) 
    {
      AutoResetTVApp_UARTSendEnterFacCmd();
    }
    else if(recStep == 2 && sendCmdCnt < 3)
    {
      AutoResetTVApp_UARTSendEnterFacCmd(); //ÏÈÔÙ·¢Ò»´Î½ø¹¤³§£¬ÈÃÖ¸Áî·¢ËÍ²½Öè¼Ó1
    }
    else if(recStep == 3 && sendCmdCnt < 3)
    {
       AutoResetTVApp_UARTSendResetCmd();
    }
    
    if(recStep <= 3 && sendCmdCnt < 3)
     osal_start_timerEx( AutoResetTV_TaskID, 
                        AutoResetTV_SEND_PERIODIC_MSG_EVT,
                        AutoResetTV_SEND_PERIODIC_MSG_TIMEOUT );

     
    // return unprocessed events
    return (events ^ AutoResetTV_SEND_PERIODIC_MSG_EVT);
  }

  // Discard unknown events
  return 0;
}


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      AutoResetTV_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void AutoResetTV_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  switch ( pkt->clusterId )
  {
    case AutoResetTV_PERIODIC_CLUSTERID:  
#ifdef CoordinatorEB
      printf("endsrc:0x%x\t rec:%d\n", pkt->srcAddr.addr.shortAddr, pkt->cmd.Data[0]);
#endif
      break;

    case AutoResetTV_RESET_OK_CLUSTERID:
#ifdef CoordinatorEB
      printf("endsrc:0x%x\t Msg:%s\n", pkt->srcAddr.addr.shortAddr, (pkt->cmd.Data));
#endif     
      break;
  }
}

/*********************************************************************
 * @fn      AutoResetTV_SendRestOKMessage
 *
 * @brief   ·¢ËÍµçÊÓ»ú¸´Î»³É¹¦ÐÅÏ¢¸øÐ­µ÷Æ÷
 *
 * @return  none
 */
void AutoResetTV_SendRestOKMessage( void )
{
  //bool buff = true;
  uint8* msg = "TV shop finished!";

  if ( AF_DataRequest( &AutoResetTV_Reset_DstAddr, &AutoResetTV_epDesc,
                       AutoResetTV_RESET_OK_CLUSTERID,
                       strlen((char*)msg),
                       msg,
                       &AutoResetTV_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}
/*********************************************************************
*********************************************************************/
