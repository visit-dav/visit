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
//                             avtSurfaceFilter.C                            //
// ************************************************************************* //

#include <avtSurfaceFilter.h>

#include <float.h>
#include <math.h>

#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkSurfaceFilter.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDataAttributes.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>

#include <ImproperUseException.h>
#include <InvalidDimensionsException.h>
#include <InvalidLimitsException.h>
#include <DebugStream.h>


// ****************************************************************************
//  Method: avtSurfaceFilter constructor
//
//  Arguments:
//      a       The attributed group with which to set the atts. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001.
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001
//    Intialize new members.
//
//    Mark C. Miller Sun Feb 29 18:08:26 PST 2004
//    Initialize zValMin and zValMax
//
//    Mark C. Miller, Tue Mar  2 09:58:49 PST 2004
//    Removed zValMin and zValMax data members
//
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004 
//    Moved geoFilter, appendFilter and edgesFilter to avtWireframeFilter.
//
//    Hank Childs, Sun Jan 30 13:55:21 PST 2005
//    Change attribute type.
//
//    Hank Childs, Fri Mar  4 08:47:07 PST 2005
//    Removed cd2pd.
//
//    Hank Childs, Mon Jun 18 09:04:39 PDT 2007
//    Set the active variable here, instead of in PerformRestriction.
//
// ****************************************************************************

avtSurfaceFilter::avtSurfaceFilter(const AttributeGroup *a)
{
    atts = *(SurfaceFilterAttributes*)a;
    filter       = vtkSurfaceFilter::New();
    stillNeedExtents = true;
    min = -1;
    max = -1;
    Ms = 1.;
    Bs = 0.;

    if (atts.GetVariable() != "default")
        SetActiveVariable(atts.GetVariable().c_str());
}


// ****************************************************************************
//  Method: avtSurfaceFilter destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001
//    Delete new members.
//
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004 
//    Moved geoFilter, appendFilter and edgesFilter to avtWireframeFilter.
//
//    Hank Childs, Fri Mar  4 08:47:07 PST 2005
//    Removed cd2pd.
//
// ****************************************************************************

avtSurfaceFilter::~avtSurfaceFilter()
{
    filter->Delete();
    filter = NULL;
}


// ****************************************************************************
//  Method:  avtSurfaceFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 05, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jun 12 13:54:14 PDT 2001
//    Changed return value to avtFilter.
//
// ****************************************************************************

avtFilter *
avtSurfaceFilter::Create(const AttributeGroup *atts)
{
    return new avtSurfaceFilter(atts);
}


// ****************************************************************************
//  Method: avtSurfaceFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtSurfaceFilter with the given
//      parameters would result in an equivalent avtSurfaceFilter.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001 
//
//  Modifications:
//
//    Hank Childs, Sun Jan 30 13:55:21 PST 2005
//    Change attribute type we are casting to.
//
// ****************************************************************************

bool
avtSurfaceFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(SurfaceFilterAttributes*)a);
}


