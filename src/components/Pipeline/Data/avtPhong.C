// ************************************************************************* //
//                                 avtPhong.C                                //
// ************************************************************************* //

#include <avtPhong.h>

#include <math.h>
#include <visitstream.h>


// ****************************************************************************
//  Method: avtPhong constructor
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 13 15:34:07 PST 2002
//    Changed default light.
//
// ****************************************************************************

avtPhong::avtPhong()
{
    //
    // Default light is coming in from side.
    //
    double l[3] = { -0.5, -0.5, -1. };
    SetLightDirection(l);

    SetAmbient(0.5);
    SetDiffuse(1.);
    SetSpecular(1.);
    SetGlossiness(1);
}


// ****************************************************************************
//  Method: avtPhong destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtPhong::~avtPhong()
{
    ;
}


// ****************************************************************************
//  Method: avtPhong::SetLightDirection
//
//  Purpose:
//      Sets the vector of the light coming from the light source towards the
//      object (focal point).
//
//  Arguments:
//      l     The new light direction.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 13 15:34:07 PST 2002
//    Normalize the light direction.
//
// ****************************************************************************

void
avtPhong::SetLightDirection(double l[3])
{
    double mag = sqrt(l[0]*l[0] + l[1]*l[1] + l[2]*l[2]);
    if (mag == 0.)
    {
        mag = 1.;
    }
    lightDirection[0] = l[0] / mag;
    lightDirection[1] = l[1] / mag;
    lightDirection[2] = l[2] / mag;

    //
    // This is the half-way between the light and the view.  The view is 0,0,-1
    //
    half[0] = lightDirection[0] + 0.;
    half[1] = lightDirection[1] + 0.;
    half[2] = lightDirection[2] + -1.;

    if (half[2] == 0.)
    {
        //
        // The light is on the opposite side of the object from the camera.
        // Put it in the camera's location.
        //
        half[2] = -1.;
    }
    else
    {
        double norm = (half[0]*half[0])+(half[1]*half[1])+(half[2]*half[2]);
        norm = sqrt(norm);
        half[0] /= norm;
        half[1] /= norm;
        half[2] /= norm;
    }
}


// ****************************************************************************
//  Method: avtPhong::GetShading
//
//  Purpose:
//      Modifies the opacity to account for shading.
//
//  Arguments:
//      distance   The distance from the sample point to the plane.  This does
//                 not need to be in specific units, because it is the same
//                 units as the gradient.
//      gradient   The gradient around a point (assumed to be the normal of
//                 an underlying surface).
//
//  Note:       We are assuming a parallel light from far away and we are doing
//              an orthographic projection (in camera space), so the location
//              of the sample does not matter.
//
//  Note:       This version of the Phong lighting model is from Marc Levoy's
//              paper on "Display of Surfaces from Volume Data".  It cites
//              P. Bui-Tuong, "Illumination for Computer-Generated Pictures",
//              CACM, June 1975.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 13 15:34:07 PST 2002
//    Do not use specular highlighting.
//
// ****************************************************************************

double
avtPhong::GetShading(double distance, const double gradient[3]) const
{
    double mag = sqrt(gradient[0]*gradient[0] + gradient[1]*gradient[1]
                      + gradient[2]*gradient[2]);
    if (mag == 0)
    {
        return 1.;
    }  

    double norm[3];
    norm[0] = gradient[0] / mag;
    norm[1] = gradient[1] / mag;
    norm[2] = gradient[2] / mag;

    double dot = lightDirection[0]*norm[0] + lightDirection[1]*norm[1] 
                 + lightDirection[2]*norm[2];
    if (dot < 0)
    {
        dot = -dot;
    }
    return ambient + (1. - ambient)*dot;
}


