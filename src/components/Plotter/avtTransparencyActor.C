/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtTransparencyActor.C                          //
// ************************************************************************* //

#include <avtTransparencyActor.h>

#include <float.h>

#include <avtParallel.h>
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkAxisDepthSort.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkDepthSortPolyData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkGeometryFilter.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkParallelImageSpaceRedistributor.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkVisItPolyDataNormals.h>

#include <DebugStream.h>
#include <BadIndexException.h>
#include <TimingsManager.h>

using     std::vector;


// ****************************************************************************
//  Method: avtTransparencyActor constructor
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 17:51:34 PDT 2002
//    Initialize perfectSort.
//
//    Hank Childs, Sun Jul 14 15:49:58 PDT 2002
//    Use new VTK module to do all six sorts.
//
//    Jeremy Meredith, Fri Jul 26 14:30:40 PDT 2002
//    Default perfect sorting to true.
//
//    Chris Wojtan, Wed Jul 7 10:17 PDT 2004
//    Added parallel support
//
//    Jeremy Meredith, Thu Oct 21 12:16:38 PDT 2004
//    Enhanced parallel support.  Made it use avtParallel code instead of
//    ifdefs so we didn't have to build a parallel version of this library.
//
//    Brad Whitlock, Mon Nov 1 15:33:56 PST 2004
//    Passed rank and size into the image space redistributor so that
//    library could be built on MacOS X.
//
//    Kathleen Bonnell, Tue Jan 11 16:06:33 PST 2005 
//    Initialize is2Dimensional.
//
// ****************************************************************************

avtTransparencyActor::avtTransparencyActor()
{
    appender = vtkAppendPolyData::New();
    myMapper = vtkPolyDataMapper::New();
    myMapper->ImmediateModeRenderingOn();
    myActor  = vtkActor::New();
    myActor->SetMapper(myMapper);
    
    parallelFilter = vtkParallelImageSpaceRedistributor::New();
    parallelFilter->SetRankAndSize(PAR_Rank(), PAR_Size());

    //
    // Tell the mapper that we are going to set up an RGBA field ourselves.
    //
    myMapper->SetColorModeToDefault();
    myMapper->ColorByArrayComponent("Colors", 0);
    myMapper->SetScalarModeToUsePointFieldData();

    axisSort = vtkAxisDepthSort::New();
    axisSort->SetInput(appender->GetOutput());

    perfectSort = vtkDepthSortPolyData::New();
    perfectSort->SetDepthSortModeToBoundsCenter();

    if (PAR_Size() > 1)
    {
        parallelFilter->SetInput(appender->GetOutput());
        perfectSort->SetInput(parallelFilter->GetOutput());
    }
    else
    {
        perfectSort->SetInput(appender->GetOutput());
    }

    usePerfectSort = true;
    lastCamera = vtkMatrix4x4::New();

    inputModified = true;
    renderingSuspended = false;
    is2Dimensional = false;
}


// ****************************************************************************
//  Method: avtTransparencyActor destructor
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 17:51:34 PDT 2002
//    Delete perfectSort.
//
//    Hank Childs, Sun Jul 14 15:49:58 PDT 2002
//    Use new VTK module to do all six sorts.
//
//    Chris Wojtan, Wed Jul 7 10:17 PDT 2004
//    Added parallel support
//
// ****************************************************************************

avtTransparencyActor::~avtTransparencyActor()
{
    if (appender != NULL)
    {
        appender->Delete();
        appender = NULL;
    }
    if (myActor != NULL)
    {
        myActor->Delete();
        myActor = NULL;
    }
    if (myMapper != NULL)
    {
        myMapper->Delete();
        myMapper = NULL;
    }
    if (axisSort != NULL)
    {
        axisSort->Delete();
        axisSort = NULL;
    }
    if (perfectSort != NULL)
    {
        perfectSort->Delete();
        perfectSort = NULL;
    }
    if (lastCamera != NULL)
    {
        lastCamera->Delete();
        lastCamera = NULL;
    }

    if(parallelFilter != NULL)
    {
        parallelFilter->Delete();
        parallelFilter = NULL;
    }
}


// ****************************************************************************
//  Method: avtTransparencyActor::InputWasModified
//
//  Purpose:
//      This is a hint to the transparency actor that one of its inputs was
//      modified.  It will not look through the inputs when rendering each
//      frame, so this is the mechanism to indicate that it should recalculate.
//
//  Arguments:
//      <unused>   The index of the input that was modified.
//
//  Programmer:  Hank Childs
//  Creation:    July 8, 2002
//
//  Modifications:
//
//    Mark C. Miller, Thu Jan 20 22:27:39 PST 2005
//    Added opacity arg. Added code to stick opacity value in map
//
// ****************************************************************************

