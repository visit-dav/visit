// ************************************************************************* //
//                              avtDatasetQuery.h                            //
// ************************************************************************* //

#ifndef AVT_DATASET_QUERY_H
#define AVT_DATASET_QUERY_H
#include <query_exports.h>


#include <avtDataObjectQuery.h>
#include <avtDatasetSink.h>
#include <QueryAttributes.h>
#include <string.h>

class vtkDataSet;

// ****************************************************************************
//  Class: avtDatasetQuery
//
//  Purpose:
//      This is a data object query whose input is a dataset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Made queryAtts a protected member so that derived types may have access
//    to the atts used to perform the query.  Added private Execute method
//    that operates on a data tree.  Added private members totalNodes and
//    currentNode for progress-tracking.  Removed un-used SetAtts method.
//
//    Jeremy Meredith, Fri Apr 11 10:04:16 PDT 2003
//    Added a const to SetMessage so c-strings would be accepted.
//
//    Jeremy Meredith, Thu Apr 17 12:55:25 PDT 2003
//    Made some methods visible/virtual to subclasses so they could override
//    them. This was necessary so I could create the avtTwoPassDatasetQuery.
//
//    Kathleen Bonnell,  Fri Jul 11 16:17:12 PDT 2003
//    Added value and Set/Get methods. Renamed Set/GetMessage to 
//    Set/GetResultMessage
//    
//    Kathleen Bonnell,  Wed Nov 12 18:18:19 PST 2003 
//    Made resValue a doubleVector.  Add more Get/Set methods so that queries
//    that only return 1 value don't need to be updated. 
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Kathleen Bonnell, Thu Apr  1 19:02:38 PST 2004 
//    Added method PerformQueryInTime.
//
//    Kathleen Bonnell, Fri Apr  2 08:51:17 PST 2004 
//    Changed args to PerformQueryInTime.
//
//    Kathleen Bonnell, Thu Jun 24 07:45:47 PDT 2004 
//    Added intVector and string arguments to PerformQueryInTime.
//
// ****************************************************************************

class QUERY_API avtDatasetQuery : public avtDataObjectQuery, 
                                     public avtDatasetSink
{
  public:
                              avtDatasetQuery();
    virtual                  ~avtDatasetQuery();


    virtual void             PerformQuery(QueryAttributes *);
    virtual void             PerformQueryInTime(QueryAttributes*,
                                 const int, const int, const int,
                                 const int, doubleVector &,
                                 intVector &, std::string &);
    virtual std::string      GetResultMessage(void) { return resMsg; };
    virtual void             SetResultMessage(const std::string &m) 
                                 { resMsg = m; }; 

    virtual double           GetResultValue(const int i = 0); 
    virtual void             SetResultValue(const double &d, const int i = 0);

    virtual void             AddResultValue(const double &d)
                                 { resValue.push_back(d); };
    virtual doubleVector     GetResultValues(void) { return resValue; };
    virtual void             SetResultValues(const doubleVector &d)
                                 { resValue = d; };

  protected:

    virtual void             PreExecute(void);
    virtual void             PostExecute(void);

    virtual void             Execute(vtkDataSet*, const int) = 0;
    virtual avtDataObject_p  ApplyFilters(avtDataObject_p);

    QueryAttributes          queryAtts;

    int                      totalNodes;
    int                      currentNode;

  private:
    virtual void             Execute(avtDataTree_p);
    std::string              resMsg;
    doubleVector             resValue;
};


#endif


