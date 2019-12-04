// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GENERATE_INFO_H
#define GENERATE_INFO_H

#include <QTextStream>
#include "Field.h"
#include <visitstream.h>
#include "Plugin.h"

#define WRITE_LICENSE

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
// ****************************************************************************

class InfoGeneratorPlugin : public Plugin
{
  public:
    QString generatorName;

    InfoGeneratorPlugin(const QString &n,const QString &l,const QString &t,
        const QString &vt,const QString &dt, const QString &v, const QString &ifile,
        bool hw, bool ho, bool onlyengine, bool noengine) : 
        Plugin(n,l,t,vt,dt,v,ifile,hw,ho,onlyengine,noengine), generatorName("xml2info")
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
                c << atts->functions[n]->def << endl;
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
                c << atts->functions[i]->def << endl;
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
            h << "    // User-defined functions" << endl;

        if(publicFuncs.size() > 0)
        {
             if(writeDecl)
                 h << "  public:" << endl;

             for(size_t j = 0; j < publicFuncs.size(); ++j)
             {
                 if(writeDecl)
                     h << "    " << atts->functions[publicFuncs[j]]->decl << endl;
                 else
                 {
                     h << atts->functions[publicFuncs[j]]->def << endl;
                 }
            }
        }

        if(protectedFuncs.size() > 0)
        {
             if(writeDecl)
                 h << "  protected:" << endl;

             for(size_t j = 0; j < protectedFuncs.size(); ++j)
             {
                 if(writeDecl)
                     h << "    " << atts->functions[protectedFuncs[j]]->decl << endl;
                 else
                 {
                     h << atts->functions[protectedFuncs[j]]->def << endl;
                 }
            }
        }

