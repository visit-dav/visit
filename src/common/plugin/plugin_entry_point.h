/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
*  - Neither the NAME of  the LLNS/LLNL nor the NAMEs of  its contributors may
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

#ifndef PLUGIN_ENTRY_POINT_H
#define PLUGIN_ENTRY_POINT_H

/* Plugin entry point macros. This enables easy customization of plugin entry
 * point functions, etc.
 */
#define VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)     NAME##_Get##FLAVOR##Info
#define VISIT_PLUGIN_ENTRY_FUNC_STR(NAME,FLAVOR) #NAME"_Get"#FLAVOR"Info"
#define VISIT_PLUGIN_ENTRY_ARGS void
#define VISIT_PLUGIN_ENTRY_NAMED_ARGS void
#define VISIT_PLUGIN_ENTRY_ARGS_DECLARE
#define VISIT_PLUGIN_ENTRY_ARGS_CALL

#define VISIT_PLOT_PLUGIN_ENTRY(NAME, FLAVOR) \
extern "C" PLOT_EXPORT FLAVOR##PlotPluginInfo* VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    return new NAME##FLAVOR##PluginInfo; \
}
#define VISIT_PLOT_PLUGIN_ENTRY_EV(NAME, FLAVOR) \
extern "C" PLOT_EXPORT FLAVOR##PlotPluginInfo* VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    NAME##FLAVOR##PluginInfo::InitializeGlobalObjects(); \
    return new NAME##FLAVOR##PluginInfo; \
}

#define VISIT_OPERATOR_PLUGIN_ENTRY(NAME, FLAVOR) \
extern "C" OP_EXPORT FLAVOR##OperatorPluginInfo* VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    return new NAME##FLAVOR##PluginInfo; \
}
#define VISIT_OPERATOR_PLUGIN_ENTRY_EV(NAME, FLAVOR) \
extern "C" OP_EXPORT FLAVOR##OperatorPluginInfo* VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    NAME##FLAVOR##PluginInfo::InitializeGlobalObjects(); \
    return new NAME##FLAVOR##PluginInfo; \
}

/* NOTE: We can't seem to use VISIT_PLUGIN_ENTRY_FUNC in this one because
 *       it keeps evaluating to 1_GetEngineInfo when we do ITAPS.
 */
#define VISIT_DATABASE_PLUGIN_ENTRY(NAME, FLAVOR) \
extern "C" DBP_EXPORT FLAVOR##DatabasePluginInfo* NAME##_Get##FLAVOR##Info(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    return new NAME##FLAVOR##PluginInfo; \
}

#endif
