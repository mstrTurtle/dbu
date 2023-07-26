// -*- C++ -*-

//=============================================================================
/**
 *  @file    FtpControlHandler.h
 *  @details 实现与FTP控制连接交互的类定义。
 *
 *  @author JiayuXiao <xiaojiayu@scutech.com>
 */
//=============================================================================
#pragma once
#include "ace/Event_Handler.h"
class FtpControlHandler : ACE_Event_Handler
{
  int section_no_;

  int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
  int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
};
