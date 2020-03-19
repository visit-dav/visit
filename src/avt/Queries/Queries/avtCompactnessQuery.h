// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtCompactnessQuery.h                           //
// ************************************************************************* //

#ifndef AVT_COMPACTNESS_QUERY_H
#define AVT_COMPACTNESS_QUERY_H
#include <query_exports.h>

#include <avtTwoPassDatasetQuery.h>

#include <string>
#include <vector>

class vtkDataSet;
class vtkCell;

// ****************************************************************************
//  Class: avtCompactnessQuery
//
//  Purpose:
//      This query sums all of the values for a variable.
//
//  Programmer: Jeremy Meredith
//  Creation:   April  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 17 12:49:37 PDT 2003
//    Added additional queries which use a density variable.
//    Made it inherit from the (new) two-pass query.
//
//    Jeremy Meredith, Wed Jul 23 13:29:57 PDT 2003
//    Turned xBound and yBound into class data members, and made them
//    STL vectors, and made it collect the boundary points
//    across all domains and processors.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Kathleen Bonnell, Fri Sep  3 10:10:28 PDT 2004 
//    Added VerifyInput. 
//
//    Kathleen Biagas, Tue Apr 22 07:45:21 MST 2014
//    Use double instead of float.
//
// ****************************************************************************

class QUERY_API avtCompactnessQuery : public avtTwoPassDatasetQuery
{
  public:
                                    avtCompactnessQuery();
    virtual                        ~avtCompactnessQuery();

    virtual const char             *GetType(void)
                                             { return "avtCompactnessQuery"; };
    virtual const char             *GetDescription(void)
                                             { return descriptionBuffer; };

  protected:
    int                             numDomains;

    std::vector<double>             xBound;
    std::vector<double>             yBound;

    double                          totalXSectArea;
    double                          totalRotVolume;
    double                          distBound_da_xsa;
    double                          distBound_da_vol;
    double                          distBound_dv_xsa;
    double                          distBound_dv_vol;
    double                          distOrigin_da;

    bool                            densityValid;
    double                          totalRotMass;
    double                          centMassX;
    double                          centMassY;
    double                          distBound_dv_den_vol;
    double                          distCMass_dv_den_vol;

    char                            descriptionBuffer[1024];

    virtual void                    Execute1(vtkDataSet *, const int);
    virtual void                    Execute2(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    MidExecute();
    virtual void                    PostExecute(void);
    virtual void                    VerifyInput(void);

    static void                     Get2DCellCentroid(vtkCell*,double&,double&);
    static double                   Get2DCellArea(vtkCell*);
    static double                   Get2DTriangleArea(double*,double*,double*);

};


#endif