void
avtTransparencyActor::InputWasModified(int transparencyIndex, double opacity)
{
    inputModified = true;
    if (opacity != -1.0)
        inputsOpacities[transparencyIndex] = opacity;
}


// ****************************************************************************
//  Method: avtTransparencyActor::UsePerfectSort
//
//  Purpose:
//      Tells the transparency actor to use the best possible sort for the
//      next frame.
//
//  Arguments:
//      perfect    True if future updates should sort perfectly
//
//  Programmer:  Hank Childs
//  Creation:    July 11, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Jul 26 14:29:06 PDT 2002
//    Made it take a bool, since we can turn it on or off as a permanent
//    mode of operation now.
//
//    Jeremy Meredith, Thu Oct 21 12:18:23 PDT 2004
//    Call TransparenciesExist now because the logic has become more complex.
//    Specifically, it may have needed an update, and it needed unification
//    in parallel.
//
// ****************************************************************************

bool
avtTransparencyActor::UsePerfectSort(bool perfect)
{
    usePerfectSort = perfect;
    return TransparenciesExist();
}


// ****************************************************************************
//  Method: avtTransparencyActor::AddInput
//
//  Purpose:
//      Adds the VTK constructs for a plot.
//
//  Arguments:
//      d       A vector of datasets.
//      m       A vector of mappers.
//      a       A vector of actors.
//
//  Returns:    The index of the newly added input.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 16:02:45 PDT 2002
//    Add visibility.
//
//    Mark C. Miller, Thu Jan 20 22:27:39 PST 2005
//    Initialized inputsOpacities
//
// ****************************************************************************

int
avtTransparencyActor::AddInput(vector<vtkDataSet *> &d, 
                          vector<vtkDataSetMapper *> &m, vector<vtkActor *> &a)
{
    int  i;

    int index = datasets.size();
    datasets.push_back(d);
    mappers.push_back(m);
    actors.push_back(a);
    useActor.push_back(true);
    visibility.push_back(true);

    int size = d.size();
    vector<vtkPolyData *> pd;
    for (i = 0 ; i < size ; i++)
    {
        pd.push_back(NULL);
    }
    preparedDataset.push_back(pd);

    inputModified = true;

    inputsOpacities[index] = 1.0;

    return index;
}


// ****************************************************************************
//  Method: avtTransparencyActor::ReplaceInput
//
//  Purpose:
//      Allows you to replace the input for the i'th input with a new set of
//      VTK constructs.
//
//  Arguments:
//      ind     The index of these VTK constructs.
//      d       A vector of datasets.
//      m       A vector of mappers.
//      a       A vector of actors.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
// ****************************************************************************

void
avtTransparencyActor::ReplaceInput(int ind, vector<vtkDataSet *> &d, 
                          vector<vtkDataSetMapper *> &m, vector<vtkActor *> &a)
{
    int   i;

    if (ind >= datasets.size() || ind < 0)
    {
        EXCEPTION2(BadIndexException, ind, datasets.size());
    }

    datasets[ind] = d;
    mappers[ind]  = m;
    actors[ind]   = a;

    for (i = 0 ; i < preparedDataset[ind].size() ; i++)
    {
        if (preparedDataset[ind][i] != NULL)
        {
            preparedDataset[ind][i]->Delete();
            preparedDataset[ind][i] = NULL;
        }
    }

    inputModified = true;
}


// ****************************************************************************
//  Method: avtTransparencyActor::RemoveInput
//
//  Purpose:
//      Removes the i'th input.  This means it will free up any memory
//      associated with the index.
//
//  Arguments:
//      ind     The index of the VTK constructs.
//
//  Programmer: Hank Childs
//  Creation:   June 25, 2003
//
//  Modifications:
//
//    Mark C. Miller, Thu Jan 20 22:27:39 PST 2005
//    Set inputsOpacities for associated input to zero
//
// ****************************************************************************

void
avtTransparencyActor::RemoveInput(int ind)
{
    if (ind >= useActor.size() || ind < 0)
    {
        EXCEPTION2(BadIndexException, ind, useActor.size());
    }

    for (int i = 0 ; i < preparedDataset[ind].size() ; i++)
    {
        if (preparedDataset[ind][i] != NULL)
        {
            preparedDataset[ind][i]->Delete();
            preparedDataset[ind][i] = NULL;
        }
    }

    inputsOpacities[ind] = 0.0;

    useActor[ind] = false;
}


// ****************************************************************************
//  Method: avtTransparencyActor::TurnOffInput
//
//  Purpose:
//      Turns off the i'th input.  This means it will not render.
//
//  Arguments:
//      ind     The index of the VTK constructs.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
// ****************************************************************************

void
avtTransparencyActor::TurnOffInput(int ind)
{
    if (ind >= useActor.size() || ind < 0)
    {
        EXCEPTION2(BadIndexException, ind, useActor.size());
    }

    useActor[ind] = false;
}


