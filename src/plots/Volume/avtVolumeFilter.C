/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                              avtVolumeFilter.C                            //
// ************************************************************************* //

#include <avtVolumeFilter.h>

#include <WindowAttributes.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>

#include <avtCommonDataFunctions.h>
#include <avtCompositeRF.h>
#include <avtDatasetExaminer.h>
#include <avtFlatLighting.h>
#include <avtIntegrationRF.h>
#include <avtOpacityMap.h>
#include <avtOpacityMapSamplePointArbitrator.h>
#include <avtParallel.h>
#include <avtRay.h>
#include <avtPhong.h>
#include <avtRayTracer.h>
#include <avtResampleFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtView3D.h>
#include <avtViewInfo.h>

#include <DebugStream.h>
#include <InvalidDimensionsException.h>
#include <InvalidVariableException.h>
#include <snprintf.h>


//
// Function Prototypes
//

static void CreateViewInfoFromViewAttributes(avtViewInfo &,
                                             const View3DAttributes &);


// ****************************************************************************
//  Method: avtVolumeFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 24 16:23:41 PST 2004
//    Removed references to data members that have been moved.
//
// ****************************************************************************

avtVolumeFilter::avtVolumeFilter()
{
    primaryVariable = NULL;
}


// ****************************************************************************
//  Method: avtVolumeFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 24 16:23:41 PST 2004
//    Removed references to data members that have been moved.
//
// ****************************************************************************

avtVolumeFilter::~avtVolumeFilter()
{
    if (primaryVariable != NULL)
    {
        delete [] primaryVariable;
        primaryVariable = NULL;
    }
}


// ****************************************************************************
//  Method: avtVolumeFilter::SetAttributes
//
//  Purpose:
//      Sets the attributes of the software override filter.
//
//  Arguments:
//      a       The attributes for the filter.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
avtVolumeFilter::SetAttributes(const VolumeAttributes &a)
{
    atts = a;
}


// ****************************************************************************
//  Method: avtVolumeFilter::Execute
//
//  Purpose:
//      Just passes the data through.  The volume filter really only does
//      the work in "RenderImage".
//
//  Programmer: Hank Childs
//  Creation:   November 11, 2004
//
// ****************************************************************************

void
avtVolumeFilter::Execute(void)
{
    avtDataObject_p input = GetInput();
    GetOutput()->Copy(*input);
}


// ****************************************************************************
//  Method: avtVolumeFilter::RenderImage
//
//  Purpose:
//      If we are supposed to do software rendering, then go ahead and do a
//      volume plot.  If we are supposed to do the hardware accelerated then
//      resample onto a rectilinear grid.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Dec  5 09:13:09 PST 2001
//    Get extents from the actual data.  Make sure those extents are from
//    the right variable.
//
//    Brad Whitlock, Wed Dec 5 11:16:38 PDT 2001
//    Added code to set the window's background mode and color.
//
//    Hank Childs, Wed Dec 12 10:54:58 PST 2001
//    Allow for extents to be set artificially.
//
//    Hank Childs, Fri Dec 21 07:52:45 PST 2001
//    Do a better job of using extents.
//
//    Hank Childs, Wed Jan 23 11:20:50 PST 2002
//    Add support for small cells.
//
//    Hank Childs, Wed Feb  6 09:22:21 PST 2002
//    Add support for running in parallel with more processors than domains.
//
//    Hank Childs, Fri Feb  8 19:03:49 PST 2002
//    Add support for setting the number of samples per ray.
//
//    Hank Childs, Fri Mar 15 18:11:12 PST 2002
//    Add support for dataset examiner.
//
//    Kathleen Bonnell, Wed Oct 23 13:27:56 PDT 2002  
//    Set queryable to false for the image and dataset object's validity. 
//
//    Hank Childs, Mon Jul  7 22:24:26 PDT 2003
//    If an error occurred, pass that message on.
//
//    Eric Brugger, Wed Aug 20 10:28:00 PDT 2003
//    Modified to handle the splitting of the view attributes into 2d and
//    3d parts.
//
//    Hank Childs, Tue Dec 16 10:43:53 PST 2003
//    Do a better job of setting up variable names based on rules that exclude
//    "vtk" and "avt" substrings.
//
//    Hank Childs, Wed Nov 24 16:23:41 PST 2004
//    Renamed from Execute.  Also removed any logic for resampling the dataset,
//    which is now done by the volume plot.
//
//    Hank Childs, Tue Nov 30 08:28:09 PST 2004
//    Fixed problem with identifying opacity variable in relation to skipping
//    variables with vtk and avt prefixes.
//
//    Hank Childs, Tue Dec 21 16:42:19 PST 2004
//    Incorporate attenuation.
//
//    Hank Childs, Sat Jan 29 10:37:19 PST 2005
//    Use opacity map sample arbitrator.
//
//    Hank Childs, Sat Jan  7 17:50:22 PST 2006
//    Use weighting variable for kernel based sampling.
//
//    Hank Childs, Mon Sep 11 14:46:07 PDT 2006
//    Add support for the integration ray function.
//
//    Hank Childs, Tue Mar 13 16:13:05 PDT 2007
//    Pass distance to integration ray function.
//
//    Hank Childs, Sat Aug 30 10:51:40 PDT 2008
//    Turn on shading.
//
// ****************************************************************************

