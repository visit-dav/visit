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
//                             avtLabelPlot.C                                //
// ************************************************************************* //

#include <avtLabelPlot.h>

#include <avtCallback.h>
#include <avtCondenseDatasetFilter.h>
#include <avtExtents.h>
#include <avtGhostZoneAndFacelistFilter.h>
#include <avtLabelFilter.h>
#include <avtLabelsMapper.h>
#include <avtLabelSubsetsFilter.h>
#include <avtMesaLabelRenderer.h>
#include <avtOpenGLLabelRenderer.h>
#include <vtkLookupTable.h>
#include <avtUserDefinedMapper.h>
#include <avtVariableLegend.h>

#include <avtVertexNormalsFilter.h>
#include <TimingsManager.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

// ****************************************************************************
// Method: avtLabelPlot constructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
//   Hank Childs, Sat Dec  3 20:39:35 PST 2005
//   Change test for whether or not we are doing software rendering.
//
// ****************************************************************************

avtLabelPlot::avtLabelPlot() : avtSurfaceDataPlot()
{
    labelFilter = NULL;
    ghostAndFaceFilter = NULL;
    condenseFilter = NULL;
    normalFilter = NULL;
    labelSubsetsFilter = NULL;

    if (avtCallback::GetSoftwareRendering())
        renderer = new avtMesaLabelRenderer;
    else
        renderer = new avtOpenGLLabelRenderer;

    varLegend = new avtVariableLegend;
    varLegend->SetTitle("Label");
    vtkLookupTable *lut = vtkLookupTable::New();
    varLegend->SetLookupTable(lut);
    lut->Delete();
    varLegend->SetColorBarVisibility(0);
    varLegend->SetVarRangeVisibility(0);
    varLegendRefPtr = varLegend;

    avtCustomRenderer_p cr;
    CopyTo(cr, renderer);
    labelMapper = new avtLabelsMapper(cr);
}


// ****************************************************************************
// Method: avtLabelPlot destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
// ****************************************************************************

avtLabelPlot::~avtLabelPlot()
{
    if (ghostAndFaceFilter != NULL)
    {
        delete ghostAndFaceFilter;
        ghostAndFaceFilter = NULL;
    }
    if (condenseFilter != NULL)
    {
        delete condenseFilter;
        condenseFilter = NULL;
    }
    if (normalFilter != NULL)
    {
        delete normalFilter;
        normalFilter = NULL;
    }
    if (labelFilter != NULL)
    {
        delete labelFilter;
        labelFilter = NULL;
    }
    if (labelSubsetsFilter != NULL)
    {
        delete labelSubsetsFilter;
        labelSubsetsFilter = NULL;
    }

    renderer = NULL;

    if (labelMapper != NULL)
    {
        delete labelMapper;
        labelMapper = NULL;
    }

    //
    // Do not delete the varLegend since it is being held by varLegendRefPtr.
    //
}

// ****************************************************************************
//  Method:  avtLabelPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jan 7 14:58:26 PST 2004
//
// ****************************************************************************

avtPlot*
avtLabelPlot::Create()
{
    return new avtLabelPlot;
}

// ****************************************************************************
// Method: avtLabelPlot::GetMapper
//
// Purpose: 
//   Returns the label plot's mapper.
//
// Returns:    A pointer to the plot's mapper.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:48:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

avtMapper *
avtLabelPlot::GetMapper(void)
{
    return labelMapper;
}

// ****************************************************************************
// Method: avtLabelPlot::SetForegroundColor
//
// Purpose: 
//   Sets the foreground color into the label plot's renderer.
//
// Arguments:
//   c : The new foreground color.
//
// Returns:    True if the color was set; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:49:08 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
avtLabelPlot::SetForegroundColor(const double *c)
{
    return renderer->SetForegroundColor(c);
}

// ****************************************************************************
// Method: avtLabelPlot::SetLegend
//
// Purpose: 
//   Sets whether the legend is on or off.
//
// Arguments:
//   legendOn : Whether the legend should be on.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:50:17 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelPlot::SetLegend(bool legendOn)
{
    if (legendOn)
    {
        varLegend->LegendOn();
    }
    else
    {
        varLegend->LegendOff();
    }
}

