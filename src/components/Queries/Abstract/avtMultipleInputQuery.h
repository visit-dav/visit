// ************************************************************************* //
//                          avtMultipleInputQuery.h                          //
// ************************************************************************* //

#ifndef AVT_MULTIPLE_INPUT_QUERY_H
#define AVT_MULTIPLE_INPUT_QUERY_H

#include <query_exports.h>

#include <avtDataObjectQuery.h>
#include <avtMultipleInputSink.h>

#include <QueryAttributes.h>
#include <string.h>


// ****************************************************************************
//  Class: avtMultipleInputQuery
//
//  Purpose:
//      An abstract base class for queries that take multiple inputs.
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class QUERY_API avtMultipleInputQuery : public virtual avtDataObjectQuery, 
                                        public virtual avtMultipleInputSink
{
  public:
                              avtMultipleInputQuery();
    virtual                  ~avtMultipleInputQuery();

    virtual void             PerformQuery(QueryAttributes *);
    virtual std::string      GetResultMessage(void) { return resMsg; };
    virtual void             SetResultMessage(const std::string &m) 
                                 { resMsg = m; }; 
    virtual double           GetResultValue(void) { return resValue; };
    virtual void             SetResultValue(const double &d) { resValue = d; };

  protected:
    virtual void             Execute(void) = 0;

    QueryAttributes          queryAtts;

  private:
    std::string              resMsg;
    double                   resValue;
};


#endif