// ****************************************************************************
//  Method: avtTransparencyActor::TurnOnInput
//
//  Purpose:
//      Turns on the i'th input.  This means it will render.
//
//  Arguments:
//      ind     The index of the VTK constructs.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
// ****************************************************************************

void
avtTransparencyActor::TurnOnInput(int ind)
{
    if (ind >= useActor.size() || ind < 0)
    {
        EXCEPTION2(BadIndexException, ind, useActor.size());
    }

    useActor[ind] = true;
}


// ****************************************************************************
//  Method: avtTransparencyActor::SetVisibility
//
//  Purpose:
//      Sets the visibility of a certain actor.
//
//  Arguments:
//      ind     The index of the VTK constructs.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2002
//
// ****************************************************************************

void
avtTransparencyActor::SetVisibility(int ind, bool val)
{
    if (ind >= useActor.size() || ind < 0)
    {
        EXCEPTION2(BadIndexException, ind, useActor.size());
    }

    visibility[ind] = val;
}


// ****************************************************************************
//  Method: avtTransparencyActor::VisibilityOff
//
//  Purpose:
//      Tells the transparency actor not to render (this is used for navigate
//      bounding box mode).
//
//  Programmer: Hank Childs
//  Creation:   July 19, 2002
//
// ****************************************************************************

void
avtTransparencyActor::VisibilityOff(void)
{
    myActor->SetVisibility(0);
}


// ****************************************************************************
//  Method: avtTransparencyActor::VisibilityOn
//
//  Purpose:
//      Tells the transparency actor that it should render (this is used for 
//      navigate bounding box mode).
//
//  Programmer: Hank Childs
//  Creation:   July 19, 2002
//
//  Modifications:
//
//    Hank Childs, Wed May  7 13:50:00 PDT 2003
//    Only turn on the actor conditionally.  This will prevent an error msg.
//
//    Jeremy Meredith, Thu Oct 21 12:18:23 PDT 2004
//    Call TransparenciesExist now because the logic has become more complex.
//    Specifically, it may have needed an update, and it needed unification
//    in parallel.
//
// ****************************************************************************

void
avtTransparencyActor::VisibilityOn(void)
{
    if (TransparenciesExist())
        myActor->SetVisibility(1);
}


// ****************************************************************************
//  Method: avtTransparencyActor::PrepareForRender
//
//  Purpose:
//      Gets ready for a render -- this means making sure that the poly data is
//      all set and that we are using the correct sort.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 17:51:34 PDT 2002
//    Allow for using a better sort in some instances.
//
//    Hank Childs, Sun Jul 14 15:49:58 PDT 2002
//    Use new VTK module to do all six axis sorts.
//
//    Jeremy Meredith, Fri Jul 26 14:29:50 PDT 2002
//    No longer disable perfect sorting.  Our clients will disable it
//    when they determine it is okay.
//
//    Chris Wojtan, Fri Jun 25 16:13 PDT 2004
//    Ignore sorting and other unnecessary computation if the data is
//    2-dimensional
//
//    Chris Wojtan, Thurs Jul 8 16:18 PDT 2004
//    Force recalculation if parallel rendering is enabled
//
//    Jeremy Meredith, Thu Oct 21 15:05:29 PDT 2004
//    Use PAR_Size instead of an ifdef so we don't have to build a parallel
//    library.  Disable axis sort in parallel because it's only slowing us
//    down (never use it).  
//
// ****************************************************************************

