#ifndef VISIT_PLANE_TOOL_INTERFACE_H
#define VISIT_PLANE_TOOL_INTERFACE_H
#include <viswindow_exports.h>
#include <avtToolInterface.h>

// ****************************************************************************
// Class: avtPlaneToolInterface
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
//   Brad Whitlock, Wed Oct 9 11:41:54 PDT 2002
//   I added the GetHaveRadius method.
//
// ****************************************************************************

class VISWINDOW_API avtPlaneToolInterface : public avtToolInterface
{
   public:
     avtPlaneToolInterface(const VisWindow *v);
     virtual ~avtPlaneToolInterface();

     void SetOrigin(double, double, double);
     void SetNormal(double, double, double);
     void SetUpAxis(double, double, double);
     void SetRadius(const double radius);

     const double *GetOrigin() const;
     const double *GetNormal() const;
     const double *GetUpAxis() const;
     double GetRadius() const;
     bool GetHaveRadius() const;
};

#endif
