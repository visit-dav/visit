// ************************************************************************* //
//                         avtVariablePixelizer.h                            //
// ************************************************************************* //

#ifndef AVT_VARIABLE_PIXELIZER_H
#define AVT_VARIABLE_PIXELIZER_H
#include <pipeline_exports.h>


#include <avtPixelizer.h>

class   vtkLookupTable;


// ****************************************************************************
//  Class: avtVariablePixelizer
//
//  Purpose:
//      Pixelizes values based on value.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
// ****************************************************************************

class PIPELINE_API avtVariablePixelizer : public avtPixelizer
{
  public:
                      avtVariablePixelizer(double, double);
    virtual          ~avtVariablePixelizer();

    virtual void      GetColor(double value, double intensity,
                               unsigned char rgb[3]);
    void              SetLookupTable(vtkLookupTable *);

  protected:
    vtkLookupTable   *lut;
    double            minVal;
    double            maxVal;
};


#endif


