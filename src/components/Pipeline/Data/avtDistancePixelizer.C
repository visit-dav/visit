// ************************************************************************* //
//                            avtDistancePixelizer.C                         //
// ************************************************************************* //

#include <avtDistancePixelizer.h>

#include <vtkLookupTable.h>


// ****************************************************************************
//  Method: avtDistancePixelizer constructor
//
//  Arguments:
//      min      The minimum possible distance to the value.
//      max      The maximum possible distance to the value.
//      
//  Programmer:  Hank Childs
//  Creation:    December 5, 2000
//
// ****************************************************************************

avtDistancePixelizer::avtDistancePixelizer(double min, double max)
{
    minDistance = min;
    maxDistance = max;
    lut = vtkLookupTable::New();
    lut->SetTableRange(minDistance, maxDistance);
    lut->SetHueRange(0.6667, 0.);
    lut->Build();
}


// ****************************************************************************
//  Method: avtDistance destructor
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
// ****************************************************************************

avtDistancePixelizer::~avtDistancePixelizer()
{
    if (lut != NULL)
    {
        lut->Delete();
        lut = NULL;
    }
}


// ****************************************************************************
//  Method: avtDistancePixelizer::SetLookupTable
//
//  Purpose:
//      Overrides the default lookup table.  This routine does not need to be
//      called for the pixelizer to work properly.
//
//  Arguments:
//      newlut   The new lookup table.
//
//  Programmer:  Hank Childs
//  Creation:    December 5, 2000
//
// ****************************************************************************

void
avtDistancePixelizer::SetLookupTable(vtkLookupTable *newlut)
{
    if (newlut != NULL)
    {
        if (lut != NULL)
        {
            lut->Delete();
            lut = NULL;
        }

        lut = newlut;
        lut->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtDistancePixelizer::GetColor
//
//  Purpose:
//      Gets the color appropriate for a distance.  Also performs shading based
//      on the intensity.
//
//  Arguments:
//      dist        The distance to some critical value.
//      intensity   The shading at that point.
//      rgb         Where to put the resulting rgb values.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:20:35 PDT 2001
//    Cast to fix compiler warning.
//
// ****************************************************************************

void
avtDistancePixelizer::GetColor(double dist, double intensity,
                               unsigned char rgb[3])
{
    unsigned char *lutsrgb = lut->MapValue(dist);
    rgb[0] = (unsigned char)(lutsrgb[0] * intensity);
    rgb[1] = (unsigned char)(lutsrgb[1] * intensity);
    rgb[2] = (unsigned char)(lutsrgb[2] * intensity);
}