// ****************************************************************************
// Method: avtLabelPlot::ApplyOperators
//
// Purpose:
//     Applies the operators associated with a Label plot.  
//     The output from this method is a query-able object.
//
// Arguments:
//     input   The input data object.
//
// Returns:    The data object after the Label plot has been applied.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
// ****************************************************************************

avtDataObject_p
avtLabelPlot::ApplyOperators(avtDataObject_p input)
{
    debug3 << "avtLabelPlot::ApplyOperators" << endl;
    return input;
}


// ****************************************************************************
// Method: avtLabelPlot::ApplyRenderingTransformation
//
// Purpose:
//     Applies the rendering transformation associated with a Label plot.  
//
// Arguments:
//     input   The input data object.
//
// Returns:    The data object after the Label plot has been applied.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Thu Jan  6 10:34:57 PST 2005
//   Removed TRY-CATCH block in favor of testing for ValidVariable.
//
//   Brad Whitlock, Tue Aug 2 15:05:15 PST 2005
//   I added code to set the variable's type in the label filter so the
//   filter can conditionally apply some special subset-related features.
//
//   Hank Childs, Wed Dec 20 10:50:37 PST 2006
//   Tell ghost and face filter that we need to always remove ghosts and to
//   only produce poly data.  It used to do this by default, but its default
//   behavior changed.
//
// ****************************************************************************

avtDataObject_p
avtLabelPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    debug3 << "avtLabelPlot::ApplyRenderingTransformation: start" << endl;

    avtDataObject_p dob = input;
    int total = visitTimer->StartTimer();
    int onefilter;

    //
    // Throw out the ghost cells.
    //
    onefilter = visitTimer->StartTimer();
    if(ghostAndFaceFilter != NULL)
        delete ghostAndFaceFilter;
    ghostAndFaceFilter = new avtGhostZoneAndFacelistFilter;
    ghostAndFaceFilter->SetUseFaceFilter(true);
    ghostAndFaceFilter->MustCreatePolyData();
    ghostAndFaceFilter->GhostDataMustBeRemoved();
    ghostAndFaceFilter->SetInput(dob);
    dob = ghostAndFaceFilter->GetOutput();
    visitTimer->StopTimer(onefilter, "avtGhostZoneAndFacelistFilter");

    //
    // If the variable that we're plotting is node centered or if it is
    // a mesh then remove any points that are not used in cells so we
    // don't end up adding labels for them.
    //
    if(condenseFilter != NULL)
    {
        delete condenseFilter; 
        condenseFilter = NULL;
    }
    bool removeExtraNodes = true;
    if (dob->GetInfo().GetAttributes().ValidVariable(varname))
    {
        removeExtraNodes = (AVT_NODECENT == 
            dob->GetInfo().GetAttributes().GetCentering(varname));
    }
    else 
    {
        debug1 << "We could not determine the variable centering for "
               << varname << " so let's assume that we need to remove "
               <<"extra nodes." << endl;
    }
    
    if(removeExtraNodes)
    {
        onefilter = visitTimer->StartTimer();
        condenseFilter = new avtCondenseDatasetFilter;
        condenseFilter->BypassHeuristic(true);
        condenseFilter->KeepAVTandVTK(true);
        condenseFilter->SetInput(dob);
        dob = condenseFilter->GetOutput();
        visitTimer->StopTimer(onefilter, "avtCondenseDatasetFilter");
    }

    //
    // Create vectors if the dataset's spatial dimension is greater
    // than 2 so we get normals that we can quantize in the label filter.
    //
    if(normalFilter != NULL)
    {
        delete normalFilter;
        normalFilter = NULL;
    }
    if(dob->GetInfo().GetAttributes().GetSpatialDimension() > 2)
    {
        onefilter = visitTimer->StartTimer();
        dob->GetInfo().GetValidity().SetNormalsAreInappropriate(false);
        normalFilter = new avtVertexNormalsFilter;
        normalFilter->SetInput(dob);
        dob = normalFilter->GetOutput();
        dob->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
        visitTimer->StopTimer(onefilter, "avtVertexNormalsFilter");
    }

    if(atts.GetVarType() == LabelAttributes::LABEL_VT_MATERIAL ||
       atts.GetVarType() == LabelAttributes::LABEL_VT_SUBSET)
    {
        // Apply the label subsets filter here.
        onefilter = visitTimer->StartTimer();
        if(labelSubsetsFilter != NULL)
            delete labelSubsetsFilter;
        labelSubsetsFilter = new avtLabelSubsetsFilter;
        labelSubsetsFilter->SetNeedMIR(atts.GetVarType() == 
            LabelAttributes::LABEL_VT_MATERIAL);
        labelSubsetsFilter->SetInput(dob);
        dob = labelSubsetsFilter->GetOutput();
        visitTimer->StopTimer(onefilter, "avtLabelSubsetsFilter");
    }

    //
    // Create additional label information.
    //
    onefilter = visitTimer->StartTimer();
    if(labelFilter != NULL)
        delete labelFilter;
    labelFilter = new avtLabelFilter;
    labelFilter->SetInput(dob);
    dob = labelFilter->GetOutput();
    visitTimer->StopTimer(onefilter, "avtLabelFilter");
    visitTimer->StopTimer(total, "avtLabelPlot::ApplyRenderingTransformation");

    return dob;
}


