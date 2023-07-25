#pragma once
#include <stdio.h>

#ifdef _WIN32
#include "winsock.h"
#else
#include <arpa/inet.h>
#include <netdb.h>
#endif

namespace dbu {

inline int
resolve(char* dom, struct in_addr* addr)
{
  struct hostent* he;

  he = gethostbyname(dom);
  if (he == NULL) // 如果有问题
  {
    switch (h_errno) {
      case HOST_NOT_FOUND: // 主机没找到
        fputs("The host was not found.\n", stderr);
        break;
      case NO_ADDRESS: // 有名字但没ip
        fputs("The name is valid but it has no address.\n", stderr);
        break;
      case NO_RECOVERY: // 不可恢复
        fputs("A non-recoverable name server error occurred.\n", stderr);
        break;
      case TRY_AGAIN: // 暂时不可用
        fputs("The name server is temporarily unavailable.", stderr);
        break;
    }
  } else // 如果一切正常
  {
    addr = (((struct in_addr*)
               he->h_addr_list[0])); // addr_list抽出一个来，用ntoa转换并打印
    return 0;
  }

  return he != NULL;
}

}
