// ************************************************************************* //
//                              VisWinQuery.h                                //
// ************************************************************************* //

#ifndef VIS_WIN_QUERY_H
#define VIS_WIN_QUERY_H
#include <viswindow_exports.h>

#include <VisualCueInfo.h>
#include <VisWinColleague.h>
#include <VisWindowTypes.h>
#include <avtPickActor.h>
#include <avtLineoutActor.h>

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
//    Kathleen Bonnell, Fri Jun  6 15:17:45 PDT 2003  
//    Added FullFrameOn/Off methods. 
//    
//    Kathleen Bonnell, Wed Jun 25 14:30:39 PDT 2003 
//    Removed type & its Set/Get methods.  Removed unused methods
//    ClearQueries, ClearAllQueries. Removed attachmentPoint/secondaryPoint
//    and their Set methods.
//    
//    Kathleen Bonnell, Tue Jul  8 20:02:21 PDT 2003 
//    Add method ReAddToWindow. 
//    
//    Kathleen Bonnell, Fri Feb 20 12:37:26 PST 2004 
//    Added methods CreateTranslationVector, CreateShiftVector, 
//    CalculateShiftDistance.
//
//    Mark C. Miller Wed Jun  9 17:44:38 PDT 2004
//    Modified with PickEntry and LineEntry data types in vectors of pickPoints
//    and refLines. Changed some interfaces to use VisualCueInfo.
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

    void                          QueryIsValid(const VisualCueInfo *, const VisualCueInfo *);
    void                          UpdateQuery(const VisualCueInfo *);
    void                          DeleteQuery(const VisualCueInfo *);

    void                          Lineout(const VisualCueInfo *);
    void                          ClearLineouts(void);

    void                          Pick(const VisualCueInfo *);
    void                          ClearPickPoints(void);

    virtual void                  FullFrameOn(const double, const int);
    virtual void                  FullFrameOff(void);
    virtual void                  ReAddToWindow(void);

    void                          GetVisualCues(const VisualCueInfo::CueType cueType,
                                      std::vector<const VisualCueInfo *>& cues) const;

  protected:
    void                          CreateTranslationVector(const double, 
                                      const int, float vec[3]);
    void                          CreateTranslationVector(float vec[3]);
    void                          CreateShiftVector(float vec[3], const float);
    float                         CalculateShiftDistance(void);

    struct PickEntry {
        avtPickActor_p pickActor;
        VisualCueInfo vqInfo;
    };

    struct LineEntry {
        avtLineoutActor_p lineActor;
        VisualCueInfo vqInfo;
    };

    std::vector< PickEntry > pickPoints;
    std::vector< LineEntry > lineOuts;

    bool                       hidden;
};


#endif