avtImage_p
avtVolumeFilter::RenderImage(avtImage_p opaque_image,
                             const WindowAttributes &window)
{
    //
    // We need to create a dummy pipeline with the volume renderer that we
    // can force to execute within our "Execute".  Start with the source.
    //
    avtSourceFromAVTDataset termsrc(GetTypedInput());

    //
    // Set up the volume renderer.
    //
    avtRayTracer *software = new avtRayTracer;
    software->SetInput(termsrc.GetOutput());
    software->InsertOpaqueImage(opaque_image);
   
    unsigned char vtf[4*256];
    atts.GetTransferFunction(vtf);
    avtOpacityMap om(256);
    om.SetTable(vtf, 256, atts.GetOpacityAttenuation());
    double actualRange[2];
    bool artificialMin = atts.GetUseColorVarMin();
    bool artificialMax = atts.GetUseColorVarMax();
    if (!artificialMin || !artificialMax)
    {
        GetDataExtents(actualRange, primaryVariable);
        UnifyMinMax(actualRange, 2);
    }
    double range[2];
    range[0] = (artificialMin ? atts.GetColorVarMin() : actualRange[0]);
    range[1] = (artificialMax ? atts.GetColorVarMax() : actualRange[1]);
    om.SetMin(range[0]);
    om.SetMax(range[1]);

    if (atts.GetRendererType() == VolumeAttributes::RayCastingIntegration)
    {
        if (!artificialMin)
            range[0] = 0.;
        if (!artificialMax)
        {
/* Don't need this code, because the rays will be in depth ... 0->1.
            double bounds[6];
            GetSpatialExtents(bounds);
            UnifyMinMax(bounds, 6);
            double diag = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                               (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                               (bounds[5]-bounds[4])*(bounds[5]-bounds[4]));
            range[1] = (actualRange[1]*diag) / 2.;
 */
            range[1] = (actualRange[1]) / 4.;
        }
    }

    avtFlatLighting fl;
    avtLightingModel *lm = &fl;
    avtPhong phong;
    if (atts.GetLightingFlag())
    {
        lm = &phong;
    }
    else
    {
        lm = &fl;
    }

    //
    // Determine which variables to use and tell the ray function.
    //
    VarList vl;
    avtDataset_p input = GetTypedInput();
    avtDatasetExaminer::GetVariableList(input, vl);
    int primIndex = -1;
    int opacIndex = -1;
    int gradIndex = -1;
    int count = 0;
    char gradName[128];
    SNPRINTF(gradName, 128, "_%s_gradient", primaryVariable);
    
    for (int i = 0 ; i < vl.nvars ; i++)
    {
        if ((strstr(vl.varnames[i].c_str(), "vtk") != NULL) &&
            (strstr(vl.varnames[i].c_str(), "avt") != NULL))
            continue;

        if (vl.varnames[i] == primaryVariable)
        {
            primIndex = count;
        }
        if (vl.varnames[i] == atts.GetOpacityVariable())
        {
            opacIndex = count;
        }
        if (vl.varnames[i] == gradName)
        {
            gradIndex = count;
        }
        count += vl.varsizes[i];
    }

    if (primIndex == -1)
    {
        if (vl.nvars <= 0)
        {
            debug1 << "Could not locate primary variable "
                   << primaryVariable << ", assuming that we are running "
                   << "in parallel and have more processors than domains."
                   << endl;
        }
        else
        {
            EXCEPTION1(InvalidVariableException, primaryVariable);
        }
    }
    if (opacIndex == -1)
    {
        if (atts.GetOpacityVariable() == "default")
        {
            opacIndex = primIndex;
        }
        else if (vl.nvars <= 0)
        {
            debug1 << "Could not locate opacity variable "
                   << atts.GetOpacityVariable().c_str() << ", assuming that we "
                   << "are running in parallel and have more processors "
                   << "than domains." << endl;
        }
        else
        {
            EXCEPTION1(InvalidVariableException,atts.GetOpacityVariable());
        }
    }
    if (atts.GetLightingFlag() && gradIndex == -1)
    {
        if (vl.nvars <= 0)
        {
            debug1 << "Could not locate gradient variable, assuming that we "
                   << "are running in parallel and have more processors "
                   << "than domains." << endl;
        }
        else
        {
            EXCEPTION1(InvalidVariableException,gradName);
        }
    }

    int newPrimIndex = UnifyMaximumValue(primIndex);
    if (primIndex >= 0 && newPrimIndex != primIndex)
    {
        //
        // We shouldn't ever have different orderings for our variables.
        //
        EXCEPTION1(InvalidVariableException, primaryVariable);
    }
    primIndex = newPrimIndex;

    int newOpacIndex = UnifyMaximumValue(opacIndex);
    if (opacIndex >= 0 && newOpacIndex != opacIndex)
    {
        //
        // We shouldn't ever have different orderings for our variables.
        //
        EXCEPTION1(InvalidVariableException, atts.GetOpacityVariable());
    }
    opacIndex = newOpacIndex;

    int newGradIndex = UnifyMaximumValue(gradIndex);
    if (gradIndex >= 0 && newGradIndex != gradIndex)
    {
        //
        // We shouldn't ever have different orderings for our variables.
        //
        EXCEPTION1(InvalidVariableException, gradName);
    }
    gradIndex = newGradIndex;

    avtOpacityMap *om2 = NULL;
    if (primIndex == opacIndex)
    {
        // Note that we are forcing the color variables range onto the
        // opacity variable.
        om2 = &om;
    }
    else
    {
        om2 = new avtOpacityMap(256);
        om2->SetTable(vtf, 256, atts.GetOpacityAttenuation());
        double range[2];

        bool artificialMin = atts.GetUseOpacityVarMin();
        bool artificialMax = atts.GetUseOpacityVarMax();
        if (!artificialMin || !artificialMax)
        {
            InputSetActiveVariable(atts.GetOpacityVariable().c_str());
            avtDatasetExaminer::GetDataExtents(input, range);
            UnifyMinMax(range, 2);
            InputSetActiveVariable(primaryVariable);
        }
        range[0] = (artificialMin ? atts.GetOpacityVarMin() : range[0]);
        range[1] = (artificialMax ? atts.GetOpacityVarMax() : range[1]);
        om2->SetMin(range[0]);
        om2->SetMax(range[1]);
        // LEAK!!
    }
    avtCompositeRF *compositeRF = new avtCompositeRF(lm, &om, om2);
    avtIntegrationRF *integrateRF = new avtIntegrationRF(lm);

    compositeRF->SetColorVariableIndex(primIndex);
    compositeRF->SetOpacityVariableIndex(opacIndex);
    if (atts.GetLightingFlag())
        compositeRF->SetGradientVariableIndex(gradIndex);
    integrateRF->SetPrimaryVariableIndex(primIndex);
    integrateRF->SetRange(range[0], range[1]);
    if (atts.GetSampling() == VolumeAttributes::KernelBased)
    {
        software->SetKernelBasedSampling(true);
        compositeRF->SetWeightVariableIndex(vl.nvars);
    }
    
    if (atts.GetRendererType() == VolumeAttributes::RayCastingIntegration)
        software->SetRayFunction(integrateRF);
    else
        software->SetRayFunction(compositeRF);

    software->SetSamplesPerRay(atts.GetSamplesPerRay());

    const int *size = window.GetSize();
    software->SetScreen(size[0], size[1]);

    const View3DAttributes &view = window.GetView3D();
    avtViewInfo vi;
    CreateViewInfoFromViewAttributes(vi, view);
    software->SetView(vi);
    if (atts.GetRendererType() == VolumeAttributes::RayCastingIntegration)
    {
        integrateRF->SetDistance(view.GetFarPlane()-view.GetNearPlane());
        integrateRF->SetWindowSize(size[0], size[1]);
    }

    double view_dir[3];
    view_dir[0] = vi.focus[0] - vi.camera[0];
    view_dir[1] = vi.focus[1] - vi.camera[1];
    view_dir[2] = vi.focus[2] - vi.camera[2];
    double mag = sqrt(view_dir[0]*view_dir[0] + view_dir[1]*view_dir[1]
                      + view_dir[2]*view_dir[2]);
    if (mag != 0.) // only 0 if focus and camera are the same
    {
        view_dir[0] /= mag;
        view_dir[1] /= mag;
        view_dir[2] /= mag;
    }
    lm->SetViewDirection(view_dir);
    lm->SetViewUp(vi.viewUp);
    lm->SetLightInfo(window.GetLights());
    const RenderingAttributes &render_atts = window.GetRenderAtts();
    if (render_atts.GetSpecularFlag())
    {
        lm->SetSpecularInfo(render_atts.GetSpecularFlag(), 
                            render_atts.GetSpecularCoeff(),
                            render_atts.GetSpecularPower());
    }

    //
    // Set the volume renderer's background color and mode from the
    // window attributes.
    //
    software->SetBackgroundMode(window.GetBackgroundMode());
    software->SetBackgroundColor(window.GetBackground());
    software->SetGradientBackgroundColors(window.GetGradBG1(),
                                          window.GetGradBG2());

    //
    // We have to set up a sample point "arbitrator" to allow small cells
    // to be included in the final picture.
    //
    avtOpacityMapSamplePointArbitrator arb(om2, opacIndex);
    avtRay::SetArbitrator(&arb);

    //
    // Do the funny business to force an update.
    //
    avtDataObject_p dob = software->GetOutput();
    dob->Update(GetGeneralContract());

    if (atts.GetRendererType() == VolumeAttributes::RayCastingIntegration)
        integrateRF->OutputRawValues("integration.data");

    //
    // Free up some memory and clean up.
    //
    delete software;
    avtRay::SetArbitrator(NULL);
    delete compositeRF;
    delete integrateRF;

    //
    // Copy the output of the volume renderer to our output.
    //
    avtImage_p output;
    CopyTo(output, dob);
    return  output;
}


