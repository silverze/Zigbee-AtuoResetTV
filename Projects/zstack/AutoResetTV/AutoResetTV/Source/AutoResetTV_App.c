#include "AutoResetTV_App.h"

/*registeredRestTVTaskID ����ע�ᴮ��TV��λ�ɹ���־���OSAL�¼�*/
static uint8 registeredRestTVTaskID;
uint8 index = 0;
extern uint8 recStep = 1;//���ڽ���TV����ָ���
extern uint8 sendCmdCnt = 0;//����ָ��ʹ���
char* mathStr = "hop finished and standby";  //(S)shop finished and standby

//������������
static void AutoResetTVApp_SendFlagMsg(void);
static void AutoResetTVApp_MatchShopedStr(uint8 port);//ƥ��(S)shop finished and standby�ַ���
static bool AutoResetTVApp_ReturnCMDConfim( uint8 port );

/***************************************************************************************************
 * @fn      RegisterResetTVFlag
 *
 * @brief   ע��TV��λ�ɹ�����ID    
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
 * @brief   ��������Զ���λ��������
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
  case 1:  //�жϽ��빤��ģʽָ���  
    sendCmdCnt++;
    if( AutoResetTVApp_ReturnCMDConfim(port) ) 
      recStep = 2;
    break;
    
  case 2 :
    recStep = 3;
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
 * @fn      AutoResetTVApp_ReturnCMDConfim
 *
 * @brief   У��TV����ָ���Ƿ���ȷ�������������أ�
 *
 * @param   port     - UART port
 *          event    - Event that causes the callback
 *
 *
 * @return  None
 ***************************************************************************************************/
static bool AutoResetTVApp_ReturnCMDConfim( uint8 port )
{
  uint8 ch;
  bool ret = false;
  static uint8 i = 0;
  uint8 check_cmd[] = {0xAB, 0x05, 0x0A, 0xDF, 0x4E};
  
  while (Hal_UART_RxBufLen(port))
  {
    HalUARTRead (port, &ch, 1);
    if(ch == check_cmd[i])
    {
      i++;
      if(i == 5)
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
  
  return ret;  
}

/***************************************************************************************************
 * @fn      AutoResetTVApp_MatchShopedStr
 *
 * @brief   ƥ��(S)shop finished and standby�ַ���
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
      AutoResetTVApp_SendFlagMsg();//���ͳɹ���Ϣ��Ӧ�ò�
      HalLedSet (HAL_LED_3, HAL_LED_MODE_TOGGLE);  
    }   
  }
}


/***************************************************************************************************
 * @fn      AutoResetTVApp_SendFlagMsg
 *
 * @brief   ���͵��ӻ���λ�ɹ���ϵͳ��Ϣ
 *
 * @return  None
 ***************************************************************************************************/
static void AutoResetTVApp_SendFlagMsg(void)
{
  tvResetFlag_t *msgPtr; //TV��λ��־��Ϣָ��
  
  // Send the Msg to the task
  msgPtr = (tvResetFlag_t *)osal_msg_allocate( sizeof(tvResetFlag_t) );
  if ( msgPtr )
  {
    msgPtr->hdr.event = TV_RESET_FLAG;
    msgPtr->flag      = true;
    
    osal_msg_send( registeredRestTVTaskID, (uint8 *)msgPtr ); //����ϵͳ��Ϣ��Ӧ��
  }
}

/*********************************************************************
 * @fn      AutoResetTVApp_UARTSendResetCmd
 *
 * @brief   ���͸�λָ�TV.
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
 * @brief   ���ͽ��빤��ģʽָ�TV.
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
