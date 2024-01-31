// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GENERATE_INFO_H
#define GENERATE_INFO_H

#include <QTextStream>
#include "Field.h"
#include <visitstream.h>
#include "Plugin.h"


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
//    Cyrus Harrison, Wed Mar  7 09:48:24 PST 2007
//    Allow for engine-specific code in a plugin's source files
//
//    Brad Whitlock, Sat Sep 29 14:49:09 PST 2007
//    Replace getenv("USER") with a safer function so we don't crash on Windows.
//
//   Mark C. Miller, Wed Jul 25 16:42:47 PDT 2007
//   Fixed allocation for ffl in SetupDatabase to use nTimesteps, not nList
//
//   Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//   Renamed GetDefaultExtensions GetDfltExtsFromGen and moved from
//   CommonPluginInfo to GeneralPluginInfo. Renamed GetFilenames to
//   GetFilenamesFromGen and moved to GeneralPluginInfo. This is to support
//   matching plugin by extension/filename without having loaded all the
//   plugins.
//
//   Hank Childs, Thu Jan 10 14:33:30 PST 2008
//   Added filenames, specifiedFilenames.
//
//   Brad Whitlock, Thu Feb 28 16:51:35 PST 2008
//   Made it use a base class.
//
//   Brad Whitlock, Fri Apr 25 10:43:46 PDT 2008
//   Changed code generation to better support internationalization. I also
//   added support for putting user functions in the .code file and having
//   them take effect here.
//
//   Cyrus Harrison, Tue Jul 22 10:27:10 PDT 2008
//   Removed name argument to CreatePluginWizard.
//
//   Cyrus Harrison, Thu Sep 18 13:39:40 PDT 2008
//   Changed code generation to include const qualifers for database info.
//
//   Jeremy Meredith, Wed Nov 19 15:48:21 EST 2008
//   Account for NULL attributes for when an XML file is missing attributes
//   (e.g. a database plugin).
//
//   Kathleen Bonnell, Tue Mar  3 10:41:17 PST 2009
//   Added Permits2DViewScaling & PermitsCurveViewScaling to Viewer plot Info.
//
//   Tom Fogal, Wed Jul  1 23:46:57 MDT 2009
//   Make sure to export interface symbols.
//
//   Hank Childs, Thu Nov 12 15:51:05 PST 2009
//   Removed the specialized function naming from AddMacOSXMacro and made that
//   behavior be used all the time.  Before this change: every plugin had
//   its own VisItVersionString, GetGeneralInfo, etc, and only on Mac's would
//   it be Pseudocolor_VisItVersionString, Pseudocolor_GetGeneralInfo, etc.
//   Now we have Pseudocolor_VisItVersionString, Pseudocolor_GetGeneralInfo,
//   etc., all the time.  This is needed for static builds.
//
//   Jeremy Meredith, Tue Dec 29 11:21:30 EST 2009
//   Replaced "Extensions" and "Filenames" with "FilePatterns".  Added
//   filePatternsStrict and opensWholeDirectory.
//
//   Jeremy Meredith, Thu Jan 28 13:11:07 EST 2010
//   MTSD files can now be grouped not just into a faux MD format by having
//   more than one block, but also into a longer sequence of MT files,
//   each chunk with one or more timesteps.
//
//   Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//   MTMD files can now be grouped into longer sequences.  Similar to the
//   previous change.
//
//   Brad Whitlock, Thu Feb  4 16:11:35 PST 2010
//   I added support for generating a GetCategoryName method for operator plugins.
//
//   Kathleen Bonnell, Wed Sep 29 08:49:20 PDT 2010
//   Added ability to override operator common method 'GetCreatedExpressions.'
//
//   Hank Childs, Mon Jan 31 22:17:51 PST 2011
//   Fix oversight where vectors aren't being generated.
//
//   Brad Whitlock, Mon Nov 21 10:20:04 PST 2011
//   Use a macro to create the plugin version symbol.
//
//   Kathleen Biagas, Wed Nov 20 13:24:29 PST 2013
//   Ensure GetCreatedExpressions logic to use expression vars is applied only
//   when needed.
//
//   Brad Whitlock, Thu Mar 20 14:29:55 PDT 2014
//   I added GetLicense for database plugins.
//
//   Brad Whitlock, Fri Sep 12 12:21:55 PDT 2014
//   Menu name translation for viewer plugins is now done in the viewer.
//
//   Brad Whitlock, Wed Sep 17 09:03:11 PDT 2014
//   Added common  base class for viewer and engine plot/operator plugins.
//
//   Eric Brugger, Wed Dec  4 10:12:23 PST 2019
//   Modified InitializeGlobalObjects to eliminate a memory leak if it is
//   called more than once.
//
//   Kathleen Biagas, Thu Jan  2 09:18:18 PST 2020
//   Added hl arg, for haslicense.
//
// ****************************************************************************

class InfoGeneratorPlugin : public Plugin
{
  public:
    QString generatorName;

    InfoGeneratorPlugin(const QString &n,const QString &l,const QString &t,
        const QString &vt,const QString &dt, const QString &v, const QString &ifile,
        bool hw, bool ho, bool hl, bool onlyengine, bool noengine) :
        Plugin(n,l,t,vt,dt,v,ifile,hw,ho,hl,onlyengine,noengine), generatorName("xml2info")
    {
    }

    virtual ~InfoGeneratorPlugin()
    {
    }

    QString
    GetClassName(const QString &qual) const
    {
        QString retval("none");
        int loc = qual.indexOf("::");
        if(loc != -1)
        {
            retval = qual.left(loc);
        }
        return retval;
    }

    // Returns true if we're replacing a required built-in function and write
    // the definition.
    bool ReplaceBuiltin(QTextStream &c, const QString &funcName)
    {
        if (!atts)
            return false;

        bool retval = false;
        for (unsigned int n=0; n<atts->functions.size(); n++)
        {
            Function *f = atts->functions[n];
            if (!f->user &&
                f->member &&
                f->target == generatorName &&
                f->name == funcName)
            {
                c << atts->functions[n]->def << Endl;
                retval = true;
                break;
            }
        }
        return retval;
    }

    // Returns true if we're going to override an optional base-class method.
    bool OverrideBuiltin(const QString &qualifiedFunctionName) const
    {
        if (!atts)
            return false;

        for (unsigned int i=0; i<atts->functions.size(); i++)
        {
            if (atts->functions[i]->name == qualifiedFunctionName &&
                atts->functions[i]->member &&
                atts->functions[i]->target == generatorName &&
                atts->functions[i]->user == false)
            {
                return true;
            }
        }

        return false;
    }

    // Write the named method override definition.
    void WriteOverrideDefinition(QTextStream &c, const QString &qualifiedFunctionName) const
    {
        if (!atts)
            return;

        for (unsigned int i=0; i<atts->functions.size(); i++)
        {
            if (atts->functions[i]->name == qualifiedFunctionName &&
                atts->functions[i]->member &&
                atts->functions[i]->target == generatorName &&
                atts->functions[i]->user == false)
            {
                c << atts->functions[i]->def << Endl;
                return;
            }
        }
    }

    void WriteUserDefinedFunctions(QTextStream &h, const QString &infoClass, bool writeDecl)
    {
        if (!atts)
            return;

        std::vector<int> publicFuncs, protectedFuncs, privateFuncs;
        for (unsigned int i=0; i<atts->functions.size(); i++)
        {
            QString functionInfoClass(GetClassName(atts->functions[i]->name));
            if(functionInfoClass != infoClass)
                continue;

            if (atts->functions[i]->user &&
                atts->functions[i]->member &&
                atts->functions[i]->target == generatorName)
            {
                if(atts->functions[i]->accessType == Function::AccessPublic)
                    publicFuncs.push_back(i);
                else if(atts->functions[i]->accessType == Function::AccessProtected)
                    protectedFuncs.push_back(i);
                else if(atts->functions[i]->accessType == Function::AccessPrivate)
                    privateFuncs.push_back(i);
            }
        }

        if(writeDecl && (publicFuncs.size() + protectedFuncs.size() + privateFuncs.size()) > 0)
            h << "    // User-defined functions" << Endl;

        if(publicFuncs.size() > 0)
        {
             if(writeDecl)
                 h << "  public:" << Endl;

             for(size_t j = 0; j < publicFuncs.size(); ++j)
             {
                 if(writeDecl)
                     h << "    " << atts->functions[publicFuncs[j]]->decl << Endl;
                 else
                 {
                     h << atts->functions[publicFuncs[j]]->def << Endl;
                 }
            }
        }

        if(protectedFuncs.size() > 0)
        {
             if(writeDecl)
                 h << "  protected:" << Endl;

             for(size_t j = 0; j < protectedFuncs.size(); ++j)
             {
                 if(writeDecl)
                     h << "    " << atts->functions[protectedFuncs[j]]->decl << Endl;
                 else
                 {
                     h << atts->functions[protectedFuncs[j]]->def << Endl;
                 }
            }
        }

        if(privateFuncs.size() > 0)
        {
             if(writeDecl)
                 h << "  private:" << Endl;

             for(size_t j = 0; j < privateFuncs.size(); ++j)
             {
                 if(writeDecl)
                     h << "    " << atts->functions[privateFuncs[j]]->decl << Endl;
                 else
                 {
                     h << atts->functions[privateFuncs[j]]->def << Endl;
                 }
            }
        }
    }

