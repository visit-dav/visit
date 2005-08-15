// ************************************************************************* //
//                              avtCentroidQuery.h                           //
// ************************************************************************* //

#ifndef AVT_CENTROID_QUERY_H
#define AVT_CENTROID_QUERY_H

#include <avtDatasetQuery.h>

#include <query_exports.h>

#include <string>
#include <vector>

class vtkDataSet;
class vtkCell;

class avtAbsValFilter;
class avtBinaryMultiplyFilter;
class avtVMetricArea;
class avtVMetricVolume;


// ****************************************************************************
//  Class: avtCentroidQuery
//
//  Purpose:
//      This query calculates the moment of inertia tensor for 3D datasets.
//
//  Programmer: Hank Childs
//  Creation:   May 17, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug 12 15:25:07 PDT 2005
//    Use absolute value of the volume or area.
//
// ****************************************************************************

class QUERY_API avtCentroidQuery : public avtDatasetQuery
{
  public:
                                    avtCentroidQuery();
    virtual                        ~avtCentroidQuery();

    virtual const char             *GetType(void)
                                     {return "avtCentroidQuery";};
    virtual const char             *GetDescription(void)
                                     {return "Calculating Centroid";};

  protected:
    double                          C[3];
    double                          total_mass;
    avtVMetricArea                 *area;
    avtVMetricVolume               *volume;
    avtBinaryMultiplyFilter        *multiply;
    avtAbsValFilter                *absval;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
};


#endif


