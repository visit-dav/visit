// ************************************************************************* //
//                             avtCurveQuery.h                               //
// ************************************************************************* //

#ifndef AVT_CURVE_QUERY_H
#define AVT_CURVE_QUERY_H

#include <query_exports.h>

#include <avtDatasetQuery.h>

class     avtCurveConstructorFilter;


// ****************************************************************************
//  Class: avtCurveQuery
//
//  Purpose:
//      An abstract query that prepares curves for querying.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

class QUERY_API avtCurveQuery : public avtDatasetQuery
{
  public:
                                 avtCurveQuery();
    virtual                     ~avtCurveQuery();

  protected:
    avtCurveConstructorFilter   *ccf;
    virtual avtDataObject_p      ApplyFilters(avtDataObject_p);

    virtual void                 Execute(vtkDataSet *, const int);
    virtual double               CurveQuery(int, const float *,
                                            const float *) = 0;
    virtual std::string          CreateMessage(double) = 0;
};


#endif


