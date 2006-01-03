// ************************************************************************* //
//                            VisitInteractor.h                              //
// ************************************************************************* //

#ifndef VISIT_INTERACTOR_H
#define VISIT_INTERACTOR_H
#include <viswindow_exports.h>


#include <vtkInteractorStyleTrackballCamera.h>
#include <avtViewInfo.h>

class VisWindow;
class VisWindowInteractorProxy;


typedef   void (*ViewCallback)(VisWindow *);


// ****************************************************************************
//  Class: VisitInteractor
//
//  Purpose:
//    An abstract type that all of Visit's interactions should be derived
//    from.  This is a derived type of vtkInteractorStyle because it is not 
//    easy to remap the mouse button presses through interactor's method, but
//    very easy to redefine the virtual methods for each button 
//    press/release.  It is derived from a trackball interactor so that we 
//    can use the trackball features free of charge.
//
//  Note:    Although this type is derived from a VTK type, it does not follow
//           the VTK contructor (ie New) and memory management (object factory)
//           constructs of VTK.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Thu Jul  6 14:48:45 PDT 2000
//    Disable keystrokes.
//
//    Hank Childs, Mon Dec 18 09:18:35 PST 2000
//    Removed bounding box mode as a part of a VisIt interactor.
//
//    Hank Childs, Tue Jul 17 08:54:08 PDT 2001
//    Add callback when the view changes.
//
//    Eric Brugger, Tue Aug 21 09:54:42 PDT 2001
//    I removed the avtViewInfo argument from ViewCallback.
//
//    Hank Childs, Mon Mar 18 13:17:52 PST 2002
//    Added better support for keeping track of button-push operations.
//
//    Eric Brugger, Thu Apr 11 16:05:10 PDT 2002
//    I moved the vector and matrix manipulation methods from Navigate3D
//    to VisitInteractor.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Inherit from vtkInteractorStyleTrackballCamera as 
//    vtkInteractorStyleTrackball has been deprecated.  Comply with new
//    interactor interface by removing arguments from all Button methods, 
//    OnMouseMove, and PrepTrackball.  Added members that were part of old
//    parent class, but are not in the new: OldX/Y, Center, MotionFactor.
//
//    Hank Childs, Tue Jul 20 10:54:45 PDT 2004
//    Add methods to check on whether buttons are depressed.
//
//    Eric Brugger, Mon Nov 22 08:30:11 PST 2004
//    I added ZoomCamera2D.
//
//    Eric Brugger, Tue Dec 28 13:30:15 PST 2004
//    I moved RotateCamera, PanCamera and ZoomCamera from the FlyThrough
//    class as RotateAboutCamera3D, PanCamera3D and DollyCameraAndFocus3D.
//    I moved RotateCamera, PanCamera and ZoomCamera from the Navigate3D
//    class as RotateAboutFocus3D, PanImage3D and ZoomImage3D.  I added
//    DollyCameraTowardFocus3D.
//
//    Hank Childs, Thu Dec 29 10:30:53 PST 2005
//    Added Boolean argument to IssueViewCallback.
//
// ****************************************************************************

class VISWINDOW_API VisitInteractor : public vtkInteractorStyleTrackballCamera
{
  public:
                                VisitInteractor(VisWindowInteractorProxy &);
    virtual                    ~VisitInteractor();
 
    static void                 RegisterViewCallback(ViewCallback);

    virtual void                OnLeftButtonDown();
    virtual void                OnLeftButtonUp();
    virtual void                OnMiddleButtonDown();
    virtual void                OnMiddleButtonUp();
    virtual void                OnRightButtonDown();
    virtual void                OnRightButtonUp();
    virtual void                OnChar();

    virtual void                OnMouseMove();

    virtual void                StartLeftButtonAction();
    virtual void                AbortLeftButtonAction();
    virtual void                EndLeftButtonAction();

    virtual void                StartMiddleButtonAction();
    virtual void                AbortMiddleButtonAction();
    virtual void                EndMiddleButtonAction();

    virtual void                StartRightButtonAction();
    virtual void                AbortRightButtonAction();
    virtual void                EndRightButtonAction();

    virtual void                SetInteractor(vtkRenderWindowInteractor *);

