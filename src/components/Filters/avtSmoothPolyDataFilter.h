// ************************************************************************* //
//                          avtSmoothPolyDataFilter.h                        //
// ************************************************************************* //

#ifndef AVT_SMOOTH_POLY_DATA_FILTER_H
#define AVT_SMOOTH_POLY_DATA_FILTER_H

#include <filters_exports.h>

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
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class AVTFILTERS_API avtSmoothPolyDataFilter : public avtStreamer
{
  public:
                             avtSmoothPolyDataFilter();
    virtual                 ~avtSmoothPolyDataFilter();

    virtual const char      *GetType(void) 
                                 { return "avtSmoothPolyDataFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Smoothing geometry"; };

    void SetSmoothingLevel(int);
    
  protected:
    int     smoothingLevel;

    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
};


#endif