// ****************************************************************************
// Method: avtLabelPlot::CustomizeBehavior
//
// Purpose:
//     Customizes the behavior as appropriate for a Label plot.  This includes
//     behavior like shifting towards or away from the screen.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
// ****************************************************************************

void
avtLabelPlot::CustomizeBehavior(void)
{
    behavior->SetLegend(varLegendRefPtr);
    behavior->SetShiftFactor(0.5);
    behavior->SetRenderOrder(MUST_GO_LAST);
    behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);

    debug4 << "avtLabelPlot::CustomizeBehavior: Labels = " << endl;
    std::vector<std::string> labels;
    behavior->GetInfo().GetAttributes().GetLabels(labels);
    for(int i = 0; i < labels.size(); ++i)
        debug4 << "\tlabel["<<i<<"] = " << labels[i].c_str() << endl;
    debug4 << endl;
}


// ****************************************************************************
// Method: avtLabelPlot::CustomizeMapper
//
// Purpose:
//     A hook from the base class that allows the plot to change its mapper
//     based on the dataset input. 
//
// Arguments:
//     doi     The data object information.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Thu Jan  6 10:34:57 PST 2005
//   Removed TRY-CATCH block in favor of testing for ValidVariable.
//
//   Mark C. Miller, Sat Dec  2 18:58:26 PST 2006
//   Initialized e to fix valgrind error
//
//   Brad Whitlock, Thu Dec 13 14:57:09 PST 2007
//   Set the cell and node origin into the renderer.
//
// ****************************************************************************

