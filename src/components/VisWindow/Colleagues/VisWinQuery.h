// ************************************************************************* //
//                              VisWinQuery.h                                //
// ************************************************************************* //

#ifndef VIS_WIN_QUERY_H
#define VIS_WIN_QUERY_H
#include <viswindow_exports.h>

#include <VisWinColleague.h>
#include <VisWindowTypes.h>
#include <avtPickActor.h>
#include <avtLineoutActor.h>
#include <Line.h>
#include <PickAttributes.h>

// ****************************************************************************
//  Class: VisWinQuery
//
//  Purpose:
//    Handles queries. (Currently only pick & lineout). 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 13, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar 18 09:27:06 PST 2002
//    To better control size of pick letters, changed signature of method 
//    ComputePickScaleFactor.  Add method SetPickScale, member hidden.  
//    Removed member pickScaleFactor.
//
//    Kathleen Bonnell, Tue Mar 26 16:08:23 PST 2002  
//    Moved pick-specific code to avtPickActor.  Keep a list of pick actors.
//    
//    Kathleen Bonnell, Tue Apr 15 15:38:14 PDT 2002  
//    Added support for other query types.  Specifically, Lineout. 
//    
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002 
//    Added UpdateQuery, DeleteQuery.
//    
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002 
//    Changed argument for QueryIsValid and Lineout to Line* to convey more
//    information than simply color. 
//    
//    Kathleen Bonnell, Thu Dec 19 13:32:47 PST 2002 
//    Removed member designator and method GetNextDesignator. Added 
//    argument to QueryIsValid.
//    
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003 
//    Replaced char * argument of QueryIsValid and Pick methods with 
//    PickAttributes.
//    
// ****************************************************************************

class VISWINDOW_API VisWinQuery : public VisWinColleague
{
  public:
                                  VisWinQuery(VisWindowColleagueProxy &);
    virtual                      ~VisWinQuery();

    void                          EndBoundingBox(void);
    void                          StartBoundingBox(void);

    virtual void                  SetForegroundColor(float, float, float);
    virtual void                  UpdateView(void);


    void                          SetQueryType(QUERY_TYPE);
    QUERY_TYPE                    GetQueryType(void) const;

    void                          SetAttachmentPoint(float, float, float);
    void                          SetSecondaryPoint(float, float, float);

    void                          QueryIsValid(const PickAttributes *, const Line *);
    void                          UpdateQuery(const Line *);
    void                          DeleteQuery(const Line *);

    void                          ClearQueries(void);
    void                          ClearAllQueries(void);

    void                          Lineout(const Line *);
    void                          ClearLineouts(void);

    void                          Pick(const PickAttributes *);
    void                          ClearPickPoints(void);

  protected:
    std::vector< avtPickActor_p >      pickPoints;
    std::vector< avtLineoutActor_p >   lineOuts;

    bool                               hidden;
    QUERY_TYPE                         type;
    float                              attachmentPoint[3]; 
    float                              secondaryPoint[3]; 
};


#endif
