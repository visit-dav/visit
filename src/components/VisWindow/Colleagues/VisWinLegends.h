// ************************************************************************* //
//                              VisWinLegends.h                              //
// ************************************************************************* //

#ifndef VIS_WIN_LEGENDS_H
#define VIS_WIN_LEGENDS_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>

class     vtkTextActor;


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
//    Eric Brugger, Mon Jul 14 16:27:28 PDT 2003
//    Changed the way database information is handled.
//
//    Eric Brugger, Wed Jul 16 09:47:15 PDT 2003
//    Removed GetPosition.
//
//    Kathleen Bonnell, Thu Nov 13 12:21:25 PST 2003 
//    Added bool return type to CreateDatabaseInfo. 
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

  protected:
    vtkTextActor                 *dbInfoActor;
    bool                          dbInfoIsAdded;
    bool                          mainDBInfoVisible;
    bool                          legendVisible;
    bool                          homogeneous;
    
    static const float            leftColumnPosition;
    static const float            rightColumnPosition;
    static const float            dbInfoHeight;
    static const float            dbInfoWidth;

    void                          PositionLegends(std::vector<avtActor_p> &);
    void                          UpdateDBInfo(std::vector<avtActor_p> &);

  private:
    static bool                   CreateDatabaseInfo(char *,
                                                     avtDataAttributes &);
};


#endif


