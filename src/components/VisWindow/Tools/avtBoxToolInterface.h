#ifndef VISIT_BOX_TOOL_INTERFACE_H
#define VISIT_BOX_TOOL_INTERFACE_H
#include <viswindow_exports.h>
#include <avtToolInterface.h>

// ****************************************************************************
// Class: avtBoxToolInterface
//
// Purpose:
//   This class contains the information passed to users of the box tool.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 07:24:15 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class VISWINDOW_API avtBoxToolInterface : public avtToolInterface
{
public:
    avtBoxToolInterface(const VisWindow *v);
    virtual ~avtBoxToolInterface();

    void          SetExtents(const double *d);
    const double *GetExtents() const;
    bool          Initialized() const { return initialized; };
    void          UnInitialize() { initialized = false; };
private:
    bool initialized;
};

#endif