void
avtTransparencyActor::PrepareForRender(vtkCamera *cam)
{
    // if this is a 2D plot, we don't need to sort anything
    if (is2Dimensional)
    {
        return;
    }

    //
    // Determine if our poly-data input is up-to-date.
    //
    bool needToRecalculate = inputModified;
    if (useActor.size() != lastExecutionActorList.size())
    {
        needToRecalculate = true;
    }
    else
    {
        for (int i = 0 ; i < useActor.size() ; i++)
        {
            if (useActor[i] != lastExecutionActorList[i])
            {
                needToRecalculate = true;
            }
        }
    }

    // If parallel transparency is enabled, we should recalculate the
    // ordering each time to ensure that each processor handles its
    // correct group of data.
    if (PAR_Size() > 1)
        needToRecalculate = true;

    //
    // The routine to set up our actual big actor is *long* -- push it off to
    // a subroutine.
    //
    if (needToRecalculate)
    {
        SetUpActor();
    }

    // If we are in parallel, then an axis sort won't help us because it
    // cannot also sort across processors, so we always need a perfect sort.
    if (PAR_Size() > 1 || usePerfectSort)
    {
        perfectSort->SetCamera(cam);
        myMapper->SetInput(perfectSort->GetOutput());
        vtkMatrix4x4 *mat = cam->GetViewTransformMatrix();
        lastCamera->DeepCopy(mat);
    }
    else
    {
        vtkMatrix4x4 *mat = cam->GetViewTransformMatrix();
        bool equal = true;
        for (int i = 0 ; i < 16 ; i++)
        {
            if (mat->Element[i/4][i%4] != lastCamera->Element[i/4][i%4])
            {
               equal = false;
               break;
            }
        }

        if (!equal)
        {
            //
            // Based on what the direction of project is, set up the best 
            // sorting.
            //
            double proj[3];
            cam->GetDirectionOfProjection(proj);
            int biggest = 0;
            if (fabs(proj[biggest]) < fabs(proj[1]))
                biggest = 1;
            if (fabs(proj[biggest]) < fabs(proj[2]))
                biggest = 2;
            biggest += 1;
            if (proj[biggest-1] < 0.)
            {
                biggest *= -1;
            }
            switch (biggest)
            {
              case -3:
                myMapper->SetInput(axisSort->GetMinusZOutput());
                break;
              case -2:
                myMapper->SetInput(axisSort->GetMinusYOutput());
                break;
              case -1:
                myMapper->SetInput(axisSort->GetMinusXOutput());
                break;
              case 1:
                myMapper->SetInput(axisSort->GetPlusXOutput());
                break;
              case 2:
                myMapper->SetInput(axisSort->GetPlusYOutput());
                break;
              case 3:
                myMapper->SetInput(axisSort->GetPlusZOutput());
                break;
            }
        }
    }
}


// ****************************************************************************
//  Method: avtTransparencyActor::AddToRenderer
//
//  Purpose:
//      Adds the VTK actor for this AVT actor to the specified renderer.
//
//  Arguments:
//      ren     The renderer to add this actor to.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//    Chris Wojtan, Wed Jul 7 10:01 PDT 2004
//    Pass renderer into parallel transparency filter.
//
//    Jeremy Meredith, Thu Oct 21 15:08:16 PDT 2004
//    Use PAR_Size instead of an ifdef.
//
// ****************************************************************************

void
avtTransparencyActor::AddToRenderer(vtkRenderer *ren)
{
    ren->AddActor(myActor);
    if (PAR_Size() > 1)
        parallelFilter->SetRenderer(ren);
}


// ****************************************************************************
//  Method: avtTransparencyActor::RemoveFromRenderer
//
//  Purpose:
//      Removes the VTK actor for this AVT actor from the specified renderer.
//
//  Arguments:
//      ren     The renderer to remove this actor from.
//
//  Programmer: Hank Childs
//  Creation:   July 9, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed May 17 15:08:39 PDT 2006
//    GetProps->RemoveItem has been deprecated, use GetViewProps.
//
// ****************************************************************************

void
avtTransparencyActor::RemoveFromRenderer(vtkRenderer *ren)
{
    //
    // This is supposed to approximate the RemoveActor call of
    // vtkRenderer.  That call also tells the actor to release its
    // graphics resources, which does not work well for us, since
    // we remove the actors every time we add new plots (the viewer
    // does a ClearPlots) and also when the vis window re-orders the
    // actors.
    //
    // THIS IS A MAINTENANCE ISSUE.  This routine should be the same
    // as vtkRenderer::RemoveActor, but does not call
    // ReleaseGraphicsResources (which is actually called indirectly
    // through vtkViewport::RemoveProp).
    //
    //ren->RemoveActor(actors[i]);
    //
    ren->GetActors()->RemoveItem(myActor);
    myActor->RemoveConsumer(ren);
    ren->GetViewProps()->RemoveItem(myActor);
}


// ****************************************************************************
//  Method: avtTransparency::SetUpActor
//
//  Purpose:
//      This is the big routine to pull all the poly data together into one
//      poly data that can then be sorted.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 16:07:53 PDT 2002
//    Incorporate the idea of visibility.  Also remove actor from the scene if
//    it has nothing to draw.
//
//    Hank Childs, Sat Jul 13 13:26:10 PDT 2002
//    Do a better job of copying over the input actors' attributes.
//
//    Hank Childs, Sun Jul 14 15:49:58 PDT 2002
//    Use new VTK module to do all six axis sorts.
//
//    Jeremy Meredith, Thu Oct 21 15:11:19 PDT 2004
//    Force a re-execution of the appender so all processors do the
//    same thing in parallel.  Call TransparenciesExist now because
//    the logic has become more complex.  Specifically, it may have
//    needed an update, and it needed unification in parallel.  Honor
//    the suspension of transparent rendering for two-pass mode.
//
//    Kathleen Bonnell, Wed May 17 15:08:39 PDT 2006
///   Ensure that appender has non-NULL input (can have empty input, not NULL).
// ****************************************************************************

