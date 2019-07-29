// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_STATE_H
#define VIEWER_STATE_H
#include <AttributeSubject.h>
#include <vector>
#include <viewerrpc_exports.h>

class PlotInfoAttributes;

#define VIEWER_BEGIN_FREELY_EXCHANGED_STATE

//
// This macro is responsible for setting up the order of the objects that
// make up the communication interface between the viewer and its clients.
// You only need to add objects here if you need to add to the interface.
//

#define VIEWER_OBJECT_CREATION \
    VIEWER_REGISTER_OBJECT(ViewerRPC,                ViewerRPC, false)\
    VIEWER_REGISTER_OBJECT(PostponedAction,          PostponedAction, false)\
    VIEWER_REGISTER_OBJECT(SyncAttributes,           SyncAttributes, false)\
    VIEWER_REGISTER_OBJECT(MessageAttributes,        MessageAttributes, false)\
    VIEWER_REGISTER_OBJECT(StatusAttributes,         StatusAttributes, false)\
    VIEWER_REGISTER_OBJECT(DatabaseMetaData,         avtDatabaseMetaData, false)\
    VIEWER_REGISTER_OBJECT(SILAttributes,            SILAttributes, false)\
    VIEWER_REGISTER_OBJECT(DBPluginInfoAttributes,   DBPluginInfoAttributes, false)\
    VIEWER_REGISTER_OBJECT(ConstructDataBinningAttributes,   ConstructDataBinningAttributes, false)\
    VIEWER_REGISTER_OBJECT(ClientMethod,             ClientMethod, false)\
    VIEWER_REGISTER_OBJECT(ClientInformation,        ClientInformation, false)\
    VIEWER_REGISTER_OBJECT(ClientInformationList,    ClientInformationList, false)\
    VIEWER_REGISTER_OBJECT(SimulationUIValues,       SimulationUIValues, false)\
    /*The following objects can be sent to the viewer anytime.*/\
    VIEWER_BEGIN_FREELY_EXCHANGED_STATE\
    VIEWER_REGISTER_OBJECT(ExportDBAttributes,       ExportDBAttributes, false)\
    VIEWER_REGISTER_OBJECT(PluginManagerAttributes,  PluginManagerAttributes, false)\
    VIEWER_REGISTER_OBJECT(AppearanceAttributes,     AppearanceAttributes, true)\
    VIEWER_REGISTER_OBJECT(GlobalAttributes,         GlobalAttributes, true)\
    VIEWER_REGISTER_OBJECT(DatabaseCorrelationList,  DatabaseCorrelationList, true)\
    VIEWER_REGISTER_OBJECT(PlotList,                 PlotList, true)\
    VIEWER_REGISTER_OBJECT(HostProfileList,          HostProfileList, true)\
    VIEWER_REGISTER_OBJECT(SaveWindowAttributes,     SaveWindowAttributes, true)\
    VIEWER_REGISTER_OBJECT(EngineList,               EngineList, true)\
    VIEWER_REGISTER_OBJECT(ColorTableAttributes,     ColorTableAttributes, true)\
    VIEWER_REGISTER_OBJECT(ExpressionList,           ExpressionList, true)\
    VIEWER_REGISTER_OBJECT(AnnotationAttributes,     AnnotationAttributes, true)\
    VIEWER_REGISTER_OBJECT(SILRestrictionAttributes, SILRestrictionAttributes, true)\
    VIEWER_REGISTER_OBJECT(ViewAxisArrayAttributes,  ViewAxisArrayAttributes, true)\
    VIEWER_REGISTER_OBJECT(ViewCurveAttributes,      ViewCurveAttributes, true)\
    VIEWER_REGISTER_OBJECT(View2DAttributes,         View2DAttributes, true)\
    VIEWER_REGISTER_OBJECT(View3DAttributes,         View3DAttributes, true)\
    VIEWER_REGISTER_OBJECT(LightList,                LightList, true)\
    VIEWER_REGISTER_OBJECT(AnimationAttributes,      AnimationAttributes, true)\
    VIEWER_REGISTER_OBJECT(PickAttributes,           PickAttributes, true)\
    VIEWER_REGISTER_OBJECT(PrinterAttributes,        PrinterAttributes, true)\
    VIEWER_REGISTER_OBJECT(WindowInformation,        WindowInformation, true)\
    VIEWER_REGISTER_OBJECT(RenderingAttributes,      RenderingAttributes, true)\
    VIEWER_REGISTER_OBJECT(KeyframeAttributes,       KeyframeAttributes, true)\
    VIEWER_REGISTER_OBJECT(QueryList,                QueryList, true)\
    VIEWER_REGISTER_OBJECT(QueryAttributes,          QueryAttributes, true)\
    VIEWER_REGISTER_OBJECT(MaterialAttributes,       MaterialAttributes, true)\
    VIEWER_REGISTER_OBJECT(GlobalLineoutAttributes,  GlobalLineoutAttributes, true)\
    VIEWER_REGISTER_OBJECT(AnnotationObjectList,     AnnotationObjectList, true)\
    VIEWER_REGISTER_OBJECT(QueryOverTimeAttributes,  QueryOverTimeAttributes, true)\
    VIEWER_REGISTER_OBJECT(InteractorAttributes,     InteractorAttributes, true)\
    VIEWER_REGISTER_OBJECT(ProcessAttributes,        ProcessAttributes, true)\
    VIEWER_REGISTER_OBJECT(MovieAttributes,          MovieAttributes, true)\
    VIEWER_REGISTER_OBJECT(MeshManagementAttributes, MeshManagementAttributes, true)\
    VIEWER_REGISTER_OBJECT(LogRPC                  , ViewerRPC, true)\
    VIEWER_REGISTER_OBJECT(FileOpenOptions,          FileOpenOptions, false)\
    VIEWER_REGISTER_OBJECT(SelectionProperties,      SelectionProperties, false)\
    VIEWER_REGISTER_OBJECT(SelectionList,            SelectionList, false)\
    VIEWER_REGISTER_OBJECT(ViewerClientAttributes,   ViewerClientAttributes, false)\
    VIEWER_REGISTER_OBJECT(ViewerClientInformation,  ViewerClientInformation, false) \
    VIEWER_REGISTER_OBJECT(SeedMeAttributes,         SeedMeAttributes, true)\
    VIEWER_REGISTER_OBJECT(CinemaAttributes,          CinemaAttributes, true)\

