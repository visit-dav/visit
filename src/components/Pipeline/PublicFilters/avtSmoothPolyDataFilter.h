// ************************************************************************* //
//                          avtSmoothPolyDataFilter.h                        //
// ************************************************************************* //

#ifndef AVT_SMOOTH_POLY_DATA_FILTER_H
#define AVT_SMOOTH_POLY_DATA_FILTER_H

#include <pipeline_exports.h>

#include <avtStreamer.h>


// ****************************************************************************
//  Class: avtSmoothPolyDataFilter
//
//  Purpose:
//      Smooths geometry.
//
//  Programmer: Jeremy Meredith
//  Creation:   December  6, 2002
//
//  Modifications:
//
// ****************************************************************************

class PIPELINE_API avtSmoothPolyDataFilter : public avtStreamer
{
  public:
                             avtSmoothPolyDataFilter() {;};
    virtual                 ~avtSmoothPolyDataFilter() {;};

    virtual const char      *GetType(void) { return "avtSmoothPolyDataFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Smoothing geometry"; };

    void SetSmoothingLevel(int);
    
  protected:
    int     smoothingLevel;

    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
};


#endif


