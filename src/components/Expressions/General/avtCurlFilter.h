// ************************************************************************* //
//                               avtCurlFilter.h                             //
// ************************************************************************* //

#ifndef AVT_CURL_FILTER_H
#define AVT_CURL_FILTER_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtCurlFilter
//
//  Purpose:
//      A filter that calculates the curl.  The curl takes in a vector and
//      produces a vector.  This depends on several partial derivatives, 
//      which are accomplished using the gradient expression.
//
//      Because we need to use other expressions, this is a derived type of
//      the macro expression filter.
//
//      curl of vector {u,v,w} = { grad(w)[1]-grad(v)[2], 
//                                 grad(u)[2]-grad(w)[0],
//                                 grad(v)[0]-grad(u)[1] }
//
//      Curl has the following physical interpretation:
//      Imagine you have a pinwheel -- a disc with some squares placed along
//      the disc in a direction orthogonal to the disc (so that air can spin
//      the disc by pushing on the squares).
//      If at some point (X,Y,Z) the curl is (a,b,c), then placing the disc so
//      that it is normal to (a,b,c) will give the fastest possible rotational 
//      speed that is attainable by having the center of the pinwheel at 
//      (X,Y,Z).
//
//      Also: John Boyd felt that we should define curl for 2D variables as 
//      well.  In this case, only the third component of the vector will be
//      non-zero, so we return a scalar (instead of a vector) in this case.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Aug 19 08:50:02 PDT 2005
//    Move definition of GetVariableDimension to the .C file.
//
// ****************************************************************************

class EXPRESSION_API avtCurlFilter : public avtMacroExpressionFilter
{
  public:
                              avtCurlFilter();
    virtual                  ~avtCurlFilter();

    virtual const char       *GetType(void)   { return "avtCurlFilter"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating Curl"; };

  protected:
    virtual int               GetVariableDimension();
    virtual void              GetMacro(std::vector<std::string> &, 
                                       std::string &, Expression::ExprType &);
};


#endif