// ****************************************************************************
//  Method: avtSurfaceFilter::ExecuteData
//
//  Purpose:
//      Maps this input 2d dataset to a 3d dataset by setting z coordinates
//      to a scaled version of the point/cell data.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label as a string.
//
//  Returns:      The output dataset. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Mar 20 08:58:14 PST 2001
//    Use input's centering rather than guessing.
//
//    Kathleen Bonnell, Tue Mar 20 09:48:07 PST 2001
//    Modified so that output (outUG) is created with 'New'.  Changed so
//    that scalar range and spatial extents are calculated from input 
//    avtDataset (input) instead of the input vtkDataset (inDS), so that 
//    values are correct across multiple domains.
//
//    Hank Childs, Sun Mar 25 11:44:12 PST 2001
//    Account for new interface with data object information.
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Implemented skew scaling. 
//
//    Kathleen Bonnell, Tue Apr 10 11:46:11 PDT 2001 
//    Renamed method as ExecuteData. 
//
//    Kathleen Bonnell, Wed Jun 13 11:46:02 PDT 2001
//    Preserve ghost-cell information if present in input dataset.
//
//    Hank Childs, Fri Sep  7 18:56:02 PDT 2001
//    Use doubles instead of floats.
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001 
//    Moved calculation of scale factors to method CalculateScaleValues. 
//    Added filters to extract edges after surface is constructed, needed
//    for rendering wireframe correctly.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray 
//    and vtkFloatArray instead.
//
//    Kathleen Bonnell, Fri Oct 10 10:48:24 PDT 2003
//    Preserve original-cell information if present in input dataset. 
//
//    Mark C. Miller, Sun Feb 29 18:08:26 PST 2004
//    Added code to compute zValMin and zValMax
//
//    Mark C. Miller, Tue Mar  2 09:58:49 PST 2004
//    Removed zValMin and zValMax data members
//
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004 
//    Moved geoFilter, appendFilter and edgesFilter to avtWireframeFilter.
//    Since vtkCellDataToPointData does not preserve PointData or FieldData,
//    (avtOriginalNodeNumbers, avtOriginalDimensions), etc. needed by Pick
//    and Query, only use the Scalars generated by cd2pd in the output of 
//    this filter.
//
//    Hank Childs, Fri Jul 30 12:26:20 PDT 2004
//    Update zValMin and zValMax.
//
//    Hank Childs, Sun Jan 30 13:55:21 PST 2005
//    Use SurfaceFilterAttributes, not SurfaceAttributes.
//
//    Hank Childs, Sun Jan 30 14:26:28 PST 2005
//    Qualify which variable we want to know the centering of.
//
//    Hank Childs, Fri Mar  4 08:47:07 PST 2005
//    Create cd2pd on an as-needed basis.
//
//    Hank Childs, Wed Mar  9 15:45:44 PST 2005
//    Fix memory leak.
//    
//    Sean Ahern, Tue Aug 14 11:51:00 EDT 2007
//    Allowed us a quick use of a "zero" variable.
//
//    Kathleen Bonnell, Tue Sep 11 08:52:45 PDT 2007 
//    Changed 'and' to '&&' for compilation on Windows. 
//
//    Hank Childs, Tue Dec 18 17:02:57 PST 2007
//    Beef up support for non-regular variables, especially including
//    the case with the "ZeroFlag".
//
// ****************************************************************************

vtkDataSet *
avtSurfaceFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkDataArray *inScalars = NULL;
    vtkFloatArray *outScalars  = vtkFloatArray::New();
    outScalars->SetNumberOfComponents(1);
    bool zf = atts.GetZeroFlag();
    bool usingDefaultVar = (atts.GetVariable() == "default");

    const char *varname = NULL;
    if ((zf == false) && (!usingDefaultVar))
        varname = atts.GetVariable().c_str();

    avtCentering cent = AVT_NODECENT; // right default if ZeroFlag is true

    TRY
    {
        if (!zf)
        {
            cent = GetInput()->GetInfo().GetAttributes().GetCentering(varname);
        }
    }
    CATCH(VisItException &v)
    {
        EXCEPTION1(VisItException, "VisIt could not determine which variable to "
                        "elevate by.  Are you possibly applying the Elevate "
                        "operator to a boundary or mesh plot?");
    }
    ENDTRY

    vtkCellDataToPointData *cd2pd = NULL;
    if (cent == AVT_ZONECENT)
    {
        //
        // The input is zone-centered, but this filter needs
        // node-centered data, so put it through a cell-to-point filter.
        //
        cd2pd = vtkCellDataToPointData::New();
        if ((zf == false) && (!usingDefaultVar))
        {
            vtkDataArray *tmp = inDS->GetCellData()->GetScalars();
            if (tmp != NULL && atts.GetVariable() != tmp->GetName())
            {
                tmp->Register(NULL);
                inDS->GetCellData()->SetScalars(inDS->GetCellData()
                                       ->GetArray(atts.GetVariable().c_str()));
                inDS->GetCellData()->AddArray(tmp);
                tmp->Delete();
            }
            else
                inDS->GetCellData()->SetScalars(inDS->GetCellData()
                                       ->GetArray(atts.GetVariable().c_str()));
        }
        cd2pd->SetInput(inDS);
        cd2pd->Update();
        inScalars = cd2pd->GetOutput()->GetPointData()->GetScalars();
    }
    else 
    {
        if (zf == false)
        {
            if (!usingDefaultVar)
                inScalars = 
                        inDS->GetPointData()->GetArray(atts.GetVariable().c_str());
            else
                inScalars = inDS->GetPointData()->GetScalars();
        }
    }

    if ((zf == false) && (inScalars == NULL))
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The data could not be elevated because "
                                      "of an internal error in VisIt.  VisIt "
                                      "was not able to retrieve the desired "
                                      "variable.");
            haveIssuedWarning = true;
        }
        outScalars->Delete();
        return NULL;
    }

    // Convert the scalars point data based on the scaling factors

    double zVal;
    int numScalars;
    if (zf == true)
        numScalars = inDS->GetNumberOfPoints();
    else
        numScalars = inScalars->GetNumberOfTuples();
    outScalars->SetNumberOfTuples(numScalars);

    bool doLog = false;
    bool doSkew = false;
    if (atts.GetUseXYLimits())
    {
        doLog = atts.GetScaling() == SurfaceFilterAttributes::Log;
        doSkew = atts.GetScaling() == SurfaceFilterAttributes::Skew;
    }
    for (int i = 0; i < numScalars; i++)
    {
        // calculate and store zVals
        if (zf == true)
            zVal = 0.;
        else
            zVal = inScalars->GetTuple1(i);
         
        if (doLog)
        {
            // min & max may have been set by user to be pos values
            // but individual data values are not guaranteed to fall
            // within that range, so check for non-positive data values
            // or log won't work.   
            if (zVal <= 0)
            {
                // The current minimum is the log of the original minimum,
                // so reverse the process to set the value. 
                zVal = pow(10.0, min);
            }
            zVal = log10(zVal);
        }
        else if (doSkew)
        {
             zVal = SkewTheValue(zVal); 
        }
        zVal = Ms * zVal + Bs;

        outScalars->SetValue(i, zVal);

        zValMin = (zVal < zValMin ? zVal : zValMin);
        zValMax = (zVal > zValMax ? zVal : zValMax);
    }

    vtkUnstructuredGrid *outUG = vtkUnstructuredGrid::New(); 

    // call the vtk filter to create the output dataset.
    filter->SetInput(inDS);
    filter->SetinScalars(outScalars);
    filter->SetOutput(outUG);
    filter->Update();

    outScalars->Delete();
    if ((zf == false) && (atts.GetGenerateNodalOutput()))
        outUG->GetPointData()->SetScalars(inScalars);

    if (cd2pd != NULL)
        cd2pd->Delete();

    ManageMemory(outUG);
    outUG->Delete();

    return (vtkDataSet*) outUG;
}


