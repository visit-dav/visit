// ************************************************************************* //
//                         avtAverageMeanCurvatureQuery.h                    //
// ************************************************************************* //

#ifndef AVT_AVERAGE_MEAN_CURVATURE_QUERY_H
#define AVT_AVERAGE_MEAN_CURVATURE_QUERY_H

#include <query_exports.h>

#include <avtWeightedVariableSummationQuery.h>


class     avtCurvatureExpression;


// ****************************************************************************
//  Class: avtAverageMeanCurvatureQuery
//
//  Purpose:
//      A query that will calculate the mean curvature and then find the
//      average of that quantity.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

class QUERY_API avtAverageMeanCurvatureQuery : public avtWeightedVariableSummationQuery
{
  public:
                         avtAverageMeanCurvatureQuery();
    virtual             ~avtAverageMeanCurvatureQuery();

    virtual const char  *GetType(void)  
                             { return "avtAverageMeanCurvatureQuery"; };

  protected:
    avtCurvatureExpression    *curvature;

    virtual avtDataObject_p    CreateVariable(avtDataObject_p d);
    virtual std::string        GetVarname(std::string &s) 
                                            { return "curvature"; };
    virtual void               VerifyInput(void);
    virtual bool               CalculateAverage(void) { return true; };
};


#endif


