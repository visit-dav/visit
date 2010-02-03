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

#include <iostream>
#include <vector>

#include <DebugStream.h>

class GeneralDatabasePluginInfo;
class MDServerDatabasePluginInfo;

#define DECLARE_DATABASE(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralDatabasePluginInfo *X##_GetGeneralInfo(); \
   extern "C" MDServerDatabasePluginInfo *X##_GetMDServerInfo();

// Declare the databases.
#define PLUGIN_VERB DECLARE_DATABASE
#include <enabled_databases.h>
#undef PLUGIN_VERB


#define CHECK_PLUGIN(X) \
   if (sym == #X"VisItPluginVersion") \
       retval = (void *) &X##VisItPluginVersion; \
   else if (sym == #X"_GetGeneralInfo") \
       retval = (void *) &X##_GetGeneralInfo; \
   else if (sym == #X"_GetMDServerInfo") \
       retval = (void *) &X##_GetMDServerInfo; \


#define PLUGIN_VERB CHECK_PLUGIN
static void *
database_dlsym(const std::string &sym)
{
    void *retval = NULL;
    #include <enabled_databases.h>
    return retval;
}
#undef PLUGIN_VERB

void *
fake_dlsym(const std::string &sym)
{
    debug1 << "Asked for " << sym << endl;
    void *ptr = NULL;
    if((ptr = database_dlsym(sym)) != NULL)
        return ptr;

    debug1 << "fake_dlsym can't find symbol " << sym << endl;
    return NULL;
}

#define ADD_DATABASE_PLUGIN(X) \
    libs.push_back(std::pair<std::string, std::string>("", "libM"#X"Database.a"));; \
    libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Database.a"));

void
StaticGetSupportedLibs(std::vector<std::pair<std::string, std::string> > &libs,
                       const std::string &pluginType)
{   
    if (pluginType == "database")
    {
        // Add the databases
#define PLUGIN_VERB ADD_DATABASE_PLUGIN
#include <enabled_databases.h>
#undef PLUGIN_VERB
    }
}


