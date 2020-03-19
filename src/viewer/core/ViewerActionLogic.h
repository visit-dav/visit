// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_ACTION_LOGIC_H
#define VIEWER_ACTION_LOGIC_H
#include <viewercore_exports.h>
#include <ViewerRPC.h>
#include <ViewerBase.h>

class DataNode;
class ViewerWindow;
class ViewerWindowManager;

// ****************************************************************************
// Class: ViewerActionBase
//
// Purpose:
//   This is an abstract base class for actions that the viewer can perform.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 15:10:25 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerActionLogic : public ViewerBase
{
public:
    ViewerActionLogic(ViewerWindow *win);
    virtual ~ViewerActionLogic();

    ViewerWindow *GetWindow() const;

    virtual void Execute() = 0;
    void PostponeExecute(int activeAction, bool toggled);

    virtual bool CopyFrom(const ViewerActionLogic *);
    virtual bool CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *,const std::string &);

    void         SetRPCType(ViewerRPC::ViewerRPCType);
    ViewerRPC::ViewerRPCType GetRPCType() const;
    std::string  GetName() const;

    static  void SetArgs(const ViewerRPC &a);
    static  const ViewerRPC &GetArgs();

    static void SetPostponeActionCallback(void (*cb)(int, const ViewerRPC &, void *),
                                          void *cbdata);

protected:
    virtual void Setup(int activeAction, bool toggled);

    ViewerWindow               *window;
    ViewerRPC::ViewerRPCType    rpcType;

    static ViewerWindowManager *windowMgr;
    static ViewerRPC            args;

    static void (*PostponeActionCB)(int, const ViewerRPC &, void *);
    static void *PostponeActionCBData;
};

#endif
