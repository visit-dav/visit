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

#include <iostream.h>
#include <vector>

#include <DebugStream.h>

class GeneralDatabasePluginInfo;
class EngineDatabasePluginInfo;
class GeneralPlotPluginInfo;
class EnginePlotPluginInfo;
class GeneralOperatorPluginInfo;
class EngineOperatorPluginInfo;

#define DECLARE_DATABASE(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralDatabasePluginInfo *X##_GetGeneralInfo(void); \
   extern "C" EngineDatabasePluginInfo *X##_GetEngineInfo(void);

#define DECLARE_OPERATOR(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralOperatorPluginInfo *X##_GetGeneralInfo(void); \
   extern "C" EngineOperatorPluginInfo *X##_GetEngineInfo(void);

#define DECLARE_PLOT(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralPlotPluginInfo *X##_GetGeneralInfo(void); \
   extern "C" EnginePlotPluginInfo *X##_GetEngineInfo(void);

// Declare the plots.
#define PLUGIN_VERB DECLARE_PLOT
#include <enabled_plots.h>
#undef PLUGIN_VERB

// Declare the operators.
#define PLUGIN_VERB DECLARE_OPERATOR
#include <enabled_operators.h>
#undef PLUGIN_VERB

// Declare the databases.
#define PLUGIN_VERB DECLARE_DATABASE
#include <enabled_databases.h>
#undef PLUGIN_VERB


#define CHECK_PLUGIN(X) \
   if (sym == #X"VisItPluginVersion") \
       retval = (void *) &X##VisItPluginVersion; \
   else if (sym == #X"_GetGeneralInfo") \
       retval = (void *) &X##_GetGeneralInfo; \
   else if (sym == #X"_GetEngineInfo") \
       retval = (void *) &X##_GetEngineInfo; \


// Split apart to make it compile faster
#define PLUGIN_VERB CHECK_PLUGIN
static void *
plot_dlsym(const std::string &sym)
{
    void *retval = NULL;
    #include <enabled_plots.h>
    return retval;
}

static void *
operator_dlsym(const std::string &sym)
{
    void *retval = NULL;
    #include <enabled_operators.h>
    return retval;
}

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
    if((ptr = plot_dlsym(sym)) != NULL)
        return ptr;
    if((ptr = operator_dlsym(sym)) != NULL)
        return ptr;
    if((ptr = database_dlsym(sym)) != NULL)
        return ptr;

    debug1 << "fake_dlsym can't find symbol " << sym << endl;
    return NULL;
}


#ifdef PARALLEL
  #define ADD_DATABASE_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Database_par.a"));; \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Operator.a"));
#else
  #define ADD_DATABASE_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Database_ser.a"));; \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Operator.a"));
#endif

#ifdef PARALLEL
  #define ADD_OPERATOR_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Operator_par.a"));; \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Operator.a"));
#else
  #define ADD_OPERATOR_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Operator_ser.a"));; \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Operator.a"));
#endif

#ifdef PARALLEL
  #define ADD_PLOT_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Plot_par.a")); \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Plot.a"));
#else
  #define ADD_PLOT_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Plot_ser.a")); \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Plot.a"));
#endif

void
StaticGetSupportedLibs(std::vector<std::pair<std::string, std::string> > &libs,
                       const std::string &pluginType)
{
    if (pluginType == "plot")
    {
        // Add the plots.
#define PLUGIN_VERB ADD_PLOT_PLUGIN
#include <enabled_plots.h>
#undef PLUGIN_VERB
    }

    if (pluginType == "operator")
    {
        // Add the operators
#define PLUGIN_VERB ADD_OPERATOR_PLUGIN
#include <enabled_operators.h>
#undef PLUGIN_VERB
    }
    
    if (pluginType == "database")
    {
        // Add the databases
#define PLUGIN_VERB ADD_DATABASE_PLUGIN
#include <enabled_databases.h>
#undef PLUGIN_VERB
    }

#undef PLUGIN_VERB
}


