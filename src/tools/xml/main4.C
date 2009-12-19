/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include "main.h"
#include "GenerateInfo.h"
#include "main.C"

void
CallGenerator(const QString &docType, Attribute *attribute, Plugin *plugin, const QString &)
{
    // info writer mode
    if (docType == "Plugin" && plugin->type == "database")
    {
        QFile *fih;
        if ((fih = Open(plugin->name+"PluginInfo.h")) != 0)
        {
            QTextStream ih(fih);
            plugin->WriteInfoHeader(ih);
            fih->close();
            delete fih;
        }

        QFile *fic;
        if ((fic = Open(plugin->name+"PluginInfo.C")) != 0)
        {
            QTextStream ic(fic);
            plugin->WriteInfoSource(ic);
            fic->close();
            delete fic;
        }

        QFile *fcc;
        if ((fcc = Open(plugin->name+"CommonPluginInfo.C")) != 0)
        {
            QTextStream cc(fcc);
            plugin->WriteCommonInfoSource(cc);
            fcc->close();
            delete fcc;
        }

        QFile *fmc;
        if ((fmc = Open(plugin->name+"MDServerPluginInfo.C")) != 0)
        {
            QTextStream mc(fmc);
            plugin->WriteMDServerInfoSource(mc);
            fmc->close();
            delete fmc;
        }

        QFile *fec;
        if ((fec = Open(plugin->name+"EnginePluginInfo.C")) != 0)
        {
            QTextStream ec(fec);
            plugin->WriteEngineInfoSource(ec);
            fec->close();
            delete fec;
        }
    }
    else
    {
        QFile *fih;
        if ((fih = Open(plugin->name+"PluginInfo.h")) != 0)
        {
            QTextStream ih(fih);
            plugin->WriteInfoHeader(ih);
            fih->close();
            delete fih;
        }

        QFile *fic;
        if ((fic = Open(plugin->name+"PluginInfo.C")) != 0)
        {
            QTextStream ic(fic);
            plugin->WriteInfoSource(ic);
            fic->close();
            delete fic;
        }

        QFile *fcc;
        if ((fcc = Open(plugin->name+"CommonPluginInfo.C")) != 0)
        {
            QTextStream cc(fcc);
            plugin->WriteCommonInfoSource(cc);
            fcc->close();
            delete fcc;
        }

        QFile *fgc;
        if ((fgc = Open(plugin->name+"GUIPluginInfo.C")) != 0)
        {
            QTextStream gc(fgc);
            plugin->WriteGUIInfoSource(gc);
            fgc->close();
            delete fgc;
        }

        QFile *fvc;
        if ((fvc = Open(plugin->name+"ViewerPluginInfo.C")) != 0)
        {
            QTextStream vc(fvc);
            plugin->WriteViewerInfoSource(vc);
            fvc->close();
            delete fvc;
        }

        QFile *fec;
        if ((fec = Open(plugin->name+"EnginePluginInfo.C")) != 0)
        {
            QTextStream ec(fec);
            plugin->WriteEngineInfoSource(ec);
            fec->close();
            delete fec;
        }

        QFile *fsc;
        if ((fsc = Open(plugin->name+"ScriptingPluginInfo.C")) != 0)
        {
            QTextStream sc(fsc);
            plugin->WriteScriptingInfoSource(sc);
            fsc->close();
            delete fsc;
        }
    }
}
