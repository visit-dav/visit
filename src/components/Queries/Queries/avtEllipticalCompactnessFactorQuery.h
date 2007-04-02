// ************************************************************************* //
//                   avtEllipticalCompactnessFactorQuery.h                   //
// ************************************************************************* //

#ifndef AVT_ELLIPTICAL_COMPACTNESS_FACTOR_QUERY_H
#define AVT_ELLIPTICAL_COMPACTNESS_FACTOR_QUERY_H

#include <avtTwoPassDatasetQuery.h>

#include <query_exports.h>

#include <string>
#include <vector>

class vtkDataSet;
class vtkCell;

class avtBinaryMultiplyFilter;
class avtVMetricVolume;
class avtRevolvedVolume;


// ****************************************************************************
//  Class: avtEllipticalCompactnessFactorQuery
//
//  Purpose:
//      Calculates the spherical compactness factor of a data set.  This will
//      calculate the volume of the data set.  It will then calculate what the
//      radius of that volume would be if it was a perfect sphere.  Then it
//      will calculate what percentage of the data set is within that sphere.
//      (Note that this last step is the hardest, since it requires determining
//      where the center of the idealized sphere should be placed.)
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2005
//
// ****************************************************************************

class QUERY_API avtEllipticalCompactnessFactorQuery 
    : public avtTwoPassDatasetQuery
{
  public:
                                    avtEllipticalCompactnessFactorQuery();
    virtual                        ~avtEllipticalCompactnessFactorQuery();

    virtual const char             *GetType(void)
                         {return "avtEllipticalCompactnessFactorQuery";};
    virtual const char             *GetDescription(void)
                         {return "Calculating Elliptical Compactness Factor";};

  protected:
    double                          centroid[3];
    double                          sphere_center[3];
    double                          radius;
    double                          total_volume;
    double                          volume_inside;
    bool                            is2D;
    avtRevolvedVolume              *rev_volume;
    avtVMetricVolume               *volume;

    virtual void                    Execute1(vtkDataSet *, const int);
    virtual void                    Execute2(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    MidExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
};


#endif


