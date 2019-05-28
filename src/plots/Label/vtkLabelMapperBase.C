/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <vtkLabelMapperBase.h>


#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCompositeDataIterator.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkUnsignedIntArray.h>

#include <snprintf.h>

#include <DebugStream.h>
#include <vtkVisItUtility.h>


//
// Inline matrix operations
//

inline int
LabelHorizontalJustificationToVTK(LabelAttributes::LabelHorizontalAlignment h)
{
    int vtk_h = VTK_TEXT_CENTERED;
    if (h == LabelAttributes::Left)
        vtk_h = VTK_TEXT_LEFT;
    else if (h == LabelAttributes::Right)
        vtk_h = VTK_TEXT_RIGHT;

    return vtk_h;
}

inline int
LabelVerticalJustificationToVTK(LabelAttributes::LabelVerticalAlignment v)
{
    int vtk_v = VTK_TEXT_CENTERED;
    if (v == LabelAttributes::Top)
        vtk_v = VTK_TEXT_TOP;
    else if (v == LabelAttributes::Bottom)
        vtk_v = VTK_TEXT_BOTTOM;

    return vtk_v;
}



// ****************************************************************************
// Method: vtkLabelMapperBase::vtkLabelMapperBase
//
// Purpose: 
//   Constructor for the vtkLabelMapperBase class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:10 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Aug 8 17:35:11 PST 2005
//   Added zbuffer stuff.
//
// ****************************************************************************

vtkLabelMapperBase::vtkLabelMapperBase() : 
    VarName(), TextMappers(), LabelPositions(), GlobalLabel()
{
    this->NodeLabelProperty = vtkSmartPointer<vtkTextProperty>::New();
    this->NodeLabelProperty->SetFontSize(12);
    this->NodeLabelProperty->SetFontFamilyToArial();

    this->CellLabelProperty = vtkSmartPointer<vtkTextProperty>::New();
    this->CellLabelProperty->SetFontSize(12);
    this->CellLabelProperty->SetFontFamilyToArial();

    this->MaxLabelSize = 36;
    this->TreatAsASCII = false;
    this->RenderLabels3D = false;

    this->CellOrigin = 0;
    this->NodeOrigin = 0;

    this->ForegroundColor[0] = 0.;
    this->ForegroundColor[1] = 0.;
    this->ForegroundColor[2] = 0.;

    this->SpatialExtents[0] = 0.;
    this->SpatialExtents[1] = 0.;
    this->SpatialExtents[2] = 0.;
    this->SpatialExtents[3] = 0.;
    this->SpatialExtents[4] = 0.;
    this->SpatialExtents[5] = 0.;
    this->UseGlobalLabel = false;
    this->RendererAction = 0;

    this->numXBins = 0;
    this->numYBins = 0;
    this->LabelBins = NULL;
    this->maxLabelLength = 0;
    this->maxLabelRows = 0;

    this->CellLabelsCache = NULL;
    this->CellLabelsCacheSize = 0;

    this->NodeLabelsCache = NULL;
    this->NodeLabelsCacheSize = 0;
    for (int i  = 0; i < 256; ++i)
        visiblePoint[i] = false;
}

// ****************************************************************************
// Method: vtkLabelMapperBase::~vtkLabelMapperBase
//
// Purpose: 
//   Destructor for the vtkLabelMapperBase class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:32 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:12:27 PST 2004
//   Changed the routine to release the display lists.
//
// ****************************************************************************

vtkLabelMapperBase::~vtkLabelMapperBase()
{
    this->NodeLabelProperty->Delete();
    this->CellLabelProperty->Delete();
    // vtkSmartPointer handles deletion of the vtk objects.
    this->TextMappers.clear(); 
}

//----------------------------------------------------------------------------
void vtkLabelMapperBase::SetInputData(vtkDataObject* input)
{
  this->SetInputDataInternal(0, input);
}

//----------------------------------------------------------------------------
vtkDataSet *vtkLabelMapperBase::GetInput()
{
    return vtkDataSet::SafeDownCast(this->GetInputDataObject(0, 0));
}

