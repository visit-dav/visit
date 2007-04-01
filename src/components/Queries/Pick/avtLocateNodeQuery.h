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
    int                             RGridFindNode(vtkDataSet *, float&, float*);
    int                             DeterminePickedNode(vtkDataSet *, int, float*);
    int                             FindClosestPoint(vtkDataSet *, float*, int &);
};


#endif


