// ************************************************************************* //
//                           avtTransparencyActor.C                          //
// ************************************************************************* //

#include <avtTransparencyActor.h>

#include <float.h>

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkAxisDepthSort.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkDepthSortPolyData.h>
#include <vtkGeometryFilter.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

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
// ****************************************************************************

avtTransparencyActor::avtTransparencyActor()
{
    appender = vtkAppendPolyData::New();
    myMapper = vtkPolyDataMapper::New();
    myMapper->ImmediateModeRenderingOn();
    myActor  = vtkActor::New();
    myActor->SetMapper(myMapper);

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
    perfectSort->SetInput(appender->GetOutput());
    usePerfectSort = true;
    lastCamera = vtkMatrix4x4::New();

    inputModified = true;
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
// ****************************************************************************

void
avtTransparencyActor::InputWasModified(int)
{
    inputModified = true;
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
// ****************************************************************************

bool
avtTransparencyActor::UsePerfectSort(bool perfect)
{
    usePerfectSort = perfect;
    return (appender->GetNumberOfInputs() > 0);
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
// ****************************************************************************

void
avtTransparencyActor::VisibilityOn(void)
{
    if (appender->GetNumberOfInputs() > 0)
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
// ****************************************************************************

void
avtTransparencyActor::PrepareForRender(vtkCamera *cam)
{
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

    //
    // The routine to set up our actual big actor is *long* -- push it off to
    // a subroutine.
    //
    if (needToRecalculate)
    {
        SetUpActor();
    }

    if (usePerfectSort)
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
            float proj[3];
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
// ****************************************************************************

void
avtTransparencyActor::AddToRenderer(vtkRenderer *ren)
{
    ren->AddActor(myActor);
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
    ren->GetProps()->RemoveItem(myActor);
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
// ****************************************************************************

void
avtTransparencyActor::SetUpActor(void)
{
    appender->RemoveAllInputs();
    int numActors = datasets.size();
    vtkActor *repActor = NULL;
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
                    appender->AddInput(preparedDataset[i][j]);
                    repActor = actors[i][j];
                }
            }
        }
    }

    //
    // If we don't have anything to render, don't have our actor draw.
    //
    if (appender->GetNumberOfInputs() == 0)
    {
        myActor->SetVisibility(0);
    }
    else
    {
        myActor->SetVisibility(1);

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
        // pauses as we cross between our view directions.
        //
        int sorting = visitTimer->StartTimer();
        axisSort->Update();
        visitTimer->StopTimer(sorting, "Sorting triangles for transparency");
        visitTimer->DumpTimings();
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
        // We will be drawing this, so turn there's off.
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
    vtkPolyData *pd = NULL;
    if (in_ds->GetDataObjectType() == VTK_POLY_DATA)
    {
        pd = (vtkPolyData *) in_ds;
    }
    else
    {
        gf->SetInput(in_ds);
        pd = gf->GetOutput();
        pd->Update();
    }

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
        float *color   = actor->GetProperty()->GetColor();
        float  opacity = actor->GetProperty()->GetOpacity();
        unsigned char rgba[4];
        rgba[0] = (unsigned char) (color[0] * 255);
        rgba[1] = (unsigned char) (color[1] * 255);
        rgba[2] = (unsigned char) (color[2] * 255);
        rgba[3] = (unsigned char) (opacity * 255);
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
            float opacity = actor->GetProperty()->GetOpacity();
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
                float pt[3];
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
            float opacity = actor->GetProperty()->GetOpacity();
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
        }
    }

    if (pd->GetPointData()->GetNormals() != NULL)
    {
        prepDS->GetPointData()->SetNormals(pd->GetPointData()->GetNormals());
    }
    else if (pd->GetCellData()->GetNormals() != NULL)
    {
        prepDS->GetCellData()->SetNormals(pd->GetCellData()->GetNormals());
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
avtTransparencyActor::ScaleByVector(const float vec[3])
{
    myActor->SetScale(vec[0], vec[1], vec[2]);
}