vtkMTimeType vtkLabelMapperBase::GetMTime()
{
    vtkMTimeType mtime = this->Superclass::GetMTime();
    return mtime;
}


// ****************************************************************************
// Method: vtkLabelMapperBase::ReleaseGraphicsResources
//
// Purpose: 
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 16:00:15 PST 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:10:03 PST 2004
//   I made it clear the label cache and release the graphical resources.
//
// ****************************************************************************

void
vtkLabelMapperBase::ReleaseGraphicsResources(vtkWindow *win)
{
    ClearLabelCaches();
}


//----------------------------------------------------------------------------
void vtkLabelMapperBase::RenderOverlay(vtkViewport *viewport,
                                   vtkActor2D *actor)
{
    for (size_t i=0; i<this->TextMappers.size(); i++)
    {
        double x[3];
        x[0] = this->LabelPositions[3*i];
        x[1] = this->LabelPositions[3*i + 1];
        x[2] = this->LabelPositions[3*i + 2];

        double* pos = x;

        actor->GetPositionCoordinate()->SetValue(pos);
        this->TextMappers[i]->RenderOverlay(viewport, actor);
  }
}

bool CompareViews(avtViewInfo a, avtViewInfo b)
{
    bool mod = false;
    if(a.camera[0] != b.camera[0])
       mod = true;
    else if(a.camera[1] != b.camera[1])
       mod = true;
    else if(a.camera[2] != b.camera[2])
       mod = true;
    else if(a.viewUp[0] != b.viewUp[0])
       mod = true;
    else if(a.viewUp[1] != b.viewUp[1])
       mod = true;
    else if(a.viewUp[2] != b.viewUp[2])
       mod = true;
    return mod;
}

//----------------------------------------------------------------------------
void vtkLabelMapperBase::RenderOpaqueGeometry(vtkViewport *viewport,
                                          vtkActor2D *actor)
{
    // Updates the input pipeline if needed.
    this->Update();

    vtkDataObject *inputDO = this->GetInputDataObject(0, 0);
    if (!inputDO)
    {
        this->TextMappers.clear();
        this->LabelPositions.clear();
        vtkErrorMacro(<<"Need input data to render labels (2)");
        return;
    }

    vtkDataSet *input = vtkDataSet::SafeDownCast(inputDO);
    if (!input)
    {
        this->TextMappers.clear();
        this->LabelPositions.clear();
        vtkErrorMacro(<<"Input should be a vtkDataSet.");
        return;
    }

    vtkRenderer *ren = vtkRenderer::SafeDownCast(viewport);
    avtViewInfo temp;
    temp.SetViewFromCamera(ren->GetActiveCamera());
    bool viewChanged = CompareViews(visit_view, temp);
    if (viewChanged)
        visit_view = temp;
    if ( this->GetMTime() > this->BuildTime ||
         inputDO->GetMTime() > this->BuildTime ||
         viewChanged)
    {
        this->BuildLabels(vtkRenderer::SafeDownCast(viewport));
    }

    this->SetTextAtts(viewport);

    for (size_t i=0; i<this->TextMappers.size(); i++)
    {
        double* pos = &this->LabelPositions[3*i];
        actor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
        actor->GetPositionCoordinate()->SetValue(pos);
        this->TextMappers[i]->RenderOpaqueGeometry(viewport, actor);
    }
}

//----------------------------------------------------------------------------
int vtkLabelMapperBase::FillInputPortInformation(
  int vtkNotUsed( port ), vtkInformation* info)
{
    // Can handle composite datasets.
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
    return 1;
}

//----------------------------------------------------------------------------
void vtkLabelMapperBase::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    if (this->CellLabelProperty)
    {
        os << indent << "CellLabelTextProperty:\n";
        this->CellLabelProperty->PrintSelf(os, indent.GetNextIndent());
    }
    else
    {
        os << indent << "CellLabelTextProperty: (none)\n";
    }
    if (this->NodeLabelProperty)
    {
        os << indent << "NodeLabelTextProperty:\n";
        this->NodeLabelProperty->PrintSelf(os, indent.GetNextIndent());
    }
    else
    {
        os << indent << "NodeLabelTextProperty: (none)\n";
    }
}



