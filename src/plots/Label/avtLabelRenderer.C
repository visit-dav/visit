/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                             avtLabelRenderer.C                            //
// ************************************************************************* //

#include "avtLabelRenderer.h"

#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkGeometryFilter.h>
#include <vtkIntArray.h>

#include <vtkMath.h>
#include <vtkMatrix4x4.h>

#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkUnsignedIntArray.h>

#include <avtCallback.h>
#include <avtVector.h>

#include <snprintf.h>

#include <DebugStream.h>
#include <TimingsManager.h>


// ****************************************************************************
// Constructor:  avtLabelRenderer::avtLabelRenderer
//
// Programmer:  Brad Whitlock
// Creation:    October 1, 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//
//    Hank Childs, Thu Jul 21 14:45:04 PDT 2005
//    Initialize MAX_LABEL_SIZE.
//
//    Brad Whitlock, Tue Aug 2 15:26:43 PST 2005
//    I removed the single cell/node stuff.
//
// ****************************************************************************

avtLabelRenderer::avtLabelRenderer() : avtCustomRenderer(), globalLabel()
{
    input = 0;
    varname = 0;
    numXBins = 10;
    numYBins = 10;
    labelBins = 0;
    maxLabelLength = 1;
    maxLabelRows = 0;
    MAX_LABEL_SIZE = 36;
    rendererAction = RENDERER_ACTION_NOTHING;

    fgColor[0] = 1.;
    fgColor[1] = 1.;
    fgColor[2] = 1.;
    fgColor[3] = 1.;

    treatAsASCII = false;
    renderLabels3D = false;

    spatialExtents[0] = 0.;
    spatialExtents[1] = 0.;
    spatialExtents[2] = 0.;
    spatialExtents[3] = 0.;
    spatialExtents[4] = 0.;
    spatialExtents[5] = 0.;

    for(int i = 0; i < 256; ++i)
        visiblePoint[i] = false;

    useGlobalLabel = false;
}

// ****************************************************************************
// Destructor:  avtLabelRenderer::~avtLabelRenderer
//
// Programmer:  Brad Whitlock
// Creation:    October 1, 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:04:38 PST 2004
//   Changed the call to clear the label caches.
//
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//
// ****************************************************************************

avtLabelRenderer::~avtLabelRenderer()
{
    for (std::map<vtkDataSet*,vtkPolyData*>::iterator it = inputMap.begin();
         it != inputMap.end();
         it++)
    {
        it->second->Delete();
    }
    inputMap.clear();

    if(labelBins != 0)
        delete [] labelBins;

    if(varname != 0)
        delete [] varname;

    ClearLabelCaches();
}

// ****************************************************************************
// Method: avtLabelRenderer::SetForegroundColor
//
// Purpose: 
//   Sets the foreground color to be used by the labels if they are being
//   drawn in the foreground color.
//
// Arguments:
//   fg : The foreground color.
//
// Returns:    True if the color was set; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:55:14 PDT 2004
//
// Modifications:
//   Brad Whitlock, Thu Aug 4 11:14:27 PDT 2005
//   Changed when the function returns true.
//
// ****************************************************************************

bool
avtLabelRenderer::SetForegroundColor(const double *fg)
{
    bool retVal = false;

    if (atts.GetSpecifyTextColor1() ||
        (atts.GetVarType() == LabelAttributes::LABEL_VT_MESH &&
         atts.GetSpecifyTextColor2()))
    {
       if (fgColor[0] != fg[0] || fgColor[1] != fg[1] || fgColor[2] != fg[2])
       {
           retVal = true; 
       }
    }

    fgColor[0] = fg[0];
    fgColor[1] = fg[1];
    fgColor[2] = fg[2];

    return retVal;
}

