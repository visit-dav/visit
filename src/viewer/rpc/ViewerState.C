// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerState.h>

#include <avtDatabaseMetaData.h>
#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <AppearanceAttributes.h>
#include <CinemaAttributes.h>
#include <ClientMethod.h>
#include <ClientInformation.h>
#include <ClientInformationList.h>
#include <ColorTableAttributes.h>
#include <ConstructDataBinningAttributes.h>
#include <DatabaseCorrelationList.h>
#include <DBPluginInfoAttributes.h>
#include <ExportDBAttributes.h>
#include <ExpressionList.h>
#include <EngineList.h>
#include <FileOpenOptions.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <HostProfileList.h>
#include <VisItInit.h>
#include <InteractorAttributes.h>
#include <KeyframeAttributes.h>
#include <LightList.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
#include <MessageAttributes.h>
#include <MovieAttributes.h>
#include <ParentProcess.h>
#include <PickAttributes.h>
#include <PlotInfoAttributes.h>
#include <PlotList.h>
#include <PluginManagerAttributes.h>
#include <PostponedAction.h>
#include <PrinterAttributes.h>
#include <ProcessAttributes.h>
#include <QueryAttributes.h>
#include <QueryList.h>
#include <RenderingAttributes.h>
#include <SaveWindowAttributes.h>
#include <SelectionList.h>
#include <SelectionProperties.h>
#include <SimulationUIValues.h>
#include <StatusAttributes.h>
#include <SILRestrictionAttributes.h>
#include <SyncAttributes.h>
#include <QueryOverTimeAttributes.h>
#include <ViewAxisArrayAttributes.h>
#include <ViewCurveAttributes.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <WindowInformation.h>
#include <ViewerClientAttributes.h>
#include <ViewerClientInformation.h>

//
// Undefine the macro so we can redefine it for various purposes in this file.
//
#ifdef VIEWER_REGISTER_OBJECT
#undef VIEWER_REGISTER_OBJECT
#endif

// ****************************************************************************
// Method: ViewerState::ViewerState
//
// Purpose: 
//   Constructor for the ViewerState class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 17:39:59 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerState::ViewerState() : objVector()
{
#undef VIEWER_BEGIN_FREELY_EXCHANGED_STATE
#define VIEWER_BEGIN_FREELY_EXCHANGED_STATE freelyExchangedState = objVector.size();
#define VIEWER_REGISTER_OBJECT(Name, T, ps) ptr##Name = (T *)Register(new T(), true, ps);

    VIEWER_OBJECT_CREATION

#undef VIEWER_REGISTER_OBJECT
    nPlots = nOperators = 0;
}

// ****************************************************************************
// Method: ViewerState::ViewerState
//
// Purpose: 
//   Copy constructor for the ViewerState class.
//
// Arguments:
//   vs : The ViewerState object that we're copying.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 17:39:59 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerState::ViewerState(const ViewerState &vs) : objVector()
{
#define VIEWER_REGISTER_OBJECT(Name, T, ps) ptr##Name = (T *)Register(vs.ptr##Name->NewInstance(true), true, ps);

    VIEWER_OBJECT_CREATION

#undef VIEWER_REGISTER_OBJECT
    nPlots = vs.nPlots;
    nOperators = vs.nOperators;

    // Since the above VIEWER_OBJECT_CREATION macro exists to ensure that 
    // the static objects are copied, and their pointers get put into
    // the local members, we must copy plot and operator attributes here.
    size_t nObjects = objVector.size();
    for(size_t i = nObjects; i < vs.objVector.size(); ++i)
    {
        ObjectRecord rec;
        rec.object = vs.objVector[i].object->NewInstance(true);
        rec.owns = vs.objVector[i].owns;
        rec.partialSend = vs.objVector[i].partialSend;
        rec.objPurpose = vs.objVector[i].objPurpose;
        objVector.push_back(rec);
    }
}

// ****************************************************************************
// Method: ViewerState::~ViewerState
//
// Purpose: 
//   Destructor for the ViewerState class.
//
// Note:       It only destroys objects that it owns.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:39:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerState::~ViewerState()
{
    // Delete the objects through the objVector.
    for(size_t i = 0; i < objVector.size(); ++i)
    {
        if(objVector[i].owns)
            delete objVector[i].object;
    }
}

// ****************************************************************************
// Method: ViewerState::GetStateObject
//
// Purpose: 
//   Returns a pointer to the i'th state object.
//
// Arguments:
//   i : The index of the state object that we want.
//
// Returns:    A pointer to the i'th state object or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:39:51 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ViewerState::GetStateObject(int i)
{
    return (i >= 0 && (size_t)i < objVector.size()) ?
           objVector[i].object : 0;
}

const AttributeSubject *
ViewerState::GetStateObject(int i) const
{
    return (i >= 0 && (size_t)i < objVector.size()) ?
           objVector[i].object : 0;
}

// ****************************************************************************
// Method: ViewerState::GetPartialSendFlag
//
// Purpose: 
//   Gets the partial send flag for the i'th state object.
//
// Arguments:
//   i : The index of the state object.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:40:30 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerState::GetPartialSendFlag(int i) const
{
    return (i >= 0 && (size_t)i < objVector.size()) ?
           objVector[i].partialSend : false;
}

// ****************************************************************************
// Method: ViewerState::GetNumStateObjects
//
// Purpose: 
//   Returns the number of state objects.
//
// Returns:    The number of state objects.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:40:58 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

int
ViewerState::GetNumStateObjects() const
{
    return objVector.size();
}