        if(privateFuncs.size() > 0)
        {
             if(writeDecl)
                 h << "  private:" << endl;

             for(size_t j = 0; j < privateFuncs.size(); ++j)
             {
                 if(writeDecl)
                     h << "    " << atts->functions[privateFuncs[j]]->decl << endl;
                 else
                 {
                     h << atts->functions[privateFuncs[j]]->def << endl;
                 }
            }
        }
    }

    void WriteInfoHeader(QTextStream &h)
    {
        if (type=="operator")
        {
            h << copyright_str << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//  File: "<<name<<"PluginInfo.h" << endl;
            h << "// ************************************************************************* //" << endl;
            h << endl;
            h << "#ifndef "<<name.toUpper()<<"_PLUGIN_INFO_H" << endl;
            h << "#define "<<name.toUpper()<<"_PLUGIN_INFO_H" << endl;
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
            h << "//  Programmer: generated by xml2info" << endl;
            h << "//  Creation:   omitted"<< endl;
            h << "//" << endl;
            h << "//  Modifications:" << endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << endl;
            h << "class "<<name<<"GeneralPluginInfo : public virtual GeneralOperatorPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual const char *GetName() const;" << endl;
            h << "    virtual const char *GetVersion() const;" << endl;
            h << "    virtual const char *GetID() const;" << endl;
            h << "    virtual bool  EnabledByDefault() const;" << endl;
            if(category.length() > 0)
                h << "    virtual const char *GetCategoryName() const;" << endl;
            h << "};" << endl;
            h << endl;

            // CommonPluginInfo
            QString infoName = name + "CommonPluginInfo";
            h << "class "<<infoName<<" : public virtual CommonOperatorPluginInfo, public virtual "<<name<<"GeneralPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual AttributeSubject *AllocAttributes();" << endl;
            h << "    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);" << endl;
            if(OverrideBuiltin(infoName + "::GetUserSelectable"))
                h << "    virtual bool GetUserSelectable() const;" << endl;
            if(OverrideBuiltin(infoName + "::GetVariableTypes"))
                h << "    virtual int GetVariableTypes() const;" << endl;
            if(OverrideBuiltin(infoName + "::GetVariableMask"))
                h << "    virtual int GetVariableMask() const;" << endl;
            if(createExpression)
            {
                h << "    virtual ExpressionList *GetCreatedExpressions(const avtDatabaseMetaData *) const;" << endl;
            }
            h << "};" << endl;
            h << endl;

            // GUIPluginInfo
            infoName = name + "GUIPluginInfo";
            h << "class "<<infoName<<" : public virtual GUIOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual QString *GetMenuName() const;" << endl;
            h << "    virtual QvisPostableWindowObserver *CreatePluginWindow(int type," << endl;
            h << "        AttributeSubject *attr, const QString &caption, const QString &shortName," << endl;
            h << "        QvisNotepadArea *notepad);" << endl;
            if(OverrideBuiltin(infoName + "::CreatePluginWizard"))
            {
                h << "   virtual QvisWizard *CreatePluginWizard(AttributeSubject *attr," << endl;
                h << "        QWidget *parent);" << endl;
            }
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;

            // ViewerEnginePluginInfo
            infoName = name + "ViewerEnginePluginInfo";
            h << "class "<<infoName<<" : public virtual ViewerEngineOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual AttributeSubject *GetClientAtts();" << endl;
            h << "    virtual AttributeSubject *GetDefaultAtts();" << endl;
            h << "    virtual void SetClientAtts(AttributeSubject *atts);" << endl;
            h << "    virtual void GetClientAtts(AttributeSubject *atts);" << endl;
            if(OverrideBuiltin(infoName + "::GetClientAtts3"))
                h << "    virtual void GetClientAtts(AttributeSubject *atts, const bool, const bool);" << endl;
            h << endl;
            h << "    virtual void InitializeOperatorAtts(AttributeSubject *atts," << endl;
            h << "                                        const avtPlotMetaData &plot," << endl;
            h << "                                        const bool fromDefault);" << endl;
            h << "    virtual void UpdateOperatorAtts(AttributeSubject *atts," << endl;
            h << "                                    const avtPlotMetaData &plot);" << endl;
            if(OverrideBuiltin(infoName + "::GetOperatorVarDescription"))
            {
                h << "    virtual std::string GetOperatorVarDescription(AttributeSubject *atts," << endl;
                h << "                                                  const avtPlotMetaData &plot);" << endl;
            }
            h << "    virtual const char *GetMenuName() const;" << endl;
            if(OverrideBuiltin(infoName + "::Removeable"))
                h << "    virtual bool Removeable() const;" << endl;
            if(OverrideBuiltin(infoName + "::Moveable"))
                h << "    virtual bool Moveable() const;" << endl;
            if(OverrideBuiltin(infoName + "::AllowsSubsequentOperators"))
                h << "    virtual bool AllowsSubsequentOperators() const;" << endl;
            if(OverrideBuiltin(infoName + "::GetCreatedVariables"))
                h << "    virtual ExpressionList *GetCreatedVariables(const char *mesh);" << endl;
            h << endl;
            h << "    static void InitializeGlobalObjects();" << endl;
            h << "  private:" << endl;
            h << "    static "<<atts->name<<" *defaultAtts;" << endl;
            h << "    static "<<atts->name<<" *clientAtts;" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;

            // ViewerPluginInfo
            infoName = name + "ViewerPluginInfo";
            h << "class "<<infoName<<" : public virtual ViewerOperatorPluginInfo, public virtual "<<name<<"ViewerEnginePluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;

            // EnginePluginInfo
            infoName = name + "EnginePluginInfo";
            h << "class "<<infoName<<" : public virtual EngineOperatorPluginInfo, public virtual "<<name<<"ViewerEnginePluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual avtPluginFilter *AllocAvtPluginFilter();" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;

            // ScriptingPluginInfo
            infoName = name + "ScriptingPluginInfo";
            h << "class "<<infoName<<" : public virtual ScriptingOperatorPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual void InitializePlugin(AttributeSubject *subj, void *data);" << endl;
            h << "    virtual void *GetMethodTable(int *nMethods);" << endl;
            h << "    virtual bool TypesMatch(void *pyobject);" << endl;
            h << "    virtual char *GetLogString();" << endl;
            h << "    virtual void SetDefaults(const AttributeSubject *atts);" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;
            h << "#endif" << endl;
        }
        else if (type=="plot")
        {
            h << copyright_str << endl;
            h << "// ************************************************************************* //" << endl;
            h << "//                               "<<name<<"PluginInfo.h                            //" << endl;
            h << "// ************************************************************************* //" << endl;
            h << endl;
            h << "#ifndef "<<name.toUpper()<<"_PLUGIN_INFO_H" << endl;
            h << "#define "<<name.toUpper()<<"_PLUGIN_INFO_H" << endl;
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
            h << "//  Programmer: generated by xml2info" << endl;
            h << "//  Creation:   omitted"<< endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << endl;

            // GeneralPluginInfo
            h << "class "<<name<<"GeneralPluginInfo: public virtual GeneralPlotPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual const char *GetName() const;" << endl;
            h << "    virtual const char *GetVersion() const;" << endl;
            h << "    virtual const char *GetID() const;" << endl;
            h << "    virtual bool  EnabledByDefault() const;" << endl;
            h << "};" << endl;
            h << endl;
            h << "class "<<name<<"CommonPluginInfo : public virtual CommonPlotPluginInfo, public virtual "<<name<<"GeneralPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual AttributeSubject *AllocAttributes();" << endl;
            h << "    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);" << endl;
            h << "    virtual int GetVariableTypes() const;" << endl;
            h << "};" << endl;
            h << endl;

            // GUIPluginInfo
            QString infoName = name + "GUIPluginInfo";
            h << "class "<<infoName<<" : public virtual GUIPlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual QString *GetMenuName() const;" << endl;
            h << "    virtual QvisPostableWindowObserver *CreatePluginWindow(int type," << endl;
            h << "        AttributeSubject *attr, const QString &caption, const QString &shortName," << endl;
            h << "        QvisNotepadArea *notepad);" << endl;
            if(OverrideBuiltin(infoName + "::CreatePluginWizard"))
            {
                h << "    virtual QvisWizard *CreatePluginWizard(AttributeSubject *attr, QWidget *parent," << endl;
                h << "        const std::string &varName, const avtDatabaseMetaData *md," << endl;
                h << "        const ExpressionList *expList); " << endl;
            }
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;

            // ViewerEnginePluginInfo
            infoName = name + "ViewerEnginePluginInfo";
            h << "class "<<infoName<<" : public virtual ViewerEnginePlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual AttributeSubject *GetClientAtts();" << endl;
            h << "    virtual AttributeSubject *GetDefaultAtts();" << endl;
            h << "    virtual void SetClientAtts(AttributeSubject *atts);" << endl;
            h << "    virtual void GetClientAtts(AttributeSubject *atts);" << endl;
            h << endl;
            h << "    virtual avtPlot *AllocAvtPlot();" << endl;
            h << endl;
            if(OverrideBuiltin(infoName + "::ProvidesLegend"))
                h << "    virtual bool ProvidesLegend() const;" << endl;
            if(OverrideBuiltin(infoName + "::PermitsCurveViewScaling"))
                h << "    virtual bool PermitsCurveViewScaling() const;" << endl;
            if(OverrideBuiltin(infoName + "::Permits2DViewScaling"))
                h << "    virtual bool Permits2DViewScaling() const;" << endl;
            h << "    virtual void InitializePlotAtts(AttributeSubject *atts, const avtPlotMetaData &);" << endl;
            if(OverrideBuiltin(infoName + "::ReInitializePlotAtts"))
                h << "    virtual void ReInitializePlotAtts(AttributeSubject *atts, const avtPlotMetaData &);" << endl;
            if(OverrideBuiltin(infoName + "::ResetPlotAtts"))
                h << "    virtual void ResetPlotAtts(AttributeSubject *atts, const avtPlotMetaData &);" << endl;

            if(OverrideBuiltin(infoName + "::SupportsAnimation"))
                h << "    virtual bool SupportsAnimation() const;" << endl;
            if(OverrideBuiltin(infoName + "::AnimationReset"))
                h << "    virtual bool AnimationReset(AttributeSubject *atts, const avtPlotMetaData &plot);" << endl;
            if(OverrideBuiltin(infoName + "::AnimationStep"))
                h << "    virtual bool AnimationStep(AttributeSubject *atts, const avtPlotMetaData  &plot);" << endl;

            h << "    virtual const char *GetMenuName() const;" << endl;
            h << "    static void InitializeGlobalObjects();" << endl;
            h << "  private:" << endl;
            h << "    static "<<atts->name<<" *defaultAtts;" << endl;
            h << "    static "<<atts->name<<" *clientAtts;" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;

            // ViewerPluginInfo
            infoName = name + "ViewerPluginInfo";
            h << "class "<<infoName<<" : public virtual ViewerPlotPluginInfo, public virtual "<<name<<"ViewerEnginePluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            if(iconFile.length() > 0)
                h << "    virtual const char **XPMIconData() const;" << endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayCreate"))
                h << "    virtual void *AlternateDisplayCreate(ViewerPlot *plot);" << endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayDestroy"))
                h << "    virtual void AlternateDisplayDestroy(void *dpy);" << endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayClear"))
                h << "    virtual void AlternateDisplayClear(void *dpy);" << endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayHide"))
                h << "    virtual void AlternateDisplayHide(void *dpy);" << endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayShow"))
                h << "    virtual void AlternateDisplayShow(void *dpy);" << endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayIconify"))
                h << "    virtual void AlternateDisplayIconify(void *dpy);" << endl;
            if(OverrideBuiltin(infoName + "::AlternateDisplayDeIconify"))
                h << "    virtual void AlternateDisplayDeIconify(void *dpy);" << endl;
            h << endl;
            //h << "  private:" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;

            // EnginePluginInfo
            infoName = name + "EnginePluginInfo";
            h << "class "<<infoName<<" : public virtual EnginePlotPluginInfo, public virtual "<<name<<"ViewerEnginePluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;

            // ScriptingPluginInfo
            infoName = name + "ScriptingPluginInfo";
            h << "class "<<infoName<<" : public virtual ScriptingPlotPluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual void InitializePlugin(AttributeSubject *subj, void *data);" << endl;
            h << "    virtual void *GetMethodTable(int *nMethods);" << endl;
            h << "    virtual bool TypesMatch(void *pyobject);" << endl;
            h << "    virtual char *GetLogString();" << endl;
            h << "    virtual void SetDefaults(const AttributeSubject *atts);" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << endl;
            h << "#endif" << endl;
        }
        else if (type=="database")
        {
            h << copyright_str << endl;
            h << "// ****************************************************************************" << endl;
            h << "//                               "<<name<<"PluginInfo.h" << endl;
            h << "// ****************************************************************************" << endl;
            h << "" << endl;
            h << "#ifndef "<<name.toUpper()<<"_PLUGIN_INFO_H" << endl;
            h << "#define "<<name.toUpper()<<"_PLUGIN_INFO_H" << endl;
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
            h << "//  Programmer: generated by xml2info" << endl;
            h << "//  Creation:   omitted" << endl;
            h << "//" << endl;
            h << "//  Modifications:" << endl;
            h << "//" << endl;
            h << "// ****************************************************************************" << endl;
            h << "" << endl;
            h << "class "<<name<<"GeneralPluginInfo : public virtual GeneralDatabasePluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual const char *GetName() const;" << endl;
            h << "    virtual const char *GetVersion() const;" << endl;
            h << "    virtual const char *GetID() const;" << endl;
            h << "    virtual bool  EnabledByDefault() const;" << endl;
            h << "    virtual bool  HasWriter() const;" << endl;
            h << "    virtual std::vector<std::string> GetDefaultFilePatterns() const;" << endl;
            h << "    virtual bool  AreDefaultFilePatternsStrict() const;" << endl;
            h << "    virtual bool  OpensWholeDirectory() const;" << endl;
            h << "};" << endl;
            h << "" << endl;
            h << "class "<<name<<"CommonPluginInfo : public virtual CommonDatabasePluginInfo, public virtual "<<name<<"GeneralPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual DatabaseType              GetDatabaseType();" << endl;
            h << "    virtual avtDatabase              *SetupDatabase(const char * const *list," << endl;
            h << "                                                    int nList, int nBlock);" << endl;
            if (hasoptions)
            {
                h << "    virtual DBOptionsAttributes *GetReadOptions() const;"
                  << endl;
                h << "    virtual DBOptionsAttributes *GetWriteOptions() const;"
                  << endl;
                QString infoName = name + "CommonPluginInfo";
                if(OverrideBuiltin(infoName + "::SetReadOptions"))
                    h << "    virtual void                 SetReadOptions(DBOptionsAttributes *);" << endl;
            }
#ifdef WRITE_LICENSE
            h << "    virtual std::string               GetLicense() const;" << endl;
#endif
            h << "};" << endl;
            h << "" << endl;
            QString infoName = name + "MDServerPluginInfo";
            h << "class "<<infoName<<" : public virtual MDServerDatabasePluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    // this makes compilers happy... remove if we ever have functions here" << endl;
            h << "    virtual void dummy();" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << "" << endl;
            infoName = name + "EnginePluginInfo";
            h << "class "<<infoName<<" : public virtual EngineDatabasePluginInfo, public virtual "<<name<<"CommonPluginInfo" << endl;
            h << "{" << endl;
            h << "  public:" << endl;
            h << "    virtual avtDatabaseWriter        *GetWriter(void);" << endl;
            WriteUserDefinedFunctions(h, infoName, true);
            h << "};" << endl;
            h << "" << endl;
            h << "#endif" << endl;
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
        c << "#include <visit-config.h>" << endl;
        c << "VISIT_PLUGIN_VERSION("<<name<<","<<Export(type.toStdString()) << ")" << endl;
        c << endl;
    }
    void WriteInfoSource(QTextStream &c)
    {
        c << copyright_str << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//  File: "<<name<<"PluginInfo.C" << endl;
        c << "// ************************************************************************* //" << endl;
        c << endl;
        c << "#include <"<<name<<"PluginInfo.h>" << endl;
        if (type!="database")
            c << "#include <"<<atts->name<<".h>" << endl;
        c << endl;
        AddVersion(c);
        if (type=="operator")
            c << "VISIT_OPERATOR_PLUGIN_ENTRY(" << name << ",General)" << endl;
        else if (type=="plot")
            c << "VISIT_PLOT_PLUGIN_ENTRY(" << name << ",General)" << endl;
        else if (type=="database")
            c << "VISIT_DATABASE_PLUGIN_ENTRY(" << name << ",General)" << endl;
        c << endl;
        c << "// ****************************************************************************" << endl;
        c << "//  Method: "<<name<<"GeneralPluginInfo::GetName" << endl;
        c << "//" << endl;
        c << "//  Purpose:" << endl;
        c << "//    Return the name of the "<<type<<" plugin." << endl;
        c << "//" << endl;
        c << "//  Returns:    A pointer to the name of the "<<type<<" plugin." << endl;
        c << "//" << endl;
        c << "//  Programmer: generated by xml2info" << endl;
        c << "//  Creation:   omitted"<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "const char *" << endl;
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
        c << "//  Programmer: generated by xml2info" << endl;
        c << "//  Creation:   omitted"<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "const char *" << endl;
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
        c << "//  Programmer: generated by xml2info" << endl;
        c << "//  Creation:   omitted"<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "const char *" << endl;
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
        c << "//  Programmer: generated by xml2info" << endl;
        c << "//  Creation:   omitted"<< endl;
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
        c << "bool" << endl;
        c << name<<"GeneralPluginInfo::EnabledByDefault() const" << endl;
        c << "{" << endl;
        c << "    return "<<Bool2Text(enabledByDefault)<<";" << endl;
        c << "}" << endl;

        if(type == "operator" && category.length() > 0)
        {
            QString funcName(name + "GeneralPluginInfo::GetCategoryName");
            c << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<funcName << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return the category name to which the operator belongs." << endl;
            c << "//" << endl;
            c << "//  Returns:    Return the category name to which the operator belongs." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "const char *" << endl;
            c << funcName<<"() const" << endl;
            c << "{" << endl;
            c << "    return \"" << category << "\";" << endl;
            c << "}" << endl;
        }

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
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "bool" << endl;
            c << name<<"GeneralPluginInfo::HasWriter() const" << endl;
            c << "{" << endl;
            c << "    return "<<Bool2Text(haswriter)<<";" << endl;
            c << "}" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method:  "<<name<<"GeneralPluginInfo::GetDefaultFilePatterns" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Returns the default patterns for a "<<name<<" database." << endl;
            c << "//" << endl;
            c << "//  Programmer:  generated by xml2info" << endl;
            c << "//  Creation:    omitted" << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "std::vector<std::string>" << endl;
            c << ""<<name<<"GeneralPluginInfo::GetDefaultFilePatterns() const" << endl;
            c << "{" << endl;
            c << "    std::vector<std::string> defaultPatterns;" << endl;
            for (size_t i=0; i<filePatterns.size(); i++)
            {
                c << "    defaultPatterns.push_back(\""<<filePatterns[i]<<"\");" << endl;
            }
            c << "" << endl;
            c << "    return defaultPatterns;" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method:  "<<name<<"GeneralPluginInfo::AreDefaultFilePatternsStrict" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Returns if the file patterns for a "<<name<<" database are" << endl;
            c << "//    intended to be interpreted strictly by default." << endl;
            c << "//" << endl;
            c << "//  Programmer:  generated by xml2info" << endl;
            c << "//  Creation:    omitted" << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "bool" << endl;
            c << ""<<name<<"GeneralPluginInfo::AreDefaultFilePatternsStrict() const" << endl;
            c << "{" << endl;
            c << "    return "<<(filePatternsStrict ? "true" : "false")<<";" << endl;
            c << "}" << endl;
            c << "" << endl;
            c << "// ****************************************************************************" << endl;
            c << "//  Method:  "<<name<<"GeneralPluginInfo::OpensWholeDirectory" << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Returns if the "<<name<<" plugin opens a whole directory name" << endl;
            c << "//    instead of a single file." << endl;
            c << "//" << endl;
            c << "//  Programmer:  generated by xml2info" << endl;
            c << "//  Creation:    omitted" << endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "bool" << endl;
            c << ""<<name<<"GeneralPluginInfo::OpensWholeDirectory() const" << endl;
            c << "{" << endl;
            c << "    return "<<(opensWholeDirectory ? "true" : "false")<<";" << endl;
            c << "}" << endl;
        }
    }
    void AddExpressionFromMD(QTextStream &c, QString opName, std::vector<QString> outtypes, QString typeToLookForS, QString typeToLookForP)
    {
        c << "    int num" << typeToLookForP << " = md->GetNum" << typeToLookForP << "();" << endl;
        c << "    for (i = 0 ; i < num" << typeToLookForP << " ; i++)" << endl;
        c << "    {" << endl;
        c << "        const avt" << typeToLookForS << "MetaData *mmd = md->Get" << typeToLookForS << "(i);" << endl;
        c << "        if (mmd->hideFromGUI || !mmd->validVariable)" << endl;
        c << "            continue;" << endl;
        for (size_t j = 0 ; j < outtypes.size() ; j++)
        {
            c << "        {" << endl;
            c << "            Expression e2;" << endl;
            c << "            sprintf(name, \"operators/" << opName << "/%s\", mmd->name.c_str());" << endl;
            c << "            e2.SetName(name);" << endl;
            c << "            e2.SetType(Expression::" << outtypes[j] << ");" << endl;
            c << "            e2.SetFromOperator(true);" << endl;
            c << "            e2.SetOperatorName(\"" << opName << "\");" << endl;
            c << "            sprintf(defn, \"cell_constant(<%s>, 0.)\", mmd->name.c_str());" << endl;
            c << "            e2.SetDefinition(defn);" << endl;
            c << "            el->AddExpressions(e2);" << endl;
            c << "        }" << endl;
        }
        c << "    }" << endl;
    }
    void AddExpressionFromExpr(QTextStream &c, QString opName, std::vector<QString> outtypes, QString exprType)
    {
        c << "        if (e.GetType() == Expression::" << exprType << ")" << endl;
        c << "        {" << endl;
        for (size_t j = 0 ; j < outtypes.size() ; j++)
        {
            c << "            {" << endl;
            c << "                if (e.GetFromOperator() || e.GetAutoExpression())" << endl;
            c << "                    continue; // weird ordering behavior otherwise" << endl;
            c << "                Expression e2;" << endl;
            c << "                sprintf(name, \"operators/" << opName << "/%s\", e.GetName().c_str());" << endl;
            c << "                e2.SetName(name);" << endl;
            c << "                e2.SetType(Expression::" << outtypes[j] << ");" << endl;
            c << "                e2.SetFromOperator(true);" << endl;
            c << "                e2.SetOperatorName(\"" << opName << "\");" << endl;
            c << "                sprintf(defn, \"cell_constant(<%s>, 0.)\", e.GetName().c_str());" << endl;
            c << "                e2.SetDefinition(defn);" << endl;
            c << "                el->AddExpressions(e2);" << endl;
            c << "            }" << endl;
        }
        c << "        }" << endl;
    }

    void WriteCommonInfoSource(QTextStream &c)
    {
        if (type=="database")
        {
            c << copyright_str << endl;
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

            // Write GetDatabaseType
            QString funcName(name + "CommonPluginInfo::GetDatabaseType");
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method:  "<<funcName<< endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Returns the type of a "<<name<<" database." << endl;
                c << "//" << endl;
                c << "//  Programmer:  generated by xml2info" << endl;
                c << "//  Creation:    omitted" << endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << "DatabaseType" << endl;
                c << ""<<funcName<<"()" << endl;
                c << "{" << endl;
                c << "    return DB_TYPE_"<<dbtype.toUpper()<<";" << endl;
                c << "}" << endl;
            }
            c << "" << endl;

            // Write SetupDatabase
            funcName = name + "CommonPluginInfo::SetupDatabase";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
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
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted" << endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << "avtDatabase *" << endl;
                c << ""<<funcName<<"(const char *const *list," << endl;
                c << "                                   int nList, int nBlock)" << endl;
                c << "{" << endl;
                if (dbtype == "Custom")
                {
                    c << "    return new avt"<<name<<"Database(list[0]);" << endl;
                }
                else if (dbtype == "STSD")
                {
                    c << "    int nTimestep = nList / nBlock;" << endl;
                    c << "    avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[nTimestep];" << endl;
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
                    c << "    int nTimestepGroups = nList / nBlock;" << endl;
                    c << "    avtMTSDFileFormat ***ffl = new avtMTSDFileFormat**[nTimestepGroups];" << endl;
                    c << "    for (int i = 0 ; i < nTimestepGroups ; i++)" << endl;
                    c << "    {" << endl;
                    c << "        ffl[i] = new avtMTSDFileFormat*[nBlock];" << endl;
                    c << "        for (int j = 0 ; j < nBlock ; j++)" << endl;
                    c << "        {" << endl;
                    if (hasoptions)
                        c << "            ffl[i][j] = new avt"<<name<<"FileFormat(list[i*nBlock + j], readOptions);" << endl;
                    else
                        c << "            ffl[i][j] = new avt"<<name<<"FileFormat(list[i*nBlock + j]);" << endl;
                    c << "        }" << endl;
                    c << "    }" << endl;
                    c << "    avtMTSDFileFormatInterface *inter " << endl;
                    c << "           = new avtMTSDFileFormatInterface(ffl, nTimestepGroups, nBlock);" << endl;
                    c << "    return new avtGenericDatabase(inter);" << endl;
                }
                else if (dbtype == "MTMD")
                {
                    c << "    // ignore any nBlocks past 1" << endl;
                    c << "    int nTimestepGroups = nList / nBlock;" << endl;
                    c << "    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];" << endl;
                    c << "    for (int i = 0 ; i < nTimestepGroups ; i++)" << endl;
                    c << "    {" << endl;
                    if (hasoptions)
                        c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i*nBlock], readOptions);" << endl;
                    else
                        c << "        ffl[i] = new avt"<<name<<"FileFormat(list[i*nBlock]);" << endl;
                    c << "    }" << endl;
                    c << "    avtMTMDFileFormatInterface *inter " << endl;
                    c << "           = new avtMTMDFileFormatInterface(ffl, nTimestepGroups);" << endl;
                    c << "    return new avtGenericDatabase(inter);" << endl;
                }
                else
                {
                    throw QString("Unknown database type '%1'").arg(dbtype);
                }
                c << "}" << endl;
            }

            if (hasoptions)
            {
                // Write GetReadOptions
                funcName = name + "CommonPluginInfo::GetReadOptions";
                if(!ReplaceBuiltin(c, funcName))
                {
                    c << "" << endl;
                    c << "// ****************************************************************************" << endl;
                    c << "//  Method: "<<funcName << endl;
                    c << "//" << endl;
                    c << "//  Purpose:" << endl;
                    c << "//      Gets the read options." << endl;
                    c << "//" << endl;
                    c << "//  Programmer: generated by xml2info" << endl;
                    c << "//  Creation:   omitted"<< endl;
                    c << "//" << endl;
                    c << "// ****************************************************************************" << endl;
                    c << endl;
                    c << "DBOptionsAttributes *" << endl;
                    c << funcName<<"() const" << endl;
                    c << "{" << endl;
                    c << "    return Get"<<name<<"ReadOptions();" << endl;
                    c << "}" << endl;
                }
                funcName = name + "CommonPluginInfo::GetWriteOptions";
                if(!ReplaceBuiltin(c, funcName))
                {
                    c << "// ****************************************************************************" << endl;
                    c << "//  Method: "<<funcName << endl;
                    c << "//" << endl;
                    c << "//  Purpose:" << endl;
                    c << "//      Gets the write options." << endl;
                    c << "//" << endl;
                    c << "//  Programmer: generated by xml2info" << endl;
                    c << "//  Creation:   omitted"<< endl;
                    c << "//" << endl;
                    c << "// ****************************************************************************" << endl;
                    c << endl;
                    c << "DBOptionsAttributes *" << endl;
                    c << funcName << "() const" << endl;
                    c << "{" << endl;
                    c << "    return Get"<<name<<"WriteOptions();" << endl;
                    c << "}" << endl;
                }
                funcName = name + "CommonPluginInfo::SetReadOptions";
                ReplaceBuiltin(c, funcName);
            }