// ****************************************************************************
//  Method: avtVolumeFilter::GetNumberOfStages
//
//  Purpose:
//      Determines the number of stages based on the window size and number
//      of samples.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2005
//
// ****************************************************************************

int
avtVolumeFilter::GetNumberOfStages(const WindowAttributes &a)
{
    return avtRayTracer::GetNumberOfStages(a.GetSize()[0], a.GetSize()[1],
                                           atts.GetSamplesPerRay());
}


// ****************************************************************************
//  Function: CreateViewInfoFromViewAttributes
//
//  Purpose:
//      It appears that we have three view holders around: ViewAttributes,
//      avtViewInfo, and avtViewXD.  I'm not sure why this is.  We have
//      ViewAttributes and we want avtViewInfo.  Conversion routines exist
//      for an intermediate form, avtView3D.  Convert to that intermediate
//      form and use that.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
//  Modifications:
//    Eric Brugger, Tue Jun 10 15:59:11 PDT 2003
//    I renamed camera to view normal in the view attributes.
//
//    Hank Childs, Tue Jul  8 22:43:39 PDT 2003
//    Copy over image zoom, pan.
//
//    Eric Brugger, Wed Aug 20 10:28:00 PDT 2003
//    Modified to handle the splitting of the view attributes into 2d and
//    3d parts.
//
// ****************************************************************************