// ****************************************************************************
// Method:  avtLabelRenderer::Render
//
// Purpose:
//   Set up things necessary to call the renderer implentation.  Make a new
//   implementation object if things have changed.
//
// Arguments:
//   ds         the dataset to render
//
// Programmer:  Brad Whitlock
// Creation:    Thu Jan 22 11:16:18 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis.  Also, added a call
//    to free the geometry filter when we were done with it.
//
//    Hank Childs, Thu Jul 21 16:40:13 PDT 2005
//    Initialize MAX_LABEL_SIZE based on variable type.
//
//    Brad Whitlock, Tue May 1 15:48:05 PST 2007
//    Increased the size of MAX_LABEL_SIZE to include enough room for minus
//    signs on numbers that get printed to scientific notation.
//
// ****************************************************************************

void
avtLabelRenderer::Render(vtkDataSet *ds)
{
    if (ds->GetNumberOfPoints() == 0)
    {
        debug1 << "avtLabelRenderer::Render: The input dataset had no points!" << endl;
        return;
    }

    //
    // For efficiency: if input type is vtkPolyData, there's no 
    // need to pass it through the geometry filter.
    //

    if (!inputMap[ds])
    {
        // we haven't seen this input yet
        if (ds->GetDataObjectType() != VTK_POLY_DATA) 
        {
            vtkGeometryFilter *gf = vtkGeometryFilter::New();
            gf->SetInput(ds);
            input = vtkPolyData::New();
            gf->SetOutput(input);
            gf->Update();
            gf->Delete();
        }
        else 
        {
            input = (vtkPolyData *)ds;
            input->Register(NULL);
        }

        inputMap[ds] = input;
    }
    else
    {
        // This one is a re-run, so it's safe to re-use whatever
        // polydata we decided to use for it last time.
        input = inputMap[ds];
    }

    // 
    // Create an appropriate MAX_LABEL_SIZE for the data.
    //
    vtkDataArray *pointData = input->GetPointData()->GetArray(varname);
    vtkDataArray *cellData = input->GetCellData()->GetArray(varname);
    int biggest = 0;
    if (pointData)
        biggest = (biggest > pointData->GetNumberOfComponents() ? biggest
                                       : pointData->GetNumberOfComponents());
    if (cellData)
        biggest = (biggest > cellData->GetNumberOfComponents() ? biggest
                                       : cellData->GetNumberOfComponents());
    // Check for Mesh plot
    if (biggest == 0)
        biggest = 3;
    MAX_LABEL_SIZE = 13*biggest;

    //
    // Render the labels using the derived type's RenderLabels method.
    //
    RenderLabels();
}

// ****************************************************************************
// Method: avtLabelRenderer::ReleaseGraphicsResources
//
// Purpose: 
//   This method is called when the renderer must release its graphical
//   resources.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 16:05:29 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::ReleaseGraphicsResources()
{
    ClearLabelCaches();
}

// ****************************************************************************
// Method: avtLabelRenderer::ResetLabelBins
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
avtLabelRenderer::ResetLabelBins()
{
    int n = numXBins * numYBins;
    if(labelBins == 0)
        labelBins = new LabelInfo[n];

    for(int i = 0; i < n; ++i)
    {
        labelBins[i].type = 0;
        labelBins[i].label = 0;
    }
}

// ****************************************************************************
// Method: avtLabelRenderer::SetVariable
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
avtLabelRenderer::SetVariable(const char *name)
{
    //
    // If the variable is different then clear the flags indicating that
    // we have valid cached node and cell labels.
    //
    if(varname != 0 && name != 0 && strcmp(name, varname) != 0)
    {
        ClearLabelCaches();
    }

    // Store the new variable name.
    delete [] varname;
    varname = new char[strlen(name) + 1];
    strcpy(varname, name);
}

// ****************************************************************************
// Method: avtLabelRenderer::SetTreatAsASCII
//
// Purpose: 
//   Causes the renderer to draw character data as characters.
//
// Arguments:
//   val : Whether to draw char data as characters.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:59:18 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::SetTreatAsASCII(bool val)
{
    treatAsASCII = val;
}

// ****************************************************************************
// Method: avtLabelRenderer::Set3D
//
// Purpose: 
//   Tells the renderer whether to draw the labels as 3D.
//
// Arguments:  
//   val : Whether to draw in 3D.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:59:55 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::Set3D(bool val)
{
    renderLabels3D = val;
}

