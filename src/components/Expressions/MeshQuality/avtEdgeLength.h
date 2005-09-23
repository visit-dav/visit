// ************************************************************************* //
//                             avtEdgeLength.h                               //
// ************************************************************************* //

#ifndef AVT_EDGE_LENGTH_H
#define AVT_EDGE_LENGTH_H


#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtEdgeLength
//
//  Purpose:
//
//     Calculates the minimum or maximum edge length for a zone.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

class EXPRESSION_API avtEdgeLength : public avtSingleInputExpressionFilter
{
  public:
                                avtEdgeLength();

    virtual const char         *GetType(void) { return "avtEdgeLength"; };
    virtual const char         *GetDescription(void)
                                    { return "Calculating edge length"; };

    void                        SetTakeMin(bool tm) { takeMin = tm; };
    
  protected:
    bool                        takeMin;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *);
    virtual bool                IsPointVariable(void)  { return false; };

    double                      GetEdgeLength(vtkCell *);
};


#endif


