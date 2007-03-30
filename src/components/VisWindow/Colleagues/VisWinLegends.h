// ************************************************************************* //
//                              VisWinLegends.h                              //
// ************************************************************************* //

#ifndef VIS_WIN_LEGENDS_H
#define VIS_WIN_LEGENDS_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>

class     vtkTextActor;


//
// I would like to make this a constant integer associated with the class, but
// I am concerned that some compilers will barf with that.  Will have to live
// with a #define to avoid magic numbers.
//
#define MAX_LEGENDS 6


// ****************************************************************************
//  Class: VisWinLegends
//
//  Purpose:
//      Manages the legends for each plot -- their placement and whether they
//      should have annotation for cycles/times, etc.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Apr 11 12:11:13 PDT 2002
//    Added flag to turn off the main database.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    vtkScaledTextActor has been deprecated in favor of vtkTextActor. 
//    vtkTextActor defines its own mapper, so vtkTextMapper not needed. 
//
// ****************************************************************************

class VISWINDOW_API VisWinLegends : public VisWinColleague
{
  public:
                                  VisWinLegends(VisWindowColleagueProxy &);
    virtual                      ~VisWinLegends();

    virtual void                  SetForegroundColor(float, float, float);
    virtual void                  UpdatePlotList(std::vector<avtActor_p> &);

    void                          SetVisibility(bool db, bool legend);
    void                          RemoveDBInfos(void);
    void                          AddDBInfos(void);

  protected:
    vtkTextActor                 *dbInfoActor[MAX_LEGENDS];
    bool                          dbInfoIsAdded[MAX_LEGENDS];
    bool                          mainDBInfoVisible;
    bool                          legendVisible;
    bool                          homogeneous;
    int                           numLegends;
    
    static const float            leftColumnPosition;
    static const float            rightColumnPosition;
    static const float            dbInfoHeight;
    static const float            dbInfoWidth;

    void                          PositionLegends(std::vector<avtActor_p> &);
    void                          UpdateDBInfo(std::vector<avtActor_p> &);

    float                         GetPosition(int);
};


#endif


