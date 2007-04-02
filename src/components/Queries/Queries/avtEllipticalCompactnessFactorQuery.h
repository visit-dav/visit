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
//      Calculates the elliptical compactness factor of a data set.  This will
//      calculate the volume of the data set.  It will then propose many
//      possible ellipses that have the same volume.  Then it
//      will calculate what percentage of the data set is within each of
//      the ellipses.  The ECF is the highest percentage.
//
//  Programmer: Hank Childs
//  Creation:   May 16, 2006
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
    double                          ellipse_center[3];
    // NOTE: numGuesses should have an integer sqrt.
    static const int                numGuesses = 289;
    double                          x_radius[numGuesses];
    double                          y_radius[numGuesses];
    double                          z_radius[numGuesses];
    double                          bounds[6];
    double                          total_volume;
    double                          volume_inside[numGuesses];
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


