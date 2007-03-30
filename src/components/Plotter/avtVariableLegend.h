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
// ****************************************************************************

class PLOTTER_API avtVariableLegend : public avtLegend
{
  public:
                               avtVariableLegend();
                               avtVariableLegend(int arg);
    virtual                   ~avtVariableLegend();

    void                       SetRange(float min, float max);
    void                       SetVarRange(float min, float max);
    void                       GetVarRange(float &, float &);
    virtual void               SetVarName(const char *name);
    void                       SetTitle(const char *);
    void                       SetMessage(const char *);
    void                       SetColorBar(const int);
    void                       SetScaling(int mode = 0, float skew = 1.);
    void                       SetLookupTable(vtkLookupTable *);

  protected:
    float                      min, max;
    char *                     title;

    vtkVerticalScalarBarActor *sBar;
    vtkLookupTable            *lut;

    virtual void               ChangePosition(float, float);
};


#endif


