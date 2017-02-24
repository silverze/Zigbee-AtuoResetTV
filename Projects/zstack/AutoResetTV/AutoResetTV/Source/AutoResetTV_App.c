#include "AutoResetTV_App.h"


/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8 recStep = 1;//���ڽ���TV����ָ���
extern uint8 sendCmdCnt = 0;//����ָ��ʹ���


/*registeredRestTVTaskID ����ע�ᴮ��TV��λ�ɹ���־���OSAL�¼�*/
static uint8 registeredRestTVTaskID;
uint8 index = 0;
char* mathStr = "hop finished and standby";  //(S)shop finished and standby
uint8 param[50] = {0};//�洢TVָ��ķ��ز��� 

//������������
static void AutoResetTVApp_SendFlagMsg(void);
static void AutoResetTVApp_SendGetTVSNMsg(void);
static void AutoResetTVApp_MatchShopedStr(uint8 port);//ƥ��(S)shop finished and standby�ַ���
static bool AutoResetTVApp_ReceiveReturnData( uint8 port, bool bParam );
//static bool AutoResetTVApp_ReceiveReturnParam( uint8 ch);

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
 * @brief   У��TV����ָ���Ƿ���ȷ
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
    
    if(isParam)//������������ص�TVָ��
    {
      if( i < 5 && ch == check_cmd[i])
      {
        i++;
      }
      else if (i >= 5)//��һ�η�������ȷ
      {
        if( ( i == 5 && ch == 0xAB ) || i == 6 ) //�ڶ��εĿ�ͷ��������յ����ݳ���
        {
          param[i - 5] = ch; 
          i++;
        }
        else if( i > 6)
        {
          param[i - 5] = ch;
          i++;
          if( (i - 5) == param[1]) //i-5,Ϊ������һ�εĳ��ȼ������ڶ��ν���OK
          {
            ret = true;
            break;
          }
        }
        else //�ڶ��η��ش���
        {
          i = 0;
          ret = false;
          break;
        }        
      }
      else      //��һ�η��ش���
      {
        i = 0;
        ret = false;
        break;
      }    
    }
    else //������������ָ��
    {
      if(ch == check_cmd[i])
      {
        i++;
        if( i == 5) //��ʱTV����ȷ����ָ��
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
      AutoResetTVApp_SendFlagMsg();//���͸�λ�ɹ���Ϣ��Ӧ�ò�
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

static void AutoResetTVApp_SendGetTVSNMsg(void)
{
  getTVSN_t *msgPtr;
    // Send the Msg to the task
  msgPtr = (getTVSN_t *)osal_msg_allocate( sizeof(getTVSN_t) );
  if ( msgPtr )
  {
    msgPtr->hdr.event = GET_TV_SN;
    msgPtr->tvSN_data = param; //��zigbee�ն˻�ȡ��TV�ڲ���SN����OSAL��Ϣ����
    
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
 * @fn      AutoResetTVApp_UARTSendGetSNCmd
 *
 * @brief   ���ͻ�ȡ���ӻ�SNָ�TV.
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
