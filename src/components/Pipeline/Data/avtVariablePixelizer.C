// ************************************************************************* //
//                            avtVariablePixelizer.C                         //
// ************************************************************************* //

#include <avtVariablePixelizer.h>

#include <vtkLookupTable.h>


// ****************************************************************************
//  Method: avtVariablePixelizer constructor
//
//  Arguments:
//      min      The minimum possible value.
//      max      The maximum possible value.
//      
//  Programmer:  Hank Childs
//  Creation:    December 5, 2000
//
// ****************************************************************************

avtVariablePixelizer::avtVariablePixelizer(double min, double max)
{
    minVal = min;
    maxVal = max;
    lut = vtkLookupTable::New();
    lut->SetTableRange(minVal, maxVal);
    lut->SetHueRange(0.6667, 0.);
    lut->Build();
}


// ****************************************************************************
//  Method: avtVariable destructor
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
// ****************************************************************************

avtVariablePixelizer::~avtVariablePixelizer()
{
    if (lut != NULL)
    {
        lut->Delete();
        lut = NULL;
    }
}


// ****************************************************************************
//  Method: avtVariablePixelizer::SetLookupTable
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
avtVariablePixelizer::SetLookupTable(vtkLookupTable *newlut)
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
//  Method: avtVariablePixelizer::GetColor
//
//  Purpose:
//      Gets the color appropriate for a value.  Also performs shading based
//      on the intensity.
//
//  Arguments:
//      val         The value along the ray (could be maximum or average, but
//                  it is not important for coloring).
//      intensity   The shading at that point.
//      rgb         Where to put the resulting rgb values.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:20:35 PDT 2001
//    Cast to get rid of compiler warning.
//
// ****************************************************************************

void
avtVariablePixelizer::GetColor(double val, double intensity,
                               unsigned char rgb[3])
{
    unsigned char *lutsrgb = lut->MapValue(val);
    rgb[0] = (unsigned char)(lutsrgb[0] * intensity);
    rgb[1] = (unsigned char)(lutsrgb[1] * intensity);
    rgb[2] = (unsigned char)(lutsrgb[2] * intensity);
}


