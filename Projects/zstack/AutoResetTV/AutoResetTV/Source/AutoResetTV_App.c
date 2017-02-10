#include "AutoResetTV_App.h"


/*registeredRestTVTaskID 用来注册串口TV复位成功标志监控OSAL事件*/
static uint8 registeredRestTVTaskID;

static void AutoResetTVApp_SendFlagMsg(void);

/***************************************************************************************************
 * @fn      RegisterResetTVFlag
 *
 * @brief   注册TV复位成功任务ID    
 *
 * @param   task_id 
 *
 * @return None 
 *
 ***************************************************************************************************/
void AutoResetTVApp_RegisterTaskID( uint8 task_id )
{
    registeredRestTVTaskID = task_id;
}


/***************************************************************************************************
 * @fn      AutoResetTVApp_UartProcessData
 *
 * @brief   处理电视自动复位串口数据
 *
 * @param   port     - UART port
 *          event    - Event that causes the callback
 *
 *
 * @return  None
 ***************************************************************************************************/
void AutoResetTVApp_UartProcessData( uint8 port, uint8 event )
{
  uint8  ch;
  static uint8 index = 0;
  char* mathStr = "hop finished and standby";  //(S)shop finished and standby

  while (Hal_UART_RxBufLen(port))
  {
    HalUARTRead (port, &ch, 1);
    
    if(ch == mathStr[index])
      index++;
    else
      index = 0;
    
    if(index == strlen(mathStr))
    {
      index = 0;
      AutoResetTVApp_SendFlagMsg();//发送成功消息给应用层
      HalLedSet (HAL_LED_3, HAL_LED_MODE_TOGGLE);  
    }   
  }
}

/***************************************************************************************************
 * @fn      AutoResetTVApp_SendFlagMsg
 *
 * @brief   发送电视机复位成功的系统消息
 *
 * @return  None
 ***************************************************************************************************/
void AutoResetTVApp_SendFlagMsg(void)
{
  tvResetFlag_t *msgPtr; //TV复位标志消息指针
  
  // Send the Msg to the task
  msgPtr = (tvResetFlag_t *)osal_msg_allocate( sizeof(tvResetFlag_t) );
  if ( msgPtr )
  {
    msgPtr->hdr.event = TV_RESET_FLAG;
    msgPtr->flag      = true;
    
    osal_msg_send( registeredRestTVTaskID, (uint8 *)msgPtr ); //发送系统消息至应用
  }
}

/*********************************************************************
 * @fn      AutoResetTVApp_UARTSendResetCmd
 *
 * @brief   发送复位指令到TV.
 *
 * @return  none
 */
void AutoResetTVApp_UARTSendResetCmd( void )
{
  uint8 cmd[] = {0xAA, 0x06, 0x19, 0x01, 0x1D, 0x77};
  int8 i;
  
  for(i = 0; i < 6; i++)
    printf("%c",cmd[i]);
}

/*********************************************************************
 * @fn      AutoResetTVApp_UARTSendEnterFacCmd
 *
 * @brief   发送进入工厂模式指令到TV.
 *
 * @return  none
 */
void AutoResetTVApp_UARTSendEnterFacCmd( void )
{
  uint8 cmd[] = {0xAA, 0x06, 0x10, 0x01, 0xA7, 0xEF};
  int8 i;
  
  for(i = 0; i < 6; i++)
    printf("%c", cmd[i]);
}
