// ************************************************************************* //
//                                   Frame.C                                 //
// ************************************************************************* //

#include <Frame.h>


//
// Static function prototypes.
//

static inline void    Cross(float [3], const float [3], const float [3]);
static inline float   Dot(const float [3], const float [3]);
static inline void    SolveWithT(float [4], const float [3], const float [3], 
                                 const float [3], const float [3], float);

// ****************************************************************************
//  Method: Frame constructor
//
//  Programmer: Hank Childs
//  Creation:   June 25, 2000
//
// ****************************************************************************

Frame::Frame()
{
    //
    // Set the frame to be the Cartesian frame.
    //
    float   bb0[3] = { 1., 0., 0. };
    float   bb1[3] = { 0., 1., 0. };
    float   bb2[3] = { 0., 0., 1. };
    float   ori[3] = { 0., 0., 0. };
    SetFrame(bb0, bb1, bb2, ori);
}


// ****************************************************************************
//  Method: Frame::SetFrame
//
//  Purpose:
//      Sets the frame to have the basis vectors and origin specified.
//
//  Arguments:
//      a0      The first basis vector for the frame.
//      a1      The second basis vector for the frame.
//      a2      The third basis vector for the frame.
//      or      The origin for the frame.
//
//  Programmer: Hank Childs
//  Creation:   June 25, 2000
//
// ****************************************************************************

void
Frame::SetFrame(const float a0[3], const float a1[3], const float a2[3],
                const float or[3])
{
    //
    // Copy over the arguments to our data members.
    //
    for (int i = 0 ; i < 3 ; i++)
    {
        b0[i]     = a0[i];
        b1[i]     = a1[i];
        b2[i]     = a2[i];
        origin[i] = or[i];
    }

    CalculateTransformationMatrix();
}


// ****************************************************************************
//  Method: Frame::CalculateTransformationMatrix
//
//  Purpose:
//      Determines the transformation matrix.  Algorithm taken from handout of
//      UCDavis ECS175 class.  Also available on web from Ken Joy.  Calculate
//      the Cartesian-Frame-To-Frame conversion matrix.
//
//  Programmer: Hank Childs
//  Creation:   June 23, 2000
//
// ****************************************************************************

void
Frame::CalculateTransformationMatrix(void)
{
    //
    // Calculate the elements of the matrix by using Cramer's Rule and
    // different choices of a vector t.
    //
    float  intermediate[3];
    Cross(intermediate, b1, b2);
    float  det = Dot(b0, intermediate);
    float  t[3];

    //
    // Set the parts of the matrix that are already determined.
    //
    e[0][3] = 0.;
    e[1][3] = 0.;
    e[2][3] = 0.;
    e[3][3] = 1.;

    //
    // Choice 1: t= <1,0,0>
    //
    t[0] = 1.;
    t[1] = 0.;
    t[2] = 0.;
    SolveWithT(e[0], b0, b1, b2, t, det);

    //
    // Choice 2: t= <0,1,0>
    //
    t[0] = 0.;
    t[1] = 1.;
    t[2] = 0.;
    SolveWithT(e[1], b0, b1, b2, t, det);

    //
    // Choice 3: t= <0,0,1>
    //
    t[0] = 0.;
    t[1] = 0.;
    t[2] = 1.;
    SolveWithT(e[2], b0, b1, b2, t, det);

    //
    // Choice 4: t= 0 - origin
    //
    t[0] = -origin[0];
    t[1] = -origin[1];
    t[2] = -origin[2];
    SolveWithT(e[3], b0, b1, b2, t, det);
}


// ****************************************************************************
//  Method: Frame::GetCoordsInFrame
//
//  Purpose:
//      Determines what the coordinates of point are in this frame.
//
//  Arguments:
//      point    The point (in Cartesian coordinates) to translate.
//      u        The u-coordinate of point in this frame.
//      v        The v-coordinate of point in this frame.
//      w        The w-coordinate of point in this frame.
//
//  Programmer: Hank Childs
//  Creation:   June 23, 2000
//
// ****************************************************************************

void
Frame::GetCoordsInFrame(const float point[3], float &u, float &v, float &w)
{
    //
    // This is just a matter of using the transformation matrix we calculated
    // earlier.
    //
    u = e[0][0]*point[0] + e[1][0]*point[1] + e[2][0]*point[2] + e[3][0]*1.;
    v = e[0][1]*point[0] + e[1][1]*point[1] + e[2][1]*point[2] + e[3][1]*1.;
    w = e[0][2]*point[0] + e[1][2]*point[1] + e[2][2]*point[2] + e[3][2]*1.;
}


// ****************************************************************************
//  Method: Cross
//
//  Purpose:
//      Performs the cross product of two vectors.
//
//  Arguments:
//      out    The resulting vector of the cross product.
//      in1    The first argument of the cross product.
//      in2    The second argument of the cross product.
//
//  Programmer: Hank Childs
//  Creation:   June 25, 2000
//
// ****************************************************************************

static inline void
Cross(float out[3], const float in1[3], const float in2[3])
{
    out[0] = in1[1]*in2[2] - in1[2]*in2[1];
    out[1] = in1[2]*in2[0] - in1[0]*in2[2];
    out[2] = in1[0]*in2[1] - in1[1]*in2[0];
}


// ****************************************************************************
//  Method: Dot
//
//  Purpose:
//      Performs the dot product of two vectors.
//
//  Arguments:
//      in1    The first argument of the dot product.
//      in2    The second argument of the dot product.
//
//  Returns:    The dot product of in1 and in2.
//
//  Programmer: Hank Childs
//  Creation:   June 25, 2000
//
// ****************************************************************************

static inline float
Dot(const float in1[3], const float in2[3])
{
    return (in1[0]*in2[0] + in1[1]*in2[1] + in1[2]*in2[2]);
}


// ****************************************************************************
//  Method: SolveWithT
//
//  Purpose:
//      A helper function to (hopefully) eliminate errors when using Cramer's
//      method to find the conversion matrix from the Cartesian frame to
//      this one.  Different choices of t can be used to solve different rows
//      of the conversion matrix.
//
//  Arugments:
//      e      One row of the conversion matrix.
//      b0     The first basis vector of the new frame.
//      b1     The second basis vector of the new frame.
//      b2     The third basis vector of the new frame.
//      t      A vector that is appropriate for e.
//      det    The determinant.  This is dot(b0, cross(b1, b2)).  Passed in
//             as an argument to prevent recalculation.
//
//  Programmer: Hank Childs
//  Creation:   June 25, 2000
//
// ****************************************************************************

static inline void
SolveWithT(float e[4], const float b0[3], const float b1[3], const float b2[3],
           const float t[3], float det)
{
    float  intermediate[3];
    float  d1, d2, d3;

    Cross(intermediate, b1, b2);
    d1 = Dot(intermediate, t);
    e[0] = d1 / det;

    Cross(intermediate, t, b2);
    d2 = Dot(intermediate, b0);
    e[1] = d2 / det;

    Cross(intermediate, b1, t);
    d3 = Dot(intermediate, b0);
    e[2] = d3 / det;
}