// ****************************************************************************
//  Method: avtSurfaceFilter::SkewTheValue
//
//  Purpose:
//      Takes the value and skews it according to the skewfactor. 
//
//  Arguments:
//      val   The value to be skewed.
//
//  Returns:
//      The value after the skew function has been applied to it.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 29, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001
//    Removed arguments that can be accessed directly in this method.
//
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002  
//    Handle max==min to avoid divide by zero errors. 
//
// ****************************************************************************

double
avtSurfaceFilter::SkewTheValue(const double val) 
{
    double skew = atts.GetSkewFactor();
    if (skew <= 0. || skew == 1.)
    {
        return val;
    }

    double range = (max == min ) ? 1. : max - min ;
    double rangeInverse = 1. / range;
    double logSkew = log(skew);
    double k = range / (skew -1.);
    double v2 = (val - min) * rangeInverse;
    double v = k * (exp(v2 * logSkew) -1.) + min;
    return v;
}


// ****************************************************************************
//  Method: avtSurfaceFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Reflect how the surface filter changes a data object.
//
//  Programmer: Hank Childs
//  Creation:   June 12, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 20 17:03:17 PDT 2001
//    Remove original spatial extents since they no longer apply.
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Kathleen Bonnell, Fri Oct 10 10:48:24 PDT 2003
//    Set PointsWereTransformed to true. 
//
//    Kathleen Bonnell, Thu May  6 17:51:15 PDT 2004
//    Invalidate zones so that pick will function properly. 
//
//    Brad Whitlock, Thu Jul 22 16:16:58 PST 2004
//    Changed the Z-axis's units so they match the variable's units if
//    the variable has any units.
//
//    Hank Childs, Mon Aug 30 08:04:05 PDT 2004
//    Set the label for the surface plot as well.
//
//    Hank Childs, Sun Jan 30 14:10:22 PST 2005
//    It may be that we don't have a valid variable (filled boundary plus
//    elevate operator).  Be more careful.  Also remove, TRY/CATCH blocks
//    that could be avoided through if tests.
//
// ****************************************************************************