    void WriteInfoHeader(QTextStream &h)
    {
        if (type=="operator")
        {
            h << copyright_str << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  File: "<<name<<"PluginInfo.h" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << Endl;
            h << "#ifndef "<<name.toUpper()<<"_PLUGIN_INFO_H" << Endl;
            h << "#define "<<name.toUpper()<<"_PLUGIN_INFO_H" << Endl;
            h << "#include <OperatorPluginInfo.h>" << Endl;
            h << "#include <operator_plugin_exports.h>" << Endl;
            h << Endl;
            h << "class "<<atts->name<<";" << Endl;
            h << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  Class: "<<name<<"PluginInfo" << Endl;
            h << "//" << Endl;
            h << "//  Purpose:" << Endl;
            h << "//    Five classes that provide all the information about an "<<name<<" operator" << Endl;
            h << "//" << Endl;
            h << "//  Programmer: generated by xml2info" << Endl;
            h << "//  Creation:   omitted"<< Endl;
            h << "//" << Endl;
            h << "//  Modifications:" << Endl;
            h << "//" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << Endl;
            h << "class "<<name<<"GeneralPluginInfo : public virtual GeneralOperatorPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual const char *GetName() const;" << Endl;
            h << "    virtual const char *GetVersion() const;" << Endl;
            h << "    virtual const char *GetID() const;" << Endl;
            h << "    virtual bool  EnabledByDefault() const;" << Endl;
            if(category.length() > 0)
                h << "    virtual const char *GetCategoryName() const;" << Endl;
            h << "};" << Endl;
            h << Endl;

            // CommonPluginInfo
            QString infoName = name + "CommonPluginInfo";
            h << "class "<<infoName<<" : public virtual CommonOperatorPluginInfo, public virtual "<<name<<"GeneralPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual AttributeSubject *AllocAttributes();" << Endl;
            h << "    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);" << Endl;
            if(OverrideBuiltin(infoName + "::GetUserSelectable"))
                h << "    virtual bool GetUserSelectable() const;" << Endl;
            if(OverrideBuiltin(infoName + "::GetVariableTypes"))
                h << "    virtual int GetVariableTypes() const;" << Endl;
            if(OverrideBuiltin(infoName + "::GetVariableMask"))
                h << "    virtual int GetVariableMask() const;" << Endl;
            if(createExpression)
            {
                h << "    virtual ExpressionList *GetCreatedExpressions(const avtDatabaseMetaData *) const;" << Endl;
            }
            h << "};" << Endl;
            h << Endl;

            // GUIPluginInfo
            infoName = name + "GUIPluginInfo";
            h << "class "<<infoName<<" : public virtual GUIOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual QString *GetMenuName() const;" << Endl;
            h << "    virtual QvisPostableWindowObserver *CreatePluginWindow(int type," << Endl;
            h << "        AttributeSubject *attr, const QString &caption, const QString &shortName," << Endl;
            h << "        QvisNotepadArea *notepad);" << Endl;
            if(OverrideBuiltin(infoName + "::CreatePluginWizard"))
            {
                h << "   virtual QvisWizard *CreatePluginWizard(AttributeSubject *attr," << Endl;
                h << "        QWidget *parent);" << Endl;
            }
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;

            // ViewerEnginePluginInfo
            infoName = name + "ViewerEnginePluginInfo";
            h << "class "<<infoName<<" : public virtual ViewerEngineOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual AttributeSubject *GetClientAtts();" << Endl;
            h << "    virtual AttributeSubject *GetDefaultAtts();" << Endl;
            h << "    virtual void SetClientAtts(AttributeSubject *atts);" << Endl;
            h << "    virtual void GetClientAtts(AttributeSubject *atts);" << Endl;
            if(OverrideBuiltin(infoName + "::GetClientAtts3"))
                h << "    virtual void GetClientAtts(AttributeSubject *atts, const bool, const bool);" << Endl;
            h << Endl;
            h << "    virtual void InitializeOperatorAtts(AttributeSubject *atts," << Endl;
            h << "                                        const avtPlotMetaData &plot," << Endl;
            h << "                                        const bool fromDefault);" << Endl;
            h << "    virtual void UpdateOperatorAtts(AttributeSubject *atts," << Endl;
            h << "                                    const avtPlotMetaData &plot);" << Endl;
            if(OverrideBuiltin(infoName + "::GetOperatorVarDescription"))
            {
                h << "    virtual std::string GetOperatorVarDescription(AttributeSubject *atts," << Endl;
                h << "                                                  const avtPlotMetaData &plot);" << Endl;
            }
            h << "    virtual const char *GetMenuName() const;" << Endl;
            if(OverrideBuiltin(infoName + "::Removeable"))
                h << "    virtual bool Removeable() const;" << Endl;
            if(OverrideBuiltin(infoName + "::Moveable"))
                h << "    virtual bool Moveable() const;" << Endl;
            if(OverrideBuiltin(infoName + "::AllowsSubsequentOperators"))
                h << "    virtual bool AllowsSubsequentOperators() const;" << Endl;
            if(OverrideBuiltin(infoName + "::GetCreatedVariables"))
                h << "    virtual ExpressionList *GetCreatedVariables(const char *mesh);" << Endl;
            h << Endl;
            h << "    static void InitializeGlobalObjects();" << Endl;
            h << "  private:" << Endl;
            h << "    static "<<atts->name<<" *defaultAtts;" << Endl;
            h << "    static "<<atts->name<<" *clientAtts;" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;

            // ViewerPluginInfo
            infoName = name + "ViewerPluginInfo";
            h << "class "<<infoName<<" : public virtual ViewerOperatorPluginInfo, public virtual "<<name<<"ViewerEnginePluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;

            // EnginePluginInfo
            infoName = name + "EnginePluginInfo";
            h << "class "<<infoName<<" : public virtual EngineOperatorPluginInfo, public virtual "<<name<<"ViewerEnginePluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual avtPluginFilter *AllocAvtPluginFilter();" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;

            // ScriptingPluginInfo
            infoName = name + "ScriptingPluginInfo";
            h << "class "<<infoName<<" : public virtual ScriptingOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual void InitializePlugin(AttributeSubject *subj, void *data);" << Endl;
            h << "    virtual void *GetMethodTable(int *nMethods);" << Endl;
            h << "    virtual bool TypesMatch(void *pyobject);" << Endl;
            h << "    virtual char *GetLogString();" << Endl;
            h << "    virtual void SetDefaults(const AttributeSubject *atts);" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;
            h << "#endif" << Endl;
        }
        else if (type=="plot")
        {
            h << copyright_str << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << "//  "<<name<<"PluginInfo.h" << Endl;
            h << "// ************************************************************************* //" << Endl;
            h << Endl;
            h << "#ifndef "<<name.toUpper()<<"_PLUGIN_INFO_H" << Endl;
            h << "#define "<<name.toUpper()<<"_PLUGIN_INFO_H" << Endl;
            h << "#include <PlotPluginInfo.h>" << Endl;
            h << "#include <plot_plugin_exports.h>" << Endl;
            h << Endl;
            h << "class "<<atts->name<<";" << Endl;
            h << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  Class: "<<name<<"PluginInfo" << Endl;
            h << "//" << Endl;
            h << "//  Purpose:" << Endl;
            h << "//    Five classes that provide all the information about a "<<name<< Endl;
            h << "//    plot plugin.  The information is broken up into five classes since" << Endl;
            h << "//    portions of it are only relevant to particular components within" << Endl;
            h << "//    visit.  There is the general information which all the components" << Endl;
            h << "//    are interested in, the gui information which the gui is interested in," << Endl;
            h << "//    the viewer information which the viewer is interested in, the" << Endl;
            h << "//    engine information which the engine is interested in, and finally a." << Endl;
            h << "//    scripting portion that enables the Python VisIt extension to use the" << Endl;
            h << "//    plugin." << Endl;
            h << "//" << Endl;
            h << "//  Programmer: generated by xml2info" << Endl;
            h << "//  Creation:   omitted"<< Endl;
            h << "//" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << Endl;

            // GeneralPluginInfo
            h << "class "<<name<<"GeneralPluginInfo: public virtual GeneralPlotPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual const char *GetName() const;" << Endl;
            h << "    virtual const char *GetVersion() const;" << Endl;
            h << "    virtual const char *GetID() const;" << Endl;
            h << "    virtual bool  EnabledByDefault() const;" << Endl;
            h << "};" << Endl;
            h << Endl;
            h << "class "<<name<<"CommonPluginInfo : public virtual CommonPlotPluginInfo, public virtual "<<name<<"GeneralPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual AttributeSubject *AllocAttributes();" << Endl;
            h << "    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);" << Endl;
            h << "    virtual int GetVariableTypes() const;" << Endl;
            h << "};" << Endl;
            h << Endl;

            // GUIPluginInfo
            QString infoName = name + "GUIPluginInfo";
            h << "class "<<infoName<<" : public virtual GUIPlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual QString *GetMenuName() const;" << Endl;
            h << "    virtual QvisPostableWindowObserver *CreatePluginWindow(int type," << Endl;
            h << "        AttributeSubject *attr, const QString &caption, const QString &shortName," << Endl;
            h << "        QvisNotepadArea *notepad);" << Endl;
            if(OverrideBuiltin(infoName + "::CreatePluginWizard"))
            {
                h << "    virtual QvisWizard *CreatePluginWizard(AttributeSubject *attr, QWidget *parent," << Endl;
                h << "        const std::string &varName, const avtDatabaseMetaData *md," << Endl;
                h << "        const ExpressionList *expList);" << Endl;
            }
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;

            // ViewerEnginePluginInfo
            infoName = name + "ViewerEnginePluginInfo";
            h << "class "<<infoName<<" : public virtual ViewerEnginePlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual AttributeSubject *GetClientAtts();" << Endl;
            h << "    virtual AttributeSubject *GetDefaultAtts();" << Endl;
            h << "    virtual void SetClientAtts(AttributeSubject *atts);" << Endl;
            h << "    virtual void GetClientAtts(AttributeSubject *atts);" << Endl;
            h << Endl;
            h << "    virtual avtPlot *AllocAvtPlot();" << Endl;
            h << Endl;
            if(OverrideBuiltin(infoName + "::ProvidesLegend"))
                h << "    virtual bool ProvidesLegend() const;" << Endl;
            if(OverrideBuiltin(infoName + "::PermitsCurveViewScaling"))
                h << "    virtual bool PermitsCurveViewScaling() const;" << Endl;
            if(OverrideBuiltin(infoName + "::Permits2DViewScaling"))
                h << "    virtual bool Permits2DViewScaling() const;" << Endl;
            h << "    virtual void InitializePlotAtts(AttributeSubject *atts, const avtPlotMetaData &);" << Endl;
            if(OverrideBuiltin(infoName + "::ReInitializePlotAtts"))
                h << "    virtual void ReInitializePlotAtts(AttributeSubject *atts, const avtPlotMetaData &);" << Endl;
            if(OverrideBuiltin(infoName + "::ResetPlotAtts"))
                h << "    virtual void ResetPlotAtts(AttributeSubject *atts, const avtPlotMetaData &);" << Endl;

            if(OverrideBuiltin(infoName + "::SupportsAnimation"))
                h << "    virtual bool SupportsAnimation() const;" << Endl;
            if(OverrideBuiltin(infoName + "::AnimationReset"))
                h << "    virtual bool AnimationReset(AttributeSubject *atts, const avtPlotMetaData &plot);" << Endl;
            if(OverrideBuiltin(infoName + "::AnimationStep"))
                h << "    virtual bool AnimationStep(AttributeSubject *atts, const avtPlotMetaData  &plot);" << Endl;

            h << "    virtual const char *GetMenuName() const;" << Endl;
            h << "    static void InitializeGlobalObjects();" << Endl;
            h << "  private:" << Endl;
            h << "    static "<<atts->name<<" *defaultAtts;" << Endl;
            h << "    static "<<atts->name<<" *clientAtts;" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;

            // ViewerPluginInfo
            infoName = name + "ViewerPluginInfo";
            h << "class "<<infoName<<" : public virtual ViewerPlotPluginInfo, public virtual "<<name<<"ViewerEnginePluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << Endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayCreate"))
                h << "    virtual void *AlternateDisplayCreate(ViewerPlot *plot);" << Endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayDestroy"))
                h << "    virtual void AlternateDisplayDestroy(void *dpy);" << Endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayClear"))
                h << "    virtual void AlternateDisplayClear(void *dpy);" << Endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayHide"))
                h << "    virtual void AlternateDisplayHide(void *dpy);" << Endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayShow"))
                h << "    virtual void AlternateDisplayShow(void *dpy);" << Endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayIconify"))
                h << "    virtual void AlternateDisplayIconify(void *dpy);" << Endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayDeIconify"))
                h << "    virtual void AlternateDisplayDeIconify(void *dpy);" << Endl;
            h << Endl;
            //h << "  private:" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;

