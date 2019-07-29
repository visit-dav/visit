// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef IO_ACTIONS_H
#define IO_ACTIONS_H
#include <ViewerActionLogic.h>
#include <vectortypes.h>

class EngineKey;
class ViewerPlotList;

// ****************************************************************************
// Class: ExportColorTableAction
//
// Purpose:
//   Handles ViewerRPC::ExportColorTableRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ExportColorTableAction : public ViewerActionLogic
{
public:
    ExportColorTableAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ExportColorTableAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ExportDBAction
//
// Purpose:
//   Handles ViewerRPC::ExportDBRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ExportDBAction : public ViewerActionLogic
{
public:
    ExportDBAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ExportDBAction() {}

    virtual void Execute();

private:
    int GetActivePlotNetworkIds(ViewerPlotList *plist,
                                intVector &networkIds,
                                EngineKey &key);
};

// ****************************************************************************
// Class: ExportEntireStateAction
//
// Purpose:
//   Handles ViewerRPC::ExportEntireStateRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ExportEntireStateAction : public ViewerActionLogic
{
public:
    ExportEntireStateAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ExportEntireStateAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ImportEntireStateAction
//
// Purpose:
//   Handles ViewerRPC::ImportEntireStateRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ImportEntireStateAction : public ViewerActionLogic
{
public:
    ImportEntireStateAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ImportEntireStateAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ImportEntireStateWithDifferentSourcesAction
//
// Purpose:
//   Handles ViewerRPC::ImportEntireStateWithDifferentSourcesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ImportEntireStateWithDifferentSourcesAction : public ViewerActionLogic
{
public:
    ImportEntireStateWithDifferentSourcesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ImportEntireStateWithDifferentSourcesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SaveWindowAction
//
// Purpose:
//   Handles ViewerRPC::SaveWindowRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SaveWindowAction : public ViewerActionLogic
{
public:
    SaveWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SaveWindowAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: WriteConfigFileAction
//
// Purpose:
//   Handles ViewerRPC::WriteConfigFileRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API WriteConfigFileAction : public ViewerActionLogic
{
public:
    WriteConfigFileAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~WriteConfigFileAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ExportHostProfileAction
//
// Purpose:
//   Handles ViewerRPC::ExportHostProfileRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  3 16:14:51 PDT 2016
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ExportHostProfileAction : public ViewerActionLogic
{
public:
    ExportHostProfileAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ExportHostProfileAction() {}

    virtual void Execute();
};

#endif
