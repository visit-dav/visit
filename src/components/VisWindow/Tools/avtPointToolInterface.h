#ifndef VISIT_POINT_TOOL_INTERFACE_H
#define VISIT_POINT_TOOL_INTERFACE_H
#include <viswindow_exports.h>
#include <avtToolInterface.h>

// ****************************************************************************
// Class: avtPointToolInterface
//
// Purpose:
//   This class contains the information passed to users of the point tool.
//
// Notes:      
//
// Programmer: Akira Haddox 
// Creation:   Mon Jun  9 09:37:49 PDT 2003
//
// Modifications:
//
// ****************************************************************************

class VISWINDOW_API avtPointToolInterface : public avtToolInterface
{
   public:
     avtPointToolInterface(const VisWindow *v);
     virtual ~avtPointToolInterface();

     void SetPoint(double, double, double);

     const double *GetPoint() const;
};

#endif