    virtual bool                LeftButtonIsDown(void)
                                                  { return leftButtonDown; };
    virtual bool                MiddleButtonIsDown(void)
                                                  { return middleButtonDown; };
    virtual bool                RightButtonIsDown(void)
                                                  { return rightButtonDown; };

  protected:
    VisWindowInteractorProxy   &proxy;

    int                         lastX, lastY;
    int                         lastCallbackX, lastCallbackY;
    bool                        leftButtonDown;
    bool                        middleButtonDown;
    bool                        rightButtonDown;

    float                       OldX, OldY;
    float                       spinOldX, spinOldY;
    int                         spinNewX, spinNewY;
    float                       Center[2];
    float                       MotionFactor;


    void                        PrepTrackball();
    void                        NewButtonPress();

    void                        MotionBegin(void);
    void                        MotionEnd(void);
    void                        IssueViewCallback(bool = false);
    void                        StartBoundingBox(void);
    void                        EndBoundingBox(void);

    void                        ZoomCamera2D(const int, const int);
    void                        PanImage3D(const int, const int);
    void                        PanCamera3D(const int, const int);
    void                        ZoomImage3D(const int, const int);
    void                        DollyCameraTowardFocus3D(const int, const int);
    void                        DollyCameraAndFocus3D(const int, const int);
    void                        RotateAboutFocus3D(const int, const int,
                                    const bool);
    void                        RotateAboutCamera3D(const int, const int);

    inline void                 VectorSet(double *, const double, const double,
                                    const double) const;
    inline void                 VectorCross(const double *, const double *,
                                    double *) const;
    inline void                 VectorCopy(const double *, double *) const;
    inline void                 VectorSubtract(const double *, const double *,
                                    double *) const;
    inline double               VectorLength(const double *) const;
    inline void                 VectorScale(double *, const double) const;
    inline void                 VectorPrint(const char *, const double *) const;
    inline void                 VectorMatrixMult(const double [3],
                                    const double [3][3], double [3]) const;

    inline void                 MatrixSet(double [3][3], const double [3],
                                    const double [3], const double [3]) const;
    inline void                 MatrixTranspose(const double [3][3],
                                    double [3][3]) const;
    inline void                 MatrixMult(const double [3][3],
                                    const double [3][3], double [3][3]) const;
    inline void                 MatrixPrint(const char *, const double [3][3])
                                    const;

  private:
    static ViewCallback         viewCallback;
};

// ****************************************************************************
//  Method: VisitInteractor::VectorSet
//
//  Purpose:
//    Set a 3 dimensional vector given its components.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::VectorSet(double v[3], const double v0, const double v1,
    const double v2) const
{
    v[0] = v0;
    v[1] = v1;
    v[2] = v2;
}

// ****************************************************************************
//  Method: VisitInteractor::VectorCross
//
//  Purpose:
//    Calculate the cross product of two 3 dimensional vectors.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::VectorCross(const double v1[3], const double v2[3],
    double vout[3]) const
{
    vout[0] = (v1[1]*v2[2]) - (v1[2]*v2[1]);
    vout[1] = (v1[2]*v2[0]) - (v1[0]*v2[2]);
    vout[2] = (v1[0]*v2[1]) - (v1[1]*v2[0]);
}

// ****************************************************************************
//  Method: VisitInteractor::VectorCopy
//
//  Purpose:
//    Copy a 3 dimensional vector.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::VectorCopy(const double v[3], double vout[3]) const
{
    vout[0] = v[0];
    vout[1] = v[1];
    vout[2] = v[2];
}

// ****************************************************************************
//  Method: VisitInteractor::VectorSubtract
//
//  Purpose:
//    Calculate the difference between two 3 dimensional vectors.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::VectorSubtract(const double v1[3], const double v2[3],
    double vout[3]) const
{
    vout[0] = v1[0] - v2[0];
    vout[1] = v1[1] - v2[1];
    vout[2] = v1[2] - v2[2];
}

