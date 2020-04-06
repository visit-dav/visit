// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtTransparencyActor.h                           //
// ************************************************************************* //

#ifndef AVT_TRANSPARENCY_ACTOR_H
#define AVT_TRANSPARENCY_ACTOR_H

#include <plotter_exports.h>

#include <vector>
#include <map>
#include <cstring>

class     vtkActor;
class     vtkProperty;
class     vtkAppendPolyData;
class     vtkAxisDepthSort;
class     vtkCamera;
class     vtkDataSet;
class     vtkDataSetMapper;
class     vtkDepthSortPolyData;
class     vtkMatrix4x4;
class     vtkPolyData;
class     vtkPolyDataMapper;
class     vtkRenderer;
class     vtkParallelImageSpaceRedistributor;

class     ColorAttribute;


// ****************************************************************************
//  Class: avtTransparencyActor
//
//  Purpose:
//      Rendering transparent polygons correctly requires sorting them in front
//      to back order.  Since the AVT model is to keep groups of polygons
//      separated, this sorting would be impossible.  This class collects the
//      inputs from all of the plots and keeps them together so they can be
//      sorted.
//
//  Programmer: Hank Childs
//  Creation:   July 3, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 16:02:45 PDT 2002
//    Added the concept of visibility.  This is "different" from useActor
//    because the VisWindow has both concepts and they don't always agree.  At
//    this point it was easier to adopt its concept than re-write that code.
//
//    Hank Childs, Sun Jul 14 15:49:58 PDT 2002
//    Make use of one VTK module to do all the sorting.
//
//    Jeremy Meredith, Fri Jul 26 14:28:09 PDT 2002
//    Made perfect sorting a permanent mode instead of a one-frame mode.
//
//    Kathleen Bonnell, Wed Jul 16 16:39:02 PDT 2003
//    Added ScaleByVector method. 
//
//    Kathleen Bonnell, Wed Dec  3 16:42:38 PST 2003 
//    Added TransparenciesExist method.
//
//    Chris Wojtan, Fri Jun 25 15:15 PDT 2004
//    Added is2Dimensional bool and a function to set and retrieve its value
//
//    Hank Childs, Wed Sep  8 17:55:34 PDT 2004
//    No longer inline is2Dimensional.
//
//    Chris Wojtan, Wed Jul 7 10:27 PDT 2004
//    Added MyParallelFilter for improved parallel transparency
//
//    Jeremy Meredith, Thu Oct 21 12:09:00 PDT 2004
//    Renamed the parallel filter.
//
//    Tom Fogal, Sun May 24 19:34:25 MDT 2009
//    Added const to method.
//
//    Tom Fogal, Mon May 25 14:03:14 MDT 2009
//    Added caching for transparency calculation.
//
//    Hank Childs, Wed Feb 17 18:19:53 CST 2010
//    Add support for changes in specular lighting.
//
//    Burlen Loring, Fri Aug 14 11:33:43 PDT 2015
//    Added method to determine if a compositing order for
//    transparent geometry can be found and to compute it.
//    Added a method to set a flag to skip camera sort.
//
//    Burlen Loring, Wed Aug 19 13:41:17 PDT 2015
//    Factored some code into functions, added enum for
//    constructing sort operations mask which tells the
//    actor which type of sort to do. track actor mtime
//    so we can skip update. added filter for local
//    geometry sort that is used with ordered compositing.
//    make SetUpActor public.
//
//    Burlen Loring, Wed Aug 19 13:45:22 PDT 2015
//    Added method for controling use of display lists.
//
//    Kathleen Biagas, Tue Jul 12 13:41:21 PDT 2016
//    Add ability to store and replace visibility state of actors.
//
// ****************************************************************************

class PLOTTER_API avtTransparencyActor
{
  public:
                                     avtTransparencyActor();
    virtual                         ~avtTransparencyActor();

    int                              AddInput(std::vector<vtkDataSet *> &,
                                              std::vector<vtkDataSetMapper *>&,
                                              std::vector<vtkActor *> &);
    void                             ReplaceInput(int,
                                              std::vector<vtkDataSet *> &,
                                              std::vector<vtkDataSetMapper *>&,
                                              std::vector<vtkActor *> &);
    void                             ReplaceActorVisibility(int,
                                              std::vector<int> &);

    void                             TurnOffInput(int);
    void                             TurnOnInput(int);
    void                             RemoveInput(int);
    void                             InputWasModified(int, double=-1.0);
    void                             SetVisibility(int, bool);
    void                             VisibilityOff(void);
    void                             VisibilityOn(void);

    void                             PrepareForRender(vtkCamera *);
    bool                             UsePerfectSort(bool);

    bool                             TransparenciesExist(void);
    bool                             TransparenciesMightExist(void) const;
    void                             InvalidateTransparencyCache() {
                                         cachedTransparencies = false;
                                     }

    void                             AddToRenderer(vtkRenderer *);
    void                             RemoveFromRenderer(vtkRenderer *);
    void                             ScaleByVector(const double vec[3]);
    void                             SetSpecularProperties(bool flag,
                                           double coeff, double power,
                                           const ColorAttribute &color);

    void                             SuspendRendering() { renderingSuspended = true;  }
    void                             ResumeRendering()  { renderingSuspended = false; }

    bool                             GetIs2Dimensional() const
                                                    { return is2Dimensional; };
    void                             SetIs2Dimensional(bool val);

    enum                             {SORT_NONE=0x00, SORT_DEPTH=0x01, SORT_DISTRIBUTE=0x02};
    void                             SetSortOp(int op){ sortOp = op; }
    int                              GetSortOp() const { return sortOp; }

    virtual bool                     ComputeCompositingOrder(vtkCamera *cam, std::vector<int> &order);

    void                             SetUpActor();

  protected:
    void                             SyncProps();
    int                              SyncProps(vtkProperty *dest, vtkProperty *source);
    bool                             CameraChanged(vtkCamera *cam);
    vtkPolyData*                     GetAxisSortOutput(vtkCamera *cam);

  protected:
    std::vector<std::vector <vtkDataSet *> >         datasets;
    std::vector<std::vector <vtkDataSetMapper *> >   mappers;
    std::vector<std::vector <vtkActor *> >           actors;
    std::vector<std::vector <int> >                  actorVis;

    std::map<int,double>                             inputsOpacities;

    std::vector<std::vector <vtkPolyData *> >        preparedDataset;

    std::vector<bool>                                useActor;
    std::vector<bool>                                visibility;
    std::vector<bool>                                lastExecutionActorList;
    bool                                             inputModified;
    unsigned long long                               actorMTime;

    vtkAppendPolyData                               *appender;
    vtkParallelImageSpaceRedistributor              *distribute;
    vtkActor                                        *myActor;
    vtkPolyDataMapper                               *myMapper;

    vtkAxisDepthSort                                *axisSort;
    vtkDepthSortPolyData                            *distributeDepthSort;
    vtkDepthSortPolyData                            *depthSort;
    bool                                             usePerfectSort;
    bool                                             is2Dimensional;
    vtkMatrix4x4                                    *lastCamera;

    bool                                             renderingSuspended;
    bool                                             transparenciesExist;
    bool                                             cachedTransparencies;

    int                                              sortOp;

    void                                             PrepareDataset(size_t, size_t);
    void                                             DetermineTransparencies();
};
#endif