void
avtTransparencyActor::SetUpActor(void)
{
    appender->RemoveAllInputs();
    int numActors = datasets.size();
    vtkActor *repActor = NULL;
    bool addedInput = false;
    for (int i = 0 ; i < numActors ; i++)
    {
        if (useActor[i] && visibility[i] == true)
        {
            int numParts = datasets[i].size();
            for (int j = 0 ; j < numParts ; j++)
            {
                PrepareDataset(i, j);
                if (preparedDataset[i][j] != NULL)
                {
                    addedInput = true;
                    appender->AddInput(preparedDataset[i][j]);
                    repActor = actors[i][j];
                }
            }
        }
    }

    // 
    //  VTK pipeline now requires its filters to have non-null inputs.
    // 
    if (!addedInput)
    {
        // use empty input? 
        vtkPolyData *pd = vtkPolyData::New();
        appender->AddInput(pd);
        pd->Delete();
    }

    // Force the appender to update; this is needed in parallel SR mode
    // because all processors need to re-execute the pipeline, and if
    // not all processors have data then they might not re-execute.
    // See VisIt00005467.
    appender->Modified();

    //
    // If we don't have anything to render, don't have our actor draw.
    //
    if (!TransparenciesExist())
    {
        myActor->SetVisibility(0);
    }
    else
    {
        //
        // Just because we have geometry doesn't mean we will be
        // rendering it, because in two-pass parallel SR mode we
        // must disable transparent geometry for the first pass.
        //
        myActor->SetVisibility(renderingSuspended ? 0 : 1);

        //
        // If their actor has some special properties set up, try to preserve
        // those.  Note that this is confusing logic if there are actors with
        // different properties -- we will just be taking them for one of them.
        //
        if (repActor != NULL)
        {
            vtkProperty *myProp  = myActor->GetProperty();
            vtkProperty *repProp = repActor->GetProperty();
            myProp->SetInterpolation(repProp->GetInterpolation());
            myProp->SetRepresentation(repProp->GetRepresentation());
            // If we copy over lighting, odd things happen.  This is in the
            // system as HYPer4112.
            //myProp->SetAmbient(repProp->GetAmbient());
            //myProp->SetDiffuse(repProp->GetDiffuse());
            myProp->SetSpecular(repProp->GetSpecular());
            myProp->SetSpecularPower(repProp->GetSpecularPower());
            myProp->SetAmbientColor(repProp->GetAmbientColor());
            myProp->SetDiffuseColor(repProp->GetDiffuseColor());
            myProp->SetSpecularColor(repProp->GetSpecularColor());
            myProp->SetEdgeVisibility(repProp->GetEdgeVisibility());
            myProp->SetLineWidth(repProp->GetLineWidth());
            myProp->SetLineStipplePattern(repProp->GetLineStipplePattern());
            myProp->SetLineStippleRepeatFactor(
                                        repProp->GetLineStippleRepeatFactor());
            myProp->SetPointSize(repProp->GetPointSize());
            myProp->SetBackfaceCulling(repProp->GetBackfaceCulling());
            myProp->SetFrontfaceCulling(repProp->GetFrontfaceCulling());
        }

        //
        // Sort all the data from all six axis directions.  This will prevent
        // pauses as we cross between our view directions.  Note that if we
        // are in parallel, then we must be doing SR mode transparency and
        // and Axis sort doesn't do us any good since it sorts only within
        // a processor.
        //
        if (PAR_Size() > 1)
        {
            debug4 << "Skipping axis sorting because we are in parallel.\n";
        }
        else
        {
            int sorting = visitTimer->StartTimer();
            axisSort->Update();
            visitTimer->StopTimer(sorting,"Sorting triangles for transparency");
            visitTimer->DumpTimings();
        }
    }

    //
    // Maintain our internal state for next time.
    //
    lastExecutionActorList = useActor;
}


// ****************************************************************************
//  Method: avtTransparencyActor::PrepareDataset
//
//  Purpose:
//      This routine will take a dataset and map colors onto it.  This will
//      allow it to be merged later.
//
//  Arguments:
//      input       The input index.
//      subinput    The piece of the input.
//
//  Programmer:   Hank Childs
//  Creation:     July 8, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Nov 20 14:29:21 PST 2002
//    Re-order setting of normals.  They were getting removed by subsequent
//    calls.
//
//    Hank Childs, Thu May  6 08:37:25 PDT 2004
//    Do a better job of handling normals for cell-based normals.  This is more
//    important because the poly data mapper no longer calculates them for us.
//
//    Hank Childs, Fri Dec 29 09:53:13 PST 2006
//    Accomodate situations where rectilinear and curvilinear grids are
//    getting shipped down as is (i.e. not poly data).  This means we must
//    add normals and remove ghost data.
//
//    Hank Childs, Mon Feb 12 14:02:16 PST 2007
//    Make sure that the colors that get generated are directly related
//    to the poly data we are rendering.  Some mappers are linked to 
//    vtkRectilinearGrids or vtkStructuredGrids.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//    All rectilinear grids that made it here get converted to polydata now --
//    the difference with these is that they need an extra transform applied.
//    These should all be 2D grids (i.e. external polygons) anyway, so the
//    performance penalty is small.
//
// ****************************************************************************

