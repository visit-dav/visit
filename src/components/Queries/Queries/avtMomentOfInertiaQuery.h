// ************************************************************************* //
//                        avtMomentOfInertiaQuery.h                          //
// ************************************************************************* //

#ifndef AVT_MOMENT_OF_INERTIA_QUERY_H
#define AVT_MOMENT_OF_INERTIA_QUERY_H

#include <avtDatasetQuery.h>

#include <query_exports.h>

#include <string>
#include <vector>

class vtkDataSet;
class vtkCell;

class avtBinaryMultiplyFilter;
class avtVMetricVolume;


// ****************************************************************************
//  Class: avtMomentOfInertiaQuery
//
//  Purpose:
//      This query calculates the moment of inertia tensor for 3D datasets.
//
//  Programmer: Hank Childs
//  Creation:   May 17, 2005
//
// ****************************************************************************

class QUERY_API avtMomentOfInertiaQuery : public avtDatasetQuery
{
  public:
                                    avtMomentOfInertiaQuery();
    virtual                        ~avtMomentOfInertiaQuery();

    virtual const char             *GetType(void)
                                     {return "avtMomentOfInertiaQuery";};
    virtual const char             *GetDescription(void)
                                     {return "Calculating Moment of Inertia";};

  protected:
    double                          I[9];
    avtVMetricVolume               *volume;
    avtBinaryMultiplyFilter        *multiply;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual void                    VerifyInput(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
};


#endif