// ****************************************************************************
// Method: vtkLabelMapperBase::SetTextAtts
//
// Purpose: 
//   Sets the color based on the type of variable being plotted and the
//   desired node or cell coloring.
//
// Arguments:
//   index : If it is 0 then do node coloring, else cell coloring.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 4 10:33:05 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
vtkLabelMapperBase::SetTextAtts(vtkViewport *vp)
{
    double rgb[3];
    int *sz = vp->GetSize();
    if(atts.GetVarType() == LabelAttributes::LABEL_VT_MESH)
    {
        // Node font atts
        FontAttributes nodeFA = atts.GetTextFont2();
        if(nodeFA.GetUseForegroundColor())
        {
            this->NodeLabelProperty->SetColor(this->ForegroundColor);
        }
        else
        {
            nodeFA.GetColor().GetRgb(rgb);
            this->NodeLabelProperty->SetColor(rgb);
        }

        this->NodeLabelProperty->SetFontSize(nodeFA.GetScale()*0.01*sz[1]);
        this->NodeLabelProperty->SetFontFamily((int)nodeFA.GetFont());
        this->NodeLabelProperty->SetBold(nodeFA.GetBold());
        this->NodeLabelProperty->SetItalic(nodeFA.GetItalic());

        // Cell font atts
        FontAttributes cellFA = atts.GetTextFont1();
        if(cellFA.GetUseForegroundColor())
        {
            this->CellLabelProperty->SetColor(this->ForegroundColor);
        }
        else
        {
            cellFA.GetColor().GetRgb(rgb);
            this->CellLabelProperty->SetColor(rgb);
        }
        this->CellLabelProperty->SetFontSize(cellFA.GetScale()*0.01*sz[1]);
        this->CellLabelProperty->SetFontFamily((int)cellFA.GetFont());
        this->CellLabelProperty->SetBold(cellFA.GetBold());
        this->CellLabelProperty->SetItalic(cellFA.GetItalic());

    }
    else
    {
        // Label font atts
        FontAttributes labelFA = atts.GetTextFont1();
        if(labelFA.GetUseForegroundColor())
        {
            this->NodeLabelProperty->SetColor(this->ForegroundColor);
            this->CellLabelProperty->SetColor(this->ForegroundColor);
        }
        else
        {
            labelFA.GetColor().GetRgb(rgb);
            this->NodeLabelProperty->SetColor(rgb);
            this->CellLabelProperty->SetColor(rgb);
        }
        this->NodeLabelProperty->SetFontSize(labelFA.GetScale()*0.01*sz[1]);
        this->NodeLabelProperty->SetFontFamily((int)labelFA.GetFont());
        this->NodeLabelProperty->SetBold(labelFA.GetBold());
        this->NodeLabelProperty->SetItalic(labelFA.GetItalic());
        this->CellLabelProperty->SetFontSize(labelFA.GetScale()*0.01*sz[1]);
        this->CellLabelProperty->SetFontFamily((int)labelFA.GetFont());
        this->CellLabelProperty->SetBold(labelFA.GetBold());
        this->CellLabelProperty->SetItalic(labelFA.GetItalic());
    }

    this->CellLabelProperty->SetJustification(
        LabelHorizontalJustificationToVTK(atts.GetHorizontalJustification()));
    this->NodeLabelProperty->SetJustification(
        LabelHorizontalJustificationToVTK(atts.GetHorizontalJustification()));
    this->CellLabelProperty->SetVerticalJustification(
        LabelVerticalJustificationToVTK(atts.GetVerticalJustification()));
    this->NodeLabelProperty->SetVerticalJustification(
        LabelVerticalJustificationToVTK(atts.GetVerticalJustification()));

}


