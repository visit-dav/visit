// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SET_WINANNOT_ATTS_RPC_H
#define SET_WINANNOT_ATTS_RPC_H
#include <engine_rpc_exports.h>

#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <VisItRPC.h>
#include <VisualCueList.h>
#include <WindowAttributes.h>

#include <string>

class AttributeGroup;

// ****************************************************************************
//  Class:  SetWinAnnotAttsRPC
//
//  Purpose:
//    Implements an RPC to set the current window and annotation attributes.
//
//  Programmer:  Mark C. Miller 
//  Creation:    15Jul03 
//
//  Modifications:
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added data member and methods for extents type string
//
//    Mark C. Miller, Tue May 25 17:06:12 PDT 2004
//    Added AnnotationObjectList member
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added visual cue list data member
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added frame and state data member and methods
//
//    Mark C. Miller, Wed Oct  6 18:36:47 PDT 2004
//    Added array of view extents
//
//    Mark C. Miller, Tue Oct 19 19:44:00 PDT 2004
//    Added string for color table name
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
// ****************************************************************************
class ENGINE_RPC_API SetWinAnnotAttsRPC : public BlockingRPC
{
public:
    SetWinAnnotAttsRPC();
    virtual ~SetWinAnnotAttsRPC();

    virtual const std::string TypeName() const { return "SetWinAnnotAttsRPC"; }

    // Invokation method
    void operator()(const WindowAttributes*,
                    const AnnotationAttributes*,
                    const AnnotationObjectList*,
                    const std::string,
                    const VisualCueList*,
                    const int*,
                    const double*,
                    const std::string,
                    const int);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetWindowAtts(const WindowAttributes*);
    void SetAnnotationAtts(const AnnotationAttributes*);
    void SetAnnotationObjectList(const AnnotationObjectList*);
    void SetExtentTypeString(const std::string);
    void SetVisualCueList(const VisualCueList*);
    void SetFrameAndState(const int*);
    void SetViewExtents(const double*);
    void SetChangedCtName(const std::string);
    void SetWindowID(int);

    // Property getting methods
    const WindowAttributes &GetWindowAtts() const;
    const AnnotationAttributes &GetAnnotationAtts() const;
    const AnnotationObjectList &GetAnnotationObjectList() const;
    const std::string &GetExtentTypeString() const;
    const VisualCueList &GetVisualCueList() const;
    const int* GetFrameAndState() const;
    const double *GetViewExtents() const;
    const std::string &GetChangedCtName() const;
    int GetWindowID() const;

private:
    WindowAttributes win;
    AnnotationAttributes annot;
    AnnotationObjectList aolist;
    std::string extstr;
    VisualCueList cuelist;
    int fands[7];
    double vexts[6];
    std::string ctname;
    int windowID;
};

#endif
