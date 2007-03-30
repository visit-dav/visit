// ************************************************************************* //
//                         avtDistancePixelizer.h                            //
// ************************************************************************* //

#ifndef AVT_DISTANCE_PIXELIZER_H
#define AVT_DISTANCE_PIXELIZER_H
#include <pipeline_exports.h>


#include <avtPixelizer.h>

class   vtkLookupTable;


// ****************************************************************************
//  Class: avtDistancePixelizer
//
//  Purpose:
//      Pixelizes values based on distance.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
// ****************************************************************************

class PIPELINE_API avtDistancePixelizer
{
  public:
                      avtDistancePixelizer(double, double);
    virtual          ~avtDistancePixelizer();

    virtual void      GetColor(double value, double intensity,
                               unsigned char rgb[3]);
    void              SetLookupTable(vtkLookupTable *);

  protected:
    vtkLookupTable   *lut;
    double            minDistance;
    double            maxDistance;
};


#endif