void
avtTransparencyActor::PrepareDataset(int input, int subinput)
{
    inputModified = false;

    vtkDataSet       *in_ds  = datasets[input][subinput];
    vtkActor         *actor  = actors[input][subinput];
    vtkDataSetMapper *mapper = mappers[input][subinput];

    //
    // If we don't have valid input, there isn't a lot we can do.
    //
    if (in_ds == NULL || actor == NULL || mapper == NULL)
    {
        preparedDataset[input][subinput] = NULL; // should be already, but...
        return;
    }

    //
    // Check to see if we have already done all the necessary calculations.
    //
    vtkDataSet *ds = preparedDataset[input][subinput];
    if (ds != NULL && in_ds->GetMTime() < ds->GetMTime() &&
        actor->GetMTime() < ds->GetMTime() &&
        mapper->GetMTime() < ds->GetMTime())
    {
        //
        // Our last preparation of this dataset is still good.  No need to redo
        // the same work twice.
        //
        return;
    }

    //
    // If we have made it this far, we need to recalculate.  Go ahead and clean
    // up from any previous executions.
    //
    if (preparedDataset[input][subinput] != NULL)
    {
        preparedDataset[input][subinput]->Delete();
        preparedDataset[input][subinput] = NULL;
    }
        
    //
    // If this actor is fully opaque, then we are not needed.
    //
    if (actor->GetProperty()->GetOpacity() >= 1.)
    {
        actor->SetVisibility(1);
        return;
    }
    else
    {
        //
        // We will be drawing this, so turn theirs off.
        //
        actor->SetVisibility(0);
    }

    //
    // If the actor is fully transparent, there's no need for us to process it.
    //
    if (actor->GetProperty()->GetOpacity() <= 0.)
    {
        return;
    }

    //
    // Okay, the real recalculation work starts here.  Start by confirming
    // that we are actually dealing with polydata.
    //
    vtkGeometryFilter *gf = vtkGeometryFilter::New();
    vtkDataSetRemoveGhostCells *ghost_filter=vtkDataSetRemoveGhostCells::New();
    vtkVisItPolyDataNormals *normals = vtkVisItPolyDataNormals::New();
    vtkTransformFilter *xform_filter = vtkTransformFilter::New();
    vtkTransform *xform = vtkTransform::New();
    vtkPolyData *pd = NULL;
    if (in_ds->GetDataObjectType() == VTK_POLY_DATA)
    {
        pd = (vtkPolyData *) in_ds;
    }
    else if (in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        gf->SetInput(in_ds);
        ghost_filter->SetInput(gf->GetOutput());
        ghost_filter->Update();
        pd = (vtkPolyData *) ghost_filter->GetOutput();
    }
    else if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        gf->SetInput(in_ds);
        if (mapper->GetScalarVisibility() != 0 &&
            in_ds->GetPointData()->GetScalars() == NULL &&
            in_ds->GetCellData()->GetScalars() != NULL)
            normals->SetNormalTypeToCell();
        normals->SetInput(gf->GetOutput());
        ghost_filter->SetInput(normals->GetOutput());
        // Apply any inherent rectilinear grid transforms from the input.
        if (in_ds->GetFieldData()->GetArray("RectilinearGridTransform"))
        {
            vtkDoubleArray *matrix = (vtkDoubleArray*)in_ds->GetFieldData()->
                                          GetArray("RectilinearGridTransform");
            xform->SetMatrix(matrix->GetPointer(0));
            xform_filter->SetInput(ghost_filter->GetOutput());
            xform_filter->SetTransform(xform);
            xform_filter->Update();
            pd = (vtkPolyData *) xform_filter->GetOutput();
        }
        else
        {
            ghost_filter->Update();
            pd = (vtkPolyData *) ghost_filter->GetOutput();
        }
    }
    else
    {
        gf->SetInput(in_ds);
        pd = gf->GetOutput();
        pd->Update();
    }
    mapper->SetInput(pd);

    //
    // Create the output dataset that we will be creating an RGBA field for.
    //
    vtkPolyData *prepDS = vtkPolyData::New();
    if (mapper->GetScalarVisibility() == 0)
    {
        //
        // The color and the opacity of the whole actor will be applied to
        // every triangle of this poly data.
        //
        prepDS->CopyStructure(pd);
        int npts = prepDS->GetNumberOfPoints();
        vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
        colors->SetNumberOfComponents(4);
        colors->SetNumberOfTuples(npts);
        colors->SetName("Colors");
        double *color   = actor->GetProperty()->GetColor();
        double  opacity = actor->GetProperty()->GetOpacity();
        unsigned char rgba[4];
        rgba[0] = (unsigned char) ((float)color[0] * 255);
        rgba[1] = (unsigned char) ((float)color[1] * 255);
        rgba[2] = (unsigned char) ((float)color[2] * 255);
        rgba[3] = (unsigned char) ((float)opacity * 255);
        unsigned char *ptr = (unsigned char *) colors->GetVoidPointer(0);
        for (int i = 0 ; i < npts ; i++)
        {
            ptr[4*i]   = rgba[0];
            ptr[4*i+1] = rgba[1];
            ptr[4*i+2] = rgba[2];
            ptr[4*i+3] = rgba[3];
        }
        prepDS->GetPointData()->AddArray(colors);
        colors->Delete();
        if (pd->GetPointData()->GetNormals() != NULL)
        {
            prepDS->GetPointData()->SetNormals(
                                             pd->GetPointData()->GetNormals());
        }
    }
    else
    {
        if (pd->GetPointData()->GetScalars() != NULL)
        {
            //
            // Prepare our own buffer to store the colors in.
            //
            prepDS->CopyStructure(pd);
            int npts = prepDS->GetNumberOfPoints();
            vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
            colors->SetNumberOfComponents(4);
            colors->SetNumberOfTuples(npts);
            colors->SetName("Colors");
            unsigned char *ptr = (unsigned char *) colors->GetVoidPointer(0);

            //
            // Now let the mapper create the buffer of unsigned chars that it
            // would have created if we were to let it do the actual mapping.
            //
            double opacity = actor->GetProperty()->GetOpacity();
            unsigned char *buff = 
              (unsigned char *) mapper->MapScalars(opacity)->GetVoidPointer(0);

            //
            // Now copy over the buffer and store it back with the dataset.
            //
            for (int i = 0 ; i < 4*npts ; i++)
            {
                ptr[i] = buff[i];
            }
            prepDS->GetPointData()->AddArray(colors);
            colors->Delete();
            if (pd->GetPointData()->GetNormals() != NULL)
            {
                prepDS->GetPointData()->SetNormals(
                                             pd->GetPointData()->GetNormals());
            }
        }
        else if (pd->GetCellData()->GetScalars() != NULL)
        {
            //
            // This is a sad state -- we have decided that the poly data will
            // all have point data.  But we have cell data.  So we are going to
            // dummy up a dataset that has point data, but appears to have cell
            // data (by replicating a lot of points).
            //
            int   i, j;

            //
            // Start off by replicating the connectivity.  This will speculate
            // on a point list that we will build later.
            //
            prepDS->Allocate(pd);
            int ncells = pd->GetNumberOfCells();
            vtkIdType  *cellPts = NULL;
            vtkIdType   myCellPts[100];
            vtkIdType   npts = 0;
            vector<int> ptIds;
            vector<int> cellIds;
            pd->BuildCells();
            int count = 0;
            for (i = 0 ; i < ncells ; i++)
            {
                pd->GetCellPoints(i, npts, cellPts);
                if (cellPts == NULL || npts == 0)
                {
                    continue;
                }
                for (j = 0 ; j < npts ; j++)
                {
                    ptIds.push_back(cellPts[j]);
                    cellIds.push_back(i);
                    myCellPts[j] = count;
                    count++;
                }
                
                prepDS->InsertNextCell(pd->GetCellType(i), npts, myCellPts);
            }

            //
            // Create the point list.  This will have a lot of repeated points.
            //
            vtkPoints *pts = vtkPoints::New();
            vtkPoints *in_pts = pd->GetPoints();
            pts->SetNumberOfPoints(count);
            for (i = 0 ; i < count ; i++)
            {
                double pt[3];
                in_pts->GetPoint(ptIds[i], pt);
                pts->SetPoint(i, pt);
            }
            prepDS->SetPoints(pts);
            pts->Delete();

            //
            // Now convert our cell data into point data that will appear as
            // cell data (through lots of replication of points).
            //
            vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
            colors->SetNumberOfComponents(4);
            colors->SetNumberOfTuples(count);
            colors->SetName("Colors");
            unsigned char *ptr = (unsigned char *) colors->GetVoidPointer(0);
            double opacity = actor->GetProperty()->GetOpacity();
            unsigned char *buff = 
              (unsigned char *) mapper->MapScalars(opacity)->GetVoidPointer(0);
            for (i = 0 ; i < count ; i++)
            {
                ptr[4*i]   = buff[4*cellIds[i]];
                ptr[4*i+1] = buff[4*cellIds[i]+1];
                ptr[4*i+2] = buff[4*cellIds[i]+2];
                ptr[4*i+3] = buff[4*cellIds[i]+3];
            }
            prepDS->GetPointData()->AddArray(colors);
            colors->Delete();
            vtkDataArray *cell_normals = pd->GetCellData()->GetNormals();
            if (cell_normals != NULL)
            {
                const float *cn = (float *) cell_normals->GetVoidPointer(0);
                vtkFloatArray *newNormals;
                newNormals = vtkFloatArray::New();
                newNormals->SetNumberOfComponents(3);
                newNormals->SetNumberOfTuples(count);
                newNormals->SetName("Normals");
                float *newNormalPtr = (float*)newNormals->GetVoidPointer(0);
                for (i = 0 ; i < count ; i++)
                {
                    newNormalPtr[i*3+0] = cn[cellIds[i]*3];
                    newNormalPtr[i*3+1] = cn[cellIds[i]*3+1];
                    newNormalPtr[i*3+2] = cn[cellIds[i]*3+2];
                }
                prepDS->GetPointData()->SetNormals(newNormals);
                newNormals->Delete();
            }
        }
    }

    //
    // Now that we are done preparing the dataset, let's store it off so we
    // can save the results.
    //
    preparedDataset[input][subinput] = prepDS;
    preparedDataset[input][subinput]->Register(NULL);

    //
    // Clean up memory.
    //
    gf->Delete();
    prepDS->Delete();
    ghost_filter->Delete();
    normals->Delete();
    xform_filter->Delete();
    xform->Delete();
}


