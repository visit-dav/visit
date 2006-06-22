/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef GENERATE_INFO_H
#define GENERATE_INFO_H

#include "Field.h"
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

// ****************************************************************************
//  File:  GenerateInfo
//
//  Purpose:
//    Contains a set of classes which override the default implementation
//    to create info for the plugin.
//
//  Note: This file overrides --
//    Plugin
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 14:06:08 PDT 2001
//    Major changes.  Now GUI et al. inherit from Common, then from General.
//    Added the Common plugin info class.
//
//    Jeremy Meredith, Sat Jan  5 17:52:26 PST 2002
//    Removed AllocXXXXFilter from the viewer operator info.  It was not
//    needed and caused some problems.
//
//    Jeremy Meredith, Wed Feb  6 17:25:47 PST 2002
//    Removed some debug code.
//    Changed references to plugin->name+"Attributes" to instead refer
//    directly to atts->name.
//
//    Brad Whitlock, Tue Apr 23 10:18:40 PDT 2002
//    Made plugins include an export header file. I also changed the name
//    of the method that creates GUI windows to avoid namespace conflicts on
//    MS Windows.
//
//    Brad Whitlock, Mon May 6 14:45:09 PST 2002
//    Added a new method for scripting plugins.
//
//    Jeremy Meredith, Tue Aug 27 14:32:50 PDT 2002
//    Added support for database plugins.
//    Added mfiles, dbtype, and libs.  Allowed NULL atts.
//
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Brad Whitlock, Thu Mar 13 12:39:57 PDT 2003
//    I added code to generate icon methods if the plugin has an icon file. I
//    also added code to put a GetVariableTypes method into viewer plot plugins.
//
//    Hank Childs, Fri Aug  1 10:39:50 PDT 2003
//    Added support for curves.
//
//    Kathleen Bonnell, Thu Sep 11 10:29:34 PDT 2003 
//    Added bool argument to InitializeOperatorAtts, and modified the code
//    to initialize from the default atts or client atts based on the value
//    of the flag. 
//
//    Jeremy Meredith, Tue Sep 23 17:03:25 PDT 2003
//    Made haswriter be a bool.  Added a missing semicolon in the outpus.
//    Added tensor and symmetric tensor variable types.
//
//    Brad Whitlock, Thu Aug 21 11:28:03 PDT 2003
//    I added a macro definition that gets used on MacOS X that makes the
//    plugin access function have the plugin name built into it so that we
//    don't get multiply defined symbol errors when loading multiple plugins.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added ability to disable plugins by default.
//
//    Brad Whitlock, Fri Mar 26 00:49:59 PDT 2004
//    I changed the interface to some viewer plugin methods.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Jeremy Meredith, Wed Aug 25 11:50:14 PDT 2004
//    Added the concept of an engine-only or everything-but-the-engine plugin.
//
//    Jeremy Meredith, Tue Feb 22 13:42:10 PST 2005
//    Moved the test for having a database writer into the high-level info.
//    Moved the ability to retrieve the writer itself into only the engine.
//
//    Brad Whitlock, Thu Mar 3 09:02:20 PDT 2005
//    Moved the VisIt version used to build the plugin to here instead of
//    GenerateMakefile.
//
//    Brad Whitlock, Fri Apr 1 15:53:12 PST 2005
//    Added support for label variables.
//
//    Hank Childs, Mon May 23 17:03:54 PDT 2005
//    Add support for DBOptions.  Plus fix typo in comment (SetUp -> Setup).
//
//    Hank Childs, Tue Jul 19 13:51:03 PDT 2005
//    Add support for array variables.
//
//    Brad Whitlock, Thu Jan 5 15:18:33 PST 2006
//    Changed scripting plugin info.
//
//    Hank Childs, Thu Jun  8 16:46:56 PDT 2006
//    Added copyright string.
//
// ****************************************************************************

// ----------------------------------------------------------------------------
//                             Utility Functions
// ----------------------------------------------------------------------------

