// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

        QFile *fvec;
        if ((fvec = Open(plugin->name+"ViewerEnginePluginInfo.C")) != 0)
        {
            QTextStream vc(fvec);
            plugin->WriteViewerEngineInfoSource(vc);
            fvec->close();
            delete fvec;
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