            // EnginePluginInfo
            infoName = name + "EnginePluginInfo";
            h << "class "<<infoName<<" : public virtual EnginePlotPluginInfo, public virtual "<<name<<"ViewerEnginePluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;

            // ScriptingPluginInfo
            infoName = name + "ScriptingPluginInfo";
            h << "class "<<infoName<<" : public virtual ScriptingPlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual void InitializePlugin(AttributeSubject *subj, void *data);" << Endl;
            h << "    virtual void *GetMethodTable(int *nMethods);" << Endl;
            h << "    virtual bool TypesMatch(void *pyobject);" << Endl;
            h << "    virtual char *GetLogString();" << Endl;
            h << "    virtual void SetDefaults(const AttributeSubject *atts);" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << Endl;
            h << "#endif" << Endl;
        }
        else if (type=="database")
        {
            h << copyright_str << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  "<<name<<"PluginInfo.h" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "" << Endl;
            h << "#ifndef "<<name.toUpper()<<"_PLUGIN_INFO_H" << Endl;
            h << "#define "<<name.toUpper()<<"_PLUGIN_INFO_H" << Endl;
            h << "#include <DatabasePluginInfo.h>" << Endl;
            h << "#include <database_plugin_exports.h>" << Endl;
            h << "" << Endl;
            h << "class avtDatabase;" << Endl;
            h << "class avtDatabaseWriter;" << Endl;
            h << "" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "//  Class: "<<name<<"DatabasePluginInfo" << Endl;
            h << "//" << Endl;
            h << "//  Purpose:" << Endl;
            h << "//    Classes that provide all the information about the "<<name<<" plugin." << Endl;
            h << "//    Portions are separated into pieces relevant to the appropriate" << Endl;
            h << "//    components of VisIt." << Endl;
            h << "//" << Endl;
            h << "//  Programmer: generated by xml2info" << Endl;
            h << "//  Creation:   omitted" << Endl;
            h << "//" << Endl;
            h << "//  Modifications:" << Endl;
            h << "//" << Endl;
            h << "// ****************************************************************************" << Endl;
            h << "" << Endl;
            h << "class "<<name<<"GeneralPluginInfo : public virtual GeneralDatabasePluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual const char *GetName() const;" << Endl;
            h << "    virtual const char *GetVersion() const;" << Endl;
            h << "    virtual const char *GetID() const;" << Endl;
            h << "    virtual bool  EnabledByDefault() const;" << Endl;
            h << "    virtual bool  HasWriter() const;" << Endl;
            h << "    virtual std::vector<std::string> GetDefaultFilePatterns() const;" << Endl;
            h << "    virtual bool  AreDefaultFilePatternsStrict() const;" << Endl;
            h << "    virtual bool  OpensWholeDirectory() const;" << Endl;
            h << "};" << Endl;
            h << "" << Endl;
            h << "class "<<name<<"CommonPluginInfo : public virtual CommonDatabasePluginInfo, public virtual "<<name<<"GeneralPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual DatabaseType              GetDatabaseType();" << Endl;
            h << "    virtual avtDatabase              *SetupDatabase(const char * const *list," << Endl;
            h << "                                                    int nList, int nBlock);" << Endl;
            if (hasoptions)
            {
                h << "    virtual DBOptionsAttributes      *GetReadOptions() const;"
                  << Endl;
                h << "    virtual DBOptionsAttributes      *GetWriteOptions() const;"
                  << Endl;
                QString infoName = name + "CommonPluginInfo";
                if(OverrideBuiltin(infoName + "::SetReadOptions"))
                    h << "    virtual void                          SetReadOptions(DBOptionsAttributes *);" << Endl;
            }
            if (haslicense)
            {
                h << "    virtual std::string               GetLicense() const;" << Endl;
            }
            h << "};" << Endl;
            h << "" << Endl;
            QString infoName = name + "MDServerPluginInfo";
            h << "class "<<infoName<<" : public virtual MDServerDatabasePluginInfo, public virtual "<<name<<"CommonPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    // this makes compilers happy... remove if we ever have functions here" << Endl;
            h << "    virtual void dummy();" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << "" << Endl;
            infoName = name + "EnginePluginInfo";
            h << "class "<<infoName<<" : public virtual EngineDatabasePluginInfo, public virtual "<<name<<"CommonPluginInfo" << Endl;
            h << "{" << Endl;
            h << "  public:" << Endl;
            h << "    virtual avtDatabaseWriter        *GetWriter(void);" << Endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << Endl;
            h << "" << Endl;
            h << "#endif" << Endl;
        }
    }
    // Gives the macro which causes symbol export to occur.
    QString Export(const std::string& tp)
    {
        if(tp == "database") { return "DBP_EXPORT"; }
        if(tp == "plot") { return "PLOT_EXPORT"; }
        if(tp == "operator") { return "OP_EXPORT"; }
        return "";
    }
    void AddVersion(QTextStream &c)
    {
        c << "#include <visit-config.h>" << Endl;
        c << "VISIT_PLUGIN_VERSION("<<name<<","<<Export(type.toStdString()) << ")" << Endl;
        c << Endl;
    }
    void WriteInfoSource(QTextStream &c)
    {
        c << copyright_str << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  File: "<<name<<"PluginInfo.C" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "#include <"<<name<<"PluginInfo.h>" << Endl;
        if (type!="database")
            c << "#include <"<<atts->name<<".h>" << Endl;
        c << Endl;
        AddVersion(c);
        if (type=="operator")
            c << "VISIT_OPERATOR_PLUGIN_ENTRY(" << name << ",General)" << Endl;
        else if (type=="plot")
            c << "VISIT_PLOT_PLUGIN_ENTRY(" << name << ",General)" << Endl;
        else if (type=="database")
            c << "VISIT_DATABASE_PLUGIN_ENTRY(" << name << ",General)" << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: "<<name<<"GeneralPluginInfo::GetName" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//    Return the name of the "<<type<<" plugin." << Endl;
        c << "//" << Endl;
        c << "//  Returns:    A pointer to the name of the "<<type<<" plugin." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: generated by xml2info" << Endl;
        c << "//  Creation:   omitted"<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "const char *" << Endl;
        c << name<<"GeneralPluginInfo::GetName() const" << Endl;
        c << "{" << Endl;
        c << "    return \""<<name<<"\";" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: "<<name<<"GeneralPluginInfo::GetVersion" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//    Return the version of the "<<type<<" plugin." << Endl;
        c << "//" << Endl;
        c << "//  Returns:    A pointer to the version of the "<<type<<" plugin." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: generated by xml2info" << Endl;
        c << "//  Creation:   omitted"<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "const char *" << Endl;
        c << name<<"GeneralPluginInfo::GetVersion() const" << Endl;
        c << "{" << Endl;
        c << "    return \""<<version<<"\";" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: "<<name<<"GeneralPluginInfo::GetID" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//    Return the id of the "<<type<<" plugin." << Endl;
        c << "//" << Endl;
        c << "//  Returns:    A pointer to the id of the "<<type<<" plugin." << Endl;
        c << "//" << Endl;
        c << "//  Programmer: generated by xml2info" << Endl;
        c << "//  Creation:   omitted"<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "const char *" << Endl;
        c << name<<"GeneralPluginInfo::GetID() const" << Endl;
        c << "{" << Endl;
        c << "    return \""<<name<<"_"<<version<<"\";" << Endl;
        c << "}" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  Method: "<<name<<"GeneralPluginInfo::EnabledByDefault" << Endl;
        c << "//" << Endl;
        c << "//  Purpose:" << Endl;
        c << "//    Return true if this plugin should be enabled by default; false otherwise." << Endl;
        c << "//" << Endl;
        c << "//  Returns:    true/false" << Endl;
        c << "//" << Endl;
        c << "//  Programmer: generated by xml2info" << Endl;
        c << "//  Creation:   omitted"<< Endl;
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "bool" << Endl;
        c << name<<"GeneralPluginInfo::EnabledByDefault() const" << Endl;
        c << "{" << Endl;
        c << "    return "<<Bool2Text(enabledByDefault)<<";" << Endl;
        c << "}" << Endl;

        if(type == "operator" && category.length() > 0)
        {
            QString funcName(name + "GeneralPluginInfo::GetCategoryName");
            c << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: "<<funcName << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Return the category name to which the operator belongs." << Endl;
            c << "//" << Endl;
            c << "//  Returns:    Return the category name to which the operator belongs." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "const char *" << Endl;
            c << funcName<<"() const" << Endl;
            c << "{" << Endl;
            c << "    return \"" << category << "\";" << Endl;
            c << "}" << Endl;
        }

        if (type=="database")
        {
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: "<<name<<"GeneralPluginInfo::HasWriter" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Return true if this plugin has a database writer." << Endl;
            c << "//" << Endl;
            c << "//  Returns:    true/false" << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "bool" << Endl;
            c << name<<"GeneralPluginInfo::HasWriter() const" << Endl;
            c << "{" << Endl;
            c << "    return "<<Bool2Text(haswriter)<<";" << Endl;
            c << "}" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method:  "<<name<<"GeneralPluginInfo::GetDefaultFilePatterns" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Returns the default patterns for a "<<name<<" database." << Endl;
            c << "//" << Endl;
            c << "//  Programmer:  generated by xml2info" << Endl;
            c << "//  Creation:    omitted" << Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "std::vector<std::string>" << Endl;
            c << ""<<name<<"GeneralPluginInfo::GetDefaultFilePatterns() const" << Endl;
            c << "{" << Endl;
            c << "    std::vector<std::string> defaultPatterns;" << Endl;
            for (size_t i=0; i<filePatterns.size(); i++)
            {
                c << "    defaultPatterns.push_back(\""<<filePatterns[i]<<"\");" << Endl;
            }
            c << "" << Endl;
            c << "    return defaultPatterns;" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method:  "<<name<<"GeneralPluginInfo::AreDefaultFilePatternsStrict" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Returns if the file patterns for a "<<name<<" database are" << Endl;
            c << "//    intended to be interpreted strictly by default." << Endl;
            c << "//" << Endl;
            c << "//  Programmer:  generated by xml2info" << Endl;
            c << "//  Creation:    omitted" << Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "bool" << Endl;
            c << ""<<name<<"GeneralPluginInfo::AreDefaultFilePatternsStrict() const" << Endl;
            c << "{" << Endl;
            c << "    return "<<(filePatternsStrict ? "true" : "false")<<";" << Endl;
            c << "}" << Endl;
            c << "" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  Method:  "<<name<<"GeneralPluginInfo::OpensWholeDirectory" << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Returns if the "<<name<<" plugin opens a whole directory name" << Endl;
            c << "//    instead of a single file." << Endl;
            c << "//" << Endl;
            c << "//  Programmer:  generated by xml2info" << Endl;
            c << "//  Creation:    omitted" << Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "bool" << Endl;
            c << ""<<name<<"GeneralPluginInfo::OpensWholeDirectory() const" << Endl;
            c << "{" << Endl;
            c << "    return "<<(opensWholeDirectory ? "true" : "false")<<";" << Endl;
            c << "}" << Endl;
        }
    }
    void AddExpressionFromMD(QTextStream &c, QString opName, std::vector<QString> outtypes, QString typeToLookForS, QString typeToLookForP)
    {
        c << "    int num" << typeToLookForP << " = md->GetNum" << typeToLookForP << "();" << Endl;
        c << "    for (int i = 0; i < num" << typeToLookForP << "; i++)" << Endl;
        c << "    {" << Endl;
        c << "        const avt" << typeToLookForS << "MetaData *mmd = md->Get" << typeToLookForS << "(i);" << Endl;
        c << "        if (mmd->hideFromGUI || !mmd->validVariable)" << Endl;
        c << "            continue;" << Endl;
        for (size_t j = 0 ; j < outtypes.size() ; j++)
        {
            c << "        {" << Endl;
            c << "            Expression e2;" << Endl;
            c << "            sprintf(name, \"operators/" << opName << "/%s\", mmd->name.c_str());" << Endl;
            c << "            e2.SetName(name);" << Endl;
            c << "            e2.SetType(Expression::" << outtypes[j] << ");" << Endl;
            c << "            e2.SetFromOperator(true);" << Endl;
            c << "            e2.SetOperatorName(\"" << opName << "\");" << Endl;
            c << "            sprintf(defn, \"cell_constant(<%s>, 0.)\", mmd->name.c_str());" << Endl;
            c << "            e2.SetDefinition(defn);" << Endl;
            c << "            el->AddExpressions(e2);" << Endl;
            c << "        }" << Endl;
        }
        c << "    }" << Endl;
    }
    void AddExpressionFromExpr(QTextStream &c, QString opName, std::vector<QString> outtypes, QString exprType)
    {
        c << "        if (e.GetType() == Expression::" << exprType << ")" << Endl;
        c << "        {" << Endl;
        for (size_t j = 0 ; j < outtypes.size() ; j++)
        {
            c << "            {" << Endl;
            c << "                if (e.GetFromOperator() || e.GetAutoExpression())" << Endl;
            c << "                    continue; // weird ordering behavior otherwise" << Endl;
            c << "                Expression e2;" << Endl;
            c << "                sprintf(name, \"operators/" << opName << "/%s\", e.GetName().c_str());" << Endl;
            c << "                e2.SetName(name);" << Endl;
            c << "                e2.SetType(Expression::" << outtypes[j] << ");" << Endl;
            c << "                e2.SetFromOperator(true);" << Endl;
            c << "                e2.SetOperatorName(\"" << opName << "\");" << Endl;
            c << "                sprintf(defn, \"cell_constant(<%s>, 0.)\", e.GetName().c_str());" << Endl;
            c << "                e2.SetDefinition(defn);" << Endl;
            c << "                el->AddExpressions(e2);" << Endl;
            c << "            }" << Endl;
        }
        c << "        }" << Endl;
    }

    void WriteCommonInfoSource(QTextStream &c)
    {
        if (type=="database")
        {
            c << copyright_str << Endl;
            c << "#include <"<<name<<"PluginInfo.h>" << Endl;
            if (dbtype != "Custom")
            {
                c << "#include <avt"<<name<<"FileFormat.h>" << Endl;
                c << "#include <avt"<<dbtype<<"FileFormatInterface.h>" << Endl;
                c << "#include <avtGenericDatabase.h>" << Endl;
            }
            else
            {
                c << "#include <avt"<<name<<"Database.h>" << Endl;
            }
            if (hasoptions)
                c << "#include <avt"<<name<<"Options.h>" << Endl;
            c << "" << Endl;

            // Write GetDatabaseType
            QString funcName(name + "CommonPluginInfo::GetDatabaseType");
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method:  "<<funcName<< Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Returns the type of a "<<name<<" database." << Endl;
                c << "//" << Endl;
                c << "//  Programmer:  generated by xml2info" << Endl;
                c << "//  Creation:    omitted" << Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << "DatabaseType" << Endl;
                c << ""<<funcName<<"()" << Endl;
                c << "{" << Endl;
                c << "    return DB_TYPE_"<<dbtype.toUpper()<<";" << Endl;
                c << "}" << Endl;
            }
            c << Endl;
            // Write SetupDatabase
            funcName = name + "CommonPluginInfo::SetupDatabase";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//      Sets up a "<<name<<" database." << Endl;
                c << "//" << Endl;
                c << "//  Arguments:" << Endl;
                c << "//      list    A list of file names." << Endl;
                c << "//      nList   The number of timesteps in list." << Endl;
                c << "//      nBlocks The number of blocks in the list." << Endl;
                c << "//" << Endl;
                c << "//  Returns:    A "<<name<<" database from list." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted" << Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << "avtDatabase *" << Endl;
                c << ""<<funcName<<"(const char *const *list," << Endl;
                c << "                                   int nList, int nBlock)" << Endl;
                c << "{" << Endl;
                if (dbtype == "Custom")
                {
                    c << "    return new avt"<<name<<"Database(list[0]);" << Endl;
                }
                else if (dbtype == "STSD")
                {
                    c << "    int nTimestep = nList / nBlock;" << Endl;
                    c << "    avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[nTimestep];" << Endl;
                    c << "    for (int i = 0; i < nTimestep; i++)" << Endl;
                    c << "    {" << Endl;
                    c << "        ffl[i] = new avtSTSDFileFormat*[nBlock];" << Endl;
                    c << "        for (int j = 0; j < nBlock; j++)" << Endl;
                    c << "        {" << Endl;
                    if (hasoptions)
                        c << "            ffl[i][j] = new avt"<<name<<"FileFormat(list[i*nBlock + j], readOptions);" << Endl;
                    else
                        c << "            ffl[i][j] = new avt"<<name<<"FileFormat(list[i*nBlock + j]);" << Endl;
                    c << "        }" << Endl;
                    c << "    }" << Endl;
                    c << "    avtSTSDFileFormatInterface *inter" << Endl;
                    c << "           = new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);" << Endl;
                    c << "    return new avtGenericDatabase(inter);" << Endl;
                }
                else if (dbtype == "STMD")
                {
                    c << "    avtSTMDFileFormat **ffl = new avtSTMDFileFormat*[nList];" << Endl;
                    c << "    for (int i = 0; i < nList; i++)" << Endl;
                    c << "    {" << Endl;
                    if (hasoptions)
                        c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i], readOptions);" << Endl;
                    else
                        c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i]);" << Endl;
                    c << "    }" << Endl;
                    c << "    avtSTMDFileFormatInterface *inter" << Endl;
                    c << "           = new avtSTMDFileFormatInterface(ffl, nList);" << Endl;
                    c << "    return new avtGenericDatabase(inter);" << Endl;
                }
                else if (dbtype == "MTSD")
                {
                    c << "    int nTimestepGroups = nList / nBlock;" << Endl;
                    c << "    avtMTSDFileFormat ***ffl = new avtMTSDFileFormat**[nTimestepGroups];" << Endl;
                    c << "    for (int i = 0; i < nTimestepGroups; i++)" << Endl;
                    c << "    {" << Endl;
                    c << "        ffl[i] = new avtMTSDFileFormat*[nBlock];" << Endl;
                    c << "        for (int j = 0; j < nBlock; j++)" << Endl;
                    c << "        {" << Endl;
                    if (hasoptions)
                        c << "            ffl[i][j] = new avt"<<name<<"FileFormat(list[i*nBlock + j], readOptions);" << Endl;
                    else
                        c << "            ffl[i][j] = new avt"<<name<<"FileFormat(list[i*nBlock + j]);" << Endl;
                    c << "        }" << Endl;
                    c << "    }" << Endl;
                    c << "    avtMTSDFileFormatInterface *inter" << Endl;
                    c << "           = new avtMTSDFileFormatInterface(ffl, nTimestepGroups, nBlock);" << Endl;
                    c << "    return new avtGenericDatabase(inter);" << Endl;
                }
                else if (dbtype == "MTMD")
                {
                    c << "    // ignore any nBlocks past 1" << Endl;
                    c << "    int nTimestepGroups = nList / nBlock;" << Endl;
                    c << "    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];" << Endl;
                    c << "    for (int i = 0; i < nTimestepGroups; i++)" << Endl;
                    c << "    {" << Endl;
                    if (hasoptions)
                        c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i*nBlock], readOptions);" << Endl;
                    else
                        c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i*nBlock]);" << Endl;
                    c << "    }" << Endl;
                    c << "    avtMTMDFileFormatInterface *inter" << Endl;
                    c << "           = new avtMTMDFileFormatInterface(ffl, nTimestepGroups);" << Endl;
                    c << "    return new avtGenericDatabase(inter);" << Endl;
                }
                else
                {
                    throw QString("Unknown database type '%1'").arg(dbtype);
                }
                c << "}" << Endl;
            }

            if (hasoptions)
            {
                // Write GetReadOptions
                funcName = name + "CommonPluginInfo::GetReadOptions";
                if(!ReplaceBuiltin(c, funcName))
                {
                    c << Endl;
                    c << "// ****************************************************************************" << Endl;
                    c << "//  Method: "<<funcName << Endl;
                    c << "//" << Endl;
                    c << "//  Purpose:" << Endl;
                    c << "//      Gets the read options." << Endl;
                    c << "//" << Endl;
                    c << "//  Programmer: generated by xml2info" << Endl;
                    c << "//  Creation:   omitted"<< Endl;
                    c << "//" << Endl;
                    c << "// ****************************************************************************" << Endl;
                    c << Endl;
                    c << "DBOptionsAttributes *" << Endl;
                    c << funcName<<"() const" << Endl;
                    c << "{" << Endl;
                    c << "    return Get"<<name<<"ReadOptions();" << Endl;
                    c << "}" << Endl;
                }
                funcName = name + "CommonPluginInfo::GetWriteOptions";
                if(!ReplaceBuiltin(c, funcName))
                {
                    c << Endl;
                    c << "// ****************************************************************************" << Endl;
                    c << "//  Method: "<<funcName << Endl;
                    c << "//" << Endl;
                    c << "//  Purpose:" << Endl;
                    c << "//      Gets the write options." << Endl;
                    c << "//" << Endl;
                    c << "//  Programmer: generated by xml2info" << Endl;
                    c << "//  Creation:   omitted"<< Endl;
                    c << "//" << Endl;
                    c << "// ****************************************************************************" << Endl;
                    c << Endl;
                    c << "DBOptionsAttributes *" << Endl;
                    c << funcName << "() const" << Endl;
                    c << "{" << Endl;
                    c << "    return Get"<<name<<"WriteOptions();" << Endl;
                    c << "}" << Endl;
                }
                c << Endl;
                funcName = name + "CommonPluginInfo::SetReadOptions";
                ReplaceBuiltin(c, funcName);
            }
            if(haslicense)
            {
                c << Endl;
                funcName = name + "CommonPluginInfo::GetLicense";
                ReplaceBuiltin(c, funcName);
            }
        }
        else
        {
            c << copyright_str << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  File: "<<name<<"CommonPluginInfo.C" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "#include <"<<name<<"PluginInfo.h>" << Endl;
            c << "#include <"<<atts->name<<".h>" << Endl;
            c << Endl;
            if (type=="operator" && createExpression)
            {
                c << "#include <Expression.h>" << Endl;
                c << "#include <ExpressionList.h>" << Endl;
                c << "#include <avtDatabaseMetaData.h>" << Endl;
                std::vector<QString> intypes = SplitValues(exprInType);
                for (size_t i = 0; i < intypes.size(); i++)
                {
                    if (intypes[i] == "mesh")
                        c << "#include <avtMeshMetaData.h>" << Endl;
                    else if (intypes[i] == "scalar")
                        c << "#include <avtScalarMetaData.h>" << Endl;
                    else if (intypes[i] == "vector")
                        c << "#include <avtVectorMetaData.h>" << Endl;
                    else if (intypes[i] == "material")
                        c << "#include <avtMaterialMetaData.h>" << Endl;
                    else if (intypes[i] == "subset")
                        c << "#include <avtSubsetsMetaData.h>" << Endl;
                    else if (intypes[i] == "species")
                        c << "#include <avtSpeciesMetaData.h>" << Endl;
                    else if (intypes[i] == "curve")
                        c << "#include <avtCurveMetaData.h>" << Endl;
                    else if (intypes[i] == "tensor")
                        c << "#include <avtTensorMetaData.h>" << Endl;
                    else if (intypes[i] == "symmetrictensor")
                        c << "#include <avtSymmetricTensorMetaData.h>" << Endl;
                    else if (intypes[i] == "label")
                        c << "#include <avtLabelMetaData.h>" << Endl;
                    else if (intypes[i] == "array")
                        c << "#include <avtArrayMetaData.h>" << Endl;
                }
                c << Endl;
            }
            QString funcName = name + "CommonPluginInfo::AllocAttributes";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Return a pointer to a newly allocated attribute subject." << Endl;
                c << "//" << Endl;
                c << "//  Returns:    A pointer to the newly allocated attribute subject." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "AttributeSubject *" << Endl;
                c << funcName << "()" << Endl;
                c << "{" << Endl;
                c << "    return new "<<atts->name<<";" << Endl;
                c << "}" << Endl;
            }
            c << Endl;
            funcName = name + "CommonPluginInfo::CopyAttributes";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Copy a "<<name<<" attribute subject." << Endl;
                c << "//" << Endl;
                c << "//  Arguments:" << Endl;
                c << "//    to        The destination attribute subject." << Endl;
                c << "//    from      The source attribute subject." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "void" << Endl;
                c << funcName<<"(AttributeSubject *to," << Endl;
                c << "    AttributeSubject *from)" << Endl;
                c << "{" << Endl;
                c << "    *(("<<atts->name<<" *) to) = *(("<<atts->name<<" *) from);" << Endl;
                c << "}" << Endl;
            }
            funcName = name + "CommonPluginInfo::GetCreatedExpressions";
            if (createExpression && !ReplaceBuiltin(c, funcName))
            {
                c << Endl;
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//      Gets the expressions created by this operator." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "ExpressionList *" << Endl;
                c << funcName<<"(const avtDatabaseMetaData *md) const" << Endl;
                c << "{" << Endl;
                std::vector<QString> intypes = SplitValues(exprInType);
                bool doMesh = false, doScalar = false, doVector = false, doTensor = false, doMaterial = false;
                bool doSubset = false, doSpecies = false, doCurve = false, doSymmTensor = false;
                bool doLabel = false, doArray = false;
                for (size_t i = 0; i < intypes.size(); i++)
                {
                    if (intypes[i] == "mesh")
                        doMesh = true;
                    if (intypes[i] == "scalar")
                        doScalar = true;
                    if (intypes[i] == "vector")
                        doVector = true;
                    if (intypes[i] == "material")
                        doMaterial = true;
                    if (intypes[i] == "subset")
                        doSubset = true;
                    if (intypes[i] == "species")
                        doSpecies = true;
                    if (intypes[i] == "curve")
                        doCurve = true;
                    if (intypes[i] == "tensor")
                        doTensor = true;
                    if (intypes[i] == "symmetrictensor")
                        doSymmTensor = true;
                    if (intypes[i] == "label")
                        doLabel = true;
                    if (intypes[i] == "array")
                        doArray = true;
                }
                std::vector<QString> outtypes = SplitValues(exprOutType);
                std::vector<QString> outExprTypes;
                bool haveScalar = false, haveTensor = false;
                for (size_t i = 0; i < outtypes.size(); i++)
                {
                    if (outtypes[i] == "mesh")
                        cerr << "Mesh expressions not currently supported." << endl;
                    if (outtypes[i] == "scalar")
                    {
                        if (!haveScalar)
                            outExprTypes.push_back("ScalarMeshVar");
                        haveScalar = true;
                    }
                    if (outtypes[i] == "vector")
                        outExprTypes.push_back("VectorMeshVar");
                    if (outtypes[i] == "material")
                        outExprTypes.push_back("Material");
                    if (outtypes[i] == "subset")
                        cerr << "Subset expressions not currently supported." << endl;
                    if (outtypes[i] == "species")
                        outExprTypes.push_back("Species");
                    if (outtypes[i] == "curve")
                        outExprTypes.push_back("CurveMeshVar");
                    if (outtypes[i] == "tensor")
                    {
                        if (!haveTensor)
                            outExprTypes.push_back("TensorMeshVar");
                        haveTensor = true;
                    }
                    if (outtypes[i] == "symmetrictensor")
                    {
                        if (!haveTensor)
                            outExprTypes.push_back("TensorMeshVar");
                        haveTensor = true;
                    }
                    if (outtypes[i] == "label")
                    {
                        if (!haveScalar)
                            outExprTypes.push_back("ScalarMeshVar");
                        haveScalar = true;
                    }
                    if (outtypes[i] == "array")
                        outExprTypes.push_back("ArrayMeshVar");
                }
                c << "    char name[1024], defn[1024];" << Endl;
                c << "    ExpressionList *el = new ExpressionList;" << Endl;
                if (doMesh)
                    AddExpressionFromMD(c, name, outExprTypes, QString("Mesh"), QString("Meshes"));
                if (doScalar)
                    AddExpressionFromMD(c, name, outExprTypes, QString("Scalar"), QString("Scalars"));
                if (doVector)
                    AddExpressionFromMD(c, name, outExprTypes, QString("Vector"), QString("Vectors"));
                if (doTensor || doSymmTensor)
                    AddExpressionFromMD(c, name, outExprTypes, QString("Tensor"), QString("Tensors"));
                if (doMaterial || doSubset || doSpecies || doCurve || doLabel || doArray)
                    cerr << "Unsupported variable type for creating an expression.  Contact a VisIt developer." << endl;

                if (doScalar || doVector || doTensor || doSymmTensor)
                {
                    c << "    const ExpressionList &oldEL = md->GetExprList();" << Endl;
                    c << "    for (int i = 0; i < oldEL.GetNumExpressions(); i++)" << Endl;
                    c << "    {" << Endl;
                    c << "        const Expression &e = oldEL.GetExpressions(i);" << Endl;
                    if (doScalar)
                        AddExpressionFromExpr(c, name, outExprTypes, QString("ScalarMeshVar"));
                    if (doVector)
                        AddExpressionFromExpr(c, name, outExprTypes, QString("VectorMeshVar"));
                    if (doTensor || doSymmTensor)
                        AddExpressionFromExpr(c, name, outExprTypes, QString("TensorMeshVar"));
                    c << "    }" << Endl;
                }
                c << "    return el;" << Endl;
                c << "}" << Endl;
                c << Endl;
            }

            if (type=="plot")
            {
                funcName = name + "CommonPluginInfo::GetVariableTypes";
                if(!ReplaceBuiltin(c, funcName))
                {
                    c << Endl;
                    c << "// ****************************************************************************" << Endl;
                    c << "// Method: "<<funcName << Endl;
                    c << "//" << Endl;
                    c << "// Purpose:" << Endl;
                    c << "//   Returns a flag indicating the types of variables that can be put in the" << Endl;
                    c << "//   plot's variable list." << Endl;
                    c << "//" << Endl;
                    c << "// Returns:    A flag indicating the types of variables that can be put in" << Endl;
                    c << "//             the plot's variable list." << Endl;
                    c << "//" << Endl;
                    c << "//  Programmer: generated by xml2info" << Endl;
                    c << "//  Creation:   omitted"<< Endl;
                    c << "//" << Endl;
                    c << "// Modifications:" << Endl;
                    c << "//" << Endl;
                    c << "// ****************************************************************************" << Endl;
                    c << Endl;
                    c << "int" << Endl;
                    c << funcName<<"() const" << Endl;
                    c << "{" << Endl;
                    c << "    return ";
                    std::vector<QString> types = SplitValues(vartype);
                    for (size_t i=0; i<types.size(); i++)
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
                    c << ";" << Endl;
                    c << "}" << Endl;
                    c << Endl;
                }
            }

            if (type == "operator")
            {
                WriteOverrideDefinition(c, name + "CommonPluginInfo::GetVariableTypes");
                WriteOverrideDefinition(c, name + "CommonPluginInfo::GetVariableMask");
                WriteOverrideDefinition(c, name + "CommonPluginInfo::GetUserSelectable");
            }

        }
    }

    void WriteGUIInfoSource(QTextStream &c)
    {
        if (type=="database")
            return;

        c << copyright_str << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  File: "<<name<<"GUIPluginInfo.C" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "#include <"<<name<<"PluginInfo.h>" << Endl;
        c << "#include <"<<atts->name<<".h>" << Endl;
        c << "#include <QApplication>" << Endl;
        if (type=="operator")
            c << "#include <Qvis"<<name<<"Window.h>" << Endl;
        else if (type=="plot")
            c << "#include <Qvis"<<name<<"PlotWindow.h>" << Endl;
        c << Endl;
        if (type=="operator")
            c << "VISIT_OPERATOR_PLUGIN_ENTRY(" << name << ",GUI)" << Endl;
        else if (type=="plot")
            c << "VISIT_PLOT_PLUGIN_ENTRY(" << name << ",GUI)" << Endl;
        c << Endl;
        QString funcName = name + "GUIPluginInfo::GetMenuName";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: "<<funcName << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Return a pointer to the name to use in the GUI menu." << Endl;
            c << "//" << Endl;
            c << "//  Returns:    A pointer to the name to use in the GUI menu." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "QString *" << Endl;
            c << funcName<<"() const" << Endl;
            c << "{" << Endl;
            if(type == "plot")
                c << "    return new QString(qApp->translate(\"PlotNames\", \""<<label<<"\"));" << Endl;
            else if(type == "operator")
                c << "    return new QString(qApp->translate(\"OperatorNames\", \""<<label<<"\"));" << Endl;
            c << "}" << Endl;
            c << Endl;
        }
        c << Endl;
        funcName = name + "GUIPluginInfo::CreatePluginWindow";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: "<<funcName << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Return a pointer to an "<<type<<"'s attribute window." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//    type      The type of the "<<type<<"." << Endl;
            c << "//    attr      The attribute subject for the "<<type<<"." << Endl;
            c << "//    notepad   The notepad to use for posting the window." << Endl;
            c << "//" << Endl;
            c << "//  Returns:    A pointer to the "<<type<<"'s attribute window." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "QvisPostableWindowObserver *" << Endl;
            c << funcName << "(int type, AttributeSubject *attr," << Endl;
            c << "    const QString &caption, const QString &shortName, QvisNotepadArea *notepad)" << Endl;
            c << "{" << Endl;
            if (type=="operator")
                c << "    return new Qvis"<<name<<"Window(type, ("<<atts->name<<" *)attr," << Endl
                  << "        caption, shortName, notepad);" << Endl;
            else if (type=="plot")
                c << "    return new Qvis"<<name<<"PlotWindow(type, ("<<atts->name<<" *)attr," << Endl
                  << "        caption, shortName, notepad);" << Endl;
            c << "}" << Endl;
        }

        WriteOverrideDefinition(c, name + "GUIPluginInfo::CreatePluginWizard");

        if(iconFile.length() > 0)
        {
            funcName = name + "GUIPluginInfo::XPMIconData";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << Endl;
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Return a pointer to the icon data." << Endl;
                c << "//" << Endl;
                c << "//  Returns:    A pointer to the icon data." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "#include <" << iconFile << ">" << Endl;
                c << "const char **" << Endl;
                c << funcName << "() const" << Endl;
                c << "{" << Endl;
                c << "    return " << iconFile.left(iconFile.length() - 4) << "_xpm;" << Endl;
                c << "}" << Endl;
            }
        }

        c << Endl;
        WriteUserDefinedFunctions(c, name + "GUIPluginInfo", false);
    }

    void WriteViewerInfoSource(QTextStream &c)
    {
        if (type=="database")
            return;

        c << copyright_str << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  File: "<<name<<"ViewerPluginInfo.C" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "#include <"<<name<<"PluginInfo.h>" << Endl;
        if (type=="operator")
            c << "#include <"<<atts->name<<".h>" << Endl;
        c << Endl;

        if (type=="operator")
            c << "VISIT_OPERATOR_PLUGIN_ENTRY_EV(" << name << ",Viewer)" << Endl;
        else if (type=="plot")
            c << "VISIT_PLOT_PLUGIN_ENTRY_EV(" << name << ",Viewer)" << Endl;
        c << Endl;

        if(iconFile.length() > 0)
        {
            QString funcName = name + "ViewerPluginInfo::XPMIconData";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << Endl;
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName<< Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Return a pointer to the icon data." << Endl;
                c << "//" << Endl;
                c << "//  Returns:    A pointer to the icon data." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "#include <" << iconFile << ">" << Endl;
                c << "const char **" << Endl;
                c << funcName<<"() const" << Endl;
                c << "{" << Endl;
                c << "    return " << iconFile.left(iconFile.length() - 4) << "_xpm;" << Endl;
                c << "}" << Endl;
            }
        }

        if(type == "plot")
        {
            WriteOverrideDefinition(c, name + "ViewerPluginInfo::AlternateDisplayCreate");
            WriteOverrideDefinition(c, name + "ViewerPluginInfo::AlternateDisplayDestroy");
            WriteOverrideDefinition(c, name + "ViewerPluginInfo::AlternateDisplayClear");
            WriteOverrideDefinition(c, name + "ViewerPluginInfo::AlternateDisplayHide");
            WriteOverrideDefinition(c, name + "ViewerPluginInfo::AlternateDisplayShow");
            WriteOverrideDefinition(c, name + "ViewerPluginInfo::AlternateDisplayIconify");
            WriteOverrideDefinition(c, name + "ViewerPluginInfo::AlternateDisplayDeIconify");
        }

        c << Endl;
        WriteUserDefinedFunctions(c, name + "ViewerPluginInfo", false);
    }

    void WriteViewerEngineInfoSource(QTextStream &c)
    {
        if (type=="database")
            return;

        c << copyright_str << Endl;
        c << "// ****************************************************************************" << Endl;
        c << "//  File: "<<name<<"ViewerEnginePluginInfo.C" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
        c << "#include <"<<name<<"PluginInfo.h>" << Endl;
        if (type=="plot")
            c << "#include <avt"<<name<<"Plot.h>" << Endl;
        c << "#include <"<<atts->name<<".h>" << Endl;
        c << Endl;
        c << "//" << Endl;
        c << "// Storage for static data elements." << Endl;
        c << "//" << Endl;
        c << atts->name<<" *"<<name<<"ViewerEnginePluginInfo::clientAtts = NULL;" << Endl;
        c << atts->name<<" *"<<name<<"ViewerEnginePluginInfo::defaultAtts = NULL;" << Endl;
        c << Endl;

        QString funcName = name + "ViewerEnginePluginInfo::InitializeGlobalObjects";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "//  Method:  "<<funcName << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Initialize the "<<type<<" atts." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "void" << Endl;
            c << funcName << "()" << Endl;
            c << "{" << Endl;
            c << "    if ("<<name<<"ViewerEnginePluginInfo::clientAtts == NULL)" << Endl;
            c << "    {" << Endl;
            c << "        "<<name<<"ViewerEnginePluginInfo::clientAtts  = new "<<atts->name<<";" << Endl;
            c << "        "<<name<<"ViewerEnginePluginInfo::defaultAtts = new "<<atts->name<<";" << Endl;
            c << "    }" << Endl;
            c << "}" << Endl;
        }
        c << Endl;
        funcName = name + "ViewerEnginePluginInfo::GetClientAtts";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: "<<funcName << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Return a pointer to the viewer client attributes." << Endl;
            c << "//" << Endl;
            c << "//  Returns:    A pointer to the viewer client attributes." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "AttributeSubject *" << Endl;
            c << funcName<<"()" << Endl;
            c << "{" << Endl;
            c << "    return clientAtts;" << Endl;
            c << "}" << Endl;
            c << Endl;
        }
        funcName = name + "ViewerEnginePluginInfo::GetDefaultAtts";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: "<<funcName << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Return a pointer to the viewer default attributes." << Endl;
            c << "//" << Endl;
            c << "//  Returns:    A pointer to the viewer default attributes." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "AttributeSubject *" << Endl;
            c << funcName<<"()" << Endl;
            c << "{" << Endl;
            c << "    return defaultAtts;" << Endl;
            c << "}" << Endl;
        }
        c << Endl;
        funcName = name + "ViewerEnginePluginInfo::SetClientAtts";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: "<<funcName << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Set the viewer client attributes." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//    atts      A pointer to the new client attributes." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "void" << Endl;
            c << funcName << "(AttributeSubject *atts)" << Endl;
            c << "{" << Endl;
            c << "    *clientAtts = *("<<atts->name<<" *)atts;" << Endl;
            c << "    clientAtts->Notify();" << Endl;
            c << "}" << Endl;
        }
        c << Endl;
        funcName = name + "ViewerEnginePluginInfo::GetClientAtts";
        if(!ReplaceBuiltin(c, funcName + "1"))
        {
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: "<<funcName<< Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Get the viewer client attributes." << Endl;
            c << "//" << Endl;
            c << "//  Arguments:" << Endl;
            c << "//    atts      A pointer to return the client default attributes in." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "void" << Endl;
            c << funcName<<"(AttributeSubject *atts)" << Endl;
            c << "{" << Endl;
            c << "    *("<<atts->name<<" *)atts = *clientAtts;" << Endl;
            c << "}" << Endl;
        }
        c << Endl;
        if (type=="operator")
        {
            funcName = name + "ViewerEnginePluginInfo::GetClientAtts";
            if(ReplaceBuiltin(c, funcName + "3"))
            {
                c << Endl;
            }

            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::Removeable");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::Moveable");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::AllowsSubsequentOperators");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::GetCreatedVariables");

            QString funcName = name + "ViewerEnginePluginInfo::InitializeOperatorAtts";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Initialize the "<<type<<" attributes to the default attributes." << Endl;
                c << "//" << Endl;
                c << "//  Arguments:" << Endl;
                c << "//    atts        The attribute subject to initialize." << Endl;
                c << "//    plot        The viewer plot that owns the operator." << Endl;
                c << "//    fromDefault True to initialize the attributes from the defaults. False" << Endl;
                c << "//                to initialize from the current attributes." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "void" << Endl;
                c << funcName << "(AttributeSubject *atts," << Endl
                  << "                                              const avtPlotMetaData &plot," << Endl
                  << "                                              const bool fromDefault)" << Endl;
                c << "{" << Endl;
                c << "    if (fromDefault)" << Endl;
                c << "        *("<<atts->name<<"*)atts = *defaultAtts;" << Endl;
                c << "    else" << Endl;
                c << "        *("<<atts->name<<"*)atts = *clientAtts;" << Endl;
                c << "" << Endl;
                c << "    UpdateOperatorAtts(atts, plot);" << Endl;
                c << "}" << Endl;
                c << "" << Endl;
            }

            funcName = name + "ViewerEnginePluginInfo::UpdateOperatorAtts";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Update the "<<type<<" attributes. This function is *only* called" << Endl;
                c << "//    when the plot variable name is set or changed. The plot variable" << Endl;
                c << "//    name is used in operator expressions and may be used as the" << Endl;
                c << "//    'default' variable in GUIs." << Endl;
                c << "//" << Endl;
                c << "//  Arguments:" << Endl;
                c << "//    atts        The attribute subject to update." << Endl;
                c << "//    plot        The viewer plot that owns the operator." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "void" << Endl;
                c << funcName << "(AttributeSubject *atts, const avtPlotMetaData &plot)" << Endl;
                c << "{" << Endl;
                c << "}" << Endl;
                c << "" << Endl;
            }

            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::GetOperatorVarDescription");
