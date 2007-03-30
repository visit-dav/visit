#ifndef VISIT_LINE_TOOL_INTERFACE_H
#define VISIT_LINE_TOOL_INTERFACE_H
#include <viswindow_exports.h>
#include <avtToolInterface.h>

// ****************************************************************************
// Class: avtLineToolInterface
//
// Purpose:
//   This class contains the information passed to users of the line tool.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:17:24 PST 2002
//
// Modifications:
//
// ****************************************************************************

class VISWINDOW_API avtLineToolInterface : public avtToolInterface
{
   public:
     avtLineToolInterface(const VisWindow *v);
     virtual ~avtLineToolInterface();

     void SetPoint1(double, double, double);
     void SetPoint2(double, double, double);

     const double *GetPoint1() const;
     const double *GetPoint2() const;
};

#endif
