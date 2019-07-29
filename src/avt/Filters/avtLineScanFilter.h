// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtLineScanFilter.h                            //
// ************************************************************************* //

#ifndef AVT_LINE_SCAN_FILTER_H
#define AVT_LINE_SCAN_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>
#include <avtVector.h>


// ****************************************************************************
//  Class: avtLineScanFilter
//
//  Purpose:
//      This should really be a query, not a filter.  It sums all of the values
//      for a variable.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Jul 28 09:44:24 PDT 2006
//    Added CylindricalExecute.
//
//    David Bremer, Tue Nov 21 17:35:17 PST 2006
//    Added cylindrical ray distribution for hohlraum flux queries.
//    In this case, generate a cylindrical column of rays, all with 
//    the same direction and evenly distributed in space.  In 2D,
//    use a rectangular column of rays.  Rays may not cover the 
//    entire mesh.  FYI, this is unrelated to "CylindricalExecute"
//
//    Eric Brugger, Mon Jul 21 13:32:33 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtLineScanFilter : public avtDataTreeIterator
{
  public:
                                    avtLineScanFilter();
    virtual                        ~avtLineScanFilter();

    void                            SetNumberOfLines(int);

    virtual const char             *GetType(void)
                                             { return "avtLineScanFilter"; };
    virtual const char             *GetDescription(void)
                                             { return "Scanning lines"; };
    virtual void                    UpdateDataObjectInfo(void);
    virtual void                    SetRandomSeed(int s) { seed = s; };

    virtual const double           *GetLines() const { return lines; }

    enum LineDistributionType {
        UNIFORM_RANDOM_DISTRIB,     //Uniform position and direction within
                                    //a circle or sphere enclosing the data
        CYLINDER_DISTRIB            //Uniform spatial distribution along the 
                                    //length of a user-defined cylinder in 3D,
                                    //or a rectangle in 2D.
    };

    virtual void                   SetUniformRandomDistrib();
    virtual void                   SetCylinderDistrib(float *pos_, 
                                                      float  theta_,
                                                      float  phi_, 
                                                      float  radius_);

  protected:
    int                             nLines;
    int                             seed;
    double                         *lines;

    LineDistributionType            distribType;
    avtVector                       pos;
    float                           theta, phi, radius;

    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);

    virtual avtDataRepresentation  *ExecuteData(avtDataRepresentation *);
    virtual vtkDataSet             *CartesianExecute(vtkDataSet *);
    virtual vtkDataSet             *CylindricalExecute(vtkDataSet *);
};


#endif


