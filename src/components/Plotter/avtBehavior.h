// ************************************************************************* //
//                                avtBehavior.h                              //
// ************************************************************************* //

#ifndef AVT_BEHAVIOR_H
#define AVT_BEHAVIOR_H
#include <plotter_exports.h>


#include <ref_ptr.h>

#include <avtDataObjectInformation.h>
#include <avtLegend.h>
#include <RenderOrder.h>
#include <VisWindowTypes.h>

class  vtkRenderer;

// ****************************************************************************
//  Class: avtBehavior
//
//  Purpose:
//      This defines the behavior of a plot.  It is all of the things that go
//      into an actor besides the actual drawable.  This always exists on the
//      viewer and exists so that VisIt can give the appearance of being
//      very interactive even when in image-mode.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 12 16:49:27 PST 2001
//    Added shift factor for plots.
//
//    Kathleen Bonnell, Tue Apr  3 15:17:06 PDT 2001 
//    Added render order for plots.
//
//    Kathleen Bonnell, Mon Nov 26 9:16:32 PST 2001 
//    Added hook to retrieve flag specifiying whether points were tranformed.
//
//    Kathleen Bonnell, Tue May  7 09:36:15 PDT 2002 
//    Added GetDataExtents method. 
//
//    Kathleen Bonnell, Wed Mar 26 14:29:23 PST 2003  
//    Renamed "GetTransformedPoints" to "RequiresReExecuteForQuery" to more
//    accurately reflect its purpose. 
//
//    Eric Brugger, Wed Aug 20 09:50:56 PDT 2003
//    Added GetWindowMode.
//
// ****************************************************************************

class PLOTTER_API avtBehavior
{
  public:
                               avtBehavior();
    virtual                   ~avtBehavior();

    void                       Add(vtkRenderer *);
    void                       Remove(vtkRenderer *);
 
    void                       SetInfo(const avtDataObjectInformation &);
    avtDataObjectInformation  &GetInfo(void) { return info; };

    void                       GetOriginalBounds(float [6]);
    void                       GetActualBounds(float [6]);
    void                       GetDataExtents(float &dmin, float &dmax);

    int                        GetDimension(void);
    WINDOW_MODE                GetWindowMode(void);
    bool                       RequiresReExecuteForQuery(void);

    void                       SetShiftFactor(float);
    float                      GetShiftFactor(void);

    void                       SetRenderOrder(int);
    void                       SetRenderOrder(RenderOrder);
    int                        GetRenderOrder(void);

    void                       SetLegend(avtLegend_p);
    avtLegend_p                GetLegend(void)       { return legend; };
    
  protected:
    avtDataObjectInformation   info;
    vtkRenderer               *renderer;
    avtLegend_p                legend;
    float                      shiftFactor;
    RenderOrder                renderOrder;
};


typedef ref_ptr<avtBehavior>  avtBehavior_p;


#endif


