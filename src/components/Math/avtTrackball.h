#ifndef AVT_TRACKBALL_H
#define AVT_TRACKBALL_H
#include <math_exports.h>

#include <avtQuaternion.h>
#include <avtMatrix.h>
#include <avtVector.h>

// ****************************************************************************
//  Class:  avtTrackball
//
//  Purpose:
//    Implement a virtual trackball using a Witch of Agnesi
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
class MATH_API avtTrackball
{
  public:
    avtTrackball();

    void                 SetCenter(const avtVector&);
    void                 SetCamera(const avtMatrix&);
    void                 ClearCamera();
    void                 SetConstrainAxis(const avtVector&);
    void                 ClearConstrainAxis();

    void                 PerformRotation(const avtVector&, const avtVector&);

    const avtQuaternion &GetQuaternion() const;
    const avtMatrix     &GetMatrix() const;
  private:
    static avtVector     Project(const avtVector&);

    bool          constrain;
    avtVector     center;
    avtVector     constrainAxis;
    avtMatrix     inverse_camera;

    avtQuaternion q;
    avtMatrix     M;
};

#endif
