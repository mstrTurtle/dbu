// -*- C++ -*-

//=============================================================================
/**
 *  @file    FtpDataHandler.h
 *  @details 实现文件列表读取与文件下载的类定义。
 *
 *  @author JiayuXiao <xiaojiayu@scutech.com>
 */
//=============================================================================
#pragma once
#include "ace/Event_Handler.h"
class FtpDataHandler : ACE_Event_Handler
{
  int section_no_;

  int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
  int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
};
