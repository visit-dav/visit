// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SimOperatorPluginManager.h>

#include <OperatorPluginInfo.h>

// ****************************************************************************
// Class: ViewerWrappedEngineOperatorPluginInfo
//
// Purpose:
//   Lets an EngineOperatorPluginInfo pretend to be a ViewerOperatorPluginInfo.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 16:16:11 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class ViewerWrappedEngineOperatorPluginInfo : public ViewerOperatorPluginInfo
{
public:
    ViewerWrappedEngineOperatorPluginInfo(EngineOperatorPluginInfo *e) : eInfo(e)
    {
    }

    virtual ~ViewerWrappedEngineOperatorPluginInfo()
    {
    }

    // GeneralOperatorPluginInfo methods
    virtual const char *GetName() const
    {
        return eInfo->GetName();
    }

    virtual const char *GetVersion() const
    {
        return eInfo->GetVersion();
    }

    virtual const char *GetID() const
    {
        return eInfo->GetID();
    }

    virtual bool  EnabledByDefault() const
    {
        return eInfo->EnabledByDefault();
    }

    virtual const char *GetCategoryName() const
    {
        return eInfo->GetCategoryName();
    }

    // CommonOperatorPluginInfo methods
    virtual AttributeSubject *AllocAttributes()
    {
        return eInfo->AllocAttributes();
    }

    virtual void CopyAttributes(AttributeSubject *to,
                                AttributeSubject *from)
    {
        eInfo->CopyAttributes(to, from);
    }

    virtual ExpressionList *GetCreatedExpressions(const avtDatabaseMetaData *md)
    {
        return eInfo->GetCreatedExpressions(md);
    }

    virtual bool GetUserSelectable() const
    {
        return eInfo->GetUserSelectable();
    }

    virtual int GetVariableTypes() const
    {
        return eInfo->GetVariableTypes();
    }

    virtual int GetVariableMask() const
    {
        return eInfo->GetVariableMask();
    }

    // ViewerEngineOperatorPluginInfo methods.
    virtual AttributeSubject *GetClientAtts()
    {
        return eInfo->GetClientAtts();
    }

    virtual AttributeSubject *GetDefaultAtts()
    {
        return eInfo->GetDefaultAtts();
    }

    virtual void SetClientAtts(AttributeSubject *atts)
    {
        eInfo->SetClientAtts(atts);
    }

    virtual void GetClientAtts(AttributeSubject *atts)
    {
        eInfo->GetClientAtts(atts);
    }

    virtual void GetClientAtts(AttributeSubject *atts,
                               const bool activeOperator,
                               const bool applyToAll)
    {
        eInfo->GetClientAtts(atts, activeOperator, applyToAll);
    }


    virtual void InitializeOperatorAtts(AttributeSubject *atts,
                                        const avtPlotMetaData &plot,
                                        const bool fromDefault)
    {
        eInfo->InitializeOperatorAtts(atts, plot, fromDefault);
    }

    virtual void UpdateOperatorAtts(AttributeSubject *atts,
                                    const avtPlotMetaData &plot)
    {
        eInfo->UpdateOperatorAtts(atts, plot);
    }

    virtual std::string GetOperatorVarDescription(AttributeSubject *atts,
                                                  const avtPlotMetaData &plot)
    {
        return eInfo->GetOperatorVarDescription(atts, plot);
    }

    virtual const char *GetMenuName() const
    {
        return eInfo->GetMenuName();
    }

    virtual bool Removeable() const
    {
        return eInfo->Removeable();
    }

    virtual bool Moveable() const
    {
        return eInfo->Moveable();
    }

    virtual bool AllowsSubsequentOperators() const
    {
        return eInfo->AllowsSubsequentOperators();
    }

    // We don't need to override ViewerOperatorPluginInfo default methods
private:
    EngineOperatorPluginInfo *eInfo;
};

// ****************************************************************************
// Method: SimOperatorPluginManager::SimOperatorPluginManager
//
// Purpose:
//   Constructor.
//
// Arguments:
//   mgr : The engine plugin manager.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 16:32:07 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimOperatorPluginManager::SimOperatorPluginManager() : OperatorPluginManager(), wrapped()
{
}

// ****************************************************************************
// Method: SimOperatorPluginManager::~SimOperatorPluginManager
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 16:32:07 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimOperatorPluginManager::~SimOperatorPluginManager()
{
    std::map<std::string, ViewerWrappedEngineOperatorPluginInfo *>::iterator it;
    for(it = wrapped.begin(); it != wrapped.end(); ++it)
        delete it->second;
}

// ****************************************************************************
// Method: SimOperatorPluginManager::GetViewerPluginInfo
//
// Purpose:
//   Retrieves a viewer plugin info object (internally using the engine plugin 
//   manager and engine plugins).
//
// Arguments:
//   id : The id of the plugin for which we want a viewer plugin.
//
// Returns:    A viewer plugin implemented by an engine plugin.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 16:33:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerOperatorPluginInfo *
SimOperatorPluginManager::GetViewerPluginInfo(const std::string &id)
{
    std::map<std::string, ViewerWrappedEngineOperatorPluginInfo *>::iterator it;
    it = wrapped.find(id);
    if(it == wrapped.end())
    {
        // Get the engine plugin from the other plot manager.
        EngineOperatorPluginInfo *eInfo = GetEnginePluginInfo(id);

        // Wrap the info we looked up.
        wrapped[id] = new ViewerWrappedEngineOperatorPluginInfo(eInfo);
        it = wrapped.find(id);
    }

    return it->second;   
}