//--------------------------------------------------------------------------
void
vtkLabelMapperBase::BuildLabels(vtkRenderer *ren)
{
  vtkDebugMacro(<<"Rebuilding labels");
  vtkDataObject* inputDO = this->GetInputDataObject(0, 0);
  vtkCompositeDataSet* cd = vtkCompositeDataSet::SafeDownCast(inputDO);
  vtkDataSet* ds = vtkDataSet::SafeDownCast(inputDO);
  if (ds)
  {
    this->BuildLabelsInternal(ds, ren);
  }
  else if (cd)
  {
    vtkCompositeDataIterator* iter = cd->NewIterator();
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal();
      iter->GoToNextItem())
    {
      ds = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (ds)
      {
        this->BuildLabelsInternal(ds, ren);
      }
    }
    iter->Delete();
  }
  else
  {
    vtkErrorMacro("Unsupported data type: " << inputDO->GetClassName());
  }

  this->BuildTime.Modified();

}


// ****************************************************************************
// Method: vtkLabelMapperBase::ResetLabelBins
//
// Notes:  Taken from avtLabelRenderer.
//
// Purpose: 
//   Resets the label bins used by the 3D binning algorithm so all of the
//   bins are empty.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:56:11 PDT 2004
//
// Modifications:
//   Brad Whitlock, Thu Aug 4 11:18:20 PDT 2005
//   Initialize the type to 0.
//
// ****************************************************************************

void
vtkLabelMapperBase::ResetLabelBins()
{
    int n = numXBins * numYBins;
    if(this->LabelBins == 0)
    {
        this->LabelBins = new LabelInfo[n];
    }

    for(int i = 0; i < n; ++i)
    {
        this->LabelBins[i].type = 0;
        this->LabelBins[i].label = 0;
    }
}

// ****************************************************************************
// Method: vtkLabelMapperBase::SetVariable
//
// Notes:  Taken from avtLabelRenderer.
//
// Purpose: 
//   Sets the name of the variable that is being rendered.
//
// Arguments:
//   name : The name of the plotted variable.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:56:56 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:19:21 PST 2004
//    The caching code changed a bit, so I decided to change the
//    invalidation to actually deallocate the memory as well.  It was
//    not a big price to pay, and the code winds up a bit simpler.
//   
// ****************************************************************************

void
vtkLabelMapperBase::SetVariable(const std::string &name)
{
    //
    // If the variable is different then clear the flags indicating that
    // we have valid cached node and cell labels.
    //
    if(!this->VarName.empty() && !name.empty() && name !=  this->VarName)
    {
        ClearLabelCaches();
    }

    // Store the new variable name.
    this->VarName = name;
}

// ****************************************************************************
// Method: vtkLabelMapperBase::GetCellCenterArray
//
//  Notes:  Taken from avtLabelRenderer.
//
// Purpose: 
//   Returns a pointer to the cell center array.
//
// Returns:    A pointer to the cell center array that the label filter 
//             calculated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:00:58 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

vtkDoubleArray *
vtkLabelMapperBase::GetCellCenterArray(vtkDataSet *input)
{
    vtkDoubleArray *cellCenters = 0;
    //
    // Look for the cell center array that the label filter calculated.
    //
    vtkDataArray *data = input->GetCellData()->GetArray("LabelFilterCellCenters");
    if(data == 0)
    {
        debug3 << "The vtkLabelMapperBase was not able to find the LabelFilterCellCenters array!" << endl;
    }
    else if(!data->IsA("vtkDoubleArray"))
    {
        debug3 << "The vtkLabelMapperBase found the LabelFilterCellCenters array but it was not a vtkDoubleArray.!" << endl;
    }
    else
        cellCenters = (vtkDoubleArray *)data;

    return cellCenters;
}


// ****************************************************************************
// Method: vtkLabelMapperBase::CreateCachedCellLabels
//
//  Notes:  Taken from avtLabelRenderer.
//
// Purpose: 
//   Creates a cache of cell labels
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 5 14:45:59 PST 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//   
//    Hank Childs, Thu Jul 21 09:33:42 PDT 2005
//    Modify BEGIN/CREATE/END _LABEL macros to accomodate multi-step label
//    creation (necessary for arrays).
//
// ****************************************************************************