#ifdef WRITE_LICENSE
            funcName = name + "CommonPluginInfo::GetLicense";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//      Gets the write options." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "std::string" << endl;
                c << funcName << "() const" << endl;
                c << "{" << endl;
                c << "    return std::string();" << endl;
                c << "}" << endl;
            }
#endif
        }
        else
        {
            c << copyright_str << endl;
            c << "// ************************************************************************* //" << endl;
            c << "//  File: "<<name<<"CommonPluginInfo.C" << endl;
            c << "// ************************************************************************* //" << endl;
            c << endl;
            c << "#include <"<<name<<"PluginInfo.h>" << endl;
            c << "#include <"<<atts->name<<".h>" << endl;
            c << endl;
            if (type=="operator" && createExpression)
            {
                c << "#include <Expression.h>" << endl;
                c << "#include <ExpressionList.h>" << endl;
                c << "#include <avtDatabaseMetaData.h>" << endl;
                c << "#include <avtMeshMetaData.h>" << endl;
                c << "#include <avtSubsetsMetaData.h>" << endl;
                c << "#include <avtScalarMetaData.h>" << endl;
                c << "#include <avtVectorMetaData.h>" << endl;
                c << "#include <avtTensorMetaData.h>" << endl;
                c << "#include <avtSymmetricTensorMetaData.h>" << endl;
                c << "#include <avtArrayMetaData.h>" << endl;
                c << "#include <avtMaterialMetaData.h>" << endl;
                c << "#include <avtSpeciesMetaData.h>" << endl;
                c << "#include <avtCurveMetaData.h>" << endl;
                c << "#include <avtLabelMetaData.h>" << endl;

                c << endl;
            }
            QString funcName = name + "CommonPluginInfo::AllocAttributes";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Return a pointer to a newly allocated attribute subject." << endl;
                c << "//" << endl;
                c << "//  Returns:    A pointer to the newly allocated attribute subject." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "AttributeSubject *" << endl;
                c << funcName << "()" << endl;
                c << "{" << endl;
                c << "    return new "<<atts->name<<";" << endl;
                c << "}" << endl;
            }
            c << endl;
            funcName = name + "CommonPluginInfo::CopyAttributes";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Copy a "<<name<<" attribute subject." << endl;
                c << "//" << endl;
                c << "//  Arguments:" << endl;
                c << "//    to        The destination attribute subject." << endl;
                c << "//    from      The source attribute subject." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "void " << endl;
                c << funcName<<"(AttributeSubject *to," << endl;
                c << "    AttributeSubject *from)" << endl;
                c << "{" << endl;
                c << "    *(("<<atts->name<<" *) to) = *(("<<atts->name<<" *) from);" << endl;
                c << "}" << endl;
            }
            funcName = name + "CommonPluginInfo::GetCreatedExpressions";
            if (createExpression && !ReplaceBuiltin(c, funcName))
            {
                c << endl;
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//      Gets the expressions created by this operator." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "ExpressionList * " << endl;
                c << funcName<<"(const avtDatabaseMetaData *md) const" << endl;
                c << "{" << endl;
                std::vector<QString> intypes = SplitValues(exprInType);
                bool doMesh = false, doScalar = false, doVector = false, doTensor = false, doMaterial = false;
                bool doSubset = false, doSpecies = false, doCurve = false, doSymmTensor = false;
                bool doLabel = false, doArray = false;
                for (size_t i = 0 ; i < intypes.size() ; i++)
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
                for (size_t i = 0 ; i < outtypes.size() ; i++)
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
                c << "    int i;" << endl;
                c << "    char name[1024], defn[1024];" << endl;
                c << "    ExpressionList *el = new ExpressionList;" << endl;
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
                    c << "    const ExpressionList &oldEL = md->GetExprList();" << endl;
                    c << "    for (i = 0 ; i < oldEL.GetNumExpressions() ; i++)" << endl;
                    c << "    {" << endl;
                    c << "        const Expression &e = oldEL.GetExpressions(i);" << endl;
                    if (doScalar)
                        AddExpressionFromExpr(c, name, outExprTypes, QString("ScalarMeshVar"));
                    if (doVector)
                        AddExpressionFromExpr(c, name, outExprTypes, QString("VectorMeshVar"));
                    if (doTensor || doSymmTensor)
                        AddExpressionFromExpr(c, name, outExprTypes, QString("TensorMeshVar"));
                    c << "    }" << endl;
                } 
                c << "    return el;" << endl;
                c << "}" << endl;
                c << endl;
            }

            if (type=="plot")
            {
                funcName = name + "CommonPluginInfo::GetVariableTypes";
                if(!ReplaceBuiltin(c, funcName))
                {
                    c << "// ****************************************************************************" << endl;
                    c << "// Method: "<<funcName << endl;
                    c << "//" << endl;
                    c << "// Purpose: " << endl;
                    c << "//   Returns a flag indicating the types of variables that can be put in the" << endl;
                    c << "//   plot's variable list." << endl;
                    c << "//" << endl;
                    c << "// Returns:    A flag indicating the types of variables that can be put in" << endl;
                    c << "//             the plot's variable list." << endl;
                    c << "//" << endl;
                    c << "//  Programmer: generated by xml2info" << endl;
                    c << "//  Creation:   omitted"<< endl;
                    c << "//" << endl;
                    c << "// Modifications:" << endl;
                    c << "//   " << endl;
                    c << "// ****************************************************************************" << endl;
                    c << endl;
                    c << "int" << endl;
                    c << funcName<<"() const" << endl;
                    c << "{" << endl;
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
                    c << ";" << endl;
                    c << "}" << endl;
                    c << endl;
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

        c << copyright_str << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//  File: "<<name<<"GUIPluginInfo.C" << endl;
        c << "// ************************************************************************* //" << endl;
        c << endl;
        c << "#include <"<<name<<"PluginInfo.h>" << endl;
        c << "#include <"<<atts->name<<".h>" << endl;
        c << "#include <QApplication>" << endl;
        if (type=="operator")
            c << "#include <Qvis"<<name<<"Window.h>" << endl;
        else if (type=="plot")
            c << "#include <Qvis"<<name<<"PlotWindow.h>" << endl;
        c << endl;
        if (type=="operator")
            c << "VISIT_OPERATOR_PLUGIN_ENTRY(" << name << ",GUI)" << endl;
        else if (type=="plot")
            c << "VISIT_PLOT_PLUGIN_ENTRY(" << name << ",GUI)" << endl;
        c << endl;
        QString funcName = name + "GUIPluginInfo::GetMenuName";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<funcName << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a pointer to the name to use in the GUI menu." << endl;
            c << "//" << endl;
            c << "//  Returns:    A pointer to the name to use in the GUI menu." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "QString *" << endl;
            c << funcName<<"() const" << endl;
            c << "{" << endl;
            if(type == "plot")
                c << "    return new QString(qApp->translate(\"PlotNames\", \""<<label<<"\"));" << endl;
            else if(type == "operator")
                c << "    return new QString(qApp->translate(\"OperatorNames\", \""<<label<<"\"));" << endl;
            c << "}" << endl;
            c << endl;
        }
        c << endl;
        funcName = name + "GUIPluginInfo::CreatePluginWindow";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<funcName << endl;
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
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "QvisPostableWindowObserver *" << endl;
            c << funcName << "(int type, AttributeSubject *attr," << endl;
            c << "    const QString &caption, const QString &shortName, QvisNotepadArea *notepad)" << endl;
            c << "{" << endl;
            if (type=="operator")
                c << "    return new Qvis"<<name<<"Window(type, ("<<atts->name<<" *)attr," << endl
                  << "        caption, shortName, notepad);" << endl;
            else if (type=="plot")
                c << "    return new Qvis"<<name<<"PlotWindow(type, ("<<atts->name<<" *)attr," << endl
                  << "        caption, shortName, notepad);" << endl;
            c << "}" << endl;
        }

        WriteOverrideDefinition(c, name + "GUIPluginInfo::CreatePluginWizard");

        if(iconFile.length() > 0)
        {
            funcName = name + "GUIPluginInfo::XPMIconData";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << endl;
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Return a pointer to the icon data." << endl;
                c << "//" << endl;
                c << "//  Returns:    A pointer to the icon data." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "#include <" << iconFile << ">" << endl;
                c << "const char **" << endl;
                c << funcName << "() const" << endl;
                c << "{" << endl;        
                c << "    return " << iconFile.left(iconFile.length() - 4) << "_xpm;" << endl;
                c << "}" << endl;
            }
        }

        c << endl;
        WriteUserDefinedFunctions(c, name + "GUIPluginInfo", false);
    }

    void WriteViewerInfoSource(QTextStream &c)
    {
        if (type=="database")
            return;

        c << copyright_str << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//  File: "<<name<<"ViewerPluginInfo.C" << endl;
        c << "// ************************************************************************* //" << endl;
        c << endl;
        c << "#include <"<<name<<"PluginInfo.h>" << endl;
        if (type=="operator")
            c << "#include <"<<atts->name<<".h>" << endl;
        c << endl;

        if (type=="operator")
            c << "VISIT_OPERATOR_PLUGIN_ENTRY_EV(" << name << ",Viewer)" << endl;
        else if (type=="plot")
            c << "VISIT_PLOT_PLUGIN_ENTRY_EV(" << name << ",Viewer)" << endl;
        c << endl;

        if(iconFile.length() > 0)
        {
            QString funcName = name + "ViewerPluginInfo::XPMIconData";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << endl;
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName<< endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Return a pointer to the icon data." << endl;
                c << "//" << endl;
                c << "//  Returns:    A pointer to the icon data." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "#include <" << iconFile << ">" << endl;
                c << "const char **" << endl;
                c << funcName<<"() const" << endl;
                c << "{" << endl;        
                c << "    return " << iconFile.left(iconFile.length() - 4) << "_xpm;" << endl;
                c << "}" << endl;
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

        c << endl;
        WriteUserDefinedFunctions(c, name + "ViewerPluginInfo", false);
    }

    void WriteViewerEngineInfoSource(QTextStream &c)
    {
        if (type=="database")
            return;

        c << copyright_str << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//  File: "<<name<<"ViewerEnginePluginInfo.C" << endl;
        c << "// ************************************************************************* //" << endl;
        c << endl;
        c << "#include <"<<name<<"PluginInfo.h>" << endl;
        if (type=="plot")
            c << "#include <avt"<<name<<"Plot.h>" << endl;
        c << "#include <"<<atts->name<<".h>" << endl;
        c << endl;
        c << "//" << endl;
        c << "// Storage for static data elements." << endl;
        c << "//" << endl;
        c << atts->name<<" *"<<name<<"ViewerEnginePluginInfo::clientAtts = NULL;" << endl;
        c << atts->name<<" *"<<name<<"ViewerEnginePluginInfo::defaultAtts = NULL;" << endl;
        c << endl;

        QString funcName = name + "ViewerEnginePluginInfo::InitializeGlobalObjects";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "//  Method:  "<<funcName << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Initialize the "<<type<<" atts." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << "void" << endl;
            c << funcName << "()" << endl;
            c << "{" << endl;
            c << "    if ("<<name<<"ViewerEnginePluginInfo::clientAtts == NULL)" << endl;
            c << "    {" << endl;
            c << "        "<<name<<"ViewerEnginePluginInfo::clientAtts  = new "<<atts->name<<";" << endl;
            c << "        "<<name<<"ViewerEnginePluginInfo::defaultAtts = new "<<atts->name<<";" << endl;
            c << "    }" << endl;
            c << "}" << endl;
        }
        c << endl;
        funcName = name + "ViewerEnginePluginInfo::GetClientAtts";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<funcName << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a pointer to the viewer client attributes." << endl;
            c << "//" << endl;
            c << "//  Returns:    A pointer to the viewer client attributes." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "AttributeSubject *" << endl;
            c << funcName<<"()" << endl;
            c << "{" << endl;
            c << "    return clientAtts;" << endl;
            c << "}" << endl;
            c << endl;
        }
        funcName = name + "ViewerEnginePluginInfo::GetDefaultAtts";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<funcName << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a pointer to the viewer default attributes." << endl;
            c << "//" << endl;
            c << "//  Returns:    A pointer to the viewer default attributes." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "AttributeSubject *" << endl;
            c << funcName<<"()" << endl;
            c << "{" << endl;
            c << "    return defaultAtts;" << endl;
            c << "}" << endl;
        }
        c << endl;
        funcName = name + "ViewerEnginePluginInfo::SetClientAtts";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<funcName << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Set the viewer client attributes." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//    atts      A pointer to the new client attributes." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "void" << endl;
            c << funcName << "(AttributeSubject *atts)" << endl;
            c << "{" << endl;
            c << "    *clientAtts = *("<<atts->name<<" *)atts;" << endl;
            c << "    clientAtts->Notify();" << endl;
            c << "}" << endl;
        }
        c << endl;
        funcName = name + "ViewerEnginePluginInfo::GetClientAtts";
        if(!ReplaceBuiltin(c, funcName + "1"))
        {
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<funcName<< endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Get the viewer client attributes." << endl;
            c << "//" << endl;
            c << "//  Arguments:" << endl;
            c << "//    atts      A pointer to return the client default attributes in." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "void" << endl;
            c << funcName<<"(AttributeSubject *atts)" << endl;
            c << "{" << endl;
            c << "    *("<<atts->name<<" *)atts = *clientAtts;" << endl;
            c << "}" << endl;
        }
        c << endl;
        if (type=="operator")
        {
            funcName = name + "ViewerEnginePluginInfo::GetClientAtts";
            if(ReplaceBuiltin(c, funcName + "3"))
            {
                c << endl;
            }

            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::Removeable");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::Moveable");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::AllowsSubsequentOperators");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::GetCreatedVariables");

            QString funcName = name + "ViewerEnginePluginInfo::InitializeOperatorAtts";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Initialize the "<<type<<" attributes to the default attributes." << endl;
                c << "//" << endl;
                c << "//  Arguments:" << endl;
                c << "//    atts        The attribute subject to initialize." << endl;
                c << "//    plot        The viewer plot that owns the operator." << endl;
                c << "//    fromDefault True to initialize the attributes from the defaults. False" << endl;
                c << "//                to initialize from the current attributes." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "void" << endl;
                c << funcName << "(AttributeSubject *atts," << endl
                  << "                                              const avtPlotMetaData &plot," << endl
                  << "                                              const bool fromDefault)" << endl;
                c << "{" << endl;
                c << "    if (fromDefault)" << endl;
                c << "        *("<<atts->name<<"*)atts = *defaultAtts;" << endl;
                c << "    else" << endl;
                c << "        *("<<atts->name<<"*)atts = *clientAtts;" << endl;
                c << "" << endl;
                c << "    UpdateOperatorAtts(atts, plot);" << endl;
                c << "}" << endl;
                c << "" << endl;
            }

            funcName = name + "ViewerEnginePluginInfo::UpdateOperatorAtts";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Update the "<<type<<" attributes when using operator expressions." << endl;
                c << "//" << endl;
                c << "//  Arguments:" << endl;
                c << "//    atts        The attribute subject to update." << endl;
                c << "//    plot        The viewer plot that owns the operator." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "void" << endl;
                c << funcName << "(AttributeSubject *atts, const avtPlotMetaData &plot)" << endl;
                c << "{" << endl;
                c << "}" << endl;
                c << "" << endl;
            }

            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::GetOperatorVarDescription");