void
CreateViewInfoFromViewAttributes(avtViewInfo &vi, const View3DAttributes &view)
{
    //
    // Conversion routines are already established for converting to 3D.
    //
    avtView3D view3d;
    view3d.normal[0] = view.GetViewNormal()[0];
    view3d.normal[1] = view.GetViewNormal()[1];
    view3d.normal[2] = view.GetViewNormal()[2];
    view3d.focus[0] = view.GetFocus()[0];
    view3d.focus[1] = view.GetFocus()[1];
    view3d.focus[2] = view.GetFocus()[2];
    view3d.viewUp[0] = view.GetViewUp()[0];
    view3d.viewUp[1] = view.GetViewUp()[1];
    view3d.viewUp[2] = view.GetViewUp()[2];
    view3d.viewAngle = view.GetViewAngle();
    view3d.parallelScale = view.GetParallelScale();
    view3d.nearPlane = view.GetNearPlane();
    view3d.farPlane = view.GetFarPlane();
    view3d.perspective = view.GetPerspective();
    view3d.imagePan[0] = view.GetImagePan()[0];
    view3d.imagePan[1] = view.GetImagePan()[1];
    view3d.imageZoom = view.GetImageZoom();

    //
    // Now View3D can be converted directly into avtViewInfo.
    //
    view3d.SetViewInfoFromView(vi);
}


