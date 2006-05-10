// ************************************************************************* //
//                    avtLocalizedCompactnessFactorQuery.h                   //
// ************************************************************************* //

#ifndef AVT_LOCALIZED_COMPACTNESS_FACTOR_QUERY_H
#define AVT_LOCALIZED_COMPACTNESS_FACTOR_QUERY_H

#include <avtDatasetQuery.h>

#include <query_exports.h>

#include <string>
#include <vector>

class vtkDataSet;
class vtkCell;

class avtBinaryMultiplyFilter;
class avtVMetricVolume;
class avtRevolvedVolume;


// ****************************************************************************
//  Class: avtLocalizedCompactnessFactorQuery
//
//  Purpose:
//      Calculates the localized compactness factor of a data set.  This is
//      defined as follows:
//      1) For each point in space, determine the compactness around that
//      point, defined as the percent of the surrounding spherical region that
//      is the same material.
//      2) The localized compactness factor is the average over all points in
//      space.
//
//  Caveats:
//      1) To ease parallel implementation issues, the LCF starts by resampling
//      onto a rectilinear grid so that this calculation can be done in serial.
//      2) User settable knobs are:
//         2a) the radius to consider
//         2b) a weighting factor that, for a given point, allows portions of
//             the spherical region that are closer to count more heavily.
//      3) This calculation works in 2D, but it assumes that the 2D data set
//         is in cylindrical coordinates and it will revolve the data set
//         around the X-axis.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
// ****************************************************************************

class QUERY_API avtLocalizedCompactnessFactorQuery 
    : public avtDatasetQuery
{
  public:
                                    avtLocalizedCompactnessFactorQuery();
    virtual                        ~avtLocalizedCompactnessFactorQuery();

    virtual const char             *GetType(void)
                         {return "avtLocalizedCompactnessFactorQuery";};
    virtual const char             *GetDescription(void)
                         {return "Calculating Localized Compactness Factor";};

  protected:
    double                          sum;
    int                             numEntries;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
};


#endif


