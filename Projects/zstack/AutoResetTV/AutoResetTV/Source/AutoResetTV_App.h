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
  bool flag; //复位状态
} tvResetFlag_t; //终端检测到TV复位完成后，发送给OSAL的事件消息结构
  
typedef struct 
{
  osal_event_hdr_t hdr;
  uint8* tvSN_data;
} getTVSN_t;//终端获取到TV内部SN后，发送给OSAL的事件消息结构

/*
 *注册监控电视机复位标志位任务
 */
extern void AutoResetTVApp_RegisterTaskID( uint8 task_id ); 
   
/*
 * 处理串口接收数据
 */
extern void AutoResetTVApp_UartProcessData( uint8 port, uint8 event );
 
/*
 * 发送复位指令到TV.
 */
extern void AutoResetTVApp_UARTSendResetCmd( void );

/*
 * 发送获取电视机SN指令到TV.
 */
extern void AutoResetTVApp_UARTSendGetSNCmd( void );

/*
 * 发送进入工厂模式指令到TV.
 */
extern void AutoResetTVApp_UARTSendEnterFacCmd( void );
  
#ifdef __cplusplus
}
#endif

#endif //AUTORESETTVAPP_H