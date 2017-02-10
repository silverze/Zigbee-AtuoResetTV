#include "AutoResetTV_App.h"


/*registeredRestTVTaskID ����ע�ᴮ��TV��λ�ɹ���־���OSAL�¼�*/
static uint8 registeredRestTVTaskID;

static void AutoResetTVApp_SendFlagMsg(void);

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
void AutoResetTVApp_SendFlagMsg(void)
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