void
avtSurfaceFilter::RefashionDataObjectInfo(void)
{
    avtDataValidity &va = GetOutput()->GetInfo().GetValidity();
    va.InvalidateSpatialMetaData();
    va.InvalidateZones();
    va.SetPointsWereTransformed(true);
    avtDataAttributes &da = GetOutput()->GetInfo().GetAttributes();
    da.SetSpatialDimension(3);
    if (da.ValidActiveVariable() && atts.GetGenerateNodalOutput())
        da.SetCentering(AVT_NODECENT);
    da.SetCanUseTransform(false);
    if (da.HasInvTransform())
    {
        da.SetCanUseInvTransform(false);
    }
    else
    {
        double tform[16] = {1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,0.,0.,0.,0.,0.,1.};
        da.SetInvTransform(tform);
    }
 
    //
    // Set the variable name as the label of the z-axis.
    //
    avtDataAttributes &in_da = GetInput()->GetInfo().GetAttributes();
    if (in_da.ValidActiveVariable())
    {
        da.SetZLabel(in_da.GetVariableName());
    }

    //
    // Set the Z-axis's units to match the variable units.
    // 
    if (da.ValidActiveVariable())
    {
        if(da.GetVariableUnits() != "")
            da.SetZUnits(da.GetVariableUnits());
    }
}


// ****************************************************************************
//  Method: avtSurfaceFilter::VerifyInput
//
//  Purpose:
//      Verifies that the input is 2D data, throws an exception if not. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
// ****************************************************************************

void
avtSurfaceFilter::VerifyInput(void)
{
    if  (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() != 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Surface", "2D");
    }
}


// ****************************************************************************
//  Method: avtSurfaceFilter::PerformRestriction
//
//  Purpose:
//    Disable dynamic load balancing if data extents cannot be retrieved now.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 2, 2001 
//
//  Modifications:
//
//    Hank Childs, Fri Mar 15 18:25:20 PST 2002
//    Account for dataset examiner.
//
//    Hank Childs, Thu Sep 23 08:07:03 PDT 2004
//    Instruct the database to create ghost zones if necessary.
//
//    Hank Childs, Sun Jan 30 14:05:25 PST 2005
//    Ask for a secondary variable.
//
//    Hank Childs, Mon Jun 18 09:04:39 PDT 2007
//    Ask for the secondary variable before this method is called.
//
// ****************************************************************************

avtPipelineSpecification_p
avtSurfaceFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    double dataExtents[2];
    double spatialExtents[6];
    if (TryDataExtents(dataExtents))
    {
        avtDataset_p input = GetTypedInput();
        avtDatasetExaminer::GetSpatialExtents(input, spatialExtents);
        CalculateScaleValues(dataExtents, spatialExtents);
        stillNeedExtents = false;
    }
    else
    {
        spec->NoDynamicLoadBalancing();
    }
    if (spec->GetDataSpecification()->MayRequireZones()) 
    {
        spec->GetDataSpecification()->TurnZoneNumbersOn();
    }
    if (spec->GetDataSpecification()->MayRequireNodes()) 
    {
        spec->GetDataSpecification()->TurnNodeNumbersOn();
    }

    //
    // We will need the ghost zones so that we can interpolate along domain
    // boundaries and get no cracks in our isosurface.
    //
    const char *varname = spec->GetDataSpecification()->GetVariable();
    avtDataAttributes &in_atts = GetInput()->GetInfo().GetAttributes();
    bool skipGhost = false;
    if (in_atts.ValidVariable(varname) &&
        in_atts.GetCentering(varname) == AVT_NODECENT)
        skipGhost = true;
    if (!skipGhost)
        spec->GetDataSpecification()->SetDesiredGhostDataType(GHOST_ZONE_DATA);

    return spec;
}