#ifdef CREATE_LABEL
#undef CREATE_LABEL
#endif
#define BEGIN_LABEL  int L = 0;
#define CREATE_LABEL L += SNPRINTF
#define END_LABEL   L = (L == -1) ? this->MaxLabelSize : L; \
              maxLabelLength = (L > maxLabelLength) ? L : maxLabelLength; \
              labelString += this->MaxLabelSize;

void
vtkLabelMapperBase::CreateCachedCellLabels(vtkDataSet *input)
{
    // Resize the cell labels cache.
    if (this->CellLabelsCache == NULL)
    {
        this->CellLabelsCacheSize = input->GetNumberOfCells();
        this->CellLabelsCache = new char[this->MaxLabelSize * this->CellLabelsCacheSize + 1];

        //
        // Include the method body with BEGIN_LABEL, END_LABEL macros defined
        // such that we immediately draw the labels without first transforming
        // them.
        //
        char *labelString = this->CellLabelsCache;
#include <CellLabels_body.C>
    }
}
#undef BEGIN_LABEL
#undef END_LABEL

// ****************************************************************************
// Method: vtkLabelMapperBase::CreateCachedNodeLabels
//
//  Notes:  Taken from avtLabelRenderer.
//
// Purpose: 
//   Creates a cache of all node labels.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 5 14:49:59 PST 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//
//    Hank Childs, Thu Jul 21 09:33:42 PDT 2005
//    Modify BEGIN/CREATE/END _LABEL macros to accomodate multi-step label
//    creation (necessary for arrays).
//
// ****************************************************************************

#define BEGIN_LABEL  int L = 0;
#define CREATE_LABEL L += SNPRINTF
#define END_LABEL    L = (L == -1) ? this->MaxLabelSize : L; \
                     maxLabelLength = (L > maxLabelLength) ? L : maxLabelLength; \
                     labelString += this->MaxLabelSize;
void
vtkLabelMapperBase::CreateCachedNodeLabels(vtkDataSet *input)
{
    // Resize the node labels cache.
    if(this->NodeLabelsCache == NULL)
    {
        this->NodeLabelsCacheSize = input->GetNumberOfPoints();
        this->NodeLabelsCache = new char[this->MaxLabelSize * this->NodeLabelsCacheSize + 1];

        //
        // Include the method body with BEGIN_LABEL, END_LABEL macros defined
        // such that we immediately draw the labels without first transforming
        // them.
        //
        char *labelString = this->NodeLabelsCache;
#include <NodeLabels_body.C>
    }
}
#undef BEGIN_LABEL
#undef END_LABEL
#undef CREATE_LABEL


// ****************************************************************************
// Method: vtkLabelMapperBase::ClearLabelCaches
//
//  Notes:  Taken from avtLabelRenderer.
//
// Purpose: 
//   Clear out the label caches.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 16:03:33 PST 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//   
// ****************************************************************************

void
vtkLabelMapperBase::ClearLabelCaches()
{
    //
    // Delete the label caches.
    //
    maxLabelLength = 0;
    maxLabelRows = 1;

    if (this->CellLabelsCache)
    {
        delete[] this->CellLabelsCache;
        this->CellLabelsCache = NULL;
    }
    if (this->NodeLabelsCache)
    {
        delete[] this->NodeLabelsCache;
        this->NodeLabelsCache = NULL;
    }

    this->CellLabelsCacheSize = 0;
    this->NodeLabelsCacheSize = 0;

    this->TextMappers.clear();
    this->LabelPositions.clear();
    this->Modified();
}