// ****************************************************************************
// Method: ViewerState::GetNumPlotStateObjects
//
// Purpose: 
//   Returns the number of plot state objects.
//
// Returns:    The number of plot state objects.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:41:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

int
ViewerState::GetNumPlotStateObjects() const
{
    return nPlots;
}

// ****************************************************************************
// Method: ViewerState::GetNumOperatorStateObjects
//
// Purpose: 
//   Returns the number of operator state objects.
//
// Returns:    The number of operator state objects.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:41:45 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

int
ViewerState::GetNumOperatorStateObjects() const
{
    return nOperators;
}

// ****************************************************************************
// Method: ViewerState::FreelyExchangedState
//
// Purpose: 
//   Returns the index above which all state objects can be sent to the client
//   without any hesitation.
//
// Returns:    The index of the beginning of the freely exchanged state objects.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:42:07 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

int
ViewerState::FreelyExchangedState() const
{
    return freelyExchangedState;
}

// ****************************************************************************
// Method: ViewerState::UpdatePointer
//
// Purpose: 
//   Updates the pointer to a state object.
//
// Arguments:
//   oldValue : The pointer to replace.
//   newValue : The new pointer value.
//   owns     : True if the ViewerState object will own the new pointer.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:42:54 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
ViewerState::UpdatePointer(AttributeSubject *oldValue, AttributeSubject *newValue,
    bool owns)
{
    for(size_t i = 0; i < objVector.size(); ++i)
    {
        if(objVector[i].object == oldValue)
        {
            if(objVector[i].owns)
                delete objVector[i].object;
            objVector[i].object = newValue;
            objVector[i].owns = owns;
            break;
        }
    }
}

// ****************************************************************************
// Method: ViewerState::Register
//
// Purpose: 
//   Registers a state object with ViewerState.
//
// Arguments:
//   obj         : The object to register.
//   owns        : True if the ViewerState object will own the object.
//   partialSend : True if it is okay to send pieces of the object to the clients.
//
// Returns:    The pointer to the object that was passed in.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:44:53 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ViewerState::Register(AttributeSubject *obj, bool owns, bool partialSend)
{
    ObjectRecord rec;
    rec.object = obj;
    rec.owns = owns;
    rec.partialSend = partialSend;
    rec.objPurpose = GeneralState;
    objVector.push_back(rec);

    return obj;
}

// ****************************************************************************
// Method: ViewerState::RegisterPlotAttributes
//
// Purpose: 
//   Registers a plot state object.
//
// Arguments:
//   obj : The plot attributes to be associated with ViewerState.
//
// Returns:    The pointer to the plot attributes
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:46:13 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Jan  7 15:00:58 PST 2009
//   I added a plot info attributes for each plot that gets registered.
//
// ****************************************************************************

AttributeSubject *
ViewerState::RegisterPlotAttributes(AttributeSubject *obj)
{
    ObjectRecord rec;
    rec.object = obj;
    rec.owns = true;
    rec.partialSend = true;
    rec.objPurpose = PlotState;
    objVector.push_back(rec);

    ObjectRecord rec2;
    rec2.object = new PlotInfoAttributes;
    rec2.owns = true;
    rec2.partialSend = false;
    rec2.objPurpose = PlotInformation;
    objVector.push_back(rec2);

    nPlots++;
    return obj;
}

// ****************************************************************************
// Method: ViewerState::RegisterOperatorAttributes
//
// Purpose: 
//   Registers an operator state object.
//
// Arguments:
//   obj : The operator attributes to be associated with ViewerState.
//
// Returns:    The pointer to the operator attributes.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:47:01 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ViewerState::RegisterOperatorAttributes(AttributeSubject *obj)
{
    ObjectRecord rec;
    rec.object = obj;
    rec.owns = true;
    rec.partialSend = true;
    rec.objPurpose = OperatorState;
    objVector.push_back(rec);
    nOperators++;
    return obj;
}

// ****************************************************************************
// Method: ViewerState::GetPlotAttributes
//
// Purpose: 
//   Get the n'th plot state object.
//
// Arguments:
//   n : The index of the plot in the order they were registered.
//
// Returns:    The pointer to the state object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:47:43 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ViewerState::GetPlotAttributes(int n) const
{
    int count = 0;
    for(size_t i = 0; i < objVector.size(); ++i)
    {
        if(objVector[i].objPurpose == PlotState)
        {
            if(count == n)
                return objVector[i].object;
            ++count;
        }
    }
    return 0;
}

// ****************************************************************************
// Method: ViewerState::GetPlotAttributes
//
// Purpose: 
//   Get the n'th plot information object.
//
// Arguments:
//   n : The index of the plot in the order they were registered.
//
// Returns:    The pointer to the plot information state object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:47:43 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

PlotInfoAttributes *
ViewerState::GetPlotInformation(int n) const
{
    int count = 0;
    for(size_t i = 0; i < objVector.size(); ++i)
    {
        if(objVector[i].objPurpose == PlotInformation)
        {
            if(count == n)
                return (PlotInfoAttributes *)objVector[i].object;
            ++count;
        }
    }
    return 0;
}

// ****************************************************************************
// Method: ViewerState::GetOperatorAttributes
//
// Purpose: 
//   Get the n'th operator state object.
//
// Arguments:
//   n : The index of the operator in the order they were registered.
//
// Returns:    The pointer to the state object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 23 11:47:43 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ViewerState::GetOperatorAttributes(int type) const
{
    int count = 0;
    for(size_t i = 0; i < objVector.size(); ++i)
    {
        if(objVector[i].objPurpose == OperatorState)
        {
            if(count == type)
                return objVector[i].object;
            ++count;
        }
    }
    return 0;
}
