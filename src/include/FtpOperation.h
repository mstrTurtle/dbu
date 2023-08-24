#ifndef OPTION_H
#define OPTION_H

#include <ace/SOCK_Stream.h>
#include <string>

using Str = std::string;
using SOCK = ACE_SOCK_Stream;

SOCK connectToFtp(Str ip, int port = 21);
int loginToFtp(SOCK control_socket, Str user = "anonymous", Str pass = "");
SOCK connectAndLoginVimFtp();
int enterPassiveAndGetDataConnection(SOCK control_socket, SOCK& dsock);
void connectLoginAndDownloadOneSegmentFromVim(Str path, off_t off, size_t size, int part_id, FILE* file);
void downloadOneSegment(SOCK control_socket, SOCK data_socket, Str path, off_t start_offset, size_t size, int part_id, FILE* file);
int quitAndClose(ACE_SOCK_Stream& control_socket);
int getFtpFileSize(SOCK sock, const std::string& path);

#endif // OPTION_H