// ****************************************************************************
//  Method: avtTransparencyActor::ScaleByVector
//
//  Purpose:
//      Scales the actor by a vector. 
//
//  Arguments:
//      vec    The vector to scale by.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 16, 2003 
//
// ****************************************************************************

void
avtTransparencyActor::ScaleByVector(const double vec[3])
{
    myActor->SetScale(vec[0], vec[1], vec[2]);
}


// ****************************************************************************
//  Method: avtTransparencyActor::TransparenciesExist
//
//  Purpose:
//    Returns true if this actor is active (appender has inputs).
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 3, 2003
//
//  Modifications:
//    Jeremy Meredith and Hank Childs, Thu Oct 21 15:27:44 PDT 2004
//    This method was not getting updated soon enough for calls to
//    it to return the correct value.  We took the key pieces
//    from PrepareDataset to get the right answer here, and I
//    added code to enforce that all processors have the same answer.
//
// ****************************************************************************

bool
avtTransparencyActor::TransparenciesExist()
{
    bool has_stuff = false;
    int numActors = datasets.size();
    for (int i = 0 ; i < numActors && !has_stuff ; i++)
    {
        if (useActor[i] && visibility[i] == true)
        {
            int numParts = datasets[i].size();
            for (int j = 0 ; j < numParts && !has_stuff ; j++)
            {
                vtkDataSet       *in_ds  = datasets[i][j];
                vtkActor         *actor  = actors[i][j];
                vtkDataSetMapper *mapper = mappers[i][j];
                if (in_ds && actor && mapper &&
                    actor->GetProperty()->GetOpacity() > 0. &&
                    actor->GetProperty()->GetOpacity() < 1.)
                {
                    has_stuff = true;
                }
            }
        }
    }

    // We need all processors to agree!
    has_stuff = UnifyMaximumValue(has_stuff);

    return has_stuff;
}


