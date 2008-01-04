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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <qstring.h>
#include <visitstream.h>
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
// ****************************************************************************

class Plugin
{
  public:
    QString name;
    QString type;
    QString label;
    QString version;
    QString vartype;
    QString dbtype;
    QString iconFile;

    bool haswriter;
    bool hasoptions;
    bool enabledByDefault;
    bool has_MDS_specific_code;
    bool hasEngineSpecificCode;
    bool onlyEnginePlugin;
    bool noEnginePlugin;

    vector<QString> cxxflags;
    vector<QString> ldflags;
    vector<QString> libs;
    vector<QString> extensions; // for DB plugins
    bool customgfiles;
    vector<QString> gfiles;     // gui
    bool customsfiles;
    vector<QString> sfiles;     // scripting
    bool customvfiles;
    vector<QString> vfiles;     // viewer
    bool custommfiles;
    vector<QString> mfiles;     // mdserver
    bool customefiles;
    vector<QString> efiles;     // engine
    bool customwfiles;
    vector<QString> wfiles;     // widget
    bool customvwfiles;
    vector<QString> vwfiles;    // viewer widget
    vector<QString> defaultgfiles;
    vector<QString> defaultsfiles;
    vector<QString> defaultvfiles;
    vector<QString> defaultmfiles;
    vector<QString> defaultefiles;
    vector<QString> defaultwfiles;

    Attribute *atts;
  public:
    Plugin(const QString &n,const QString &l,const QString &t,
           const QString &vt,const QString &dt,const QString &v, 
           const QString &ifile, bool hw, bool ho, bool onlyengine,
           bool noengine)
        :  name(n), type(t), label(l), version(v), vartype(vt), dbtype(dt), 
           iconFile(ifile), haswriter(hw), hasoptions(ho), 
           onlyEnginePlugin(onlyengine), noEnginePlugin(noengine), atts(NULL)
    {
        enabledByDefault = true;
        has_MDS_specific_code = false;
        hasEngineSpecificCode = false;
        customgfiles = false;
        customsfiles = false;
        customvfiles = false;
        custommfiles = false;
        customefiles = false;
        customwfiles = false;
        customvwfiles = false;
        gfiles.clear();
        sfiles.clear();
        vfiles.clear();
        mfiles.clear();
        efiles.clear();
        wfiles.clear();
        vwfiles.clear();
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
    };
    void Print(ostream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
        if (atts)
            atts->Print(cout);
    }
    void SaveXML(ostream &out, QString indent)
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
        }
        else if (type == "database")
        {
            WriteTagAttr(out, "dbtype", dbtype);
            WriteTagAttr(out, "haswriter", Bool2Text(haswriter));
            WriteTagAttr(out, "hasoptions", Bool2Text(hasoptions));
        }
        FinishOpenTag(out);

        if (cxxflags.size() > 0)
        {
            WriteOpenTag(out, "CXXFLAGS", indent);
            WriteValues(out, cxxflags, indent);
            WriteCloseTag(out, "CXXFLAGS", indent);
        }

        if (ldflags.size() > 0)
        {
            WriteOpenTag(out, "LDFLAGS", indent);
            WriteValues(out, ldflags, indent);
            WriteCloseTag(out, "LDFLAGS", indent);
        }

        if (libs.size() > 0)
        {
            WriteOpenTag(out, "LIBS", indent);
            WriteValues(out, libs, indent);
            WriteCloseTag(out, "LIBS", indent);
        }

        if (type == "database" && extensions.size() > 0)
        {
            WriteOpenTag(out, "Extensions", indent);
            WriteValues(out, extensions, indent);
            WriteCloseTag(out, "Extensions", indent);
        }

        if (customgfiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "G");
            FinishOpenTag(out);
            WriteValues(out, gfiles, indent);
            WriteCloseTag(out, "Files", indent);
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
        if (custommfiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "M");
            FinishOpenTag(out);
            WriteValues(out, mfiles, indent);
            WriteCloseTag(out, "Files", indent);
        }
        if (customefiles)
        {
            StartOpenTag(out, "Files", indent);
            WriteTagAttr(out, "components", "E");
            FinishOpenTag(out);
            WriteValues(out, efiles, indent);
            WriteCloseTag(out, "Files", indent);
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

        if (atts)
            atts->SaveXML(out, indent);

        WriteCloseTag(out, "Plugin", indent);
    }
};

#endif
