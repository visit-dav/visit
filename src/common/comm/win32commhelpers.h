#ifndef WIN32_COMM_HELPERS_H
#define WIN32_COMM_HELPERS_H

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>

struct hostent *CopyHostent(struct hostent *h);
void            FreeHostent(struct hostent *h);
void            LogWindowsSocketError(const char *mName, const char *fName);
#endif

#endif
