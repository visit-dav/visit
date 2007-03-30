#ifndef VISIT_SPHERE_TOOL_INTERFACE_H
#define VISIT_SPHERE_TOOL_INTERFACE_H
#include <viswindow_exports.h>
#include <avtToolInterface.h>

// ****************************************************************************
// Class: avtSphereToolInterface
//
// Purpose:
//   This class contains the information passed to users of the plane tool.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 10 15:53:09 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 11 16:38:39 PST 2002
//   Added get methods.
//
// ****************************************************************************

class VISWINDOW_API avtSphereToolInterface : public avtToolInterface
{
   public:
     avtSphereToolInterface(const VisWindow *v);
     virtual ~avtSphereToolInterface();

     void SetOrigin(double, double, double);
     void SetRadius(const double radius);

     const double *GetOrigin() const;
     double GetRadius() const;
};

#endif
