#ifndef OPTION_H
#define OPTION_H

#include <ace/SOCK_Stream.h>
#include <string>

using Str = std::string;
using SOCK = ACE_SOCK_Stream;

SOCK connectToFtp(Str ip, int port = 21);
int loginToFtp(SOCK control_socket, Str user = "anonymous", Str pass = "");
int enterPassiveAndGetDataConnection(SOCK control_socket, SOCK& dsock);
void downloadOneSegment(SOCK control_socket, SOCK data_socket, Str path, off_t start_offset, size_t size, int part_id, FILE* file);
int quitAndClose(ACE_SOCK_Stream& control_socket);
int getFtpFileSize(SOCK sock, const std::string& path);
void enterPassiveAndDownloadOneSegmentAndClose(Str path,
                          off_t off,
                          size_t size,
                          int part_id,
                          FILE* file,
                          SOCK sock);


#include <iostream>
#include <string>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <functional>

typedef std::function<int(SOCK&)> SockCreator;

SockCreator
makeLoginedSockCreator(const ACE_INET_Addr& address,
                       const std::string& username,
                       const std::string& password);

#endif // OPTION_H