// ****************************************************************************
// Method: avtLabelRenderer::SetExtents
//
// Purpose: 
//   Sets the spatial extents into the renderer.
//
// Arguments:
//   ext : The spatial extents.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:00:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::SetExtents(const float *ext)
{
    spatialExtents[0] = ext[0];
    spatialExtents[1] = ext[1];
    spatialExtents[2] = ext[2];
    spatialExtents[3] = ext[3];
    spatialExtents[4] = ext[4];
    spatialExtents[5] = ext[5];
}

// ****************************************************************************
// Method: avtLabelRenderer::SetGlobalLabel
//
// Purpose: 
//   Sets the global label to be used by the renderer.
//
// Arguments:
//   L : The global label.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 9 09:50:08 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::SetGlobalLabel(const std::string &L)
{
    globalLabel = L;
}

// ****************************************************************************
// Method: avtLabelRenderer::SetUseGlobalLabel
//
// Purpose: 
//   Tells the renderer that it should use a global label instead of creating
//   labels for each cell, node.
//
// Arguments:
//   val : Whether or not to use a global label.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 9 09:49:23 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::SetUseGlobalLabel(bool val)
{
    useGlobalLabel = val;
}

// ****************************************************************************
// Method: avtLabelRenderer::SetRendererAction
//
// Purpose: 
//   Tells the renderer what special actions it can perform during its render.
//
// Arguments:
//   a : The new set of actions.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 9 09:48:53 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::SetRendererAction(int a)
{
    rendererAction = a;
}

// ****************************************************************************
// Method: avtLabelRenderer::GetCellCenterArray
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

vtkFloatArray *
avtLabelRenderer::GetCellCenterArray()
{
    vtkFloatArray *cellCenters = 0;
    //
    // Look for the cell center array that the label filter calculated.
    //
    vtkDataArray *data = input->GetCellData()->GetArray("LabelFilterCellCenters");
    if(data == 0)
    {
        debug3 << "The avtLabelRenderer was not able to find the LabelFilterCellCenters array!" << endl;
    }
    else if(!data->IsA("vtkFloatArray"))
    {
        debug3 << "The avtLabelRenderer found the LabelFilterCellCenters array but it was not a vtkFloatArray.!" << endl;
    }
    else
        cellCenters = (vtkFloatArray *)data;

    return cellCenters;
}

// ****************************************************************************
// Method: avtLabelRenderer::CreateCachedCellLabels
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

#define BEGIN_LABEL  int L = 0;
#define CREATE_LABEL L += SNPRINTF
#define END_LABEL    L = (L == -1) ? MAX_LABEL_SIZE : L; \
                     maxLabelLength = (L > maxLabelLength) ? L : maxLabelLength; \
                     labelString += MAX_LABEL_SIZE;

void
avtLabelRenderer::CreateCachedCellLabels()
{
    char *&cellLabelsCache = cellLabelsCacheMap[input];

    // Resize the cell labels cache.
    if (cellLabelsCache == NULL)
    {
        int cellLabelsCacheSize = input->GetNumberOfCells();
        cellLabelsCacheSizeMap[input] = cellLabelsCacheSize;
        cellLabelsCache = new char[MAX_LABEL_SIZE * cellLabelsCacheSize + 1];

        //
        // Include the method body with BEGIN_LABEL, END_LABEL macros defined
        // such that we immediately draw the labels without first transforming
        // them.
        //
        char *labelString = cellLabelsCache;
#include <CellLabels_body.C>
    }
}
#undef BEGIN_LABEL
#undef END_LABEL

// ****************************************************************************
// Method: avtLabelRenderer::CreateCachedNodeLabels
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
#define END_LABEL    L = (L == -1) ? MAX_LABEL_SIZE : L; \
                     maxLabelLength = (L > maxLabelLength) ? L : maxLabelLength; \
                     labelString += MAX_LABEL_SIZE;