void
avtLabelPlot::CustomizeMapper(avtDataObjectInformation &doi)
{
    //
    // Tell the renderer the variable that we care about.
    //
    renderer->SetVariable(varname);

    //
    // Tell the renderer whether to treat the label data as ASCII.
    //
    if (doi.GetAttributes().ValidVariable(varname))
    {
        renderer->SetTreatAsASCII(doi.GetAttributes().GetTreatAsASCII(varname));
    }
    else 
    {
        // Could not get the information so set the flag to false.
        renderer->SetTreatAsASCII(false);
    }

    // Tell the renderer about the cell and node origin.
    renderer->SetCellOrigin(doi.GetAttributes().GetCellOrigin());
    renderer->SetNodeOrigin(doi.GetAttributes().GetNodeOrigin());

    //
    // Tell the renderer whether the data is 3D or not so it can make
    // some 2D optimizations.
    //
    renderer->Set3D(doi.GetAttributes().GetSpatialDimension() == 3);

    //
    // Set the dataset's extents into the renderer so we can create
    // 2D label bins in world space.
    //
    // These also returned values that might also work.
    // GetCumulativeCurrentSpatialExtents
    // GetSpatialExtents
    // GetAnySpatialExtents
    double e[6] = {0.,1.,0.,1.,0.,1.};
    doi.GetAttributes().GetTrueSpatialExtents()->CopyTo(e);
    float fe[6];
    int i;
    for(i = 0; i < 6; ++i)
        fe[i] = float(e[i]);
    renderer->SetExtents(fe);

    bool ugl = atts.GetVarType() == LabelAttributes::LABEL_VT_MATERIAL ||
               atts.GetVarType() == LabelAttributes::LABEL_VT_SUBSET;
    renderer->SetUseGlobalLabel(ugl);

    debug4 << "avtLabelPlot::CustomizeMapper: Labels = " << endl;
    std::vector<std::string> labels;
    doi.GetAttributes().GetLabels(labels);
    for(i = 0; i < labels.size(); ++i)
        debug4 << "\tlabel["<<i<<"] = " << labels[i].c_str() << endl;
    debug4 << endl;
}

// ****************************************************************************
// Method: avtLabelPlot::EnhanceSpecification
//
// Purpose: 
//   Turns on global cell and node numbers in the data specification.
//
// Arguments:
//   spec : The input data specification.
//
// Returns:    A new data specification.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:53:04 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

avtPipelineSpecification_p
avtLabelPlot::EnhanceSpecification(avtPipelineSpecification_p spec)
{
    debug3 << "avtLabelPlot::EnhanceSpecification: 0" << endl;
    avtDataSpecification_p ds = spec->GetDataSpecification();

    //
    // The pipeline specification should really be const -- it is used
    // elsewhere, so we can't modify it and return it.  Make a copy and in
    // the new copy, indicate that we need structured indices.
    //
    avtDataSpecification_p nds = new avtDataSpecification(ds);
    nds->TurnZoneNumbersOn();
    nds->TurnNodeNumbersOn();
    nds->SetNeedStructuredIndices(true);
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec, nds);

    debug3 << "avtLabelPlot::EnhanceSpecification: 1" << endl;

    return rv;
}

// ****************************************************************************
// Method: avtLabelPlot::SetAtts
//
// Purpose:
//     Sets the atts for the Label plot.
//
// Arguments:
//     atts    The attributes for this Label plot.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 17:00:04 PST 2004
//   I fixed a bug where I forgot to set the new attributes into the current
//   attributes.
//
// ****************************************************************************

void
avtLabelPlot::SetAtts(const AttributeGroup *a)
{
    const LabelAttributes *newAtts = (const LabelAttributes *)a;

    // Set the label plot attributes into the renderer.
    renderer->SetAtts(newAtts);

    // See if any attributes that require the plot to be regenerated were
    // changed and copy the state object.
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;

    // Set whether the legend is on or off.
    SetLegend(atts.GetLegendFlag());
}

// ****************************************************************************
// Method: avtLabelPlot::ReleaseData
//
// Purpose: 
//   Causes the label plot to release its data.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:54:25 PDT 2004
//
// Modifications:
//   
//   Hank Childs, Tue Nov  2 05:16:53 PST 2004
//   Go ahead and release data for the normals filter, now that a memory issue
//   has been cleaned up.
//
// ****************************************************************************

void
avtLabelPlot::ReleaseData(void)
{
    debug3 << "avtLabelPlot::ReleaseData: 0" << endl;
    if (labelFilter != NULL)
    {
        labelFilter->ReleaseData();
    }
    if (ghostAndFaceFilter != NULL)
    {
        ghostAndFaceFilter->ReleaseData();
    }
    if (condenseFilter != NULL)
    {
        condenseFilter->ReleaseData();
    }
    if (normalFilter != NULL)
    {
        normalFilter->ReleaseData();
    }
    if (labelSubsetsFilter != NULL)
    {
        labelSubsetsFilter->ReleaseData();
    }
    debug3 << "avtLabelPlot::ReleaseData: 1" << endl;
}