// ****************************************************************************
//  Method: avtSurfaceFilter::PreExecute
//
//  Purpose:
//      Sees if we still need to set the extents.  We know that we can call
//      GetDataExtents (which requires static load balancing), since if we
//      weren't able to get the extents when performing the restriction, we
//      disabled load balancing.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Mar 15 18:25:20 PST 2002
//    Account for dataset examiner.
//
//    Hank Childs, Fri Jul 30 12:24:30 PDT 2004
//    Initialize zValMin, zValMax.
//
//    Hank Childs, Sun Jan 30 14:21:29 PST 2005
//    Account for secondary variables.
//
//    Hank Childs, Tue Feb  1 13:09:37 PST 2005
//    Initialize haveIssuedWarning.
//
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtSurfaceFilter::PreExecute(void)
{
    avtStreamer::PreExecute();

    if (stillNeedExtents)
    {
        const char *varname = pipelineVariable;
        if (atts.GetVariable() != "default")
            varname = atts.GetVariable().c_str();
        double dataExtents[2];
        double spatialExtents[6];
        GetDataExtents(dataExtents, varname);
        avtDataset_p input = GetTypedInput();
        avtDatasetExaminer::GetSpatialExtents(input, spatialExtents);

        CalculateScaleValues(dataExtents, spatialExtents);
    }

    zValMin = +FLT_MAX;
    zValMax = -FLT_MAX;

    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtSurfaceFilter::CalculateScaleValues
//
//  Purpose:
//    Calculates scaling factors (taken mostly from po_surf.c MeshTV)
//
//  Arguments:
//    de        The data extents.
//    se        The spatial extents.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001
//    Use min, max associated with scaling.
//
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002  
//    Only one type of min/max (not scaling and coloring).
//    Added test for min > max. 
//
//    Hank Childs, Sun Jan 30 13:55:21 PST 2005
//    Use SurfaceFilterAttributes, not SurfaceAttributes.
//
//    Hank Childs, Tue Feb  1 11:26:46 PST 2005
//    Allow for mode where scaling is based solely on variable.
//
// ****************************************************************************

void
avtSurfaceFilter::CalculateScaleValues(double *de, double *se)
{
    double dX, dY, dZ, dXY;

    min = ( atts.GetMinFlag() ? atts.GetMin() : de[0] );
    max = ( atts.GetMaxFlag() ? atts.GetMax() : de[1] );

    if (min >= max && atts.GetMinFlag() && atts.GetMaxFlag())
    {
        EXCEPTION1(InvalidLimitsException, false); 
    }
    else if (min > max && atts.GetMinFlag())
    {
        max = min;
    }
    else if (min > max && atts.GetMaxFlag())
    {
        min = max;
    }

    if ( atts.GetScaling() == SurfaceFilterAttributes::Log )
    {
        min = log10(min);
        max = log10(max);
    }

    dX = se[1] - se[0];
    dY = se[3] - se[2];
    dZ = max - min;
    dXY = (dX > dY ? dX : dY);

    if (atts.GetUseXYLimits() == false || 0. == dZ)
    {
        Ms = 1.;
        Bs = 0.;
    }
    else
    {
        Ms = dXY / dZ;
        Bs = - (min * dXY) / dZ;
    }
}


// ****************************************************************************
//  Method: avtSurfaceFilter::ReleaseData
//
//  Purpose:
//      Releases all problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004 
//    Moved geoFilter, appendFilter and edgesFilter to avtWireframeFilter.
//
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them
//    from re-executing correctly in DLB-mode.  Also removed cd2pd.
//
//    Hank Childs, Fri Mar 11 07:37:05 PST 2005
//    Fix non-problem size leak introduced with last fix.
//
// ****************************************************************************

void
avtSurfaceFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    filter->SetInput(NULL);
    vtkUnstructuredGrid *u = vtkUnstructuredGrid::New();
    filter->SetOutput(u);
    u->Delete();
    filter->SetinScalars(NULL);
}


// ****************************************************************************
//  Method: avtSurfaceFilter::PostExecute
//
//  Purpose:
//    Send accurate Spatial Extents to output.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 24, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Jul 30 12:09:16 PDT 2004
//    Moved from avtWireframe filter.  Also re-wrote logic for getting extents.
//
// ****************************************************************************

void
avtSurfaceFilter::PostExecute(void)
{
    avtStreamer::PostExecute();

    avtDataAttributes& inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes& outAtts = GetOutput()->GetInfo().GetAttributes();

    // over-write spatial extents
    outAtts.GetTrueSpatialExtents()->Clear();
    outAtts.GetCumulativeTrueSpatialExtents()->Clear();

    // get the outputs's spatial extents
    double se[6];
    if (inAtts.GetTrueSpatialExtents()->HasExtents())
    {
        inAtts.GetTrueSpatialExtents()->CopyTo(se);
        se[4] = zValMin;
        se[5] = zValMax;
        // We can only set as cumulative, since the zVals are for the
        // current processor only.
        outAtts.GetCumulativeTrueSpatialExtents()->Set(se);
    }
    else if (inAtts.GetCumulativeTrueSpatialExtents()->HasExtents())
    {
        inAtts.GetCumulativeTrueSpatialExtents()->CopyTo(se);
        se[4] = zValMin;
        se[5] = zValMax;
        outAtts.GetCumulativeTrueSpatialExtents()->Set(se);
    }
}


