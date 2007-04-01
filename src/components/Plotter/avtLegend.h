// ************************************************************************* //
//                                avtLegend.h                                //
// ************************************************************************* //

#ifndef AVT_LEGEND_H
#define AVT_LEGEND_H
#include <plotter_exports.h>


#include <ref_ptr.h>


class  vtkVerticalScalarBarActor;
class  vtkRenderer;


// ****************************************************************************
//  Class: avtLegend
//
//  Purpose:
//      Make a legend for the plots.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Dec 22 15:57:46 PST 2000
//    Removed inheritance from avtDecorator.
//
//    Eric Brugger, Mon Jul 14 15:50:29 PDT 2003
//    Added GetLegendOn, SetFontHeight, SetTitle, SetDatabase, SetVarName,
//    SetMessage and Update.
//
//    Eric Brugger, Wed Jul 16 07:59:54 PDT 2003
//    Made private data members protected.
//
//    Eric Brugger, Thu Jul 17 08:13:45 PDT 2003
//    Added maxSize argument to GetLegendSize.
//
//    Brad Whitlock, Tue Jul 20 16:40:37 PST 2004
//    Added SetVarUnits.
//
// ****************************************************************************

class PLOTTER_API  avtLegend
{
  public:
                                  avtLegend();
    virtual                      ~avtLegend();

    virtual void                  Add(vtkRenderer *);
    virtual void                  Remove(void);

    virtual void                  GetLegendPosition(float &, float &);
    virtual void                  SetLegendPosition(float, float);
    virtual void                  GetLegendSize(float, float &, float &);
    virtual void                  LegendOn(void);
    virtual void                  LegendOff(void);
    bool                          GetLegendOn(void) const;

    virtual void                  SetForegroundColor(const float [3]);
    virtual void                  SetFontHeight(float);

    void                          SetTitle(const char *);
    void                          SetDatabaseInfo(const char *);
    void                          SetVarName(const char *);
    void                          SetVarUnits(const char *);
    void                          SetMessage(const char *);

    void                          Update();

  protected:
    float                         position[2];
    float                         size[2];
    bool                          legendOn;
    bool                          currentlyDrawn;
    vtkVerticalScalarBarActor    *legend;
    vtkRenderer                  *renderer;

    float                         fontHeight;
    char                         *title;
    char                         *databaseInfo;
    char                         *varName;
    char                         *varUnits;
    char                         *message;

    virtual void                  ChangePosition(float, float) = 0;
};


typedef ref_ptr<avtLegend> avtLegend_p;


#endif