#if 0
            funcName = name + "ViewerEnginePluginInfo::GetOperatorVarDescription";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Return the "<<type<<"  variable description." << Endl;
                c << "//" << Endl;
                c << "//  Arguments:" << Endl;
                c << "//    atts        The current attributes." << Endl;
                c << "//    plot        The viewer plot that owns the operator." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "std::string" << Endl;
                c << funcName << "(AttributeSubject *atts," << Endl
                  << "                                              const avtPlotMetaData &plot)" << Endl;
                c << "{" << Endl;
                c << "    return std::string(\"\");" << Endl;
                c << "}" << Endl;
                c << "" << Endl;
            }
#endif
        }
        if (type=="plot")
        {
            funcName = name + "ViewerEnginePluginInfo::AllocAvtPlot";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Return a pointer to a newly allocated avt plot." << Endl;
                c << "//" << Endl;
                c << "//  Returns:    A pointer to the newly allocated avt plot." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "avtPlot *" << Endl;
                c << funcName<<"()" << Endl;
                c << "{" << Endl;
                c << "    return new avt"<<name<<"Plot;" << Endl;
                c << "}" << Endl;
            }
            c << Endl;

            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::ProvidesLegend");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::PermitsCurveViewScaling");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::Permits2DViewScaling");

            funcName = name + "ViewerEnginePluginInfo::InitializePlotAtts";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//    Initialize the "<<type<<" attributes to the default attributes." << Endl;
                c << "//" << Endl;
                c << "//  Arguments:" << Endl;
                c << "//    atts      The attribute subject to initialize." << Endl;
                c << "//    plot      The viewer plot whose attributes are getting initialized." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted"<< Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << Endl;
                c << "void" << Endl;
                c << funcName<<"(AttributeSubject *atts," << Endl
                  << "    const avtPlotMetaData &)" << Endl;
                c << "{" << Endl;
                c << "    *("<<atts->name<<"*)atts = *defaultAtts;" << Endl;
                c << "}" << Endl;
            }

            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::ReInitializePlotAtts");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::ResetPlotAtts");

            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::SupportsAnimation");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::AnimationReset");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::AnimationStep");
        }

        funcName = name + "ViewerEnginePluginInfo::GetMenuName";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "//  Method: "<<funcName << Endl;
            c << "//" << Endl;
            c << "//  Purpose:" << Endl;
            c << "//    Return a pointer to the name to use in the viewer menus." << Endl;
            c << "//" << Endl;
            c << "//  Returns:    A pointer to the name to use in the viewer menus." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "const char *" << Endl;
            c << funcName<<"() const" << Endl;
            c << "{" << Endl;
            c << "    return \""<<label<<"\";" << Endl;
            c << "}" << Endl;
        }

        c << Endl;
        WriteUserDefinedFunctions(c, name + "ViewerEnginePluginInfo", false);
    }


    void WriteMDServerInfoSource(QTextStream &c)
    {
        if (type=="database")
        {
            c << copyright_str << Endl;
            c << "#include <"<<name<<"PluginInfo.h>" << Endl;
            c << "" << Endl;
            c << "VISIT_DATABASE_PLUGIN_ENTRY(" << name << ",MDServer)" << Endl;
            c << Endl;
            c << "// this makes compilers happy... remove if we ever have functions here" << Endl;
            c << "void "<<name<<"MDServerPluginInfo::dummy()" << Endl;
            c << "{" << Endl;
            c << "}" << Endl;
            c << Endl;
            WriteUserDefinedFunctions(c, name + "MDServerPluginInfo", false);
        }
    }


    void WriteEngineInfoSource(QTextStream &c)
    {
        if (type=="database")
        {
            c << copyright_str << Endl;
            c << "#include <"<<name<<"PluginInfo.h>" << Endl;
            if (haswriter)
                c << "#include <avt"<<name<<"Writer.h>" << Endl;
            c << Endl;
            c << "VISIT_DATABASE_PLUGIN_ENTRY(" << name << ",Engine)" << Endl;
            c << Endl;

            QString funcName = name + "EnginePluginInfo::GetWriter";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << Endl;
                c << "//  Method: "<<funcName << Endl;
                c << "//" << Endl;
                c << "//  Purpose:" << Endl;
                c << "//      Sets up a "<<name<<" writer." << Endl;
                c << "//" << Endl;
                c << "//  Returns:    A "<<name<<" writer." << Endl;
                c << "//" << Endl;
                c << "//  Programmer: generated by xml2info" << Endl;
                c << "//  Creation:   omitted" << Endl;
                c << "//" << Endl;
                c << "// ****************************************************************************" << Endl;
                c << "avtDatabaseWriter *" << Endl;
                c << ""<<funcName << "(void)" << Endl;
                c << "{" << Endl;
                if (haswriter)
                    if (hasoptions)
                        c << "    return new avt"<<name<<"Writer(writeOptions);"
                          << Endl;
                    else
                        c << "    return new avt"<<name<<"Writer;" << Endl;
                else
                    c << "    return NULL;" << Endl;
                c << "}" << Endl;
            }
            c << Endl;
        }
        else
        {
            c << copyright_str << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "//  File: "<<name<<"EnginePluginInfo.C" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "#include <"<<name<<"PluginInfo.h>" << Endl;
            if (type=="operator")
                c << "#include <avt"<<name<<"Filter.h>" << Endl;
            else if (type=="plot")
                c << "#include <avt"<<name<<"Plot.h>" << Endl;
            c << Endl;
            if (type=="operator")
                c << "VISIT_OPERATOR_PLUGIN_ENTRY_EV(" << name << ",Engine)" << Endl;
            else if (type=="plot")
                c << "VISIT_PLOT_PLUGIN_ENTRY_EV(" << name << ",Engine)" << Endl;
            c << Endl;

            if (type=="operator")
            {
                QString funcName = name + "EnginePluginInfo::AllocAvtPluginFilter";
                if(!ReplaceBuiltin(c, funcName))
                {
                    c << "// ****************************************************************************" << Endl;
                    c << "//  Method: "<<funcName << Endl;
                    c << "//" << Endl;
                    c << "//  Purpose:" << Endl;
                    c << "//    Return a pointer to a newly allocated avtPluginFilter." << Endl;
                    c << "//" << Endl;
                    c << "//  Returns:    A pointer to the newly allocated avtPluginFilter." << Endl;
                    c << "//" << Endl;
                    c << "//  Programmer: generated by xml2info" << Endl;
                    c << "//  Creation:   omitted"<< Endl;
                    c << "//" << Endl;
                    c << "// ****************************************************************************" << Endl;
                    c << Endl;
                    c << "avtPluginFilter *" << Endl;
                    c << funcName<<"()" << Endl;
                    c << "{" << Endl;
                    c << "    return new avt"<<name<<"Filter;" << Endl;
                    c << "}" << Endl;
                }
            }
        }
        WriteUserDefinedFunctions(c, name + "EnginePluginInfo", false);
    }

    void WriteScriptingInfoSource(QTextStream &c)
    {
        if (type=="database")
            return;

        c << copyright_str << Endl;
        c << "// ************************************************************************* //" << Endl;
        c << "//  "<<name<<"ScriptingPluginInfo.C" << Endl;
        c << "// ************************************************************************* //" << Endl;
        c << "#include <Py"<<atts->name<<".h>" << Endl;
        c << "#include <"<<name<<"PluginInfo.h>" << Endl;
        c << "" << Endl;
        if (type=="operator")
            c << "VISIT_OPERATOR_PLUGIN_ENTRY(" << name << ",Scripting)" << Endl;
        else if (type=="plot")
            c << "VISIT_PLOT_PLUGIN_ENTRY(" << name << ",Scripting)" << Endl;
        c << Endl;

        QString funcName = name + "ScriptingPluginInfo::InitializePlugin";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "// Method: "<<funcName << Endl;
            c << "//" << Endl;
            c << "// Purpose:" << Endl;
            c << "//   Calls the initialization function for the plugin." << Endl;
            c << "//" << Endl;
            c << "// Arguments:" << Endl;
            c << "//   subj    : A pointer to the plugin's state object." << Endl;
            c << "//   data    : A pointer to data to be used by the observer function." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// Modifications:" << Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void" << Endl;
            c << funcName << "(AttributeSubject *subj," << Endl;
            c << "    void *data)" << Endl;
            c << "{" << Endl;
            c << "    Py"<<atts->name<<"_StartUp(("<<atts->name<<" *)subj, data);" << Endl;
            c << "}" << Endl;
        }
        c << Endl;
        funcName = name + "ScriptingPluginInfo::GetMethodTable";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "// Method: "<<funcName << Endl;
            c << "//" << Endl;
            c << "// Purpose:" << Endl;
            c << "//   Returns a pointer to the plugin's Python method table. These methods are" << Endl;
            c << "//   added to the top-level visit module's methods." << Endl;
            c << "//" << Endl;
            c << "// Arguments:" << Endl;
            c << "//   nMethods : Returns the number of methods in the method table." << Endl;
            c << "//" << Endl;
            c << "// Returns:    A pointer to the method table." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// Modifications:" << Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "void *" << Endl;
            c << funcName << "(int *nMethods)" << Endl;
            c << "{" << Endl;
            c << "    return Py"<<atts->name<<"_GetMethodTable(nMethods);" << Endl;
            c << "}" << Endl;
        }
        c << Endl;
        funcName = name + "ScriptingPluginInfo::TypesMatch";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "// Method: " << funcName << Endl;
            c << "//" << Endl;
            c << "// Purpose:" << Endl;
            c << "//   Returns whether or not the input PyObject is "<<name<<" plot attributes." << Endl;
            c << "//" << Endl;
            c << "// Arguments:" << Endl;
            c << "//   pyobject : A PyObject cast to void*." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// Modifications:" << Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << "" << Endl;
            c << "bool" << Endl;
            c << funcName << "(void *pyobject)" << Endl;
            c << "{" << Endl;
            c << "    return Py"<<atts->name<<"_Check((PyObject *)pyobject);" << Endl;
            c << "}" << Endl;
        }
        c << Endl;
        funcName = name + "ScriptingPluginInfo::GetLogString";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "// Method: " << funcName << Endl;
            c << "//" << Endl;
            c << "// Purpose:" << Endl;
            c << "//   Gets a string representation of the current attributes." << Endl;
            c << "//" << Endl;
            c << "// Arguments:" << Endl;
            c << "//   val : Whether or not to log state information." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// Modifications:" << Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "char *" << Endl;
            c << funcName << "()" << Endl;
            c << "{" << Endl;
            c << "    std::string s(Py" << atts->name << "_GetLogString());" << Endl;
            c << "    char *v = new char[s.size() + 1];" << Endl;
            c << "    strcpy(v, s.c_str());" << Endl;
            c << "    return v;" << Endl;
            c << "}" << Endl;
        }
        c << Endl;
        funcName = name + "ScriptingPluginInfo::SetDefaults";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << Endl;
            c << "// Method: " << funcName << Endl;
            c << "//" << Endl;
            c << "// Purpose:" << Endl;
            c << "//   Used to set the default values for a plugin's state object." << Endl;
            c << "//" << Endl;
            c << "// Arguments:" << Endl;
            c << "//   atts : The new state." << Endl;
            c << "//" << Endl;
            c << "//  Programmer: generated by xml2info" << Endl;
            c << "//  Creation:   omitted"<< Endl;
            c << "//" << Endl;
            c << "// Modifications:" << Endl;
            c << "//" << Endl;
            c << "// ****************************************************************************" << Endl;
            c << Endl;
            c << "void" << Endl;
            c << funcName << "(const AttributeSubject *atts)" << Endl;
            c << "{" << Endl;
            c << "    Py" << atts->name << "_SetDefaults((const " << atts->name << " *)atts);" << Endl;
            c << "}" << Endl;
        }

        WriteUserDefinedFunctions(c, name + "ScriptingPluginInfo", false);
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       InfoGeneratorPlugin

#endif
