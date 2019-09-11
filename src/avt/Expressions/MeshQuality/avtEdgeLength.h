// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//
//    Hank Childs, Wed May 21 15:55:53 PDT 2008
//    Re-define GetVariableDimension.
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

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool                IsPointVariable(void)  { return false; };
    virtual int                 GetVariableDimension()   { return 1; };

    double                      GetEdgeLength(vtkCell *);
};


#endif


