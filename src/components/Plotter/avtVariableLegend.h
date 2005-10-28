// ************************************************************************* //
//                            avtVariableLegend.h                            //
// ************************************************************************* //

#ifndef AVT_VARIABLE_LEGEND_H
#define AVT_VARIABLE_LEGEND_H
#include <plotter_exports.h>


#include <avtLegend.h>

class  vtkVerticalScalarBarActor;
class  vtkLookupTable;


// ****************************************************************************
//  Class: avtVariableLegend
//
//  Purpose:
//      The legend for any plot that is variable based.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Dec  8 15:02:31 PST 2000 
//    Changed sBar to be of type vtkVerticalScalarBarActor, a modified
//    version of the original, better suited to VisIt.  Added method
//    'SetVarName' so that the variable name used for this plot could
//    be part of the title.
//
//    Kathleen Bonnell, Thu Mar 15 13:16:41 PST 2001 
//    Added SetTitle and SetMessage methods. 
//
//    Kathleen Bonnell, Fri Mar 30 12:30:33 PDT 2001 
//    Added SetScaling and SetVarRange methods. 
//
//    Brad Whitlock, Thu Apr 19 15:29:36 PST 2001
//    Added a second constructor that does not create the scalar bar actor.
//
//    Brad Whitlock, Fri Jun 15 10:07:21 PDT 2001
//    Added the SetLUTColors method.
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001 
//    Functionality of SetLUTColors is now in avtLookupTable.  Removed
//    this method.  Add SetLookupTable.
//
//    Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001 
//    Added member 'title'. 
//    
//    Eric Brugger, Mon Jul 14 15:53:54 PDT 2003
//    Moved SetVarName, SetTitle and SetMessage to avtLegend.
//
//    Eric Brugger, Wed Jul 16 08:15:15 PDT 2003
//    I added  GetLegendSize.  I renamed SetColorBar to SetColorBarVisibility,
//    SetRange to SetVarRangeVisibility and GetRange to GetVarRange.  I also
//    added data members to track the colorbar visibility and variable range
//    visibility.
//
//    Eric Brugger, Thu Jul 17 08:45:29 PDT 2003
//    Added maxSize argument to GetLegendSize.
//
//    Kathleen Bonnell, Tue Oct 25 11:13:14 PDT 2005 
//    Added virtual methods ChangeTitle, ChangeFontHeight.
// 
// ****************************************************************************

class PLOTTER_API avtVariableLegend : public avtLegend
{
  public:
                               avtVariableLegend();
                               avtVariableLegend(int arg);
    virtual                   ~avtVariableLegend();

    virtual void               GetLegendSize(float, float &, float &);

    void                       SetColorBarVisibility(const int);
    void                       SetRange(float min, float max);
    void                       GetRange(float &, float &);
    void                       SetScaling(int mode = 0, float skew = 1.);
    void                       SetLookupTable(vtkLookupTable *);
    void                       SetVarRangeVisibility(const int);
    void                       SetVarRange(float min, float max);

  protected:
    float                      min, max;

    vtkVerticalScalarBarActor *sBar;
    vtkLookupTable            *lut;

    int                        barVisibility;
    int                        rangeVisibility;

    virtual void               ChangePosition(float, float);
    virtual void               ChangeTitle(const char *);
    virtual void               ChangeFontHeight(float);
};


#endif


