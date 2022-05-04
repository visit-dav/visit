// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PLUGIN_H
#define PLUGIN_H
#include <QTextStream>
#include <PluginBase.h>
#include "Attribute.h"

// ****************************************************************************
//  Class:  Plugin
//
//  Purpose:
//    Abstraction for a plugin.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 14:14:45 PDT 2001
//    Added vartype.
//
//    Jeremy Meredith, Tue Aug 27 14:32:50 PDT 2002
//    Added mfiles, dbtype, extensions, and libs.  Allowed NULL atts.
//
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Sean Ahern, Fri Nov 15 15:25:23 PST 2002
//    Added "widget files" so we can have custom GUI elements.
//
//    Brad Whitlock, Thu Mar 13 11:38:23 PDT 2003
//    Added icon file so the plugin can contain an icon.
//
//    Hank Childs, Tue Sep  9 10:04:41 PDT 2003
//    Added a field to indicate whether or not there is a writer.
//
//    Jeremy Meredith, Tue Sep 23 16:17:41 PDT 2003
//    Changed haswriter to be a bool.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added ability to disable plugins by default.
//    Added avt files for databases.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Jeremy Meredith, Wed Aug 25 11:50:14 PDT 2004
//    Added the concept of an engine-only or everything-but-the-engine plugin.
//
//    Jeremy Meredith, Thu Mar 17 12:52:28 PST 2005
//    The onlyengine/noengine flags were getting save as the wrong value.
//
//    Hank Childs, Tue May 24 09:41:53 PDT 2005
//    Add hasoptions.
//
//    Brad Whitlock, Fri Feb 23 17:11:23 PST 2007
//    Added viewer widgets.
//
//    Cyrus Harrison, Wed Mar  7 09:11:05 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Hank Childs, Thu Jan 10 14:06:42 PST 2008
//    Add specifiedFilenames and filenames.
//
//    Brad Whitlock, Thu Feb 28 13:46:51 PST 2008
//    Made it inherit PluginBase, a new base class that allows all of the
//    basic attributes to be shared between Plugin and the various
//    AttsGeneratorPlugin classes.
//
//    Brad Whitlock, Thu May  8 11:49:00 PDT 2008
//    Qt 4. Use QTextStream.
//
//    Cyrus Harrison, Fri Sep 19 13:56:30 PDT 2008
//    Added support for custom libs for gui,engine,mdserver,and viewer targets.
//
//    Jeremy Meredith, Tue Sep  8 15:11:35 EDT 2009
//    Split custom engine libs into serial and parallel versions.
//
//    Jeremy Meredith, Tue Dec 29 11:21:30 EST 2009
//    Replaced "Extensions" and "Filenames" with "FilePatterns".  Removed
//    specifiedFilenames.  Added filePatternsStrict and opensWholeDirectory.
//
//    Brad Whitlock, Thu Feb  4 16:11:01 PST 2010
//    I added support for category.
//
//    Hank Childs, Thu Dec 30 13:33:19 PST 2010
//    Add support for operators that write their own expressions.
//
//    Kathleen Biagas, Thu Nov  6 11:24:21 PST 2014
//    Add support for DEFINES tag.
//
//    Kathleen Biagas, Thu Jan  2 09:18:18 PST 2020
//    Added hl argument for haslicense.
//
//    Kathleen Biagas, Tue May 3, 2022
//    Added component-specific CXXFLAGS, LDFLAGS, LIBS and DEFINES support
//    to Print.
//
// ****************************************************************************

class Plugin : public PluginBase
{
  public:
    std::vector<QString> defaultgfiles;
    std::vector<QString> defaultsfiles;
    std::vector<QString> defaultvfiles;
    std::vector<QString> defaultmfiles;
    std::vector<QString> defaultefiles;
    std::vector<QString> defaultwfiles;

    Attribute *atts;
  public:
    Plugin(const QString &n,const QString &l,const QString &t,
           const QString &vt,const QString &dt,const QString &v,
           const QString &ifile, bool hw, bool ho, bool hl, bool onlyengine,
           bool noengine)
        :  PluginBase(n,l,t,vt,dt,v,ifile,hw,ho,hl,onlyengine,noengine),
           defaultgfiles(),
           defaultsfiles(),
           defaultvfiles(),
           defaultmfiles(),
           defaultefiles(),
           defaultwfiles(),
           atts(NULL)
    {
        if (type == "database")
        {
            QString filter = QString("avt") + name + "FileFormat.C";
            defaultmfiles.push_back(filter);
            defaultefiles.push_back(filter);
        }
        else if (type == "plot")
        {
            QString filter = QString("avt") + name + "Filter.C";
            defaultvfiles.push_back(filter);
            defaultefiles.push_back(filter);
            QString widgets = QString("Qvis") + name + "PlotWindow.h";
            defaultwfiles.push_back(widgets);
        }
        else if (type == "operator")
        {
            QString filter = QString("avt") + name + "Filter.C";
            defaultvfiles.push_back(filter);
            defaultefiles.push_back(filter);
        }
    }

