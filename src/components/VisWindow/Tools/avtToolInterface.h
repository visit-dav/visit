#ifndef AVT_TOOL_INTERFACE_H
#define AVT_TOOL_INTERFACE_H
#include <viswindow_exports.h>

// Forward declarations.
class VisWindow;
class AttributeSubject;

class avtToolInterface;
typedef void (*avtToolInterfaceCallback)(const avtToolInterface &);

// ****************************************************************************
// Class: avtToolInterface
//
// Purpose:
//   When a tool wants to tell the world about new state, it uses an object
//   of this type (or derived types).
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 12:28:27 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 11 14:23:16 PST 2002
//   Added a method to return a non-const pointer to the attributes.
//
// ****************************************************************************

class VISWINDOW_API avtToolInterface
{
  public:
    avtToolInterface(const VisWindow *v);
    virtual ~avtToolInterface();

    const VisWindow *GetVisWindow() const
        { return visWindow; };

    const AttributeSubject *GetAttributes() const
        { return atts; };

    AttributeSubject *GetAttributes()
        { return atts; };

    void ExecuteCallback()
        { if(callback != 0) (*callback)(*this); };

    static void SetCallback(avtToolInterfaceCallback cb)
        { callback = cb; };

  protected:
    const VisWindow                 *visWindow;
    AttributeSubject                *atts;

    static avtToolInterfaceCallback  callback;
};

#endif
