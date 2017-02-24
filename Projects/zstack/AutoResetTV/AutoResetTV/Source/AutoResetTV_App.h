#ifndef AUTORESETTV_APP_H
#define AUTORESETTV_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************************************
 *                                               INCLUDES
 ***************************************************************************************************/
#include <stdio.h>
#include <string.h>  
#include "osal.h"
#include "hal_types.h"  
#include "hal_uart.h"
#include "hal_led.h"
  
typedef struct
{
  osal_event_hdr_t hdr;
  bool flag; //��λ״̬
} tvResetFlag_t; //�ն˼�⵽TV��λ��ɺ󣬷��͸�OSAL���¼���Ϣ�ṹ
  
typedef struct 
{
  osal_event_hdr_t hdr;
  uint8* tvSN_data;
} getTVSN_t;//�ն˻�ȡ��TV�ڲ�SN�󣬷��͸�OSAL���¼���Ϣ�ṹ

/*
 *ע���ص��ӻ���λ��־λ����
 */
extern void AutoResetTVApp_RegisterTaskID( uint8 task_id ); 
   
/*
 * �����ڽ�������
 */
extern void AutoResetTVApp_UartProcessData( uint8 port, uint8 event );
 
/*
 * ���͸�λָ�TV.
 */
extern void AutoResetTVApp_UARTSendResetCmd( void );

/*
 * ���ͻ�ȡ���ӻ�SNָ�TV.
 */
extern void AutoResetTVApp_UARTSendGetSNCmd( void );

/*
 * ���ͽ��빤��ģʽָ�TV.
 */
extern void AutoResetTVApp_UARTSendEnterFacCmd( void );
  
#ifdef __cplusplus
}
#endif

#endif //AUTORESETTVAPP_H