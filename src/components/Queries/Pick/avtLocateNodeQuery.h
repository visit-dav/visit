// ************************************************************************* //
//                           avtLocateNodeQuery.h                            //
// ************************************************************************* //

#ifndef AVT_LOCATE_NODE_QUERY_H
#define AVT_LOCATE_NODE_QUERY_H
#include <query_exports.h>

#include <avtLocateQuery.h>



// ****************************************************************************
//  Class: avtLocateNodeQuery
//
//  Purpose:
//      This query locates a node and domain given a world-coordinate point. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 18, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 17 12:58:47 PDT 2004
//    Added FindClosestPoint.
//
//    Kathleen Bonnell, Wed Jul  7 14:59:49 PDT 2004 
//    Added FindClosestPointOnLine.
//
// ****************************************************************************

class QUERY_API avtLocateNodeQuery : public avtLocateQuery
{
  public:
                                    avtLocateNodeQuery();
    virtual                        ~avtLocateNodeQuery();

    virtual const char             *GetType(void)
                                             { return "avtLocateNodeQuery"; };
    virtual const char             *GetDescription(void)
                                             { return "Locating node."; };

  protected:
    virtual void                    Execute(vtkDataSet *, const int);
    int                             RGridFindNode(vtkDataSet *, double&, 
                                                  double*);
    int                             DeterminePickedNode(vtkDataSet *, int, 
                                                       double*);
    int                             FindClosestPoint(vtkDataSet *, const int,
                                                  double*, int &);
    int                             FindClosestPointOnLine(vtkDataSet *, 
                                                           double &,
                                                           double [3]);
 
};


#endif