//
// Do forward class declarations
//
#define VIEWER_REGISTER_OBJECT(Obj, T, B) class T;
VIEWER_OBJECT_CREATION
#undef VIEWER_REGISTER_OBJECT

//
// Redefine the macro for Set/Get methods. The methods are named so that
// the type name is prepended with Set or Get. Set methods take 2 arguments
// the first argument is a pointer to the object that will replace the one
// in the ViewerState object. The 2nd argument is a bool that indicates whether
// the ViewerState object is responsible for freeing the object.
//
// Example:
//    void SetMovieAttributes(MovieAttributes *, bool owns = true);
//    MovieAttributes *GetMovieAttributes() const;
//

#define VIEWER_REGISTER_OBJECT(Name, T, B) \
        protected: \
        T *ptr##Name;\
        public: \
        T *Get##Name() const { return ptr##Name; } \
        void Set##Name(T *ptr, bool owns = true)\
        {\
            UpdatePointer((AttributeSubject*)ptr##Name, (AttributeSubject*)ptr, owns);\
            ptr##Name = ptr;\
        }

// ****************************************************************************
// Class: ViewerState
//
// Purpose:
//   This class contains pointers to the state objects that the viewer uses
//   in their xfer order. It's a convenience class that lets us create all
//   of the state objects using the copy constructor.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Thu May 5 16:40:01 PST 2005
//
// Modifications:
//   Brad Whitlock, Fri Feb 9 16:57:06 PST 2007
//   I rewrote the class and made it used by the viewer and the viewer proxy
//   so there is now just one place (here) that defines the viewer/client
//   interface.
//
//   Brad Whitlock, Wed Jan  7 14:57:20 PST 2009
//   I added support for a PlotInfoAttributes for each plot plugin.
//
// ****************************************************************************

class VIEWER_RPC_API ViewerState
{
    typedef enum {GeneralState, PlotState, PlotInformation, OperatorState} ObjectPurpose;

    struct ObjectRecord
    {
        AttributeSubject *object;
        bool              owns;
        bool              partialSend;
        ObjectPurpose     objPurpose;
    };

    void UpdatePointer(AttributeSubject *oldValue, AttributeSubject *newValue, 
                       bool owns);
    AttributeSubject *Register(AttributeSubject *obj, bool owns = true, 
                               bool partialSend = true);

    std::vector<ObjectRecord> objVector;
public:
    ViewerState();
    ViewerState(const ViewerState &);
    virtual ~ViewerState();

    // General object query methods.
    int               GetNumStateObjects() const;
    AttributeSubject *GetStateObject(int i);
    const             AttributeSubject *GetStateObject(int i) const;
    bool              GetPartialSendFlag(int i) const;

    // Plugin related methods.
    int                 GetNumPlotStateObjects() const;
    int                 GetNumOperatorStateObjects() const;
    AttributeSubject   *RegisterPlotAttributes(AttributeSubject *obj);
    AttributeSubject   *RegisterOperatorAttributes(AttributeSubject *obj);
    AttributeSubject   *GetPlotAttributes(int type) const;
    PlotInfoAttributes *GetPlotInformation(int type) const;
    AttributeSubject   *GetOperatorAttributes(int type) const;

    // State objects with indices less than this number do not automatically get
    // sent to all clients when they update inside the viewer.
    int               FreelyExchangedState() const;

    // Use the VIEWER_OBJECT_CREATION macro to add Set/Get functions to the
    // ViewerState object.
    VIEWER_OBJECT_CREATION

private:
    int freelyExchangedState, nPlots, nOperators;
};

#endif