void
avtLabelRenderer::CreateCachedNodeLabels()
{
    char *&nodeLabelsCache = nodeLabelsCacheMap[input];

    // Resize the node labels cache.
    if(nodeLabelsCache == NULL)
    {
        int nodeLabelsCacheSize = input->GetPoints()->GetNumberOfPoints();
        nodeLabelsCacheSizeMap[input] = nodeLabelsCacheSize;
        nodeLabelsCache = new char[MAX_LABEL_SIZE * nodeLabelsCacheSize + 1];

        //
        // Include the method body with BEGIN_LABEL, END_LABEL macros defined
        // such that we immediately draw the labels without first transforming
        // them.
        //
        char *labelString = nodeLabelsCache;
#include <NodeLabels_body.C>
    }
}
#undef BEGIN_LABEL
#undef END_LABEL
#undef CREATE_LABEL

// ****************************************************************************
// Method: avtLabelRenderer::ClearLabelCaches
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
avtLabelRenderer::ClearLabelCaches()
{
    //
    // Delete the label caches.
    //
    maxLabelLength = 0;
    maxLabelRows = 1;

    std::map<vtkPolyData*,char*>::iterator it;
    for (it=cellLabelsCacheMap.begin(); it != cellLabelsCacheMap.end(); it++)
    {
        delete[] it->second;
    }
    cellLabelsCacheMap.clear();

    for (it=nodeLabelsCacheMap.begin(); it != nodeLabelsCacheMap.end(); it++)
    {
        delete[] it->second;
    }
    nodeLabelsCacheMap.clear();
}

// ****************************************************************************
// Method: avtLabelRenderer::AllowLabelInBin
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
avtLabelRenderer::AllowLabelInBin(const float *screenPoint, 
    const char *labelString, int t)
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
        LabelInfo *info = labelBins + index;

        //
        // The point is on the screen but is it closer than the point that is
        // already in the bin?
        //
        if(screenPoint[2] < info->screenPoint[2] || info->label == 0)
        {
            info->screenPoint[0] = screenPoint[0];
            info->screenPoint[1] = screenPoint[1];
            info->screenPoint[2] = screenPoint[2];
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
// Method: avtLabelRenderer::DepthTestPoint
//
// Purpose: 
//   Returns whether or not the point is visible according to the Z buffer.
//
// Arguments:
//   screenPoint : The point to test for its Z. The point is in normalized
//                 device coords.
//
// Returns:    True if the point is allowed closer than the Z buffer value.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 8 09:12:22 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtLabelRenderer::DepthTestPoint(float screenPoint[3]) const
{
    return true;
}

// ****************************************************************************
//  Method:  avtLabelRenderer::SetAtts
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
// ****************************************************************************

void
avtLabelRenderer::SetAtts(const AttributeGroup *a)
{
    const LabelAttributes *newAtts = (const LabelAttributes*)a;

    //
    // See if the label display format changed.
    //
    bool labelDisplayFormatChanged = 
        (atts.GetLabelDisplayFormat() != newAtts->GetLabelDisplayFormat()) ||
        (atts.GetFormatTemplate()     != newAtts->GetFormatTemplate());
    atts = *newAtts;

    //
    // Resize the number of bins based on the number of labels that we want to display.
    //
    int nLabels = (atts.GetNumberOfLabels() < 1) ? 1 : atts.GetNumberOfLabels();
    int nBins = int(sqrt(double(nLabels)));
    if(nBins * nBins < nLabels)
        ++nBins;
    if(nBins * nBins != numXBins * numYBins)
    {
        delete [] labelBins;
        labelBins = new LabelInfo[nBins * nBins];
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
}

//
// LabelInfo constructor
//
avtLabelRenderer::LabelInfo::LabelInfo()
{
     screenPoint[0] = 0.f;
     screenPoint[1] = 0.f;
     screenPoint[2] = 0.f;
     label = 0;
     type = 0;
}

avtLabelRenderer::LabelInfo::~LabelInfo()
{
}
