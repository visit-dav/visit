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
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
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
    virtual int               GetVariableDimension() { return 3; }
    virtual void              GetMacro(std::vector<std::string> &, 
                                       std::string &, Expression::ExprType &);
};


#endif