// ****************************************************************************
//  Method: avtVolumeFilter::ModifyContract
//
//  Purpose:
//      Performs a restriction based on which filter it is using underneath.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jan 14 14:15:20 PST 2002
//    Clean up memory leak.
//
//    Hank Childs, Wed Aug 11 09:15:21 PDT 2004
//    Allow for ghost zones to be created by other filters if necessary.
//
//    Hank Childs, Wed Nov 24 16:23:41 PST 2004
//    Removed commented out code, since current code is now correct.
//
//    Kathleen Bonnell, Fri Mar  4 13:53:45 PST 2005 
//    Account for different scaling methods. 
//
//    Hank Childs, Sun Mar 13 10:00:01 PST 2005
//    Tell filters upstream that we have rectilinear optimizations.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Sat Aug 30 10:50:15 PDT 2008
//    Add the gradient variable when lighting is on.
//
//    Hank Childs, Tue Sep  2 09:48:04 PDT 2008
//    Do a better job of handling oddly formed variables and make the new
//    gradient code play well with the log feature.
//
//    Sean Ahern, Wed Sep  3 09:47:31 EDT 2008
//    Fixed gradient calculation of smoothed data.
//
//    Sean Ahern, Wed Sep 10 13:04:41 EDT 2008
//    Refined the recenter so that it always asks for nodal centering.
//
// ****************************************************************************