#if 0
            funcName = name + "ViewerEnginePluginInfo::GetOperatorVarDescription";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Return the "<<type<<"  variable description." << endl;
                c << "//" << endl;
                c << "//  Arguments:" << endl;
                c << "//    atts        The current attributes." << endl;
                c << "//    plot        The viewer plot that owns the operator." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "std::string" << endl;
                c << funcName << "(AttributeSubject *atts," << endl
                  << "                                              const avtPlotMetaData &plot)" << endl;
                c << "{" << endl;
                c << "    return std::string(\"\");" << endl;
                c << "}" << endl;
                c << "" << endl;
            }
#endif
        }
        if (type=="plot")
        {
            funcName = name + "ViewerEnginePluginInfo::AllocAvtPlot";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Return a pointer to a newly allocated avt plot." << endl;
                c << "//" << endl;
                c << "//  Returns:    A pointer to the newly allocated avt plot." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "avtPlot *" << endl;
                c << funcName<<"()" << endl;
                c << "{" << endl;
                c << "    return new avt"<<name<<"Plot;" << endl;
                c << "}" << endl;
            }
            c << endl;

            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::ProvidesLegend");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::PermitsCurveViewScaling");
            WriteOverrideDefinition(c, name + "ViewerEnginePluginInfo::Permits2DViewScaling");

            funcName = name + "ViewerEnginePluginInfo::InitializePlotAtts";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//    Initialize the "<<type<<" attributes to the default attributes." << endl;
                c << "//" << endl;
                c << "//  Arguments:" << endl;
                c << "//    atts      The attribute subject to initialize." << endl;
                c << "//    plot      The viewer plot whose attributes are getting initialized." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted"<< endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << endl;
                c << "void" << endl;
                c << funcName<<"(AttributeSubject *atts," << endl
                  << "    const avtPlotMetaData &)" << endl;
                c << "{" << endl;
                c << "    *("<<atts->name<<"*)atts = *defaultAtts;" << endl;
                c << "}" << endl;
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
            c << "// ****************************************************************************" << endl;
            c << "//  Method: "<<funcName << endl;
            c << "//" << endl;
            c << "//  Purpose:" << endl;
            c << "//    Return a pointer to the name to use in the viewer menus." << endl;
            c << "//" << endl;
            c << "//  Returns:    A pointer to the name to use in the viewer menus." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "const char *" << endl;
            c << funcName<<"() const" << endl;
            c << "{" << endl;
            c << "    return \""<<label<<"\";" << endl;
            c << "}" << endl;
        }

        c << endl;
        WriteUserDefinedFunctions(c, name + "ViewerEnginePluginInfo", false);
    }


    void WriteMDServerInfoSource(QTextStream &c)
    {
        if (type=="database")
        {
            c << copyright_str << endl;
            c << "#include <"<<name<<"PluginInfo.h>" << endl;
            c << "" << endl;
            c << "VISIT_DATABASE_PLUGIN_ENTRY(" << name << ",MDServer)" << endl;
            c << endl;
            c << "// this makes compilers happy... remove if we ever have functions here" << endl;
            c << "void "<<name<<"MDServerPluginInfo::dummy()" << endl;
            c << "{" << endl;
            c << "}" << endl;
            c << endl;
            WriteUserDefinedFunctions(c, name + "MDServerPluginInfo", false);
        }
    }


    void WriteEngineInfoSource(QTextStream &c)
    {
        if (type=="database")
        {
            c << copyright_str << endl;
            c << "#include <"<<name<<"PluginInfo.h>" << endl;
            if (haswriter)
                c << "#include <avt"<<name<<"Writer.h>" << endl;
            c << endl;
            c << "VISIT_DATABASE_PLUGIN_ENTRY(" << name << ",Engine)" << endl;
            c << endl;

            QString funcName = name + "EnginePluginInfo::GetWriter";
            if(!ReplaceBuiltin(c, funcName))
            {
                c << "// ****************************************************************************" << endl;
                c << "//  Method: "<<funcName << endl;
                c << "//" << endl;
                c << "//  Purpose:" << endl;
                c << "//      Sets up a "<<name<<" writer." << endl;
                c << "//" << endl;
                c << "//  Returns:    A "<<name<<" writer." << endl;
                c << "//" << endl;
                c << "//  Programmer: generated by xml2info" << endl;
                c << "//  Creation:   omitted" << endl;
                c << "//" << endl;
                c << "// ****************************************************************************" << endl;
                c << "avtDatabaseWriter *" << endl;
                c << ""<<funcName << "(void)" << endl;
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
            }
            c << endl;
        }
        else
        {
            c << copyright_str << endl;
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
            if (type=="operator")
                c << "VISIT_OPERATOR_PLUGIN_ENTRY_EV(" << name << ",Engine)" << endl;
            else if (type=="plot")
                c << "VISIT_PLOT_PLUGIN_ENTRY_EV(" << name << ",Engine)" << endl;
            c << endl;

            if (type=="operator")
            {
                QString funcName = name + "EnginePluginInfo::AllocAvtPluginFilter";
                if(!ReplaceBuiltin(c, funcName))
                {
                    c << "// ****************************************************************************" << endl;
                    c << "//  Method: "<<funcName << endl;
                    c << "//" << endl;
                    c << "//  Purpose:" << endl;
                    c << "//    Return a pointer to a newly allocated avtPluginFilter." << endl;
                    c << "//" << endl;
                    c << "//  Returns:    A pointer to the newly allocated avtPluginFilter." << endl;
                    c << "//" << endl;
                    c << "//  Programmer: generated by xml2info" << endl;
                    c << "//  Creation:   omitted"<< endl;
                    c << "//" << endl;
                    c << "// ****************************************************************************" << endl;
                    c << endl;
                    c << "avtPluginFilter *" << endl;
                    c << funcName<<"()" << endl;
                    c << "{" << endl;
                    c << "    return new avt"<<name<<"Filter;" << endl;
                    c << "}" << endl;
                }
            }
        }
        WriteUserDefinedFunctions(c, name + "EnginePluginInfo", false);
    }

    void WriteScriptingInfoSource(QTextStream &c)
    {
        if (type=="database")
            return;

        c << copyright_str << endl;
        c << "// ************************************************************************* //" << endl;
        c << "//                        "<<name<<"ScriptingPluginInfo.C" << endl;
        c << "// ************************************************************************* //" << endl;
        c << "#include <Py"<<atts->name<<".h>" << endl;
        c << "#include <"<<name<<"PluginInfo.h>" << endl;
        c << "" << endl;
        if (type=="operator")
            c << "VISIT_OPERATOR_PLUGIN_ENTRY(" << name << ",Scripting)" << endl;
        else if (type=="plot")
            c << "VISIT_PLOT_PLUGIN_ENTRY(" << name << ",Scripting)" << endl;
        c << endl;

        QString funcName = name + "ScriptingPluginInfo::InitializePlugin";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "// Method: "<<funcName << endl;
            c << "//" << endl;
            c << "// Purpose: " << endl;
            c << "//   Calls the initialization function for the plugin." << endl;
            c << "//" << endl;
            c << "// Arguments:" << endl;
            c << "//   subj    : A pointer to the plugin's state object." << endl;
            c << "//   data    : A pointer to data to be used by the observer function." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// Modifications:" << endl;
            c << "//   " << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void" << endl;
            c << funcName << "(AttributeSubject *subj," << endl;
            c << "    void *data)" << endl;
            c << "{" << endl;
            c << "    Py"<<atts->name<<"_StartUp(("<<atts->name<<" *)subj, data);" << endl;
            c << "}" << endl;
        }
        c << endl;
        funcName = name + "ScriptingPluginInfo::GetMethodTable";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "// Method: "<<funcName << endl;
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
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// Modifications:" << endl;
            c << "//   " << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "void *" << endl;
            c << funcName << "(int *nMethods)" << endl;
            c << "{" << endl;
            c << "    return Py"<<atts->name<<"_GetMethodTable(nMethods);" << endl;
            c << "}" << endl;
        }
        c << endl;
        funcName = name + "ScriptingPluginInfo::TypesMatch";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "// Method: " << funcName << endl;
            c << "//" << endl;
            c << "// Purpose: " << endl;
            c << "//   Returns whether or not the input PyObject is "<<name<<" plot attributes." << endl;
            c << "//" << endl;
            c << "// Arguments:" << endl;
            c << "//   pyobject : A PyObject cast to void*." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// Modifications:" << endl;
            c << "//   " << endl;
            c << "// ****************************************************************************" << endl;
            c << "" << endl;
            c << "bool" << endl;
            c << funcName << "(void *pyobject)" << endl;
            c << "{" << endl;
            c << "    return Py"<<atts->name<<"_Check((PyObject *)pyobject);" << endl;
            c << "}" << endl;
        }
        c << endl;
        funcName = name + "ScriptingPluginInfo::GetLogString";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "// Method: " << funcName << endl;
            c << "//" << endl;
            c << "// Purpose: " << endl;
            c << "//   Gets a string representation of the current attributes." << endl;
            c << "//" << endl;
            c << "// Arguments:" << endl;
            c << "//   val : Whether or not to log state information." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// Modifications:" << endl;
            c << "//   " << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "char *" << endl;
            c << funcName << "()" << endl;
            c << "{" << endl;
            c << "    std::string s(Py" << atts->name << "_GetLogString());" << endl;
            c << "    char *v = new char[s.size() + 1];" << endl;
            c << "    strcpy(v, s.c_str());" << endl;
            c << "    return v;" << endl;
            c << "}" << endl;
        }
        c << endl;
        funcName = name + "ScriptingPluginInfo::SetDefaults";
        if(!ReplaceBuiltin(c, funcName))
        {
            c << "// ****************************************************************************" << endl;
            c << "// Method: " << funcName << endl;
            c << "//" << endl;
            c << "// Purpose: " << endl;
            c << "//   Used to set the default values for a plugin's state object." << endl;
            c << "//" << endl;
            c << "// Arguments:" << endl;
            c << "//   atts : The new state." << endl;
            c << "//" << endl;
            c << "//  Programmer: generated by xml2info" << endl;
            c << "//  Creation:   omitted"<< endl;
            c << "//" << endl;
            c << "// Modifications:" << endl;
            c << "//   " << endl;
            c << "// ****************************************************************************" << endl;
            c << endl;
            c << "void" << endl;
            c << funcName << "(const AttributeSubject *atts)" << endl;
            c << "{" << endl;
            c << "    Py" << atts->name << "_SetDefaults((const " << atts->name << " *)atts);" << endl;
            c << "}" << endl;
        }

        WriteUserDefinedFunctions(c, name + "ScriptingPluginInfo", false);
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       InfoGeneratorPlugin

#endif
