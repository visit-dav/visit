// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef WIN32_COMM_HELPERS_H
#define WIN32_COMM_HELPERS_H

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <comm_exports.h>

COMM_API struct hostent *CopyHostent(struct hostent *h);
void COMM_API            FreeHostent(struct hostent *h);
void COMM_API            LogWindowsSocketError(const char *mName, const char *fName);
#endif

#endif
