#ifndef SET_WINANNOT_ATTS_RPC_H
#define SET_WINANNOT_ATTS_RPC_H
#include <engine_rpc_exports.h>

#include <string>

#include <VisItRPC.h>
#include <AnnotationAttributes.h>
#include <WindowAttributes.h>

using std::string;

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
// ****************************************************************************
class ENGINE_RPC_API SetWinAnnotAttsRPC : public BlockingRPC
{
public:
    SetWinAnnotAttsRPC();
    virtual ~SetWinAnnotAttsRPC();

    // Invokation method
    void operator()(const WindowAttributes*,
                    const AnnotationAttributes*,
                    const string);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetWindowAtts(const WindowAttributes*);
    void SetAnnotationAtts(const AnnotationAttributes*);
    void SetExtentTypeString(const string);

    // Property getting methods
    const WindowAttributes &GetWindowAtts() const;
    const AnnotationAttributes &GetAnnotationAtts() const;
    const string &GetExtentTypeString() const;

private:
    WindowAttributes win;
    AnnotationAttributes annot;
    string extstr;
};

#endif