QString
CurrentTime()
{
    char *tstr[] = {"PDT", "PST"};
    char s1[10], s2[10], s3[10], tmpbuf[200];
    time_t t;
    char *c = NULL;
    int h,m,s,y;
    t = time(NULL);
    c = asctime(localtime(&t));
    // Read the hour.
    sscanf(c, "%s %s %s %d:%d:%d %d", s1, s2, s3, &h, &m, &s, &y);
    // Reformat the string a little.
    sprintf(tmpbuf, "%s %s %s %02d:%02d:%02d %s %d",
            s1, s2, s3, h, m, s, tstr[h > 12], y);

    return QString(tmpbuf);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
class InfoGeneratorPlugin
{
  public:
    QString name;
    QString type;
    QString label;
    QString version;
    QString vartype;
    QString dbtype;
    QString iconFile;
    bool    haswriter;
    bool    hasoptions;
    bool    enabledByDefault;
    bool    has_MDS_specific_code;
    bool    onlyEnginePlugin;
    bool    noEnginePlugin;

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
    vector<QString> wfiles;     // widgets

    Attribute *atts;
  public:
    InfoGeneratorPlugin(const QString &n,const QString &l,const QString &t,
        const QString &vt,const QString &dt,const QString &v,
        const QString &ifile, bool hw, bool ho, bool onlyengine, bool noengine) : name(n), type(t), label(l),
        version(v), vartype(vt), dbtype(dt), iconFile(ifile), haswriter(hw),
        hasoptions(ho), onlyEnginePlugin(onlyengine), noEnginePlugin(noengine),
        atts(NULL)
    {
        enabledByDefault = true;
    }
    void Print(ostream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
        if (atts)
            atts->Print(cout);
    }
    void WriteInfoHeader(ostream &h)
    {
        if (type=="operator")
        {
            h << copyright_str.c_str() << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//  File: "<<name<<"PluginInfo.h" << endl;
            h << "// ************************************************************************* //" << endl;
            h << endl;
            h << "#ifndef "<<name.upper()<<"_PLUGIN_INFO_H" << endl;
            h << "#define "<<name.upper()<<"_PLUGIN_INFO_H" << endl;
            h << "#include <OperatorPluginInfo.h>" << endl;
            h << "#include <operator_plugin_exports.h>" << endl;
            h << endl;
            h << "class "<<atts->name<<";" << endl;
            h << endl;
            h << "// ****************************************************************************" << endl;
            h << "//  Class: "<<name<<"PluginInfo" << endl;
            h << "//" << endl;
            h << "//  Purpose:" << endl;
            h << "//    Five classes that provide all the information about an "<<name<<" operator" << endl;
            h << "//" << endl;
            h << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            h << "//  Creation:   "<<CurrentTime()<< endl;
            h << "//" << endl;
            h << "//  Modifications:" << endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << endl;
            h << "class "<<name<<"GeneralPluginInfo : public virtual GeneralOperatorPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual char *GetName() const;" << endl;
            h << "    virtual char *GetVersion() const;" << endl;
            h << "    virtual char *GetID() const;" << endl;
            h << "    virtual bool  EnabledByDefault() const;" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"CommonPluginInfo : public virtual CommonOperatorPluginInfo, public virtual "<<name<<"GeneralPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual AttributeSubject *AllocAttributes();" << endl;
            h << "    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"GUIPluginInfo : public virtual GUIOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual const char *GetMenuName() const;" << endl;
            h << "    virtual QvisPostableWindowObserver *CreatePluginWindow(int type," << endl;
            h << "        AttributeSubject *attr, QvisNotepadArea *notepad);" << endl;
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"ViewerPluginInfo : public virtual ViewerOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual AttributeSubject *GetClientAtts();" << endl;
            h << "    virtual AttributeSubject *GetDefaultAtts();" << endl;
            h << "    virtual void SetClientAtts(AttributeSubject *atts);" << endl;
            h << "    virtual void GetClientAtts(AttributeSubject *atts);" << endl;
            h << endl;
            h << "    virtual void InitializeOperatorAtts(AttributeSubject *atts," << endl;
            h << "                                        const ViewerPlot *plot," << endl;
            h << "                                        const bool fromDefault);" << endl;
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << endl;
            h << endl;
            h << "    static void InitializeGlobalObjects();" << endl;
            h << "  private:" << endl;
            h << "    static "<<atts->name<<" *defaultAtts;" << endl;
            h << "    static "<<atts->name<<" *clientAtts;" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"EnginePluginInfo : public virtual EngineOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual avtPluginFilter *AllocAvtPluginFilter();" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"ScriptingPluginInfo : public virtual ScriptingOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual void InitializePlugin(AttributeSubject *subj, void *data);" << endl;
            h << "    virtual void *GetMethodTable(int *nMethods);" << endl;
            h << "    virtual bool TypesMatch(void *pyobject);" << endl;
            h << "    virtual char *GetLogString();" << endl;
            h << "    virtual void SetDefaults(const AttributeSubject *atts);" << endl;
            h << "};" << endl;
            h << endl;
            h << "#endif" << endl;
        }
        else if (type=="plot")
        {
            h << copyright_str.c_str() << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//                               "<<name<<"PluginInfo.h                            //" << endl;
            h << "// ************************************************************************* //" << endl;
            h << endl;
            h << "#ifndef "<<name.upper()<<"_PLUGIN_INFO_H" << endl;
            h << "#define "<<name.upper()<<"_PLUGIN_INFO_H" << endl;
            h << "#include <PlotPluginInfo.h>" << endl;
            h << "#include <plot_plugin_exports.h>" << endl;
            h << endl;
            h << "class "<<atts->name<<";" << endl;
            h << endl;
            h << "// ****************************************************************************" << endl;
            h << "//  Class: "<<name<<"PluginInfo" << endl;
            h << "//" << endl;
            h << "//  Purpose:" << endl;
            h << "//    Five classes that provide all the information about a "<<name<< endl;
            h << "//    plot plugin.  The information is broken up into five classes since" << endl;
            h << "//    portions of it are only relevant to particular components within" << endl;
            h << "//    visit.  There is the general information which all the components" << endl;
            h << "//    are interested in, the gui information which the gui is interested in," << endl;
            h << "//    the viewer information which the viewer is interested in, the" << endl;
            h << "//    engine information which the engine is interested in, and finally a." << endl;
            h << "//    scripting portion that enables the Python VisIt extension to use the" << endl;
            h << "//    plugin." << endl;
            h << "//" << endl;
            h << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            h << "//  Creation:   "<<CurrentTime()<< endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << endl;
            h << "class "<<name<<"GeneralPluginInfo: public virtual GeneralPlotPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual char *GetName() const;" << endl;
            h << "    virtual char *GetVersion() const;" << endl;
            h << "    virtual char *GetID() const;" << endl;
            h << "    virtual bool  EnabledByDefault() const;" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"CommonPluginInfo : public virtual CommonPlotPluginInfo, public virtual "<<name<<"GeneralPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual AttributeSubject *AllocAttributes();" << endl;
            h << "    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"GUIPluginInfo: public virtual GUIPlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual const char *GetMenuName() const;" << endl;
            h << "    virtual int GetVariableTypes() const;" << endl;
            h << "    virtual QvisPostableWindowObserver *CreatePluginWindow(int type," << endl;
            h << "        AttributeSubject *attr, QvisNotepadArea *notepad);" << endl;
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"ViewerPluginInfo: public virtual ViewerPlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual AttributeSubject *GetClientAtts();" << endl;
            h << "    virtual AttributeSubject *GetDefaultAtts();" << endl;
            h << "    virtual void SetClientAtts(AttributeSubject *atts);" << endl;
            h << "    virtual void GetClientAtts(AttributeSubject *atts);" << endl;
            h << endl;
            h << "    virtual avtPlot *AllocAvtPlot();" << endl;
            h << endl;
            h << "    virtual void InitializePlotAtts(AttributeSubject *atts," << endl;
            h << "        const avtDatabaseMetaData *md," << endl;
            h << "        const char *variableName);" << endl;
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << endl;
            h << "    virtual int GetVariableTypes() const;" << endl;
            h << endl;
            h << "    static void InitializeGlobalObjects();" << endl;
            h << "  private:" << endl;
            h << "    static "<<atts->name<<" *defaultAtts;" << endl;
            h << "    static "<<atts->name<<" *clientAtts;" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"EnginePluginInfo: public virtual EnginePlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual avtPlot *AllocAvtPlot();" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"ScriptingPluginInfo : public virtual ScriptingPlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual void InitializePlugin(AttributeSubject *subj, void *data);" << endl;
            h << "    virtual void *GetMethodTable(int *nMethods);" << endl;
            h << "    virtual bool TypesMatch(void *pyobject);" << endl;
            h << "    virtual char *GetLogString();" << endl;
            h << "    virtual void SetDefaults(const AttributeSubject *atts);" << endl;
            h << "};" << endl;
            h << endl;
            h << "#endif" << endl;
        }
        else if (type=="database")
        {
            h << copyright_str.c_str() << endl;
            h << "// ****************************************************************************" << endl;
            h << "//                               "<<name<<"PluginInfo.h" << endl;
            h << "// ****************************************************************************" << endl;
            h << "" << endl;
            h << "#ifndef "<<name.upper()<<"_PLUGIN_INFO_H" << endl;
            h << "#define "<<name.upper()<<"_PLUGIN_INFO_H" << endl;
            h << "#include <DatabasePluginInfo.h>" << endl;
            h << "#include <database_plugin_exports.h>" << endl;
            h << "" << endl;
            h << "class avtDatabase;" << endl;
            h << "class avtDatabaseWriter;" << endl;
            h << "" << endl;
            h << "// ****************************************************************************" << endl;
            h << "//  Class: "<<name<<"DatabasePluginInfo" << endl;
            h << "//" << endl;
            h << "//  Purpose:" << endl;
            h << "//    Classes that provide all the information about the "<<name<<" plugin." << endl;
            h << "//    Portions are separated into pieces relevant to the appropriate" << endl;
            h << "//    components of VisIt." << endl;
            h << "//" << endl;
            h << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            h << "//  Creation:   "<<CurrentTime() << endl;
            h << "//" << endl;
            h << "//  Modifications:" << endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << "" << endl;
            h << "class "<<name<<"GeneralPluginInfo : public virtual GeneralDatabasePluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual char *GetName() const;" << endl;
            h << "    virtual char *GetVersion() const;" << endl;
            h << "    virtual char *GetID() const;" << endl;
            h << "    virtual bool  EnabledByDefault() const;" << endl;
            h << "    virtual bool  HasWriter() const;" << endl;
            h << "};" << endl;
            h << "" << endl;
            h << "class "<<name<<"CommonPluginInfo : public virtual CommonDatabasePluginInfo, public virtual "<<name<<"GeneralPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual DatabaseType              GetDatabaseType();" << endl;
            h << "    virtual std::vector<std::string>  GetDefaultExtensions();" << endl;
            h << "    virtual avtDatabase              *SetupDatabase(const char * const *list," << endl;
            h << "                                                    int nList, int nBlock);" << endl;
            if (hasoptions)
            {
                h << "    virtual DBOptionsAttributes *GetReadOptions() const;"
                  << endl;
                h << "    virtual DBOptionsAttributes *GetWriteOptions() const;"
                  << endl;
            }
            h << "};" << endl;
            h << "" << endl;
            h << "class "<<name<<"MDServerPluginInfo : public virtual MDServerDatabasePluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    // this makes compilers happy... remove if we ever have functions here" << endl;
            h << "    virtual void dummy();" << endl;
            h << "};" << endl;
            h << "" << endl;
            h << "class "<<name<<"EnginePluginInfo : public virtual EngineDatabasePluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual avtDatabaseWriter        *GetWriter(void);" << endl;
            h << "};" << endl;
            h << "" << endl;
            h << "#endif" << endl;
        }
    }
    void AddVersion(ostream &c)
    {
        c << "#include <visit-config.h>" << endl;
        c << "#if defined(__APPLE__)" << endl;
        c << "extern \"C\" const char *"<<name<<"VisItPluginVersion = VERSION;" << endl;
        c << "#else" << endl;
        c << "extern \"C\" const char *VisItPluginVersion = VERSION;" << endl;
        c << "#endif" << endl;
        c << endl;
    }
    void AddMacOSXMacro(ostream &c, const char *infoType)
    {
        c << "#if defined(__APPLE__)" << endl;
        c << "#define Get" << infoType << "Info " << name << "_Get" << infoType << "Info" << endl;
        c << "#endif" << endl << endl;
    }
    void WriteInfoSource(ostream &c)
    {
        c << copyright_str.c_str() << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//  File: "<<name<<"PluginInfo.C" << endl;
        c << "// ************************************************************************* //" << endl;
        c << endl;
        c << "#include <"<<name<<"PluginInfo.h>" << endl;
        if (type!="database")
            c << "#include <"<<atts->name<<".h>" << endl;
        c << endl;
        AddVersion(c);
        AddMacOSXMacro(c, "General");
        c << "// ****************************************************************************" << endl;
        c << "//  Function:  GetGeneralInfo" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return a new GeneralPluginInfo for the "<<name<<" "<<type<<"." << endl;
        c << "//" << endl;
        c << "//  Programmer:  "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:    "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        if (type=="operator")
            c << "extern \"C\" GeneralOperatorPluginInfo* GetGeneralInfo()" << endl;
        else if (type=="plot")
            c << "extern \"C\" GeneralPlotPluginInfo* GetGeneralInfo()" << endl;
        else if (type=="database")
            c << "extern \"C\" GeneralDatabasePluginInfo* GetGeneralInfo()" << endl;
        c << "{" << endl;
        c << "    return new "<<name<<"GeneralPluginInfo;" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"GeneralPluginInfo::GetName" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return the name of the "<<type<<" plugin." << endl;
        c << "//" << endl;
        c << "//  Returns:    A pointer to the name of the "<<type<<" plugin." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "char *" << endl;
        c << name<<"GeneralPluginInfo::GetName() const" << endl;
        c << "{" << endl;
        c << "    return \""<<name<<"\";" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"GeneralPluginInfo::GetVersion" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return the version of the "<<type<<" plugin." << endl;
        c << "//" << endl;
        c << "//  Returns:    A pointer to the version of the "<<type<<" plugin." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "char *" << endl;
        c << name<<"GeneralPluginInfo::GetVersion() const" << endl;
        c << "{" << endl;
        c << "    return \""<<version<<"\";" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"GeneralPluginInfo::GetID" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return the id of the "<<type<<" plugin." << endl;
        c << "//" << endl;
        c << "//  Returns:    A pointer to the id of the "<<type<<" plugin." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "char *" << endl;
        c << name<<"GeneralPluginInfo::GetID() const" << endl;
        c << "{" << endl;
        c << "    return \""<<name<<"_"<<version<<"\";" << endl;
        c << "}" << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"GeneralPluginInfo::EnabledByDefault" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return true if this plugin should be enabled by default; false otherwise." << endl;
        c << "//" << endl;
        c << "//  Returns:    true/false" << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "bool" << endl;
        c << name<<"GeneralPluginInfo::EnabledByDefault() const" << endl;
        c << "{" << endl;
        c << "    return "<<Bool2Text(enabledByDefault)<<";" << endl;
        c << "}" << endl;
        if (type=="database")
        {
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<name<<"GeneralPluginInfo::HasWriter" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return true if this plugin has a database writer." << endl;
            c << "//" << endl;
            c << "//  Returns:    true/false" << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "bool" << endl;
            c << name<<"GeneralPluginInfo::HasWriter() const" << endl;
            c << "{" << endl;
            c << "    return "<<Bool2Text(haswriter)<<";" << endl;
            c << "}" << endl;
        }
    }
    void WriteCommonInfoSource(ostream &c)
    {
        if (type=="database")
        {
            c << copyright_str.c_str() << endl;
            c << "#include <"<<name<<"PluginInfo.h>" << endl;
            if (dbtype != "Custom")
            {
                c << "#include <avt"<<name<<"FileFormat.h>" << endl;
                c << "#include <avt"<<dbtype<<"FileFormatInterface.h>" << endl;
                c << "#include <avtGenericDatabase.h>" << endl;
            }
            else
            {
                c << "#include <avt"<<name<<"Database.h>" << endl;
            }
            if (hasoptions)
                c << "#include <avt"<<name<<"Options.h>" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method:  "<<name<<"CommonPluginInfo::GetDatabaseType" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Returns the type of a "<<name<<" database." << endl;
            c << "//" << endl;
            c << "//  Programmer:  "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:    "<<CurrentTime() << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "DatabaseType" << endl;
            c << ""<<name<<"CommonPluginInfo::GetDatabaseType()" << endl;
            c << "{" << endl;
            c << "    return DB_TYPE_"<<dbtype.upper()<<";" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method:  "<<name<<"CommonPluginInfo::GetDefaultExtensions" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Returns the default extensions for a "<<name<<" database." << endl;
            c << "//" << endl;
            c << "//  Programmer:  "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:    "<<CurrentTime() << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "std::vector<std::string>" << endl;
            c << ""<<name<<"CommonPluginInfo::GetDefaultExtensions()" << endl;
            c << "{" << endl;
            c << "    std::vector<std::string> defaultExtensions;" << endl;
            for (int i=0; i<extensions.size(); i++)
            {
                c << "    defaultExtensions.push_back(\""<<extensions[i]<<"\");" << endl;
            }
            c << "" << endl;
            c << "    return defaultExtensions;" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<name<<"CommonPluginInfo::Setup"<<name<<"Database" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Sets up a "<<name<<" database." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//      list    A list of file names." << endl;
            c << "//      nList   The number of timesteps in list." << endl;
            c << "//      nBlocks The number of blocks in the list." << endl;
            c << "//" << endl;
            c << "//  Returns:    A "<<name<<" database from list." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime() << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "avtDatabase *" << endl;
            c << ""<<name<<"CommonPluginInfo::SetupDatabase(const char *const *list," << endl;
            c << "                                   int nList, int nBlock)" << endl;
            c << "{" << endl;
            if (dbtype == "Custom")
            {
                c << "    return new avt"<<name<<"Database(list[0]);" << endl;
            }
            else if (dbtype == "STSD")
            {
                c << "    avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[nList];" << endl;
                c << "    int nTimestep = nList / nBlock;" << endl;
                c << "    for (int i = 0 ; i < nTimestep ; i++)" << endl;
                c << "    {" << endl;
                c << "        ffl[i] = new avtSTSDFileFormat*[nBlock];" << endl;
                c << "        for (int j = 0 ; j < nBlock ; j++)" << endl;
                c << "        {" << endl;
                if (hasoptions)
                    c << "            ffl[i][j] = new avt"<<name<<"FileFormat(list[i*nBlock + j], readOptions);" << endl;
                else
                    c << "            ffl[i][j] = new avt"<<name<<"FileFormat(list[i*nBlock + j]);" << endl;
                c << "        }" << endl;
                c << "    }" << endl;
                c << "    avtSTSDFileFormatInterface *inter " << endl;
                c << "           = new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);" << endl;
                c << "    return new avtGenericDatabase(inter);" << endl;
            }
            else if (dbtype == "STMD")
            {
                c << "    avtSTMDFileFormat **ffl = new avtSTMDFileFormat*[nList];" << endl;
                c << "    for (int i = 0 ; i < nList ; i++)" << endl;
                c << "    {" << endl;
                if (hasoptions)
                    c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i], readOptions);" << endl;
                else
                    c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i]);" << endl;
                c << "    }" << endl;
                c << "    avtSTMDFileFormatInterface *inter " << endl;
                c << "           = new avtSTMDFileFormatInterface(ffl, nList);" << endl;
                c << "    return new avtGenericDatabase(inter);" << endl;
            }
            else if (dbtype == "MTSD")
            {
                c << "    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];" << endl;
                c << "    for (int i = 0 ; i < nList ; i++)" << endl;
                c << "    {" << endl;
                if (hasoptions)
                    c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i], readOptions);" << endl;
                else
                    c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i]);" << endl;
                c << "    }" << endl;
                c << "    avtMTSDFileFormatInterface *inter " << endl;
                c << "           = new avtMTSDFileFormatInterface(ffl, nList);" << endl;
                c << "    return new avtGenericDatabase(inter);" << endl;
            }
            else if (dbtype == "MTMD")
            {
                c << "    return new avtGenericDatabase(" << endl;
                c << "               new avtMTMDFileFormatInterface(" << endl;
                if (hasoptions)
                    c << "                   new avt"<<name<<"FileFormat(list[0], readOptions)));" << endl;
                else
                    c << "                   new avt"<<name<<"FileFormat(list[0])));" << endl;
            }
            else
            {
                throw QString().sprintf("Unknown database type '%s'",dbtype.latin1());
            }
            c << "}" << endl;
            if (hasoptions)
            {
                c << "" << endl;
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<name<<"CommonPluginInfo::GetReadOptions" << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//      Gets the read options." << endl;
                c << "//" << endl;
                c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
                c << "//  Creation:   "<<CurrentTime()<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "DBOptionsAttributes *" << endl;
                c << name<<"CommonPluginInfo::GetReadOptions() const" << endl;
                c << "{" << endl;
                c << "    return Get"<<name<<"ReadOptions();" << endl;
                c << "}" << endl;
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<name<<"CommonPluginInfo::GetWriteOptions" << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//      Gets the write options." << endl;
                c << "//" << endl;
                c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
                c << "//  Creation:   "<<CurrentTime()<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "DBOptionsAttributes *" << endl;
                c << name<<"CommonPluginInfo::GetWriteOptions() const" << endl;
                c << "{" << endl;
                c << "    return Get"<<name<<"WriteOptions();" << endl;
                c << "}" << endl;
            }

        }
        else
        {
            c << copyright_str.c_str() << endl;
            c << "// ************************************************************************* //" << endl;
            c << "//  File: "<<name<<"CommonPluginInfo.C" << endl;
            c << "// ************************************************************************* //" << endl;
            c << endl;
            c << "#include <"<<name<<"PluginInfo.h>" << endl;
            c << "#include <"<<atts->name<<".h>" << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<name<<"CommonPluginInfo::AllocAttributes" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a pointer to a newly allocated attribute subject." << endl;
            c << "//" << endl;
            c << "//  Returns:    A pointer to the newly allocated attribute subject." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "AttributeSubject *" << endl;
            c << name<<"CommonPluginInfo::AllocAttributes()" << endl;
            c << "{" << endl;
            c << "    return new "<<atts->name<<";" << endl;
            c << "}" << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<name<<"CommonPluginInfo::CopyAttributes" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Copy a "<<name<<" attribute subject." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//    to        The destination attribute subject." << endl;
            c << "//    from      The source attribute subject." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "void " << endl;
            c << name<<"CommonPluginInfo::CopyAttributes(AttributeSubject *to," << endl;
            c << "    AttributeSubject *from)" << endl;
            c << "{" << endl;
            c << "    *(("<<atts->name<<" *) to) = *(("<<atts->name<<" *) from);" << endl;
            c << "}" << endl;
        }
    }
    void WriteGUIInfoSource(ostream &c)
    {
        if (type=="database")
            return;

        c << copyright_str.c_str() << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//  File: "<<name<<"GUIPluginInfo.C" << endl;
        c << "// ************************************************************************* //" << endl;
        c << endl;
        c << "#include <"<<name<<"PluginInfo.h>" << endl;
        c << "#include <"<<atts->name<<".h>" << endl;
        if (type=="operator")
            c << "#include <Qvis"<<name<<"Window.h>" << endl;
        else if (type=="plot")
            c << "#include <Qvis"<<name<<"PlotWindow.h>" << endl;
        c << endl;
        AddMacOSXMacro(c, "GUI");
        c << "// ****************************************************************************" << endl;
        c << "//  Function:  GetGUIInfo" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return a new GUIPluginInfo for the "<<name<<" "<<type<<"." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        if (type=="operator")
            c << "extern \"C\" GUIOperatorPluginInfo* GetGUIInfo()" << endl;
        else if (type=="plot")
            c << "extern \"C\" GUIPlotPluginInfo* GetGUIInfo()" << endl;
        c << "{" << endl;
        c << "    return new "<<name<<"GUIPluginInfo;" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"GUIPluginInfo::GetMenuName" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return a pointer to the name to use in the GUI menu." << endl;
        c << "//" << endl;
        c << "//  Returns:    A pointer to the name to use in the GUI menu." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "const char *" << endl;
        c << name<<"GUIPluginInfo::GetMenuName() const" << endl;
        c << "{" << endl;
        c << "    return \""<<label<<"\";" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;
        if (type=="plot")
        {
            c << "// ****************************************************************************" << endl;
            c << "// Method: "<<name<<"GUIPluginInfo::GetVariableTypes" << endl;
            c << "//" << endl;
            c << "// Purpose: " << endl;
            c << "//   Returns a flag indicating the types of variables that can be put in the" << endl;
            c << "//   plot's variable list." << endl;
            c << "//" << endl;
            c << "// Returns:    A flag indicating the types of variables that can be put in" << endl;
            c << "//             the plot's variable list." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// Modifications:" << endl;
            c << "//   " << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "int" << endl;
            c << name<<"GUIPluginInfo::GetVariableTypes() const" << endl;
            c << "{" << endl;

            c << "    return ";
            vector<QString> types = SplitValues(vartype);
            for (int i=0; i<types.size(); i++)
            {
                if (i>0) c << " | ";
                if      (types[i] == "mesh")
                    c << "VAR_CATEGORY_MESH";
                else if (types[i] == "scalar")
                    c << "VAR_CATEGORY_SCALAR";
                else if (types[i] == "material")
                    c << "VAR_CATEGORY_MATERIAL";
                else if (types[i] == "vector")
                    c << "VAR_CATEGORY_VECTOR";
                else if (types[i] == "species")
                    c << "VAR_CATEGORY_SPECIES";
                else if (types[i] == "subset")
                    c << "VAR_CATEGORY_SUBSET";
                else if (types[i] == "curve")
                    c << "VAR_CATEGORY_CURVE";
                else if (types[i] == "tensor")
                    c << "VAR_CATEGORY_TENSOR";
                else if (types[i] == "symmetrictensor")
                    c << "VAR_CATEGORY_SYMMETRIC_TENSOR";
                else if (types[i] == "label")
                    c << "VAR_CATEGORY_LABEL";
                else if (types[i] == "array")
                    c << "VAR_CATEGORY_ARRAY";
            }
            c << ";" << endl;

            c << "}" << endl;
            c << endl;
        }
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"GUIPluginInfo::CreatePluginWindow" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return a pointer to an "<<type<<"'s attribute window." << endl;
        c << "//" << endl;
        c << "//  Arguments:" << endl;
        c << "//    type      The type of the "<<type<<"." << endl;
        c << "//    attr      The attribute subject for the "<<type<<"." << endl;
        c << "//    notepad   The notepad to use for posting the window." << endl;
        c << "//" << endl;
        c << "//  Returns:    A pointer to the "<<type<<"'s attribute window." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "QvisPostableWindowObserver *" << endl;
        c << name<<"GUIPluginInfo::CreatePluginWindow(int type, AttributeSubject *attr," << endl;
        c << "    QvisNotepadArea *notepad)" << endl;
        c << "{" << endl;
        if (type=="operator")
            c << "    return new Qvis"<<name<<"Window(type, ("<<atts->name<<" *)attr," << endl
              << "        \""<<label<<" operator attributes\", \""<<name<<" operator\", notepad);" << endl;
        else if (type=="plot")
            c << "    return new Qvis"<<name<<"PlotWindow(type, ("<<atts->name<<" *)attr," << endl
              << "        \""<<name<<" plot attributes\", \""<<name<<" plot\", notepad);" << endl;
        c << "}" << endl;

        if(iconFile.length() > 0)
        {
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<name<<"GUIPluginInfo::XPMIconData" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a pointer to the icon data." << endl;
            c << "//" << endl;
            c << "//  Returns:    A pointer to the icon data." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "#include <" << iconFile << ">" << endl;
            c << "const char **" << endl;
            c << name<<"GUIPluginInfo::XPMIconData() const" << endl;
            c << "{" << endl;        
            c << "    return " << iconFile.left(iconFile.length() - 4) << "_xpm;" << endl;
            c << "}" << endl;
        }
    }
    void WriteViewerInfoSource(ostream &c)
    {
        if (type=="database")
            return;

        c << copyright_str.c_str() << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//  File: "<<name<<"ViewerPluginInfo.C" << endl;
        c << "// ************************************************************************* //" << endl;
        c << endl;
        c << "#include <"<<name<<"PluginInfo.h>" << endl;
        if (type=="operator")
            c << "#include <"<<atts->name<<".h>" << endl;
        else if (type=="plot")
            c << "#include <avt"<<name<<"Plot.h>" << endl;
        c << endl;
        AddMacOSXMacro(c, "Viewer");
        c << "// ****************************************************************************" << endl;
        c << "//  Function:  GetViewerInfo" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return a new ViewerPluginInfo for the "<<name<<" "<<type<<"." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        if (type=="operator")
            c << "extern \"C\" ViewerOperatorPluginInfo* GetViewerInfo()" << endl;
        else if (type=="plot")
            c << "extern \"C\" ViewerPlotPluginInfo* GetViewerInfo()" << endl;
        c << "{" << endl;
        c << "    "<<name<<"ViewerPluginInfo::InitializeGlobalObjects();" << endl;
        c << "    return new "<<name<<"ViewerPluginInfo;" << endl;
        c << "}" << endl;
        c << endl;
        c << "//" << endl;
        c << "// Storage for static data elements." << endl;
        c << "//" << endl;
        c << atts->name<<" *"<<name<<"ViewerPluginInfo::clientAtts = NULL;" << endl;
        c << atts->name<<" *"<<name<<"ViewerPluginInfo::defaultAtts = NULL;" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method:  "<<name<<"ViewerPluginInfo::InitializeGlobalObjects" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Initialize the "<<type<<" atts." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << "void" << endl;
        c << name<<"ViewerPluginInfo::InitializeGlobalObjects()" << endl;
        c << "{" << endl;
        c << "    "<<name<<"ViewerPluginInfo::clientAtts  = new "<<atts->name<<";" << endl;
        c << "    "<<name<<"ViewerPluginInfo::defaultAtts = new "<<atts->name<<";" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"ViewerPluginInfo::GetClientAtts" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return a pointer to the viewer client attributes." << endl;
        c << "//" << endl;
        c << "//  Returns:    A pointer to the viewer client attributes." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "AttributeSubject *" << endl;
        c << name<<"ViewerPluginInfo::GetClientAtts()" << endl;
        c << "{" << endl;
        c << "    return clientAtts;" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"ViewerPluginInfo::GetDefaultAtts" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return a pointer to the viewer default attributes." << endl;
        c << "//" << endl;
        c << "//  Returns:    A pointer to the viewer default attributes." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "AttributeSubject *" << endl;
        c << name<<"ViewerPluginInfo::GetDefaultAtts()" << endl;
        c << "{" << endl;
        c << "    return defaultAtts;" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"ViewerPluginInfo::SetClientAtts" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Set the viewer client attributes." << endl;
        c << "//" << endl;
        c << "//  Arguments:" << endl;
        c << "//    atts      A pointer to the new client attributes." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "void" << endl;
        c << name<<"ViewerPluginInfo::SetClientAtts(AttributeSubject *atts)" << endl;
        c << "{" << endl;
        c << "    *clientAtts = *("<<atts->name<<" *)atts;" << endl;
        c << "    clientAtts->Notify();" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"ViewerPluginInfo::GetClientAtts" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Get the viewer client attributes." << endl;
        c << "//" << endl;
        c << "//  Arguments:" << endl;
        c << "//    atts      A pointer to return the client default attributes in." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "void" << endl;
        c << name<<"ViewerPluginInfo::GetClientAtts(AttributeSubject *atts)" << endl;
        c << "{" << endl;
        c << "    *("<<atts->name<<" *)atts = *clientAtts;" << endl;
        c << "}" << endl;
        c << endl;
        if (type=="plot")
        {
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<name<<"ViewerPluginInfo::AllocAvtPlot" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a pointer to a newly allocated avt plot." << endl;
            c << "//" << endl;
            c << "//  Returns:    A pointer to the newly allocated avt plot." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "avtPlot *" << endl;
            c << name<<"ViewerPluginInfo::AllocAvtPlot()" << endl;
            c << "{" << endl;
            c << "    return new avt"<<name<<"Plot;" << endl;
            c << "}" << endl;
            c << endl;
        }
        c << "// ****************************************************************************" << endl;
        if (type=="operator")
            c << "//  Method: "<<name<<"ViewerPluginInfo::InitializeOperatorAtts" << endl;
        else if (type=="plot")
            c << "//  Method: "<<name<<"ViewerPluginInfo::InitializePlotAtts" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Initialize the "<<type<<" attributes to the default attributes." << endl;
        c << "//" << endl;
        c << "//  Arguments:" << endl;
        c << "//    atts      The attribute subject to initialize." << endl;
        c << "//    md        The metadata used to initialize." << endl;
        c << "//    atts      The variable name used to initialize." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "void" << endl;
        if (type=="operator")
        {
            c << name<<"ViewerPluginInfo::InitializeOperatorAtts(AttributeSubject *atts," << endl
              << "                                              const ViewerPlot *plot," << endl
              << "                                              const bool fromDefault)" << endl;
            c << "{" << endl;
            c << "    if (fromDefault)" << endl;
            c << "        *("<<atts->name<<"*)atts = *defaultAtts;" << endl;
            c << "    else" << endl;
            c << "        *("<<atts->name<<"*)atts = *clientAtts;" << endl;
            c << "}" << endl;
        }
        else if (type=="plot")
        {
            c << name<<"ViewerPluginInfo::InitializePlotAtts(AttributeSubject *atts," << endl
              << "    const avtDatabaseMetaData *, const char *)" << endl;
            c << "{" << endl;
            c << "    *("<<atts->name<<"*)atts = *defaultAtts;" << endl;
            c << "}" << endl;
        }

        if (type=="plot")
        {
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "// Method: "<<name<<"ViewerPluginInfo::GetVariableTypes" << endl;
            c << "//" << endl;
            c << "// Purpose: " << endl;
            c << "//   Returns a flag indicating the types of variables that can be put in the" << endl;
            c << "//   plot's variable list." << endl;
            c << "//" << endl;
            c << "// Returns:    A flag indicating the types of variables that can be put in" << endl;
            c << "//             the plot's variable list." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// Modifications:" << endl;
            c << "//   " << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "int" << endl;
            c << name<<"ViewerPluginInfo::GetVariableTypes() const" << endl;
            c << "{" << endl;

            c << "    return ";
            vector<QString> types = SplitValues(vartype);
            for (int i=0; i<types.size(); i++)
            {
                if (i>0) c << " | ";
                if      (types[i] == "mesh")
                    c << "VAR_CATEGORY_MESH";
                else if (types[i] == "scalar")
                    c << "VAR_CATEGORY_SCALAR";
                else if (types[i] == "material")
                    c << "VAR_CATEGORY_MATERIAL";
                else if (types[i] == "vector")
                    c << "VAR_CATEGORY_VECTOR";
                else if (types[i] == "species")
                    c << "VAR_CATEGORY_SPECIES";
                else if (types[i] == "subset")
                    c << "VAR_CATEGORY_SUBSET";
                else if (types[i] == "curve")
                    c << "VAR_CATEGORY_CURVE";
                else if (types[i] == "tensor")
                    c << "VAR_CATEGORY_TENSOR";
                else if (types[i] == "symmetrictensor")
                    c << "VAR_CATEGORY_SYMMETRIC_TENSOR";
                else if (types[i] == "label")
                    c << "VAR_CATEGORY_LABEL";
                else if (types[i] == "array")
                    c << "VAR_CATEGORY_ARRAY";
            }
            c << ";" << endl;
            c << "}" << endl;
            c << endl;
        }

        if(iconFile.length() > 0)
        {
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<name<<"ViewerPluginInfo::XPMIconData" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a pointer to the icon data." << endl;
            c << "//" << endl;
            c << "//  Returns:    A pointer to the icon data." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "#include <" << iconFile << ">" << endl;
            c << "const char **" << endl;
            c << name<<"ViewerPluginInfo::XPMIconData() const" << endl;
            c << "{" << endl;        
            c << "    return " << iconFile.left(iconFile.length() - 4) << "_xpm;" << endl;
            c << "}" << endl;
        }

    }
    void WriteMDServerInfoSource(ostream &c)
    {
        if (type=="database")
        {
            c << copyright_str.c_str() << endl;
            c << "#include <"<<name<<"PluginInfo.h>" << endl;
            c << "" << endl;
            AddMacOSXMacro(c, "MDServer");
            c << "// ****************************************************************************" << endl;
            c << "//  Function:  GetMDServerInfo" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a new MDServerPluginInfo for the "<<name<<" database." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime() << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "extern \"C\" MDServerDatabasePluginInfo* GetMDServerInfo()" << endl;
            c << "{" << endl;
            c << "    return new "<<name<<"MDServerPluginInfo;" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "// this makes compilers happy... remove if we ever have functions here" << endl;
            c << "void "<<name<<"MDServerPluginInfo::dummy()" << endl;
            c << "{" << endl;
            c << "}" << endl;
        }
    }
    void WriteEngineInfoSource(ostream &c)
    {
        if (type=="database")
        {
            c << copyright_str.c_str() << endl;
            c << "#include <"<<name<<"PluginInfo.h>" << endl;
            if (haswriter)
                c << "#include <avt"<<name<<"Writer.h>" << endl;
            c << "" << endl;
            AddMacOSXMacro(c, "Engine");
            c << "// ****************************************************************************" << endl;
            c << "//  Function:  GetEngineInfo" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a new EnginePluginInfo for the "<<name<<" database." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime() << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "extern \"C\" EngineDatabasePluginInfo* GetEngineInfo()" << endl;
            c << "{" << endl;
            c << "    return new "<<name<<"EnginePluginInfo;" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<name<<"CommonPluginInfo::GetWriter" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//      Sets up a "<<name<<" writer." << endl;
            c << "//" << endl;
            c << "//  Returns:    A "<<name<<" writer." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime() << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "avtDatabaseWriter *" << endl;
            c << ""<<name<<"EnginePluginInfo::GetWriter(void)" << endl;
            c << "{" << endl;
            if (haswriter)
                if (hasoptions)
                    c << "    return new avt"<<name<<"Writer(writeOptions);" 
                      << endl;
                else
                    c << "    return new avt"<<name<<"Writer;" << endl;
            else
                c << "    return NULL;" << endl;
            c << "}" << endl;
            c << "" << endl;
        }
        else
        {
            c << copyright_str.c_str() << endl;
            c << "// ************************************************************************* //" << endl;
            c << "//  File: "<<name<<"EnginePluginInfo.C" << endl;
            c << "// ************************************************************************* //" << endl;
            c << endl;
            c << "#include <"<<name<<"PluginInfo.h>" << endl;
            if (type=="operator")
                c << "#include <avt"<<name<<"Filter.h>" << endl;
            else if (type=="plot")
                c << "#include <avt"<<name<<"Plot.h>" << endl;
            c << endl;
            AddMacOSXMacro(c, "Engine");
            c << "// ****************************************************************************" << endl;
            c << "//  Function:  GetEngineInfo" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a new EnginePluginInfo for the "<<name<<" "<<type<<"." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            if (type=="operator")
                c << "extern \"C\" EngineOperatorPluginInfo* GetEngineInfo()" << endl;
            else if (type=="plot")
                c << "extern \"C\" EnginePlotPluginInfo* GetEngineInfo()" << endl;
            c << "{" << endl;
            c << "    return new "<<name<<"EnginePluginInfo;" << endl;
            c << "}" << endl;
            c << endl;
            c << "// ****************************************************************************" << endl;
            if (type=="operator")
                c << "//  Method: "<<name<<"EnginePluginInfo::AllocAvtPluginFilter" << endl;
            else if (type=="plot")
                c << "//  Method: "<<name<<"EnginePluginInfo::AllocAvtPlot" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            if (type=="operator")
                c << "//    Return a pointer to a newly allocated avtPluginFilter." << endl;
            else if (type=="plot")
                c << "//    Return a pointer to a newly allocated avt plot." << endl;
            c << "//" << endl;
            if (type=="operator")
                c << "//  Returns:    A pointer to the newly allocated avtPluginFilter." << endl;
            else if (type=="plot")
                c << "//  Returns:    A pointer to the newly allocated avt plot." << endl;
            c << "//" << endl;
            c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
            c << "//  Creation:   "<<CurrentTime()<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            if (type=="operator")
                c << "avtPluginFilter *" << endl;
            else if (type=="plot")
                c << "avtPlot *" << endl;
            if (type=="operator")
                c << name<<"EnginePluginInfo::AllocAvtPluginFilter()" << endl;
            else if (type=="plot")
                c << name<<"EnginePluginInfo::AllocAvtPlot()" << endl;
            c << "{" << endl;
            if (type=="operator")
                c << "    return new avt"<<name<<"Filter;" << endl;
            else if (type=="plot")
                c << "    return new avt"<<name<<"Plot;" << endl;
            c << "}" << endl;
        }
    }

    void WriteScriptingInfoSource(ostream &c)
    {
        if (type=="database")
            return;

        c << copyright_str.c_str() << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//                        "<<name<<"ScriptingPluginInfo.C" << endl;
        c << "// ************************************************************************* //" << endl;
        c << "#include <"<<name<<"PluginInfo.h>" << endl;
        c << "#include <Py"<<atts->name<<".h>" << endl;
        c << "" << endl;
        AddMacOSXMacro(c, "Scripting");
        c << "// ****************************************************************************" << endl;
        c << "//  Function:  GetScriptingInfo" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return a new ScriptingPluginInfo for the " << name << " plot." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        if(type=="plot")
            c << "extern \"C\" ScriptingPlotPluginInfo* GetScriptingInfo()" << endl;
        else if(type == "operator")
            c << "extern \"C\" ScriptingOperatorPluginInfo* GetScriptingInfo()" << endl;
        c << "{" << endl;
        c << "    return new "<<name<<"ScriptingPluginInfo;" << endl;
        c << "}" << endl;
        c << "" << endl;
        c << "// ****************************************************************************" << endl;
        c << "// Method: "<<name<<"ScriptingPluginInfo::InitializePlugin" << endl;
        c << "//" << endl;
        c << "// Purpose: " << endl;
        c << "//   Calls the initialization function for the plugin." << endl;
        c << "//" << endl;
        c << "// Arguments:" << endl;
        c << "//   subj    : A pointer to the plugin's state object." << endl;
        c << "//   data    : A pointer to data to be used by the observer function." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// Modifications:" << endl;
        c << "//   " << endl;
        c << "// ****************************************************************************" << endl;
        c << "" << endl;
        c << "void" << endl;
        c << name << "ScriptingPluginInfo::InitializePlugin(AttributeSubject *subj," << endl;
        c << "    void *data)" << endl;
        c << "{" << endl;
        c << "    Py"<<atts->name<<"_StartUp(("<<atts->name<<" *)subj, data);" << endl;
        c << "}" << endl;
        c << "" << endl;
        c << "// ****************************************************************************" << endl;
        c << "// Method: "<<name<<"ScriptingPluginInfo::GetMethodTable" << endl;
        c << "//" << endl;
        c << "// Purpose: " << endl;
        c << "//   Returns a pointer to the plugin's Python method table. These methods are" << endl;
        c << "//   added to the top-level visit module's methods." << endl;
        c << "//" << endl;
        c << "// Arguments:" << endl;
        c << "//   nMethods : Returns the number of methods in the method table." << endl;
        c << "//" << endl;
        c << "// Returns:    A pointer to the method table." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// Modifications:" << endl;
        c << "//   " << endl;
        c << "// ****************************************************************************" << endl;
        c << "" << endl;
        c << "void *" << endl;
        c << name << "ScriptingPluginInfo::GetMethodTable(int *nMethods)" << endl;
        c << "{" << endl;
        c << "    return Py"<<atts->name<<"_GetMethodTable(nMethods);" << endl;
        c << "}" << endl;
        c << "" << endl;
        c << "// ****************************************************************************" << endl;
        c << "// Method: " << name << "ScriptingPluginInfo::TypesMatch" << endl;
        c << "//" << endl;
        c << "// Purpose: " << endl;
        c << "//   Returns whether or not the input PyObject is "<<name<<" plot attributes." << endl;
        c << "//" << endl;
        c << "// Arguments:" << endl;
        c << "//   pyobject : A PyObject cast to void*." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// Modifications:" << endl;
        c << "//   " << endl;
        c << "// ****************************************************************************" << endl;
        c << "" << endl;
        c << "bool" << endl;
        c << name << "ScriptingPluginInfo::TypesMatch(void *pyobject)" << endl;
        c << "{" << endl;
        c << "    return Py"<<atts->name<<"_Check((PyObject *)pyobject);" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "// Method: " << name << "ScriptingPluginInfo::GetLogString" << endl;
        c << "//" << endl;
        c << "// Purpose: " << endl;
        c << "//   Gets a string representation of the current attributes." << endl;
        c << "//" << endl;
        c << "// Arguments:" << endl;
        c << "//   val : Whether or not to log state information." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// Modifications:" << endl;
        c << "//   " << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "char *" << endl;
        c << name << "ScriptingPluginInfo::GetLogString()" << endl;
        c << "{" << endl;
        c << "    std::string s(Py" << atts->name << "_GetLogString());" << endl;
        c << "    char *v = new char[s.size() + 1];" << endl;
        c << "    strcpy(v, s.c_str());" << endl;
        c << "    return v;" << endl;
        c << "}" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "// Method: " << name << "ScriptingPluginInfo::SetDefaults" << endl;
        c << "//" << endl;
        c << "// Purpose: " << endl;
        c << "//   Used to set the default values for a plugin's state object." << endl;
        c << "//" << endl;
        c << "// Arguments:" << endl;
        c << "//   atts : The new state." << endl;
        c << "//" << endl;
        c << "//  Programmer: "<<getenv("USER")<<" -- generated by xml2info" << endl;
        c << "//  Creation:   "<<CurrentTime()<< endl;
        c << "//" << endl;
        c << "// Modifications:" << endl;
        c << "//   " << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "void" << endl;
        c << name << "ScriptingPluginInfo::SetDefaults(const AttributeSubject *atts)" << endl;
        c << "{" << endl;
        c << "    Py" << atts->name << "_SetDefaults((const " << atts->name << " *)atts);" << endl;
        c << "}" << endl;
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       InfoGeneratorPlugin

#endif