// ****************************************************************************
//  Method: avtTransparencyActor::TransparenciesMightExist
//
//  Purpose:
//    Returns true if their might be some transparency on some processor 
//
//  Programmer: Mark C. Miller 
//  Creation:   January 20, 2005
//
// ****************************************************************************

bool
avtTransparencyActor::TransparenciesMightExist() const
{
    std::map<int,double>::const_iterator it;
    bool has_transparency = false;
    for (it = inputsOpacities.begin(); it != inputsOpacities.end(); it++)
    {
        if (it->second > 0.0 && it->second < 1.0)
        {
            has_transparency = true;
            break;
        }
    }
    return has_transparency;
}

// ****************************************************************************
//  Method: avtTransparencyActor::SetIs2Dimensional
//
//  Purpose:
//      Sets whether or not the actor is 2D.
//
//  Programmer: Hank CHilds
//  Creation:   September 8, 2004
//
// ****************************************************************************

void
avtTransparencyActor::SetIs2Dimensional(bool val)
{
    if (is2Dimensional != val)
    {
        //
        // If we have any inputs left over, remove them.  This is important,
        // because the 2D execution flow will bail out before removing them,
        // which can potentially leave old 3D datasets in the same window
        // as the 2D dataset.
        //
        if (appender != NULL)
        {
            appender->RemoveAllInputs();

            // Note: with lineouts in SR mode, it can sometimes hang
            // because one processor decides to re-execute despite having
            // no inputs.  Let's just force what we think should be
            // happening anyway, and force all processors to re-execute
            // so they all go through the parallelFilter.  This is similar
            // to VisIt00005467.
            appender->Modified();
        }
    }

    is2Dimensional = val;
}