// ****************************************************************************
//  Method: VisitInteractor::VectorLength
//
//  Purpose:
//    Calculate the length of a 3 dimensional vector.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline double
VisitInteractor::VectorLength(const double v[3]) const
{
    return sqrt((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
}

// ****************************************************************************
//  Method: VisitInteractor::VectorScale
//
//  Purpose:
//    Scale a 3 dimensional vector.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::VectorScale(double v[3], const double scale) const
{
    v[0] *= scale;
    v[1] *= scale;
    v[2] *= scale;
}

// ****************************************************************************
//  Method: VisitInteractor::VectorPrint
//
//  Purpose:
//    Print a 3 dimensional vector.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::VectorPrint(const char *name, const double v[3]) const
{
    cout << name << " = ";
    cout << v[0] << ", " << v[1] << ", " << v[2] << endl;
}

// ****************************************************************************
//  Method: VisitInteractor::VectorMatrixMult
//
//  Purpose:
//    Multiply a 3 dimensional vector against a 3x3 matrix.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::VectorMatrixMult(const double v1[3], const double m[3][3],
    double vout[3]) const
{
    vout[0] = v1[0]*m[0][0] + v1[1]*m[1][0] + v1[2]*m[2][0];
    vout[1] = v1[0]*m[0][1] + v1[1]*m[1][1] + v1[2]*m[2][1];
    vout[2] = v1[0]*m[0][2] + v1[1]*m[1][2] + v1[2]*m[2][2];
}

// ****************************************************************************
//  Method: VisitInteractor::MatrixSet
//
//  Purpose:
//    Set a 3x3 matrix given three 3 dimensional vectors.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::MatrixSet(double mout[3][3], const double v0[3],
    const double v1[3], const double v2[3]) const
{
    mout[0][0] = v0[0];
    mout[0][1] = v1[0];
    mout[0][2] = v2[0];
    mout[1][0] = v0[1];
    mout[1][1] = v1[1];
    mout[1][2] = v2[1];
    mout[2][0] = v0[2];
    mout[2][1] = v1[2];
    mout[2][2] = v2[2];
}

// ****************************************************************************
//  Method: VisitInteractor::MatrixTranspose
//
//  Purpose:
//    Transpose a 3x3 matrix.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::MatrixTranspose(const double m[3][3], double mout[3][3]) const
{
    mout[0][0] = m[0][0];
    mout[0][1] = m[1][0];
    mout[0][2] = m[2][0];
    mout[1][0] = m[0][1];
    mout[1][1] = m[1][1];
    mout[1][2] = m[2][1];
    mout[2][0] = m[0][2];
    mout[2][1] = m[1][2];
    mout[2][2] = m[2][2];
}

// ****************************************************************************
//  Method: VisitInteractor::MatrixMult
//
//  Purpose:
//    Multiply two 3x3 matrices together.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::MatrixMult(const double m1[3][3], double const m2[3][3],
    double mout[3][3]) const
{
    mout[0][0] = m1[0][0]*m2[0][0] + m1[0][1]*m2[1][0] + m1[0][2]*m2[2][0];
    mout[0][1] = m1[0][0]*m2[0][1] + m1[0][1]*m2[1][1] + m1[0][2]*m2[2][1];
    mout[0][2] = m1[0][0]*m2[0][2] + m1[0][1]*m2[1][2] + m1[0][2]*m2[2][2];
    mout[1][0] = m1[1][0]*m2[0][0] + m1[1][1]*m2[1][0] + m1[1][2]*m2[2][0];
    mout[1][1] = m1[1][0]*m2[0][1] + m1[1][1]*m2[1][1] + m1[1][2]*m2[2][1];
    mout[1][2] = m1[1][0]*m2[0][2] + m1[1][1]*m2[1][2] + m1[1][2]*m2[2][2];
    mout[2][0] = m1[2][0]*m2[0][0] + m1[2][1]*m2[1][0] + m1[2][2]*m2[2][0];
    mout[2][1] = m1[2][0]*m2[0][1] + m1[2][1]*m2[1][1] + m1[2][2]*m2[2][1];
    mout[2][2] = m1[2][0]*m2[0][2] + m1[2][1]*m2[1][2] + m1[2][2]*m2[2][2];
}

// ****************************************************************************
//  Method: VisitInteractor::MatrixPrint
//
//  Purpose:
//    Print a 3x3 matrix.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
// ****************************************************************************

inline void
VisitInteractor::MatrixPrint(const char *name, const double m[3][3]) const
{
    int       i;
    int       nSpaces = strlen(name) + 3;

    cout << name << " = ";
    cout << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << endl;
    for (i = 0; i < nSpaces; i++) cout << " ";
    cout << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << endl;
    for (i = 0; i < nSpaces; i++) cout << " ";
    cout << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << endl;
}

#endif


