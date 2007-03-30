// ************************************************************************* //
//                           ViewerOperatorFactory.C                         //
// ************************************************************************* //

#include <ViewerOperatorFactory.h>

#include <AttributeSubject.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <ViewerOperator.h>


// ****************************************************************************
//  Method: ViewerOperatorFactory constructor
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 03:20:35 PDT 2001
//    Rewrote to support operator plugins.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
// ****************************************************************************

ViewerOperatorFactory::ViewerOperatorFactory()
{
    OperatorPluginManager *operatorPluginMgr = OperatorPluginManager::Instance();

    nTypes = operatorPluginMgr->GetNEnabledPlugins();

    viewerPluginInfo  = new ViewerOperatorPluginInfo*[nTypes];

    for (int i = 0; i < nTypes; ++i)
    {
        viewerPluginInfo[i]  = operatorPluginMgr->GetViewerPluginInfo(
                                           operatorPluginMgr->GetEnabledID(i));
    }
}

// ****************************************************************************
//  Method: ViewerOperatorFactory destructor
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 03:20:35 PDT 2001
//    Rewrote to support operator plugins.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
// ****************************************************************************

ViewerOperatorFactory::~ViewerOperatorFactory()
{
    delete [] viewerPluginInfo;
}

// ****************************************************************************
//  Method: ViewerOperatorFactory::GetNOperatorTypes
//
//  Purpose:
//      Return the number of operator types.
//
//  Returns:    The number of operator types.
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 03:20:35 PDT 2001
//    Rewrote to support operator plugins.
//
// ****************************************************************************

int ViewerOperatorFactory::GetNOperatorTypes() const
{
    return nTypes;
}

// ****************************************************************************
//  Method: ViewerOperatorFactory::CreateOperator
//
//  Purpose:
//      Create an operator of the specified type.
//
//  Arguments:
//      type    The type of operator to create.
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Dec 13 11:23:19 PST 2000
//    Made it also use a plot when creating an operator.
//
//    Jeremy Meredith, Thu Jul 26 03:20:35 PDT 2001
//    Rewrote to support operator plugins.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Eric Brugger, Wed Jan 15 15:56:13 PST 2003
//    Modify the method so that the plot argument is not const.
//
// ****************************************************************************

ViewerOperator *ViewerOperatorFactory::CreateOperator(const int type,
                                                      ViewerPlot *plot)
{
    //
    // Check that the type is within range and that the type has been
    // registered.
    //
    if (type < 0 || type >= nTypes)
    {
        return 0;
    }

    //
    // Call the appropriate routine.
    //
    return new ViewerOperator(type,
                              viewerPluginInfo[type],
                              plot);
}

// ****************************************************************************
//  Method:  ViewerOperatorFactory::GetDefaultAtts
//
//  Purpose:
//    Return a pointer to the default attribute subject for the specified
//    operator.
//
//  Arguments:
//    type    the type of operator
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
// ****************************************************************************

AttributeSubject *
ViewerOperatorFactory::GetDefaultAtts(const int type) const
{
    //
    // Check that the type is within range.
    //
    if (type < 0 || type >= nTypes)
    {
        return 0;
    }

    //
    // Return the appropriate attribute subject.
    //
    return viewerPluginInfo[type]->GetDefaultAtts();
}

// ****************************************************************************
//  Method:  ViewerOperatorFactory::GetClientAtts
//
//  Purpose:
//    Return a pointer to the client attribute subject for the specified
//    operator.
//
//  Arguments:
//    type    the type of operator
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
// ****************************************************************************

AttributeSubject *
ViewerOperatorFactory::GetClientAtts(const int type) const
{
    //
    // Check that the type is within range.
    //
    if (type < 0 || type >= nTypes)
    {
        return 0;
    }

    //
    // Return the appropriate attribute subject.
    //
    return viewerPluginInfo[type]->GetClientAtts();
}

// ****************************************************************************
//  Method: ViewerOperatorFactory::SetClientAttsFromDefault
//
//  Purpose:
//      Set the client attributes based on the default operator attributes
//      for the specified operator type.
//
//  Arguments:
//      type    The type of operator to set the client attributes for.
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 03:20:35 PDT 2001
//    Rewrote to support operator plugins.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
// ****************************************************************************

void ViewerOperatorFactory::SetClientAttsFromDefault(const int type)
{
    //
    // Check that the type is within range and that the type has been
    // registered.
    //
    if (type < 0 || type >= nTypes)
    {
        return;
    }

    //
    // Copy the attributes.
    //
    AttributeSubject *clientAtts  = viewerPluginInfo[type]->GetClientAtts();
    AttributeSubject *defaultAtts = viewerPluginInfo[type]->GetDefaultAtts();

    viewerPluginInfo[type]->CopyAttributes(clientAtts, defaultAtts);
    clientAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerOperatorFactory::SetDefaultAttsFromClient
//
//  Purpose:
//      Set the default attributes based on the client operator attributes
//      for the specified operator type.
//
//  Arguments:
//      type    The type of operator to set the default attributes for.
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 03:20:35 PDT 2001
//    Rewrote to support operator plugins.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
// ****************************************************************************

void ViewerOperatorFactory::SetDefaultAttsFromClient(const int type)
{
    //
    // Check that the type is within range and that the type has been
    // registered.
    //
    if (type < 0 || type >= nTypes)
    {
        return;
    }

    //
    // Copy the attributes.
    //
    viewerPluginInfo[type]->CopyAttributes(
        viewerPluginInfo[type]->GetDefaultAtts(),
        viewerPluginInfo[type]->GetClientAtts());
}
