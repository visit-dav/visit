#ifndef QUATERNION_H
#define QUATERNION_H
#include <math_exports.h>

#include <avtVector.h>
#include <avtMatrix.h>

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