avtContract_p
avtVolumeFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p newcontract = NULL;

    if (primaryVariable != NULL)
    {
        delete [] primaryVariable;
    }

    avtDataRequest_p ds = contract->GetDataRequest();
    const char *var = ds->GetVariable();

    if (atts.GetScaling() == VolumeAttributes::Linear)
    {
        newcontract = contract;
        primaryVariable = new char[strlen(var)+1];
        strcpy(primaryVariable, var);
    }
    else if (atts.GetScaling() == VolumeAttributes::Log10)
    {
        string exprName = (string)"log_" + (string)var;
        char exprDef[128];
        if (atts.GetUseColorVarMin())
        {
            char m[16];
            SNPRINTF(m, 16, "%f", atts.GetColorVarMin());
            SNPRINTF(exprDef, 128, "log10(if(gt(%s, 0), %s, %s))", 
                     var, var, m);
        }
        else 
        {
            SNPRINTF(exprDef, 128, "log10(%s)", var);
        }
        ExpressionList *elist = ParsingExprList::Instance()->GetList();
        Expression *e = new Expression();
        e->SetName(exprName.c_str());
        e->SetDefinition(exprDef); 
        e->SetType(Expression::ScalarMeshVar);
        elist->AddExpressions(*e);
        delete e;
        avtDataRequest_p nds = new 
          avtDataRequest(exprName.c_str(),
                               ds->GetTimestep(), ds->GetRestriction());
        nds->AddSecondaryVariable(var);
        newcontract = new avtContract(contract, nds);
        primaryVariable = new char[exprName.size()+1];
        strcpy(primaryVariable, exprName.c_str());
    }
    else // VolumeAttributes::Skew)
    {
        char exprName[128];
        SNPRINTF(exprName, 128, "%s_skewedby_%f", var, 
                 atts.GetSkewFactor()); 
        char exprDef[128];
        SNPRINTF(exprDef, 128, "var_skew(%s, %f)", var, 
                 atts.GetSkewFactor());
        ExpressionList *elist = ParsingExprList::Instance()->GetList();

        Expression *e = new Expression();
        e->SetName(exprName);
        e->SetDefinition(exprDef); 
        e->SetType(Expression::ScalarMeshVar);
        elist->AddExpressions(*e);
        delete e;
        avtDataRequest_p nds = 
            new avtDataRequest(exprName,
                ds->GetTimestep(), ds->GetRestriction());
        nds->AddSecondaryVariable(var);
        newcontract = new avtContract(contract, nds);
        primaryVariable = new char[strlen(exprName)+1];
        strcpy(primaryVariable, exprName);
    }

    if (atts.GetLightingFlag())
    {
        char exprName[128];
        SNPRINTF(exprName, 128, "_%s_gradient", primaryVariable);
        char exprDef[512];
        if (atts.GetSmoothData())
        {
            SNPRINTF(exprDef, 512, "gradient(recenter(<%s>, \"nodal\"))", primaryVariable);
        }
        else
        {
            SNPRINTF(exprDef, 512, "gradient(<%s>)", primaryVariable);
        }
        ExpressionList *elist = ParsingExprList::Instance()->GetList();

        Expression *e = new Expression();
        e->SetName(exprName);
        e->SetDefinition(exprDef); 
        e->SetType(Expression::VectorMeshVar);
        elist->AddExpressions(*e);
        delete e;
        ds = newcontract->GetDataRequest();
        avtDataRequest_p nds = new avtDataRequest(ds);
        nds->AddSecondaryVariable(exprName);
        newcontract = new avtContract(newcontract, nds);
    }

    newcontract->NoStreaming();
    newcontract->SetHaveRectilinearMeshOptimizations(true);
    return newcontract;
}


// ****************************************************************************
//  Method: avtVolumeFilter::VerifyInput
//
//  Purpose:
//      Verifies that the input is 3D data, throws an exception if not.
//
//  Programmer: Hank Childs
//  Creation:   September 26, 2002
//
// ****************************************************************************
 
void
avtVolumeFilter::VerifyInput(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() != 3)
    {
        EXCEPTION2(InvalidDimensionsException, "Volume", "3D");
    }
}


// ****************************************************************************
//  Method:  avtVolumeFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************

bool
avtVolumeFilter::FilterUnderstandsTransformedRectMesh()
{
    // The resampling and raycasting algorithms now all understand
    // these kinds of grids, so we can now safely return true.
    return true;
}

