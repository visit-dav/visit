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
// ****************************************************************************

class QUERY_API avtDatasetQuery : public avtDataObjectQuery, 
                                     public avtDatasetSink
{
  public:
                              avtDatasetQuery();
    virtual                  ~avtDatasetQuery() {;};


    virtual void             PerformQuery(QueryAttributes *);
    virtual std::string      GetMessage(void) { return msg; };
    virtual void             SetMessage(const std::string &m) { msg = m; }; 

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
    std::string              msg;
};


#endif


