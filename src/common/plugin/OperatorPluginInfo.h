// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            OperatorPluginInfo.h                           //
// ************************************************************************* //

#ifndef OPERATOR_PLUGIN_INFO_H
#define OPERATOR_PLUGIN_INFO_H
#include <plugin_exports.h>
#include <plugin_vartypes.h>
#include <plugin_entry_point.h>
#include <stdio.h>

// Forward declarations.
class AttributeSubject;
class QvisNotepadArea;
class QvisPostableWindowObserver;
class QvisWizard;
class QWidget;
class QString;
class avtPlotMetaData;
class avtPluginFilter;
class avtDatabaseMetaData;
class ViewerPlot;
class ExpressionList;

#include <vector>
#include <string>

// ****************************************************************************
//  Class: *OperatorPluginInfo
//
//  Purpose:
//    Four classes that provide all the information about an operator plugin.
//    The information is broken up into four classes relevant to the gui, the
//    viewer, the engine, or all three.
//
//  Programmer: Jeremy Meredith
//  Creation:   July 25, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 13:31:21 PDT 2001
//    Added Common info.  Made the specific ones inherit from the generic ones.
//
//    Brad Whitlock, Thu Jul 26 12:52:12 PDT 2001
//    Added scripting plugins.
//
//    Brad Whitlock, Mon May 6 14:18:15 PST 2002
//    I added the SetDefaults method for scripting plugins.
//
//    Brad Whitlock, Wed Mar 12 14:14:57 PST 2003
//    I added the XPMIconData method for GUI and Viewer plugins.
//
//    Brad Whitlock, Thu Apr 10 13:14:52 PST 2003
//    I added Removeable, Moveable, and AllowsSubsequentOperators.
//
//    Kathleen Bonnell, Thu Sep 11 10:18:54 PDT 2003
//    I added a bool argument to InitializeOperatorAtts. 
//
//    Jeremy Meredith, Wed Nov  5 10:28:29 PST 2003
//    Added ability to disable plugins by default.
//
//    Brad Whitlock, Tue Dec 14 09:44:04 PDT 2004
//    Added support for operator wizards.
//
//    Hank Childs, Tue Mar 22 16:06:15 PST 2005
//    Made destructor virtual.
//
//    Brad Whitlock, Thu Jan 5 14:40:42 PST 2006
//    I removed explicit support for logging from the scripting plugins.
//
//    Brad Whitlock, Tue Apr 25 16:46:49 PST 2006
//    Added new methods to the operator plugin that allow it to control
//    the types of variables that the GUI puts in the variable list.
//
//    Jeremy Meredith, Mon Feb 18 17:44:40 EST 2008
//    Added way for operators to create new variables.  The expression
//    definition is intended for intialization; the filter should override
//    the actual values of the variable.
//
//    Brad Whitlock, Fri Apr 25 10:11:20 PDT 2008
//    Made GetMenuName in the GUI info return QString so we can 
//    internationalize plot names. Added const to strings returned from
//    GeneralOperatorPluginInfo.
//
//    Brad Whitlock, Thu Feb  4 16:04:19 PST 2010
//    I added a GetCategoryName method.
//
//    Rob Sisneros, Sun Aug 29 20:13:10 CDT 2010
//    Add infrastructure for operators to create variables.
//
//    Hank Childs, Tue Aug 31 10:20:08 PDT 2010
//    Remove unused method GetCreateVariables.
//
//    Hank Childs, Thu Dec 30 12:15:39 PST 2010
//    Add the ability to create expressions based on scalars, vectors, or
//    tensors.
//
//    Brad Whitlock, Wed Sep 17 12:22:21 PDT 2014
//    Added common base class for viewer/engine plugins.
//
//    Burlen Loring, Thu Oct  8 14:31:15 PDT 2015
//    clean up a few compiler warnings
//
// ****************************************************************************

class PLUGIN_API GeneralOperatorPluginInfo
{
  public:
    virtual ~GeneralOperatorPluginInfo() {;};
    virtual const char *GetName() const = 0;
    virtual const char *GetVersion() const = 0;
    virtual const char *GetID() const = 0;
    virtual bool  EnabledByDefault() const { return true; }
    virtual const char *GetCategoryName() const { return ""; }
};

class PLUGIN_API CommonOperatorPluginInfo : public virtual GeneralOperatorPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes() = 0;
    virtual void CopyAttributes(AttributeSubject *to,
                                AttributeSubject *from) = 0;
    virtual ExpressionList *GetCreatedExpressions(const avtDatabaseMetaData *) const {return NULL;}
    virtual bool GetUserSelectable() const { return true; }
    virtual int GetVariableTypes() const { return 0; }
    virtual int GetVariableMask() const { return ~0; }
};

class PLUGIN_API GUIOperatorPluginInfo : public virtual CommonOperatorPluginInfo
{
  public:
    virtual QString *GetMenuName() const = 0;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad) = 0;
    virtual QvisWizard *CreatePluginWizard(AttributeSubject *,
        QWidget *, const char * = 0)
    {
        return 0;
    }
    virtual const char **XPMIconData() const { return 0; }
};

class PLUGIN_API ViewerEngineOperatorPluginInfo : public virtual CommonOperatorPluginInfo
{
  public:
    virtual AttributeSubject *GetClientAtts() = 0;
    virtual AttributeSubject *GetDefaultAtts() = 0;
    virtual void SetClientAtts(AttributeSubject *atts) = 0;
    virtual void GetClientAtts(AttributeSubject *atts) = 0;
    virtual void GetClientAtts(AttributeSubject *atts,
                               const bool, const bool)
    { GetClientAtts(atts); };

    virtual void InitializeOperatorAtts(AttributeSubject *atts,
                                        const avtPlotMetaData &plot,
                                        const bool fromDefault) = 0;
    virtual void UpdateOperatorAtts(AttributeSubject *atts,
                                    const avtPlotMetaData &plot) = 0;
    virtual std::string GetOperatorVarDescription(AttributeSubject *,
                                                  const avtPlotMetaData &)
                                                  { return std::string(); }
    virtual const char *GetMenuName() const = 0;

    virtual bool Removeable() const { return true; }
    virtual bool Moveable() const { return true; }
    virtual bool AllowsSubsequentOperators() const { return true; }
};

class PLUGIN_API ViewerOperatorPluginInfo : public virtual ViewerEngineOperatorPluginInfo
{
  public:
    virtual const char **XPMIconData() const { return 0; }
};

class PLUGIN_API EngineOperatorPluginInfo : public virtual ViewerEngineOperatorPluginInfo
{
  public:
    virtual avtPluginFilter *AllocAvtPluginFilter() = 0;
};

class PLUGIN_API ScriptingOperatorPluginInfo : public virtual CommonOperatorPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data) = 0;
    virtual void *GetMethodTable(int *nMethods) = 0;
    virtual char *GetLogString() = 0;
    virtual bool TypesMatch(void *) { return false; }
    virtual void SetDefaults(const AttributeSubject *) = 0;
};

#endif