// ****************************************************************************
// Method: vtkLabelMapperBase::AllowLabelInBin
//
//  Notes:  Taken from avtLabelRenderer.
//
// Purpose: 
//   This method is used in 3D label binning and it determines whether a given
//   label should be allowed in a bin.
//
// Arguments:
//   screenPoint : The label's location in normalized display space [0,1], [0,1].
//   labelString : A pointer to the label being considered.
//   t           : The type of label.
//
// Returns:    True if the label is allowed to be in the cell.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:03:34 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
vtkLabelMapperBase::AllowLabelInBin(const double *screenPoint, 
    const char *labelString, int t, const double *realPoint)
{
    bool retval = false;

    //
    // If the point is on the screen then consider it for binning.
    //
    int binx = int(screenPoint[0] * numXBins);
    int biny = int(screenPoint[1] * numYBins);

    if(binx >= 0 && binx < numXBins &&
       biny >= 0 && biny < numYBins)
    {
        int index = biny * numXBins + binx;
        LabelInfo *info = this->LabelBins + index;

        //
        // The point is on the screen but is it closer than the point that is
        // already in the bin?
        //
        if(screenPoint[2] < info->screenPoint[2] || info->label == 0)
        {
            info->screenPoint[0] = screenPoint[0];
            info->screenPoint[1] = screenPoint[1];
            info->screenPoint[2] = screenPoint[2];
            info->realPoint[0] = realPoint[0];
            info->realPoint[1] = realPoint[1];
            info->realPoint[2] = realPoint[2];
            retval = true;
            info->label = labelString;
            info->type = t;
        }
    }
    else
    {
        debug5 << "BAD binx or biny. binx=" << binx << ", biny=" << biny << endl;
    }

    return retval;
}


// ****************************************************************************
//  Method:  vtkLabelMapperBase::SetAtts
//
//  Notes:  Taken from avtLabelRenderer.
//
//  Purpose:
//    Set the attributes
//
//  Arguments:
//    a       : the new attributes
//
//  Programmer:  Brad Whitlock
//  Creation:    October 1, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Nov  8 17:19:21 PST 2004
//    The caching code changed a bit, so I decided to change the
//    invalidation to actually deallocate the memory as well.  It was
//    not a big price to pay, and the code winds up a bit simpler because
//    it is now just a single function call.
//
//    Brad Whitlock, Tue Aug 2 15:26:23 PST 2005
//    I removed the single cell/node stuff.
//
//    Dave Bremer, Wed Sep 19 19:39:40 PDT 2007
//    Clear the cache of label strings if the printf template is changed.
//
// ****************************************************************************

void
vtkLabelMapperBase::SetAtts(LabelAttributes *newAtts)
{
    //
    // See if the label display format changed.
    //
    bool labelDisplayFormatChanged = 
        (atts.GetLabelDisplayFormat() != newAtts->GetLabelDisplayFormat()) ||
        (atts.GetFormatTemplate()     != newAtts->GetFormatTemplate());

    bool setModified = atts.GetRestrictNumberOfLabels() != 
                           newAtts->GetRestrictNumberOfLabels(); 

    setModified |= atts.GetNumberOfLabels() !=
                   newAtts->GetNumberOfLabels();

    setModified |= atts.GetDrawLabelsFacing() !=
                   newAtts->GetDrawLabelsFacing();

    if(atts.GetVarType() == LabelAttributes::LABEL_VT_MESH)
    {
        setModified |= atts.GetShowNodes() != newAtts->GetShowNodes();
        setModified |= atts.GetShowCells() != newAtts->GetShowCells();
    }

    atts = *newAtts;

    //
    // Resize the number of bins based on the number of labels that we want
    // to display.
    //
    int nLabels = (atts.GetNumberOfLabels() < 1) ? 1 : atts.GetNumberOfLabels();
    int nBins = int(sqrt(double(nLabels)));
    if(nBins * nBins < nLabels)
        ++nBins;
    if(nBins * nBins != numXBins * numYBins)
    {
        if (this->LabelBins)
            delete [] this->LabelBins;
        this->LabelBins = new LabelInfo[nBins * nBins];
        numXBins = numYBins = nBins;
    }

    //
    // If we're changing the display format then we have to clear the
    // label cache.
    //
    if(labelDisplayFormatChanged)
    {
        ClearLabelCaches();
    }

    if (setModified)
        this->Modified();
}

//
// LabelInfo constructor
//
vtkLabelMapperBase::LabelInfo::LabelInfo()
{
     screenPoint[0] = 0.;
     screenPoint[1] = 0.;
     screenPoint[2] = 0.;
     realPoint[0] = 0.;
     realPoint[1] = 0.;
     realPoint[2] = 0.;
     label = 0;
     type = 0;
}

vtkLabelMapperBase::LabelInfo::~LabelInfo()
{
}
