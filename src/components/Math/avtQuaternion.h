#ifndef QUATERNION_H
#define QUATERNION_H
#include <math_exports.h>

#include <avtVector.h>
#include <avtMatrix.h>

// ****************************************************************************
//  Purpose:
//    Build a quaternion using a virtual trackball. The virtual trackball
//    is based on a Witch of Agnesi, which is a sphere unioned with a
//    plane.  The size of the sphere should really be based on the distance
//    from the center of rotation to the point on the object underneath the
//    mouse.  That point would track the mouse as closely as possible.
//    The witch of agnesi is parameterized with these 3 parameters:
//
//        AGNESI_RADIUS  0.8    The Z value at x = y = 0.0
//        COMPRESSION    3.5    Multipliers for x and y
//        SPHERE_RADIUS  1.0    Radius of the sphere embedded in the plane
//
// ****************************************************************************

class MATH_API avtQuaternion
{
  public:
    avtQuaternion();
    avtQuaternion(const avtVector&, double);
    avtQuaternion(const avtQuaternion&);
    void operator=(const avtQuaternion&);

    avtMatrix CreateRotationMatrix();

    double norm();
    void normalize();
  private:
    double x;
    double y;
    double z;
    double s;
};

#endif