    void Print(QTextStream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< "\n";
        if (atts)
            atts->Print(cOut);
    }
    void SaveXML(QTextStream &out, QString indent)
    {
        StartOpenTag(out, "Plugin", indent);
        WriteTagAttr(out, "name", name);
        WriteTagAttr(out, "type", type);
        WriteTagAttr(out, "label", label);
        WriteTagAttr(out, "version", version);
        WriteTagAttr(out, "enabled", Bool2Text(enabledByDefault));
        WriteTagAttr(out, "mdspecificcode", Bool2Text(has_MDS_specific_code));
        WriteTagAttr(out, "engspecificcode", Bool2Text(hasEngineSpecificCode));
        WriteTagAttr(out, "onlyengine", Bool2Text(onlyEnginePlugin));
        WriteTagAttr(out, "noengine", Bool2Text(noEnginePlugin));

        if (type == "plot")
        {
            WriteTagAttr(out, "vartype", vartype);
            if(iconFile.length() > 0)
                WriteTagAttr(out, "iconFile", iconFile);
        }
        else if (type == "operator")
        {
            if(iconFile.length() > 0)
                WriteTagAttr(out, "iconFile", iconFile);
            if(category.length() > 0)
                WriteTagAttr(out, "category", category);
            if (createExpression)
            {
                WriteTagAttr(out, "createExpression", Bool2Text(createExpression));
                WriteTagAttr(out, "exprInType", exprInType);
                WriteTagAttr(out, "exprOutType", exprOutType);
            }
        }
        else if (type == "database")
        {
            WriteTagAttr(out, "dbtype", dbtype);
            WriteTagAttr(out, "haswriter", Bool2Text(haswriter));
            WriteTagAttr(out, "hasoptions", Bool2Text(hasoptions));
            WriteTagAttr(out, "haslicense", Bool2Text(haslicense));
            WriteTagAttr(out, "filePatternsStrict", Bool2Text(filePatternsStrict));
            WriteTagAttr(out, "opensWholeDirectory", Bool2Text(opensWholeDirectory));
        }
        FinishOpenTag(out);

        if (cxxflags.size() > 0)
        {
            WriteOpenTag(out, "CXXFLAGS", indent);
            WriteValues(out, cxxflags, indent);
            WriteCloseTag(out, "CXXFLAGS", indent);
        }
        if (mcxxflags.size() > 0)
        {
            StartOpenTag(out, "CXXFLAGS", indent);
            WriteTagAttr(out, "components", "M");
            FinishOpenTag(out);
            WriteValues(out, mcxxflags, indent);
            WriteCloseTag(out, "CXXFLAGS", indent);
        }
        if (ecxxflagsSer.size() > 0)
        {
            StartOpenTag(out, "CXXFLAGS", indent);
            WriteTagAttr(out, "components", "ESer");
            FinishOpenTag(out);
            WriteValues(out, ecxxflagsSer, indent);
            WriteCloseTag(out, "CXXFLAGS", indent);
        }
        if (ecxxflagsPar.size() > 0)
        {
            StartOpenTag(out, "CXXFLAGS", indent);
            WriteTagAttr(out, "components", "EPar");
            FinishOpenTag(out);
            WriteValues(out, ecxxflagsPar, indent);
            WriteCloseTag(out, "CXXFLAGS", indent);
        }

        if (ldflags.size() > 0)
        {
            WriteOpenTag(out, "LDFLAGS", indent);
            WriteValues(out, ldflags, indent);
            WriteCloseTag(out, "LDFLAGS", indent);
        }
        if (mldflags.size() > 0)
        {
            StartOpenTag(out, "LDFLAGS", indent);
            WriteTagAttr(out, "components", "M");
            FinishOpenTag(out);
            WriteValues(out, mldflags, indent);
            WriteCloseTag(out, "LDFLAGS", indent);
        }
        if (eldflagsSer.size() > 0)
        {
            StartOpenTag(out, "LDFLAGS", indent);
            WriteTagAttr(out, "components", "ESer");
            FinishOpenTag(out);
            WriteValues(out, eldflagsSer, indent);
            WriteCloseTag(out, "LDFLAGS", indent);
        }
        if (eldflagsPar.size() > 0)
        {
            StartOpenTag(out, "LDFLAGS", indent);
            WriteTagAttr(out, "components", "EPar");
            FinishOpenTag(out);
            WriteValues(out, eldflagsPar, indent);
            WriteCloseTag(out, "LDFLAGS", indent);
        }

        if (libs.size() > 0)
        {
            WriteOpenTag(out, "LIBS", indent);
            WriteValues(out, libs, indent);
            WriteCloseTag(out, "LIBS", indent);
        }
        if (mlibs.size() > 0)
        {
            StartOpenTag(out, "LIBS", indent);
            WriteTagAttr(out, "components", "M");
            FinishOpenTag(out);
            WriteValues(out, mlibs, indent);
            WriteCloseTag(out, "LIBS", indent);
        }
        if (elibsSer.size() > 0)
        {
            StartOpenTag(out, "LIBS", indent);
            WriteTagAttr(out, "components", "ESer");
            FinishOpenTag(out);
            WriteValues(out, elibsSer, indent);
            WriteCloseTag(out, "LIBS", indent);
        }
        if (elibsPar.size() > 0)
        {
            StartOpenTag(out, "LIBS", indent);
            WriteTagAttr(out, "components", "EPar");
            FinishOpenTag(out);
            WriteValues(out, elibsPar, indent);
            WriteCloseTag(out, "LIBS", indent);
        }

        if (defs.size() > 0)
        {
            WriteOpenTag(out, "DEFINES", indent);
            WriteValues(out, defs, indent);
            WriteCloseTag(out, "DEFINES", indent);
        }
        if (mdefs.size() > 0)
        {
            StartOpenTag(out, "DEFINES", indent);
            WriteTagAttr(out, "components", "M");
            FinishOpenTag(out);
            WriteValues(out, mdefs, indent);
            WriteCloseTag(out, "DEFINES", indent);
        }
        if (edefsSer.size() > 0)
        {
            StartOpenTag(out, "DEFINES", indent);
            WriteTagAttr(out, "components", "ESer");
            FinishOpenTag(out);
            WriteValues(out, edefsSer, indent);
            WriteCloseTag(out, "DEFINES", indent);
        }
        if (edefsPar.size() > 0)
        {
            StartOpenTag(out, "DEFINES", indent);
            WriteTagAttr(out, "components", "EPar");
            FinishOpenTag(out);
            WriteValues(out, edefsPar, indent);
            WriteCloseTag(out, "DEFINES", indent);
        }

        if (type == "database" && filePatterns.size() > 0)
        {
            WriteOpenTag(out, "FilePatterns", indent);
            WriteValues(out, filePatterns, indent);
            WriteCloseTag(out, "FilePatterns", indent);
        }

        if (customgfiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "G");
            FinishOpenTag(out);
            WriteValues(out, gfiles, indent);
            WriteCloseTag(out, "Files", indent);
        }
        if (customglibs)
        {
            StartOpenTag(out, "LIBS", indent);
            WriteTagAttr(out, "components", "G");
            FinishOpenTag(out);
            WriteValues(out, glibs, indent);
            WriteCloseTag(out, "LIBS", indent);
        }
        if (customsfiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "S");
            FinishOpenTag(out);
            WriteValues(out, sfiles, indent);
            WriteCloseTag(out, "Files", indent);
        }
        if (customvfiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "V");
            FinishOpenTag(out);
            WriteValues(out, vfiles, indent);
            WriteCloseTag(out, "Files", indent);
        }
        if (customvlibs)
        {
            StartOpenTag(out, "LIBS", indent);
            WriteTagAttr(out, "components", "V");
            FinishOpenTag(out);
            WriteValues(out, vlibs, indent);
            WriteCloseTag(out, "LIBS", indent);
        }
        if (custommfiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "M");
            FinishOpenTag(out);
            WriteValues(out, mfiles, indent);
            WriteCloseTag(out, "Files", indent);
        }
        if (custommlibs)
        {
            StartOpenTag(out, "LIBS", indent);
            WriteTagAttr(out, "components", "M");
            FinishOpenTag(out);
            WriteValues(out, mlibs, indent);
            WriteCloseTag(out, "LIBS", indent);
        }
        if (customefiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "E");
            FinishOpenTag(out);
            WriteValues(out, efiles, indent);
            WriteCloseTag(out, "Files", indent);
        }
        if (customelibsSer)
        {
            StartOpenTag(out, "LIBS", indent);
            WriteTagAttr(out, "components", "ESer");
            FinishOpenTag(out);
            WriteValues(out, elibsSer, indent);
            WriteCloseTag(out, "LIBS", indent);
        }
        if (customelibsPar)
        {
            StartOpenTag(out, "LIBS", indent);
            WriteTagAttr(out, "components", "EPar");
            FinishOpenTag(out);
            WriteValues(out, elibsPar, indent);
            WriteCloseTag(out, "LIBS", indent);
        }
        if (customwfiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "W");
            FinishOpenTag(out);
            WriteValues(out, wfiles, indent);
            WriteCloseTag(out, "Files", indent);
        }
        if (customvwfiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "VW");
            FinishOpenTag(out);
            WriteValues(out, vwfiles, indent);
            WriteCloseTag(out, "Files", indent);
        }
        if (customjfiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "J");
            FinishOpenTag(out);
            WriteValues(out, jfiles, indent);
            WriteCloseTag(out, "Files", indent);
        }

        if (atts)
            atts->SaveXML(out, indent);

        WriteCloseTag(out, "Plugin", indent);
    }
};

#endif
