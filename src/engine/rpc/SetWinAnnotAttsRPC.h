#ifndef SET_WINANNOT_ATTS_RPC_H
#define SET_WINANNOT_ATTS_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <AnnotationAttributes.h>
#include <WindowAttributes.h>

// ****************************************************************************
//  Class:  SetWinAnnotAttsRPC
//
//  Purpose:
//    Implements an RPC to set the current window and annotation attributes.
//
//  Programmer:  Mark C. Miller 
//  Creation:    15Jul03 
//
// ****************************************************************************
class ENGINE_RPC_API SetWinAnnotAttsRPC : public BlockingRPC
{
public:
    SetWinAnnotAttsRPC();
    virtual ~SetWinAnnotAttsRPC();

    // Invokation method
    void operator()(const WindowAttributes*,
                    const AnnotationAttributes*);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetWindowAtts(const WindowAttributes*);
    void SetAnnotationAtts(const AnnotationAttributes*);

    // Property getting methods
    const WindowAttributes &GetWindowAtts() const;
    const AnnotationAttributes &GetAnnotationAtts() const;

private:
    WindowAttributes win;
    AnnotationAttributes annot;
};

#endif
