#include "AutoResetTV_App.h"


/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8 recStep = 1;//串口接收TV返回指令步骤
extern uint8 sendCmdCnt = 0;//计数指令发送次数


/*registeredRestTVTaskID 用来注册串口TV复位成功标志监控OSAL事件*/
static uint8 registeredRestTVTaskID;
uint8 index = 0;
char* mathStr = "hop finished and standby";  //(S)shop finished and standby
uint8 param[50] = {0};//存储TV指令的返回参数 

//函数声明区域
static void AutoResetTVApp_SendFlagMsg(void);
static void AutoResetTVApp_SendGetTVSNMsg(void);
static void AutoResetTVApp_MatchShopedStr(uint8 port);//匹配(S)shop finished and standby字符串
static bool AutoResetTVApp_ReceiveReturnData( uint8 port, bool bParam );
//static bool AutoResetTVApp_ReceiveReturnParam( uint8 ch);

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
  
  switch(recStep)
  {
  case 1:  //判断进入工厂模式指令返回  
    sendCmdCnt++;
    if( AutoResetTVApp_ReceiveReturnData(port, false) ) 
      recStep = 2;
    break;
    
  case 2 :
    sendCmdCnt++;
    if( AutoResetTVApp_ReceiveReturnData(port, true) )
    {
      recStep = 3;
      AutoResetTVApp_SendGetTVSNMsg();
    }
    break;
    
  case 3 : 
    sendCmdCnt++;
    AutoResetTVApp_MatchShopedStr(port);
    break;
    
  default :
    break;
  }
}

/***************************************************************************************************
 * @fn      AutoResetTVApp_ReceiveReturnData
 *
 * @brief   校验TV返回指令是否正确
 *
 * @param   port     - UART port
 *          event    - Event that causes the callback
 *
 *
 * @return  None
 ***************************************************************************************************/
static bool AutoResetTVApp_ReceiveReturnData( uint8 port, bool isParam )
{
  uint8 ch;
  bool ret = false;
  static uint8 i = 0;
  uint8 check_cmd[] = {0xAB, 0x05, 0x0A, 0xDF, 0x4E};
  
  while (Hal_UART_RxBufLen(port))
  {
    HalUARTRead (port, &ch, 1);
    
    if(isParam)//处理带参数返回的TV指令
    {
      if( i < 5 && ch == check_cmd[i])
      {
        i++;
      }
      else if (i >= 5)//第一段返回已正确
      {
        if( ( i == 5 && ch == 0xAB ) || i == 6 ) //第二段的开头、与待接收的数据长度
        {
          param[i - 5] = ch; 
          i++;
        }
        else if( i > 6)
        {
          param[i - 5] = ch;
          i++;
          if( (i - 5) == param[1]) //i-5,为减掉第一段的长度计数；第二段接收OK
          {
            ret = true;
            break;
          }
        }
        else //第二段返回错误
        {
          i = 0;
          ret = false;
          break;
        }        
      }
      else      //第一段返回错误
      {
        i = 0;
        ret = false;
        break;
      }    
    }
    else //不带参数返回指令
    {
      if(ch == check_cmd[i])
      {
        i++;
        if( i == 5) //此时TV已正确返回指令
        {
          i = 0;
          sendCmdCnt = 0;
          ret = true;
          break;
        }
      }
      else
      {
        i = 0;
        ret = false;
        break;
      }    
    }
  }
  
  return ret;
}


//static bool AutoResetTVApp_ReceiveReturnParam( uint8 ch )
//{
//  
//}


/***************************************************************************************************
 * @fn      AutoResetTVApp_MatchShopedStr
 *
 * @brief   匹配(S)shop finished and standby字符串
 *
 * @return  None
 ***************************************************************************************************/
static void AutoResetTVApp_MatchShopedStr( uint8 port )
{
  uint8 ch;
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
      AutoResetTVApp_SendFlagMsg();//发送复位成功消息给应用层
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
static void AutoResetTVApp_SendFlagMsg(void)
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

static void AutoResetTVApp_SendGetTVSNMsg(void)
{
  getTVSN_t *msgPtr;
    // Send the Msg to the task
  msgPtr = (getTVSN_t *)osal_msg_allocate( sizeof(getTVSN_t) );
  if ( msgPtr )
  {
    msgPtr->hdr.event = GET_TV_SN;
    msgPtr->tvSN_data = param; //将zigbee终端获取到TV内部的SN包成OSAL消息数据
    
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
 * @fn      AutoResetTVApp_UARTSendGetSNCmd
 *
 * @brief   发送获取电视机SN指令到TV.
 *
 * @return  none
 */
void AutoResetTVApp_UARTSendGetSNCmd( void )
{
  uint8 cmd[] = {0xAA, 0x06, 0xBE, 0x03, 0xB9, 0xDC};
  int8 i;
  for(i = 0; i < 6; i++)
    printf("%c", cmd[i]);
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
