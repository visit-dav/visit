/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#ifndef INSTALLATION_FUNCTIONS_H
#define INSTALLATION_FUNCTIONS_H
#include <string>
#include <misc_exports.h>

typedef enum {CONFIGSTATE_IOERROR,
              CONFIGSTATE_FIRSTTIME,
              CONFIGSTATE_SUCCESS} ConfigStateEnum;

// Installation directory functions
std::string MISC_API GetVisItInstallationDirectory();
std::string MISC_API GetVisItInstallationDirectory(const char *version);
bool        MISC_API ReadInstallationInfo(std::string &distName, 
                                          std::string &configName, 
                                          std::string &bankName);

std::string MISC_API GetVisItArchitectureDirectory();
std::string MISC_API GetVisItArchitectureDirectory(const char *version);
std::string MISC_API GetVisItLauncher();

// User installation / data functions
std::string MISC_API GetUserVisItDirectory();
MISC_API char *      GetDefaultConfigFile(const char *filename = 0, const char *home = 0);
MISC_API char *      GetSystemConfigFile(const char *filename = 0);

std::string MISC_API GetUserVisItRCFile();
std::string MISC_API GetSystemVisItRCFile();

int         MISC_API ConfigStateGetRunCount(ConfigStateEnum &code);
void        MISC_API ConfigStateIncrementRunCount(ConfigStateEnum &code);

// Version functions
int         MISC_API GetVisItVersionFromString(const char*, int&, int&, int&);
bool        MISC_API VisItVersionsCompatible(const char*, const char*);
bool        MISC_API VersionGreaterThan(const std::string &v1, const std::string &v2);
void        MISC_API SetIsDevelopmentVersion(bool val);
bool        MISC_API GetIsDevelopmentVersion();

#endif
