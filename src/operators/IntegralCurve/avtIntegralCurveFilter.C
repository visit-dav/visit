/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                        avtIntegralCurveFilter.C                           //
// ************************************************************************* //

#include <avtIntegralCurveFilter.h>

#include <vtkSphereSource.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkCleanPolyData.h>

#include <avtParallel.h>
#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtContract.h>
#include <avtOriginatingSource.h>
#include <avtNamedSelectionManager.h>

#include <MapNode.h>

#include <avtStreamlineIC.h>

#include <vector>
#include <limits>

std::string avtIntegralCurveFilter::colorVarArrayName = "colorVar";
std::string avtIntegralCurveFilter::thetaArrayName = "theta";
std::string avtIntegralCurveFilter::tangentsArrayName = "tangents";


// ****************************************************************************
//  Method: CreateVTKVertex
//
//  Programmer:
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//
//   Dave Pugmire, Thu Jul  1 13:55:28 EDT 2010
//   Create a vertex instead of a sphere.
//   
//    Dave Pugmire, Mon Jul 12 15:34:29 EDT 2010
//    Remove rad argument.
//    
// ****************************************************************************
#if 0
static vtkPolyData *
CreateVTKVertex(double p[3], double val,
                std::vector< std::string > secondaryVariables,
                double *secondaryVals )
{
    vtkPolyData *pd = vtkPolyData::New();

    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(1);
    points->SetPoint(0, p[0], p[1], p[2]);
    
    pd->SetPoints(points);
    points->Delete();

    vtkIdType ids[1] = {0};
    pd->Allocate(1);
    pd->InsertNextCell(VTK_VERTEX, 1, ids);

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetName("colorVar");
    arr->SetNumberOfTuples(1);
    arr->SetTuple1(0, val);

    pd->GetPointData()->SetScalars(arr);
    arr->Delete();

    // secondary scalars
    for( unsigned int i=0; i<secondaryVariables.size(); ++i )
    {
        vtkFloatArray *secondary = vtkFloatArray::New();
        secondary->SetName(secondaryVariables[i].c_str());
        secondary->SetNumberOfTuples(1);
        secondary->SetTuple1(0, secondaryVals[i]);

        pd->GetPointData()->AddArray(secondary);
        secondary->Delete();
    }

    return pd;
}
#endif

// ****************************************************************************
//  Method: avtIntegralCurveFilter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 12:42:30 PDT 2004
//    I added coloringMethod and support for ribbons.
//
//    Hank Childs, Sat Mar  3 09:52:01 PST 2007
//    Initialized useWholeBox.
//
//    Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//    Initialize integral curve direction option.
//
//    Christoph Garth, Mon Feb 25 17:12:49 PST 2008
//    Port to new integral curve infrastructure
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Kathleen Bonnell, Wed Aug 27 15:13:07 PDT 2008
//   Initialize solver.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add 3 point density vars.
//
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Added termination by number of steps.
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Initialize the initial domain load count and timer.
//
//   Dave Pugmire (on behalf of Hank Childs), Tue Feb 24 09:39:17 EST 2009
//   Initial implemenation of pathlines.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Hank Childs, Sun Mar 22 11:30:40 CDT 2009
//   Initialize specifyPoint.
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Initialize seedTimeStep0 and seedTime0.
//
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Remove radius and showStart members.
//
//   Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//   Remove data members that are being put into avtPICSFilter.
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//
//   Dave Pugmire, Fri Jun 11 15:12:04 EDT 2010
//   Remove seed densities.
//
//   Hank Childs, Sun Dec  5 10:43:57 PST 2010
//   Initialize data members for warnings.
//
//   Dave Pugmire, Mon Feb 21 08:22:30 EST 2011
//   Color by correlation distance.
//
// ****************************************************************************

avtIntegralCurveFilter::avtIntegralCurveFilter() : seedVelocity(0,0,0),
  coordinateSystem(0)

{
    dataValue = IntegralCurveAttributes::TimeAbsolute;
    displayGeometry = IntegralCurveAttributes::Lines;
    cropValue = IntegralCurveAttributes::Time;

    //
    // Initialize source values.
    //
    sourceType = IntegralCurveAttributes::Point;
    sampleDensity[0] = sampleDensity[1] = sampleDensity[2] = 0;
    sampleDistance[0] = sampleDistance[1] = sampleDistance[2] = 0.0;
    numSamplePoints = 0;
    randomSamples = false;
    randomSeed = 0;
    fill = false;
    useBBox = false;
    sourceSelection = "";

    storeVelocitiesForLighting = false;
    issueWarningForMaxStepsTermination = true;
    issueWarningForStiffness = true;
    issueWarningForCriticalPoints = true;
    criticalPointThreshold = 1e-3;
    correlationDistanceAngTol = 0.0;
    correlationDistanceMinDist = 0.0;
    correlationDistanceDoBBox = false;
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 14:18:03 PST 2004
//    Added ribbons.
//
//    Christoph Garth, Mon Feb 25 17:12:49 PST 2008
//    Port to new integral curve infrastructure
//
//    Hank Childs, Fri Aug 22 09:41:02 PDT 2008
//    Move deletion of solver to PostExecute.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//   Remove data members that are being put into avtPICSFilter.
//
// ****************************************************************************

avtIntegralCurveFilter::~avtIntegralCurveFilter()
{
}

// ****************************************************************************
//  Method:  avtIntegralCurveFilter::Create
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
// ****************************************************************************

avtFilter *avtIntegralCurveFilter::Create()
{
    return new avtIntegralCurveFilter();
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::Equivalent
//
//  Purpose: Returns true if creating a new
//      avtStatisticalTrendsFilter with the given parameters would
//      result in an equivalent avtStatisticalTrendsFilter.
//
//  Programmer: childs -- generated by xml2avt
//  Creation:   Fri Jan 25 11:02:55 PDT 2008
//
// ****************************************************************************

bool
avtIntegralCurveFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(IntegralCurveAttributes*)a);
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::ExamineContact
//
//  Purpose: Examine the contract to get the current state of the time
//    slider. The time slider state is needed in case that the start
//    and end time are relative to the time slider.
//
//  Programmer: Oliver Ruebel
//  Creation:   May 07, 2009
//
//    Oliver Ruebel, Thu May 11 10:50
//
// ****************************************************************************
void
avtIntegralCurveFilter::ExamineContract(avtContract_p in_contract)
{
    avtDataRequest_p in_dr = in_contract->GetDataRequest();

    std::string key( "PseudocolorAttributes::lineType" );
    std::string lineTypeString("");
    std::string varyTubeRadiusVariable("");

    if( in_contract->GetAttribute( key ) )
      lineTypeString = in_contract->GetAttribute( key )->AsString();
    else
      lineTypeString = std::string("");

    if( lineTypeString == "Tube" )
    {
        displayGeometry = IntegralCurveAttributes::Tubes;

        key = std::string( "PseudocolorAttributes::varyTubeRadiusVariable" );

        if( in_contract->GetAttribute( key ) )
          varyTubeRadiusVariable = in_contract->GetAttribute( key )->AsString();
        else
          varyTubeRadiusVariable = std::string("");

    }

    else if( lineTypeString == "Ribbon" )
    {
        displayGeometry = IntegralCurveAttributes::Ribbons;
    }

    // Data from all secondary variables need to be added in.
    std::vector<CharStrRef> secondaryVars =
      in_dr->GetSecondaryVariablesWithoutDuplicates();

    secondaryVariables.resize( secondaryVars.size() );

    for( unsigned int i=0; i<secondaryVars.size(); ++i )
    {
      secondaryVariables[i] = std::string( *(secondaryVars[i]) );

      if( ! varyTubeRadiusVariable.empty() &&
          varyTubeRadiusVariable == secondaryVariables[i] )
      {
        tubeVariableIndex = i;
      }
    }

    // Call the examine contract function of the super classes first
    avtPluginFilter::ExamineContract(in_contract);
    avtPICSFilter::ExamineContract(in_contract);
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::ModifyContract
//
//  Purpose:
//      Creates a contract the removes the operator-created-expression.
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
//  Modifications:
//
//    Hank Childs, Mon Jul 21 14:09:03 PDT 2008
//    Remove "colorVar" and replace it with the gradient variable.  This is 
//    a trick because the integral curve requested "colorVar", which is the
//    variable it wants to color by.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Dave Pugmire (on behalf of Hank Childs), Tue Feb 24 09:39:17 EST 2009
//   Initial implemenation of pathlines.  
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Hank Childs, Sat Jun  5 19:01:55 CDT 2010
//   Strip out the pieces that belong in PICS.
//
//   Hank Childs, Sun Nov 28 05:37:44 PST 2010
//   Always add necessary secondary variables, regardless of whether there
//   is "colorVar" in the contract.
//
//   Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//   Add vary tube radius by variable.
//
// ****************************************************************************

avtContract_p
avtIntegralCurveFilter::ModifyContract(avtContract_p in_contract)
{
    avtDataRequest_p in_dr = in_contract->GetDataRequest();
    avtDataRequest_p out_dr = NULL;
    std::string var = in_dr->GetOriginalVariable();

    in_dr->SetUsesAllDomains(true);

    if( strncmp(var.c_str(), "operators/IntegralCurve/",
                strlen("operators/IntegralCurve/")) == 0)
    {
        std::string justTheVar = var.substr(strlen("operators/IntegralCurve/"));

        outVarName = justTheVar;

        out_dr = new avtDataRequest(in_dr, justTheVar.c_str());
    }

    else if (strcmp(in_dr->GetVariable(), "colorVar") == 0 ||
             secondaryVariables.size() )
    {
        // The avtStreamlinePlot requested "colorVar", so remove that from the
        // contract now.
        out_dr = new avtDataRequest(in_dr, in_dr->GetOriginalVariable());
    }
    else
        out_dr = new avtDataRequest(in_dr);

    if (dataValue == IntegralCurveAttributes::Variable)
        out_dr->AddSecondaryVariable(dataVariable.c_str());

    avtContract_p out_contract;

    if ( *out_dr )
        out_contract = new avtContract(in_contract, out_dr);
    else
        out_contract = new avtContract(in_contract);

    return avtPICSFilter::ModifyContract(out_contract);
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Tells output that we have a new variable.
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
//  Modifications:
//
//    Brad Whitlock, Mon Jan 3 13:31:11 PST 2005
//    Set the flag that prevents normals from being generated if we're
//    displaying the integral curve as lines.
//
//    Hank Childs, Sat Mar  3 11:02:33 PST 2007
//    Make sure we have a valid active variable before setting its dimension.
//
//    Hank Childs, Sun Mar  9 07:47:05 PST 2008
//    Call the base class' method as well.
//
//    Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//    Call the new base class' (avtPICSFilter) method.
//
//    Brad Whitlock, Mon Apr  7 15:55:02 PDT 2014
//    Add filter metadata used in export.
//    Work partially supported by DOE Grant SC0007548.
//
// ****************************************************************************

void
avtIntegralCurveFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();

    // ARS - FIXME  - FIXME  - FIXME 
    // if(displayGeometry == IntegralCurveAttributes::Lines)
    //     GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);

    avtDataAttributes &in_atts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &out_atts = GetOutput()->GetInfo().GetAttributes();

    if( outVarName != "" )
    {
      std::string fullVarName = outVarName;

      out_atts.RemoveVariable(in_atts.GetVariableName());
      
      if( !out_atts.ValidVariable(fullVarName) )
      {
        out_atts.AddVariable((fullVarName).c_str());
        out_atts.SetActiveVariable(fullVarName.c_str());
        out_atts.SetVariableDimension(1);
        
        out_atts.SetVariableType(AVT_SCALAR_VAR);
      }
    }

    if (! out_atts.ValidVariable("colorVar"))
    {
        out_atts.AddVariable("colorVar");
        out_atts.SetActiveVariable("colorVar");
        out_atts.SetVariableDimension(1);
        out_atts.SetCentering(AVT_NODECENT);
    }

    out_atts.SetTopologicalDimension(1);

    avtPluginFilter::UpdateDataObjectInfo();
    avtPICSFilter::UpdateDataObjectInfo();

    out_atts.AddFilterMetaData("IntegralCurve");
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::SetAtts
//
//  Purpose:
//      Sets the atts for the IntegralCurve plot.
//
//  Arguments:
//      atts    The attributes for this IntegralCurve plot.
//
//  Programmer: Dave Pugmire -- generated by xml2avt
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtIntegralCurveFilter::SetAtts(const AttributeGroup *a)
{
    const IntegralCurveAttributes *newAtts = (const IntegralCurveAttributes *)a;

    // See if any attributes that require the plot to be regenerated were
    // changed and copy the state object.
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;

    //
    // Set the filter's attributes based on the plot attributes.
    //
    switch (atts.GetSourceType())
    {
      case IntegralCurveAttributes::Point:
        SetPointSource(atts.GetPointSource());
        break;
      case IntegralCurveAttributes::PointList:
        SetPointListSource(atts.GetPointList());
        break;

      case IntegralCurveAttributes::Line_:
        SetLineSource(atts.GetLineStart(), atts.GetLineEnd(),
                      atts.GetSampleDensity0(), atts.GetRandomSamples(),
                      atts.GetRandomSeed(), atts.GetNumberOfRandomSamples());
        break;
      case IntegralCurveAttributes::Plane:
        SetPlaneSource(atts.GetPlaneOrigin(), atts.GetPlaneNormal(),
                       atts.GetPlaneUpAxis(), atts.GetSampleDensity0(),
                       atts.GetSampleDensity1(), atts.GetSampleDistance0(),
                       atts.GetSampleDistance1(), atts.GetFillInterior(),
                       atts.GetRandomSamples(), atts.GetRandomSeed(),
                       atts.GetNumberOfRandomSamples());
        break;

      case IntegralCurveAttributes::Circle:
        SetCircleSource(atts.GetPlaneOrigin(), atts.GetPlaneNormal(),
                        atts.GetPlaneUpAxis(), atts.GetRadius(),
                        atts.GetSampleDensity0(), atts.GetSampleDensity1(),
                        atts.GetFillInterior(), atts.GetRandomSamples(),
                        atts.GetRandomSeed(), atts.GetNumberOfRandomSamples());
        break;
      case IntegralCurveAttributes::Sphere:
        SetSphereSource(atts.GetSphereOrigin(), atts.GetRadius(),
                        atts.GetSampleDensity0(), atts.GetSampleDensity1(),
                        atts.GetSampleDensity2(), atts.GetFillInterior(),
                        atts.GetRandomSamples(), atts.GetRandomSeed(),
                        atts.GetNumberOfRandomSamples());
        break;

      case IntegralCurveAttributes::Box:
        SetBoxSource(atts.GetBoxExtents(),atts.GetUseWholeBox(),
                     atts.GetSampleDensity0(), atts.GetSampleDensity1(),
                     atts.GetSampleDensity2(), atts.GetFillInterior(),
                     atts.GetRandomSamples(), atts.GetRandomSeed(),
                     atts.GetNumberOfRandomSamples());
        break;
      case IntegralCurveAttributes::Selection:
        SetSelectionSource(atts.GetSelection(), atts.GetSampleDensity0(),
                           atts.GetRandomSamples(), atts.GetRandomSeed(),
                           atts.GetNumberOfRandomSamples());
        break;
    }

    int CMFEType = (atts.GetPathlinesCMFE() == IntegralCurveAttributes::CONN_CMFE
                    ? PICS_CONN_CMFE : PICS_POS_CMFE);

    SetPathlines(atts.GetPathlines(),
                 atts.GetPathlinesOverrideStartingTimeFlag(),
                 atts.GetPathlinesOverrideStartingTime(),
                 atts.GetPathlinesPeriod(),
                 CMFEType);

    SetIntegrationDirection(atts.GetIntegrationDirection());

    SetFieldType(atts.GetFieldType());
    SetFieldConstant(atts.GetFieldConstant());
    SetVelocitySource(atts.GetVelocitySource());

    SetIntegrationType(atts.GetIntegrationType());

    SetParallelizationAlgorithm(atts.GetParallelizationAlgorithmType(), 
                                atts.GetMaxProcessCount(),
                                atts.GetMaxDomainCacheSize(),
                                atts.GetWorkGroupSize());

    if (atts.GetIntegrationType() == IntegralCurveAttributes::DormandPrince)
    {
        // For DoPri, the max time step is sent in to the PICS filter
        // as the max step length.
        double step = atts.GetMaxTimeStep();
        if (! atts.GetLimitMaximumTimestep())
            step = 0;
        SetMaxStepLength(step);
    }
    else
        SetMaxStepLength(atts.GetMaxStepLength());
    double absTol = 0.;
    bool doBBox = (atts.GetAbsTolSizeType() == IntegralCurveAttributes::FractionOfBBox);
    if (doBBox)
        absTol = atts.GetAbsTolBBox();
    else
        absTol = atts.GetAbsTolAbsolute();

    SetTolerances(atts.GetRelTol(), absTol, doBBox);

    SetTermination(atts.GetMaxSteps(),
                   atts.GetTerminateByDistance(),
                   atts.GetTermDistance(),
                   atts.GetTerminateByTime(),
                   atts.GetTermTime());

    IssueWarningForMaxStepsTermination(atts.GetIssueTerminationWarnings());
    IssueWarningForStiffness(atts.GetIssueStiffnessWarnings());
    IssueWarningForCriticalPoints(atts.GetIssueCriticalPointsWarnings(),
                                  atts.GetCriticalPointThreshold());

    SetDataValue(int(atts.GetDataValue()), atts.GetDataVariable());
    SetCropValue(int(atts.GetCropValue()));

    if (atts.GetDataValue() == IntegralCurveAttributes::CorrelationDistance)
    {
        bool doBBox = (atts.GetCorrelationDistanceMinDistType() ==
                       IntegralCurveAttributes::FractionOfBBox);

        double minDist = (doBBox ? atts.GetCorrelationDistanceMinDistBBox() :
                                   atts.GetCorrelationDistanceMinDistAbsolute());
        double angTol = atts.GetCorrelationDistanceAngTol();

        SetCorrelationDistanceTol(angTol, minDist, doBBox);
    }

    SetVelocitiesForLighting(1);
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::PreExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Nov  7 13:01:47 EST 2008
//
//  Modifications:
//
//
// ****************************************************************************

void
avtIntegralCurveFilter::PreExecute(void)
{
    avtPICSFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::PostExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Nov  7 13:01:47 EST 2008
//
//  Modifications:
//
//    Hank Childs, Fri Aug 22 09:40:21 PDT 2008
//    Move the deletion of the solver here.
//
//    Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//    Remove data members that are being put into avtPICSFilter.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
//    Hank Childs, Sun Aug 29 19:26:47 PDT 2010
//    Set the extents in more situations.
//
// ****************************************************************************

void
avtIntegralCurveFilter::PostExecute(void)
{
    avtPICSFilter::PostExecute();

    if (dataValue == IntegralCurveAttributes::SeedPointID ||
        dataValue == IntegralCurveAttributes::Vorticity ||
        dataValue == IntegralCurveAttributes::Speed ||
        dataValue == IntegralCurveAttributes::ArcLength ||
        dataValue == IntegralCurveAttributes::TimeAbsolute ||
        dataValue == IntegralCurveAttributes::TimeRelative ||
        dataValue == IntegralCurveAttributes::AverageDistanceFromSeed ||
        dataValue == IntegralCurveAttributes::Difference ||
        dataValue == IntegralCurveAttributes::Variable)
    {
        double range[2];
        avtDataset_p ds = GetTypedOutput();
        avtDatasetExaminer::GetDataExtents(ds, range, "colorVar");

        avtExtents *e;
        e = GetOutput()->GetInfo().GetAttributes()
                                            .GetThisProcsOriginalDataExtents();
        e->Merge(range);
        e = GetOutput()->GetInfo().GetAttributes()
                                           .GetThisProcsActualDataExtents();
        e->Merge(range);
    }
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::Execute
//
//  Purpose:
//      Executes the Integral Curve
//
//  Programmer: Allen Sanderson
//  Creation:   20 August 2013
//
// ****************************************************************************

void
avtIntegralCurveFilter::Execute(void)
{
    avtPICSFilter::Execute();

    std::vector<avtIntegralCurve *> ics;
    GetTerminatedIntegralCurves(ics);

    ReportWarnings( ics );
}


static float random01()
{
    return (float)rand()/(float)RAND_MAX;
}

static float random_11()
{
    return (random01()*2.0) - 1.0;
}


// ****************************************************************************
// Method:  avtIntegralCurveFilter::GetCommunicationPattern()
//
// Programmer:  Dave Pugmire
// Creation:    September  1, 2011
//
// ****************************************************************************

avtPICSFilter::CommunicationPattern
avtIntegralCurveFilter::GetCommunicationPattern()
{
  // ARS - FIXME  - FIXME  - FIXME  - FIXME  - FIXME 
    // if (! scaleTubeRadiusVariable.empty())
    //     return avtPICSFilter::ReturnToOriginatingProcessor;
    
    return avtPICSFilter::RestoreSequenceAssembleUniformly;
}

// ****************************************************************************
// Method:  avtIntegralCurveFilter::GenerateAttributeFields() const
//
// Programmer:  Dave Pugmire
// Creation:    November  5, 2010
//
// ****************************************************************************

unsigned int
avtIntegralCurveFilter::GenerateAttributeFields() const
{

    // need at least these three attributes
    unsigned int attr = avtStateRecorderIntegralCurve::SAMPLE_POSITION;

    if (storeVelocitiesForLighting)
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VELOCITY;

    // data scalars
    switch( dataValue )
    {
      case IntegralCurveAttributes::Speed:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VELOCITY;
        break;
      case IntegralCurveAttributes::TimeAbsolute:
      case IntegralCurveAttributes::TimeRelative:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_TIME;
        break;
      case IntegralCurveAttributes::Vorticity:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VORTICITY;
        break;
      case IntegralCurveAttributes::ArcLength:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_ARCLENGTH;
        break;
      case IntegralCurveAttributes::Variable:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VARIABLE;
        break;
    }

    // secondary scalar
    unsigned int attribute = avtStateRecorderIntegralCurve::SAMPLE_SECONDARY0;

    for( unsigned int i=0; i<secondaryVariables.size(); ++i )
    {
        attr |= attribute;
        attribute <<= 1;  // Bit shift gives the next enum. 
    }

    // Crop value
    if( atts.GetCropBeginFlag() || atts.GetCropBegin() )
    {
        switch( cropValue )
        {
          case IntegralCurveAttributes::Distance:
            attr |= avtStateRecorderIntegralCurve::SAMPLE_ARCLENGTH;
            break;
          case IntegralCurveAttributes::Time:
            attr |= avtStateRecorderIntegralCurve::SAMPLE_TIME;
            break;
          case IntegralCurveAttributes::StepNumber:
            break;
          default:
            break;
        }
    }

    return attr;
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::SetTermination
//
//  Purpose:
//      Sets the termination criteria for a streamline.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2010
//
// ****************************************************************************

void
avtIntegralCurveFilter::SetTermination(int maxSteps_, bool doDistance_,
                                       double maxDistance_,
                                       bool doTime_, double maxTime_)
{
    maxSteps = maxSteps_;
    doDistance = doDistance_;
    maxDistance = maxDistance_;
    doTime = doTime_;
    maxTime = maxTime_;
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::CreateIntegralCurve
//
//  Purpose:
//      Each derived type of avtPICSFilter must know how to create an integral
//      curve.  The streamline filter creates an avtStateRecorderIntegralCurve.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//    Hank Childs, Mon Oct  4 14:53:13 PDT 2010
//    Create an avtStreamline (not an avtStateRecorderIntegralCurve) and
//    put the termination criteria into the signature.
//
//   Dave Pugmire, Fri Nov  5 15:38:33 EDT 2010
//   Set maxSteps and historyMask.
//
// ****************************************************************************

avtIntegralCurve *
avtIntegralCurveFilter::CreateIntegralCurve()
{
    avtStreamlineIC *ic = new avtStreamlineIC();
    ic->maxSteps = maxSteps;
    ic->historyMask = GenerateAttributeFields();
    return ic;
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::CreateIntegralCurve
//
//  Purpose:
//      Each derived type of avtPICSFilter must know how to create an integral
//      curve.  The streamline filter creates an avtStateRecorderIntegralCurve.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//    Hank Childs, Mon Oct  4 14:53:13 PDT 2010
//    Create an avtStreamline (not an avtStateRecorderIntegralCurve) and
//    put the termination criteria into the signature.
//
//    Hank Childs, Fri Mar  9 16:50:48 PST 2012
//    Handle maximum elapsed time better for pathlines that specify start 
//    times.
//
// ****************************************************************************

avtIntegralCurve *
avtIntegralCurveFilter::CreateIntegralCurve( const avtIVPSolver* model,
                                             const avtIntegralCurve::Direction dir,
                                             const double& t_start,
                                             const avtVector &p_start,
                                             const avtVector &v_start,
                                             long ID ) 
{
    unsigned int attr = GenerateAttributeFields();
    double t_end;

    if (doPathlines)
    {
        if (dir == avtIntegralCurve::DIRECTION_BACKWARD)
            t_end = seedTime0-maxTime;
        else
            t_end = seedTime0+maxTime;
    }
    else
    {
        if (dir == avtIntegralCurve::DIRECTION_BACKWARD)
            t_end = -maxTime;
        else
            t_end = maxTime;
    }

    avtStateRecorderIntegralCurve *rv = 
        new avtStreamlineIC(maxSteps, doDistance, maxDistance, doTime, t_end,
                            attr, model, dir, t_start, p_start, v_start, ID);

    return rv;
}


// ****************************************************************************
// Method: avtIntegralCurveFilter::SetDataValue
//
// Purpose: 
//   Sets data value to use, which determines which auxiliary arrays
//   (if any) are also generated.
//
// Arguments:
//   m : The data value.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 22 12:41:08 PDT 2004
//
// Modifications:
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Added secondary variable.
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetDataValue(int m, const std::string &var)
{
    dataValue = m;
    dataVariable = var;
}

// ****************************************************************************
// Method: avtIntegralCurveFilter::SetCropValue
//
// Purpose: 
//   Sets crop value to use, which determines which auxiliary arrays
//   (if any) are also generated.
//
// Arguments:
//   m : The crop value.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 22 12:41:08 PDT 2004
//
// Modifications:
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Added secondary variable.
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetCropValue(int m)
{
    cropValue = m;
}

// ****************************************************************************
// Method: avtIntegralCurveFilter::SetDisplayGeometry
//
// Purpose: 
//   Sets the integral curve display geometry.
//
// Arguments:
//   d : The display geometry.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 22 14:18:47 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetDisplayGeometry(int d)
{
    displayGeometry = d;
}


// ****************************************************************************
// Method: avtIntegralCurveFilter::SetVelocitySource
//
// Purpose: 
//   Sets the integral curve velocity source.
//
// Arguments:
//   vel : The velocity of the point.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:58:36 PDT 2002
//
// ****************************************************************************

void
avtIntegralCurveFilter::SetVelocitySource(const double *p)
{
  seedVelocity.set(p);
}


// ****************************************************************************
// Method: avtIntegralCurveFilter::SetPointSource
//
// Purpose: 
//   Sets the integral curve point source.
//
// Arguments:
//   pt : The location of the point.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:58:36 PDT 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetPointSource(const double *p)
{
    sourceType = IntegralCurveAttributes::Point;
    points[0].set(p);
}


// ****************************************************************************
// Method: avtIntegralCurveFilter::SetLineSource
//
// Purpose: 
//   Sets the source line endpoints.
//
// Arguments:
//   pt1 : The first line endpoint.
//   pt2 : The second line endpoint.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:58:59 PDT 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetLineSource(const double *p0, const double *p1,
                                      int den, bool rand, int seed, int numPts)
{
    sourceType = IntegralCurveAttributes::Line_;
    points[0].set(p0);
    points[1].set(p1);
    
    numSamplePoints = numPts;
    sampleDensity[0] = den;
    sampleDensity[1] = 0;
    sampleDensity[2] = 0;
    
    randomSamples = rand;
    randomSeed = seed;
}


// ****************************************************************************
// Method: avtIntegralCurveFilter::SetPlaneSource
//
// Purpose: 
//   Sets the plane source information.
//
// Arguments:
//   O : The plane origin.
//   N : The plane normal.
//   U : The plane up axis.
//   R : The plane radius.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:59:47 PDT 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetPlaneSource(double O[3], double N[3], double U[3],
                                       int den1, int den2, double dist1, double dist2,
                                       bool f, 
                                       bool rand, int seed, int numPts)
{
    sourceType = IntegralCurveAttributes::Plane;
    points[0].set(O);
    vectors[0].set(N);
    vectors[1].set(U);
    
    sampleDensity[0] = den1;
    sampleDensity[1] = den2;
    sampleDensity[2] = 0;
    sampleDistance[0] = dist1;
    sampleDistance[1] = dist2;
    sampleDistance[2] = 0.0;
    numSamplePoints = numPts;

    randomSamples = rand;
    randomSeed = seed;
    fill = f;
}

// ****************************************************************************
// Method: avtIntegralCurveFilter::SetCircleSource
//
// Purpose: 
//   Sets the plane source information.
//
// Arguments:
//   O : The plane origin.
//   N : The plane normal.
//   U : The plane up axis.
//   R : The plane radius.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jun 10 10:44:02 EDT 2010
//
// Modifications:
//
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetCircleSource(double O[3], double N[3], double U[3], double r,
                                     int den1, int den2,
                                     bool f, bool rand, int seed, int numPts)
{
    sourceType = IntegralCurveAttributes::Circle;
    points[0].set(O);
    vectors[0].set(N);
    vectors[1].set(U);
    
    sampleDensity[0] = den1;
    sampleDensity[1] = den2;
    sampleDensity[2] = 0;
    sampleDistance[0] = r;
    sampleDistance[1] = 0.0;
    sampleDistance[2] = 0.0;
    numSamplePoints = numPts;

    randomSamples = rand;
    randomSeed = seed;
    fill = f;
}


// ****************************************************************************
// Method: avtIntegralCurveFilter::SetSphereSource
//
// Purpose: 
//   Sets the sphere source information.
//
// Arguments:
//   O : The sphere origin.
//   R : The sphere radius.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:00:34 PST 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetSphereSource(double O[3], double R,
                                     int den1, int den2, int den3,
                                     bool f, bool rand, int seed, int numPts)
{
    sourceType = IntegralCurveAttributes::Sphere;
    points[0].set(O);
    sampleDistance[0] = R;
    sampleDistance[1] = 0.0;
    sampleDistance[2] = 0.0;
    sampleDensity[0] = den1;
    sampleDensity[1] = den2;
    sampleDensity[2] = den3;

    numSamplePoints = numPts;
    randomSamples = rand;
    randomSeed = seed;
    fill = f;
}


// ****************************************************************************
// Method: avtIntegralCurveFilter::SetBoxSource
//
// Purpose: 
//   Sets the box source information.
//
// Arguments:
//   E : The box extents.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:01:11 PST 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetBoxSource(double E[6], bool wholeBox,
                                  int den1, int den2, int den3,
                                  bool f, bool rand, int seed, int numPts)
{
    sourceType = IntegralCurveAttributes::Box;
    points[0].set(E[0], E[2], E[4]);
    points[1].set(E[1], E[3], E[5]);

    sampleDensity[0] = den1;
    sampleDensity[1] = den2;
    sampleDensity[2] = den3;

    useBBox = wholeBox;
    numSamplePoints = numPts;
    randomSamples = rand;
    randomSeed = seed;
    fill = f;
}


// ****************************************************************************
// Method: avtIntegralCurveFilter::SetPointListSource
//
// Purpose: 
//   Sets the integral curve point list source.
//
// Arguments:
//   ptlist : A list of points
//
// Programmer: Hank Childs
// Creation:   May 3, 2009
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtIntegralCurveFilter::SetPointListSource(const std::vector<double> &ptList)
{
    sourceType = IntegralCurveAttributes::PointList;
    listOfPoints = ptList;
}


void
avtIntegralCurveFilter::SetSelectionSource(std::string nm,
                                        int stride,
                                        bool random, int seed, int numPts)

{
    sourceType = IntegralCurveAttributes::Selection;
    sourceSelection = nm;
    sampleDensity[0] = stride;
    numSamplePoints = numPts;
    randomSamples = random;
    randomSeed = seed;
}

// ****************************************************************************
// Method: avtIntegralCurveFilter::SeedInfoString
//
// Purpose: 
//   Get info string on seeds.
//
// Arguments:
//   
//
// Programmer: Dave Pugmire
// Creation:   Fri Apr  3 09:18:03 EDT 2009
//
// Modifications:
//   
//   Hank Childs, Sun May  3 12:42:38 CDT 2009
//   Add case for point lists.
//
//   Dave Pugmire (for Christoph Garth), Wed Jan 20 09:28:59 EST 2010
//   Add circle source.
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//
// ****************************************************************************

std::string
avtIntegralCurveFilter::SeedInfoString() const
{
    char buff[256];
    if (sourceType == IntegralCurveAttributes::Point)
        sprintf(buff, "Point [%g %g %g]", 
                points[0].x, points[0].y, points[0].z);
    else if (sourceType == IntegralCurveAttributes::Line_)
        sprintf(buff, "Line [%g %g %g] [%g %g %g] D: %d",
                points[0].x, points[0].y, points[0].z,
                points[1].x, points[1].y, points[1].z, sampleDensity[0]);
    else if (sourceType == IntegralCurveAttributes::Plane)
        sprintf(buff, "Plane O[%g %g %g] N[%g %g %g] D: %d %d",
                points[0].x, points[0].y, points[0].z,
                vectors[0].x, vectors[0].y, vectors[0].z,
                sampleDensity[0], sampleDensity[1]);
    else if (sourceType == IntegralCurveAttributes::Sphere)
        sprintf(buff, "Sphere [%g %g %g] %g D: %d %d",
                points[0].x, points[0].y, points[0].z, sampleDistance[0],
                sampleDensity[0], sampleDensity[1]);
    else if (sourceType == IntegralCurveAttributes::Box)
        sprintf(buff, "Box [%g %g] [%g %g] [%g %g] D: %d %d %d",
                points[0].x, points[1].x,
                points[0].y, points[1].y,
                points[0].z, points[1].z,
                sampleDensity[0], sampleDensity[1], sampleDensity[2]);
    else if (sourceType == IntegralCurveAttributes::Circle)
        sprintf(buff, "Cirlce O[%g %g %g] N[%g %g %g] R: %g D: %d %d",
                points[0].x, points[0].y, points[0].z,
                vectors[0].x, vectors[0].y, vectors[0].z,
                sampleDistance[0],
                sampleDensity[0], sampleDensity[1]);
    else if (sourceType == IntegralCurveAttributes::PointList)
        strcpy(buff, "Point list [points not printed]");
    else
        sprintf(buff, "%s", "UNKNOWN");
    
    std::string str = buff;
    return str;
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::GetInitialVelocities
//
//  Purpose:
//      Get the seed velocities out of the attributes.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2008
//
// ****************************************************************************

std::vector<avtVector>
avtIntegralCurveFilter::GetInitialVelocities(void)
{
    std::vector<avtVector> seedVels;

    seedVels.push_back( seedVelocity );

    return seedVels;
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::GetInitialLocations
//
//  Purpose:
//      Get the seed points out of the attributes.
//
//  Programmer: Hank Childs (harvested from GetStreamlinesFromInitialSeeds by
//                           David Pugmire)
//  Creation:   June 5, 2008
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Nov 10 09:20:06 EST 2010
//   Handle 2D datasets better.
//
// ****************************************************************************

std::vector<avtVector>
avtIntegralCurveFilter::GetInitialLocations(void)
{
    std::vector<avtVector> seedPts;
    
    if (randomSamples)
        srand(randomSeed);

    // Add seed points based on the source.
    if(sourceType == IntegralCurveAttributes::Point)
        GenerateSeedPointsFromPoint(seedPts);
    else if(sourceType == IntegralCurveAttributes::Line_)
        GenerateSeedPointsFromLine(seedPts);
    else if(sourceType == IntegralCurveAttributes::Plane)
        GenerateSeedPointsFromPlane(seedPts);
    else if(sourceType == IntegralCurveAttributes::Sphere)
        GenerateSeedPointsFromSphere(seedPts);
    else if(sourceType == IntegralCurveAttributes::Box)
        GenerateSeedPointsFromBox(seedPts);
    else if(sourceType == IntegralCurveAttributes::Circle)
        GenerateSeedPointsFromCircle(seedPts);
    else if(sourceType == IntegralCurveAttributes::PointList)
        GenerateSeedPointsFromPointList(seedPts);
    else if (sourceType == IntegralCurveAttributes::Selection)
        GenerateSeedPointsFromSelection(seedPts);

    //Check for 2D input.
    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        std::vector<avtVector>::iterator it;
        for (it = seedPts.begin(); it != seedPts.end(); it++)
            (*it)[2] = 0.0f;
    }

    return seedPts;
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::GenerateSeedPointsFromPoint
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
// ****************************************************************************

void
avtIntegralCurveFilter::GenerateSeedPointsFromPoint(std::vector<avtVector> &pts)
{
    pts.push_back(points[0]);
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::GenerateSeedPointsFromLine
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
// ****************************************************************************

void
avtIntegralCurveFilter::GenerateSeedPointsFromLine(std::vector<avtVector> &pts)
{
    avtVector v = points[1]-points[0];

    if (randomSamples)
    {
        for (int i = 0; i < numSamplePoints; i++)
        {
            avtVector p = points[0] + random01()*v;
            pts.push_back(p);
        }
    }
    else
    {
        double t = 0.0, dt;
        if (sampleDensity[0] == 1)
        {
            t = 0.5;
            dt = 0.5;
        }
        else
            dt = 1.0/(double)(sampleDensity[0]-1);
    
        for (int i = 0; i < sampleDensity[0]; i++)
        {
            avtVector p = points[0] + t*v;
            pts.push_back(p);
            t = t+dt;
        }
    }
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::GenerateSeedPointsFromPlane
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Jun 23 16:44:36 EDT 2010
//   Fix the centering.
//
//   Hank Childs, Sun Jul 17 17:02:36 PDT 2011
//   Space the distance in the Y direction based on the second sample distance
//   (we were doubling up the distance for the X-direction).
//
// ****************************************************************************

void
avtIntegralCurveFilter::GenerateSeedPointsFromPlane(std::vector<avtVector> &pts)
{
    //Generate all points on a plane at the origin with Normal=Z.
    //Use the following matrix to xform them to the user specified plane.
    
    avtVector X0(1,0,0), Y0(0,1,0), Z0(0,0,1), C0(0,0,0);
    avtVector Y1=vectors[1], Z1=vectors[0], C1=points[0];

    avtVector X1 = Y1.cross(Z1);
    avtMatrix m = avtMatrix::CreateFrameToFrameConversion(X1, Y1, Z1, C1,
                                                          X0, Y0, Z0, C0);
    
    float x0 = -(sampleDistance[0]/2.0);
    float y0 = -(sampleDistance[1]/2.0);
    float x1 = (sampleDistance[0]/2.0);
    float y1 = (sampleDistance[1]/2.0);

    if (randomSamples)
    {
        float dX = x1-x0, dY = y1-y0;
        if (!fill)
        {
            // There are 4 sides. Create a vector that we will shuffle each time.
            std::vector<int> sides(4);
            for (int i = 0; i < 4; i++)
                sides[i] = i;

            avtVector p;
            for (int i = 0; i < numSamplePoints; i++)
            {
                random_shuffle(sides.begin(), sides.end());
                if (sides[0] == 0) //Bottom side.
                    p.set(x0 + random01()*dX, y0, 0.0f);
                else if (sides[0] == 1) //Top side.
                    p.set(x0 + random01()*dX, y1, 0.0f);
                else if (sides[0] == 2) //Right side.
                    p.set(x0, y0+random01()*dY, 0.0f);
                else //Left side.
                    p.set(x1, y0+random01()*dY, 0.0f);
                
                p = m*p;
                pts.push_back(p);
            }
        }
        else
        {
            for (int i = 0; i < numSamplePoints; i++)
            {
                avtVector p(x0 + random01()*dX,
                            y0 + random01()*dY,
                            0.0);
                p = m*p;
                pts.push_back(p);
            }
        }
    }
    else
    {
        float dX = (x1-x0)/(float)(sampleDensity[0]-1);
        float dY = (y1-y0)/(float)(sampleDensity[1]-1);

        for (int x = 0; x < sampleDensity[0]; x++)
        {
            for (int y = 0; y < sampleDensity[1]; y++)
            {
                if (!fill &&
                    !((x == 0 || x == sampleDensity[0]-1) ||
                      (y == 0 || y == sampleDensity[1]-1)))
                {
                    continue;
                }
                
                avtVector p(x0+((float)x*dX), 
                            y0+((float)y*dY),
                            0.0);

                p = m*p;
                pts.push_back(p);
            }
        }
    }
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::GenerateSeedPointsFromCircle
//
//  Purpose:
//      
//
//  Programmer: Christoph Garth
//  Creation:   January 20, 2010
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Jun 23 16:44:36 EDT 2010
//   Add circle center for interior sampling.
//
// ****************************************************************************

void
avtIntegralCurveFilter::GenerateSeedPointsFromCircle(std::vector<avtVector> &pts)
{
    //Generate all points on a plane at the origin with Normal=Z.
    //Use the following matrix to xform them to the user specified plane.
    
    avtVector X0(1,0,0), Y0(0,1,0), Z0(0,0,1), C0(0,0,0);
    avtVector Y1=vectors[1], Z1=vectors[0], C1=points[0];
    avtVector X1 = Y1.cross(Z1);
    avtMatrix m = avtMatrix::CreateFrameToFrameConversion(X1, Y1, Z1, C1,
                                                          X0, Y0, Z0, C0);
    float R = sampleDistance[0];
    if (randomSamples)
    {
        if (fill)
        {
            int n = numSamplePoints;
            while (n)
            {
                //Randomly sample a unit square, check if pt in circle.
                float x = random_11(), y = random_11();
                if (x*x + y*y <= 1.0) //inside the circle!
                {
                    avtVector p = m * avtVector(x*R, y*R, 0.0);
                    pts.push_back(p);
                    n--;
                }
            }
        }
        else
        {
            float TWO_PI = M_PI*2.0f;
            for (int i = 0; i < numSamplePoints; i++)
            {
                float theta = random01() * TWO_PI;
                avtVector p(cos(theta)*R, sin(theta)*R, 0.0);
                p = m*p;
                pts.push_back(p);
            }
        }
    }
    else
    {
        float TWO_PI = M_PI*2.0f;
        if (fill)
        {
            float dTheta = TWO_PI / (float)sampleDensity[0];
            float dR = R/(float)sampleDensity[1];

            float theta = 0.0;                
            for (int i = 0; i < sampleDensity[0]; i++)
            {
                float r = dR;
                for (int j = 0; j < sampleDensity[1]; j++)
                {
                    avtVector p(cos(theta)*r, sin(theta)*r, 0.0);
                    p = m*p;
                    pts.push_back(p);
                    r += dR;
                }
                theta += dTheta;
                pts.push_back(points[0]);
            }
        }
        else
        {
            for (int i = 0; i < sampleDensity[0]; i++)
            {
                float t = (TWO_PI*i) / (float)sampleDensity[0];
                avtVector p(cos(t)*R, sin(t)*R, 0.0);
                p = m*p;
                pts.push_back(p);
            }
        }
    }
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::GenerateSeedPointsFromSphere
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Jun 23 16:44:36 EDT 2010
//   Bug fix for random sampling on a sphere. Implment uniform interior sampling.
//
// ****************************************************************************

void
avtIntegralCurveFilter::GenerateSeedPointsFromSphere(std::vector<avtVector> &pts)
{
    float R = sampleDistance[0];
    if (randomSamples)
    {
        if (fill)
        {
            int n = numSamplePoints;
            while (n)
            {
                //Randomly sample a unit cube, check if pt in sphere.
                float x = random_11(), y = random_11(), z = random_11();
                if (x*x + y*y  + z*z <= 1.0) //inside the sphere!
                {
                    avtVector p = avtVector(x*R, y*R, z*R) + points[0];
                    pts.push_back(p);
                    n--;
                }
            }
        }
        else
        {
            float TWO_PI = M_PI*2.0f;
            for (int i = 0; i < numSamplePoints; i++)
            {
                float theta = random01()*TWO_PI;
                float u = random_11();
                float x = sqrt(1.0-(u*u));
                avtVector p(cos(theta)*x, sin(theta)*x, u);
                p.normalize();
                p *= R;
                pts.push_back(p+points[0]);
            }
        }
    }
    else
    {
        vtkSphereSource* sphere = vtkSphereSource::New();
        sphere->SetCenter(points[0].x, points[0].y, points[0].z);
        sphere->SetRadius(R);
        sphere->SetLatLongTessellation(1);
        double t = double(30 - sampleDensity[0]) / 29.;
        double angle = t * 3. + (1. - t) * 30.;
        sphere->SetPhiResolution(int(angle));

        t = double(30 - sampleDensity[1]) / 29.;
        angle = t * 3. + (1. - t) * 30.;
        sphere->SetThetaResolution(int(angle));

        if (fill)
        {
            float dR = R/(float)sampleDensity[2];
            float r = dR;
            for (int i = 0; i < sampleDensity[2]; i++)
            {
                sphere->SetRadius(r);
                sphere->Update();

                for (int j = 0; j < sphere->GetOutput()->GetNumberOfPoints(); j++)
                {
                    double *pt = sphere->GetOutput()->GetPoint(j);
                    avtVector p(pt[0], pt[1], pt[2]);
                    pts.push_back(p);
                }
                r = r+dR;
            }
            //Add center, R=0 sample.
            pts.push_back(points[0]);

        }
        else //LAT-LONG
        {
            sphere->Update();
            for (int i = 0; i < sphere->GetOutput()->GetNumberOfPoints(); i++)
            {
                double *pt = sphere->GetOutput()->GetPoint(i);
                avtVector p(pt[0], pt[1], pt[2]);
                pts.push_back(p);
            }
        }
        
        sphere->Delete();
    }
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::GenerateSeedPointsFromBox
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Nov 10 09:20:32 EST 2010
//   If box sampling is 1, use the mid value.
//
// ****************************************************************************

void
avtIntegralCurveFilter::GenerateSeedPointsFromBox(std::vector<avtVector> &pts)
{
    if (useBBox)
    {
        double bbox[6];
        intervalTree->GetExtents(bbox);
        points[0].set(bbox[0], bbox[2], bbox[4]);
        points[1].set(bbox[1], bbox[3], bbox[5]);
    }

    avtVector diff = points[1]-points[0];

    if (randomSamples)
    {
        if (fill)
        {
            for (int i = 0; i < numSamplePoints; i++)
            {
                avtVector p(points[0].x + (diff.x * random01()),
                            points[0].y + (diff.y * random01()),
                            points[0].z + (diff.z * random01()));
                pts.push_back(p);
            }
        }
        else
        {
            // There are 6 faces. Create a vector that we will shuffle each time.
            std::vector<int> faces(6);
            for (int i = 0; i < 6; i++)
                faces[i] = i;
            
            avtVector p;
            for (int i = 0; i < numSamplePoints; i++)
            {
                random_shuffle(faces.begin(), faces.end());
                if (faces[0] == 0) //X=0 face.
                    p.set(points[0].x,
                          points[0].y + (diff.y * random01()),
                          points[0].z + (diff.z * random01()));
                else if (faces[0] == 1) //X=1 face.
                    p.set(points[1].x,
                          points[0].y + (diff.y * random01()),
                          points[0].z + (diff.z * random01()));
                else if (faces[0] == 2) //Y=0 face.
                    p.set(points[0].x + (diff.x * random01()),
                          points[0].y,
                          points[0].z + (diff.z * random01()));
                else if (faces[0] == 3) //Y=1 face.
                    p.set(points[0].x + (diff.x * random01()),
                          points[1].y,
                          points[0].z + (diff.z * random01()));
                else if (faces[0] == 4) //Z=0 face.
                    p.set(points[0].x + (diff.x * random01()),
                          points[0].y + (diff.y * random01()),
                          points[0].z);
                else if (faces[0] == 5) //Z=1 face.
                    p.set(points[0].x + (diff.x * random01()),
                          points[0].y + (diff.y * random01()),
                          points[1].z);
                pts.push_back(p);
            }
        }
    }
    else
    {
        // If sample density is 1, sample at the mid point.
        diff.x /= (sampleDensity[0] == 1 ? 2.0 : (sampleDensity[0]-1));
        diff.y /= (sampleDensity[1] == 1 ? 2.0 : (sampleDensity[1]-1));
        diff.z /= (sampleDensity[2] == 1 ? 2.0 : (sampleDensity[2]-1));

        if (sampleDensity[0] == 1)
            points[0].x += diff.x;
        if (sampleDensity[1] == 1)
            points[0].y += diff.y;
        if (sampleDensity[2] == 1)
            points[0].z += diff.z;

        if (fill)
        {
            for (int i = 0; i < sampleDensity[0]; i++)
                for (int j = 0; j < sampleDensity[1]; j++)
                    for (int k = 0; k < sampleDensity[2]; k++)
                    {
                        avtVector p(points[0].x + i*diff.x,
                                    points[0].y + j*diff.y,
                                    points[0].z + k*diff.z);
                        pts.push_back(p);
                    }
        }
        else
        {
            for (int i = 0; i < sampleDensity[0]; i++)
                for (int j = 0; j < sampleDensity[1]; j++)
                    for (int k = 0; k < sampleDensity[2]; k++)
                    {
                        if ((i == 0 || i == sampleDensity[0]-1) ||
                            (j == 0 || j == sampleDensity[1]-1) ||
                            (k == 0 || k == sampleDensity[2]-1))
                        {
                            avtVector p(points[0].x + i*diff.x,
                                        points[0].y + j*diff.y,
                                        points[0].z + k*diff.z);
                            pts.push_back(p);
                        }
                    }
        }
    }
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::GenerateSeedPointsFromPointList
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
// ****************************************************************************

void
avtIntegralCurveFilter::GenerateSeedPointsFromPointList(std::vector<avtVector> &pts)
{
    if ((listOfPoints.size() % 3) != 0)
    {
        EXCEPTION1(VisItException, "The seed points for the integral curve "
                   "are incorrectly specified.  The number of values must be a "
                   "multiple of 3 (X, Y, Z).");
    }
    
    size_t npts = listOfPoints.size() / 3;
    for (size_t i = 0 ; i < npts ; i++)
    {
        avtVector p(listOfPoints[3*i], listOfPoints[3*i+1], listOfPoints[3*i+2]);
        pts.push_back(p);
    }
}

void
avtIntegralCurveFilter::GenerateSeedPointsFromSelection(std::vector<avtVector> &pts)
{
    avtNamedSelectionManager *nsm = avtNamedSelectionManager::GetInstance();
    
    avtNamedSelection *sel = nsm->GetNamedSelection(sourceSelection.c_str());
    if (sel == NULL)
        return;
    
    std::vector<avtVector> allPts;
    sel->GetMatchingLocations(allPts);
    if (randomSamples)
    {
        random_shuffle(allPts.begin(), allPts.end());
        for (int i = 0; i < numSamplePoints; i++)
            pts.push_back(allPts[i]);
    }
    else
    {
        size_t npts = allPts.size();
        for (size_t i = 0 ; i < npts ; i+= sampleDensity[0])
            pts.push_back(allPts[i]);
    }
}


// ****************************************************************************
//  Method: avtIntegralCurveFilter::GetFieldForDomain
//
//  Purpose:
//      Calls avtPICSFilter::GetFieldForDomain and enables scalar 
//      variables according to dataValue and secondaryVariables.
//
//  Programmer: Christoph Garth
//  Creation:   July 14, 2010
//
//  Modifications:
//
//   Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//   Add vary tube radius by variable.
//
// ****************************************************************************

avtIVPField* 
avtIntegralCurveFilter::GetFieldForDomain(const BlockIDType& dom, vtkDataSet* ds)
{
    avtIVPField* field = avtPICSFilter::GetFieldForDomain( dom, ds );

    //  The dataValue variable must always be after all of the
    //  secondary variables.
    if( dataValue == IntegralCurveAttributes::Variable && 
        !dataVariable.empty() )
    {
        field->SetScalarVariable( (unsigned char)secondaryVariables.size(), dataVariable );
    }

    for( unsigned int i=0; i<secondaryVariables.size(); ++i )
    {
        field->SetScalarVariable( i, secondaryVariables[i] );
    }

    return field;
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::ReportWarnings() 
//
//  Purpose:
//      Reports any potential integration warnings
//
//  Programmer: Allen Sanderson
//  Creation:   20 August 2013
//
//  Modifications:
//
//   Dave Pugmire, Thu May  1 09:40:05 EDT 2014
//   Fix that would case the filter to hang if one process had no ICs.
//
// ****************************************************************************

void
avtIntegralCurveFilter::ReportWarnings(std::vector<avtIntegralCurve *> &ics)
{
    int numICs = (int)ics.size();
//    int numPts = 0;
    int numEarlyTerminators = 0;
    int numStiff = 0;
    int numCritPts = 0;

    if (DebugStream::Level5())
    {
        debug5 << "::CreateIntegralCurveOutput " << ics.size() << endl;
    }

    //See how many pts, ics we have so we can preallocate everything.
    for (int i = 0; i < numICs; i++)
    {
        avtStreamlineIC *ic = dynamic_cast<avtStreamlineIC*>(ics[i]);

        // NOT USED ??????????????????????????
        // size_t numSamps = (ic ? ic->GetNumberOfSamples() : 0);
        // if (numSamps > 1)
        //     numPts += numSamps;

        if (ic->TerminatedBecauseOfMaxSteps())
        {
            if (ic->SpeedAtTermination() <= criticalPointThreshold)
                numCritPts++;
            else
                numEarlyTerminators++;
        }

        if (ic->EncounteredNumericalProblems())
            numStiff++;
    }

    char str[4096] = "";

    if ((doDistance || doTime) && issueWarningForMaxStepsTermination)
    {
        SumIntAcrossAllProcessors(numEarlyTerminators);
        if (numEarlyTerminators > 0)
        {
          SNPRINTF(str, 4096,
                   "%s\n%d of your integral curves terminated because they "
                   "reached the maximum number of steps.  This may be indicative of your "
                   "time or distance criteria being too large or of other attributes being "
                   "set incorrectly (example: your step size is too small).  If you are "
                   "confident in your settings and want the particles to advect farther, "
                   "you should increase the maximum number of steps.  If you want to disable "
                   "this message, you can do this under the Advaced tab."
                   "  Note that this message does not mean that an error has occurred; it simply "
                   "means that VisIt stopped advecting particles because it reached the maximum "
                   "number of steps. (That said, this case happens most often when other attributes "
                   "are set incorrectly.)\n", str, numEarlyTerminators);
        }
    }

    if (issueWarningForCriticalPoints)
    {
        SumIntAcrossAllProcessors(numCritPts);
        if (numCritPts > 0)
        {
            SNPRINTF(str, 4096, 
                     "%s\n%d of your integral curves circled round and round a critical point (a zero"
                     " velocity location).  Normally, VisIt is able to advect the particle "
                     "to the critical point location and terminate.  However, VisIt was not able "
                     "to do this for these particles due to numerical issues.  In all likelihood, "
                     "additional steps will _not_ help this problem and only cause execution to "
                     "take longer.  If you want to disable this message, you can do this under "
                     "the Advanced tab.\n", str, numCritPts);
        }
    }

    if (issueWarningForStiffness)
    {
        SumIntAcrossAllProcessors(numStiff);
        if (numStiff > 0)
        {
            SNPRINTF(str, 4096, 
                     "%s\n%d of your integral curves were unable to advect because of \"stiffness\".  "
                     "When one component of a velocity field varies quickly and another stays "
                     "relatively constant, then it is not possible to choose step sizes that "
                     "remain within tolerances.  This condition is referred to as stiffness and "
                     "VisIt stops advecting in this case.  If you want to disable this message, "
                     "you can do this under the Advanced tab.\n", str,numStiff);
        }
    }

    if( strlen( str ) )
      avtCallback::IssueWarning(str);
}

// ****************************************************************************
//  Method: avtIntegralCurveFilter::CreateIntegralCurveOutput
//
//  Purpose:
//      Create the VTK poly data output from the streamline.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension.
//
//   Hank Childs, Tue Dec  2 13:51:19 PST 2008
//   Removed this method from avtIntegralCurveFilter to 
//   avtIntegralCurveFilter.  The motivation for this refactoring was to
//   allow other modules (Poincare) to inherit from avtIntegralCurveFilter and
//   use its parallel-aware goodness.
//
//   Dave Pugmire, Tue Dec 23 13:52:42 EST 2008
//   Removed ReportStatistics from this method.
//
//   Dave Pugmire, Tue Feb  3 11:00:54 EST 2009
//   Changed debugs.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Rename this method to reflect the new emphasis in particle advection, as
//   opposed to streamlines.
//
//   Dave Pugmire, Tue Sep 28 10:41:00 EDT 2010
//   Optimize the creation of vtkPolyData.
//
//   Dave Pugmire, Wed Sep 29 14:57:59 EDT 2010
//   Initialize scalar array if data value is solid.
//
//   Hank Childs, Wed Oct  6 20:07:28 PDT 2010
//   Initialize referenceTypeForDisplay.
//
//   Hank Childs, Fri Oct  8 14:57:13 PDT 2010
//   Check to see if any curves terminated because of the steps criteria.
//
//   Dave Pugmire, Thu Dec  2 12:49:33 EST 2010
//   Can't early return until after collective communication.
//
//   Hank Childs, Sun Dec  5 10:43:57 PST 2010
//   Issue warnings for more problems.
//
//   Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//   Add vary tube radius by variable.
//
//   Dave Pugmire, Mon Feb 21 08:22:30 EST 2011
//   Set the dataValue by correlation distance.
//
// ****************************************************************************
void
avtIntegralCurveFilter::CreateIntegralCurveOutput(std::vector<avtIntegralCurve *> &ics)
{
    if (ics.size() == 0)
        return;

    int numICs = (int)ics.size(), numPts = 0;

    if (DebugStream::Level5())
    {
        debug5 << "::CreateIntegralCurveOutput " << ics.size() << endl;
    }

    vtkAppendPolyData *append = vtkAppendPolyData::New();

    //Make a polydata.
    vtkPoints     *points   = vtkPoints::New();
    vtkCellArray  *lines    = vtkCellArray::New();
    vtkFloatArray *scalars  = vtkFloatArray::New();
    vtkFloatArray *tangents = vtkFloatArray::New();
    vtkFloatArray *thetas   = NULL;

    std::vector< vtkFloatArray * > secondarys;
    secondarys.resize(secondaryVariables.size());

    lines->Allocate(numICs);
    points->Allocate(numPts);
    scalars->Allocate(numPts);
    tangents->SetNumberOfComponents(3);
    tangents->SetNumberOfTuples(numPts);
    
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->SetLines(lines);
    scalars->SetName(colorVarArrayName.c_str());
    tangents->SetName(tangentsArrayName.c_str());

    pd->GetPointData()->SetScalars(scalars);
    pd->GetPointData()->AddArray(scalars);
    pd->GetPointData()->AddArray(tangents);

    // theta scalars
    if(displayGeometry == IntegralCurveAttributes::Ribbons)
    {
        thetas = vtkFloatArray::New();
        thetas->Allocate(numPts);
        thetas->SetName(thetaArrayName.c_str());
        pd->GetPointData()->AddArray(thetas);
    }

    // secondary scalars
    for( unsigned int i=0; i<secondaryVariables.size(); ++i )
    {
        secondarys[i] = vtkFloatArray::New();
        secondarys[i]->Allocate(numPts);
        secondarys[i]->SetName(secondaryVariables[i].c_str());
        pd->GetPointData()->AddArray(secondarys[i]);
    }

    if(displayGeometry == IntegralCurveAttributes::Tubes)
      ProcessVaryTubeRadiusByScalar(ics);

    double correlationDistMinDistToUse = correlationDistanceMinDist;
    double correlationDistAngTolToUse = 0.0;

    if (dataValue == IntegralCurveAttributes::CorrelationDistance)
    {
        if (correlationDistanceDoBBox)
            correlationDistMinDistToUse *= GetLengthScale();
        correlationDistAngTolToUse = cos(correlationDistanceAngTol *M_PI/180.0);
    }

    vtkIdType pIdx = 0;

    double cropBeginFlag  = atts.GetCropBeginFlag();
    double cropBeginValue = atts.GetCropBegin();
    double cropEndFlag    = atts.GetCropEndFlag();
    double cropEndValue   = atts.GetCropEnd();

    for (int i = 0; i < numICs; i++)
    {
        avtStateRecorderIntegralCurve *ic =
          dynamic_cast<avtStateRecorderIntegralCurve*>(ics[i]);

        size_t nSamples = (ic ? ic->GetNumberOfSamples() : 0);
        if (nSamples <= 1)
            continue;

        // When cropping save off whether one needs to interpolate and
        // the parameter values at the end points. The beginning and
        // ending indexes along the curve.
        vtkIdType cropBeginIndex = 0, cropEndIndex = 0;
        bool cropBeginInterpolate = false, cropEndInterpolate = false;
        double cropBeginParam = 0.0, cropEndParam = 1.0;

        int  beginIndex = (cropBeginFlag ? -1 : 0);
        int  endIndex   = (cropEndFlag   ? -1 : (int)nSamples-1);

        // Last crop value is need to obtain interpolation parametrization
        // values.
        double last_crop_value = 0, crop_value = 0;

        int totalSamples = 0;

        // If cropping determine how many points will be on the curve.
        if (cropBeginFlag || cropEndFlag)
        {
            for (size_t j = 0; j < nSamples; j++)
            {
                avtStateRecorderIntegralCurve::Sample s = ic->GetSample(j);

                // Save the last crop value for the interpolation.
                last_crop_value = crop_value;

                // Get the current crop value.
                switch (cropValue)
                {
                  case IntegralCurveAttributes::Distance:
                    crop_value = s.arclength;
                    break;
                  case IntegralCurveAttributes::Time:
                    crop_value = s.time;
                    break;
                  case IntegralCurveAttributes::StepNumber:
                    crop_value = j;
                    break; 
                }

                // Beginning or end point matches the crop value so
                // take those indexes.  In these cases no
                // interpolation will be needed.
                if( cropBeginValue == crop_value )
                {
                  if( cropBeginFlag && beginIndex < 0 )
                    beginIndex = (int)j;

                  if( cropEndFlag )
                    endIndex = (int)j;
                }

                // Within range so take those indexes - this is an
                // inclusive check as such interpolation will be
                // needed on the beginning and end points.
                else if( cropBeginValue < crop_value &&
                         crop_value < cropEndValue )
                {
                  if( cropBeginFlag && beginIndex < 0 )
                  {
                    // If not the first point interpolation will be needed.
                    if( 0 < j )
                    {
                      beginIndex = (int)j - 1;
                      cropBeginInterpolate = true;
                    }
                    // This happens when the crop value is smaller
                    // than the first value along the curve.
                    else
                      beginIndex = (int)j;
                  }

                  if( cropEndFlag )
                  {
                    // If not the last point interpolation will be needed.
                    if( j < nSamples-1 )
                    {
                      cropEndInterpolate = true;
                      endIndex = (int)j + 1;
                    }
                    // This happens when the crop value is larger
                    // than the last value along the curve.
                    else
                      endIndex = (int)j;
                  }
                }

                // Get the parameter for the beginning interpolation value. 
                if( cropBeginInterpolate &&
                    last_crop_value < cropBeginValue &&
                    cropBeginValue < crop_value )                   
                {
                    cropBeginParam = (cropBeginValue - last_crop_value) /
                      (crop_value - last_crop_value);
                }

                // Get the parameter for the ending interpolation value. 
                if( cropEndInterpolate &&
                    last_crop_value < cropEndValue &&
                    cropEndValue < crop_value )             
                {
                    cropEndParam = (cropEndValue - last_crop_value) /
                      (crop_value - last_crop_value);
                }
            }

            totalSamples = endIndex - beginIndex + 1;
        }
        else
          totalSamples = (int)nSamples;

        if( totalSamples < 2 )
          continue;
        
        // Create the new vtkPolyline
        vtkPolyLine *line = vtkPolyLine::New();
        line->GetPointIds()->SetNumberOfIds(totalSamples);

        float theta = 0.0, lastTime = 0.0;

        avtStateRecorderIntegralCurve::Sample s, s0 = ic->GetSample(0);

        double startTime = s0.time;
        double distance = 0;

        if( //dataValue == IntegralCurveAttributes::ArcLength ||
            dataValue == IntegralCurveAttributes::AverageDistanceFromSeed ||
            dataValue == IntegralCurveAttributes::Difference )
        {
          for (size_t j = 0; j < nSamples; j++)
          {
            s = ic->GetSample(j);

//          if( dataValue == IntegralCurveAttributes::ArcLength )
//            distance += s.arclength;

            if( dataValue == IntegralCurveAttributes::AverageDistanceFromSeed )
              distance += (s.position - s0.position).length();
            else if( dataValue == IntegralCurveAttributes::Difference )
//            distance += fabs(s.position.y - 100);
              distance += s.arclength;
          }

          if( dataValue == IntegralCurveAttributes::AverageDistanceFromSeed )
            distance /= nSamples;
        }
        
        for (int j = beginIndex; j <= endIndex; j++)
        {
          if( cropBeginInterpolate && j == beginIndex )
            cropBeginIndex = pIdx;

          if( cropEndInterpolate && j == endIndex )
            cropEndIndex = pIdx;

            s = ic->GetSample(j);

            line->GetPointIds()->SetId(j-beginIndex, pIdx);

            // Points
            points->InsertPoint(pIdx,
                                s.position.x, s.position.y, s.position.z);

            float speed = s.velocity.length();

            if (speed > 0)
                s.velocity *= 1.0f/speed;

            // Tangents
            tangents->
              InsertTuple3(pIdx, s.velocity.x, s.velocity.y, s.velocity.z);

            double data_value = 0.0f;

            // color scalars
            switch (dataValue)
            {
              case IntegralCurveAttributes::Solid:
                data_value = 0.0f;
                break;
              case IntegralCurveAttributes::SeedPointID:
                data_value = ic->id;
                break;
              case IntegralCurveAttributes::Speed:
                data_value = speed;
                break;
              case IntegralCurveAttributes::Vorticity:
                data_value = s.vorticity;
                break;
              case IntegralCurveAttributes::ArcLength:
                data_value = s.arclength;
//                data_value = distance;
                break;
              case IntegralCurveAttributes::TimeAbsolute:
                data_value = s.time;
                break;
              case IntegralCurveAttributes::TimeRelative:
                data_value = s.time - startTime;
                break;
              case IntegralCurveAttributes::AverageDistanceFromSeed:
//              data_value = (s.position - s0.position).length();
                data_value = distance;
                break;
              case IntegralCurveAttributes::Variable:
                data_value = s.variable;
                break;
              case IntegralCurveAttributes::CorrelationDistance:
                data_value =
                  ComputeCorrelationDistance(j, ic,
                                             correlationDistAngTolToUse,
                                             correlationDistMinDistToUse);
                break;
              case IntegralCurveAttributes::Difference:
                data_value = distance;
                break;
            }

            scalars->InsertTuple1(pIdx, data_value);

            // theta scalars
            if(displayGeometry == IntegralCurveAttributes::Ribbons)
            {
                float scaledVort = s.vorticity * (lastTime-s.time);
                theta += scaledVort;
                thetas->InsertTuple1(pIdx, theta);
                lastTime = s.time;
            }

            // secondary scalars
            for( unsigned int i=0; i<secondaryVariables.size(); ++i )
                secondarys[i]->InsertTuple1(pIdx, s.secondarys[i]);

            // if( atts.GetShowPoints() )
            // {
            //   double pt[3] = {s.position.x, s.position.y, s.position.z};

            //   vtkPolyData *vert = CreateVTKVertex(pt, data_value,
            //                                       secondaryVariables,
            //                                       s.secondarys );
            //   append->AddInputData(vert);
            //   vert->Delete();
            // }

            pIdx++;
        }

        // When cropping the first and last values are the oringal
        // values which may be outside crop rnge. As such, check for
        // interpolation and replace the original values with the
        // interpolated values.
        if( cropBeginInterpolate || cropEndInterpolate )
        {
          for( int j=0; j<2; ++j )
          {
            vtkIdType cropIndex0, cropIndex1, cropIndex;
            double cropParam = 0.0;
            
            // Crop the beginning point.
            if( j == 0 && cropBeginInterpolate )
            {
              cropIndex  = cropBeginIndex;
              cropIndex0 = cropBeginIndex;
              cropIndex1 = cropBeginIndex + 1;
              cropParam = cropBeginParam;
            }

            // Crop the end point.
            if( j == 1 && cropEndInterpolate )
            {
              cropIndex  = cropEndIndex;
              cropIndex0 = cropEndIndex - 1;
              cropIndex1 = cropEndIndex;
              cropParam = cropEndParam;
            }

            // Do the actual work here.
            if( (j == 0 && cropBeginInterpolate) ||
                (j == 1 && cropEndInterpolate) )
            {
              // Point
              double pt0[3] = {0,0,0}, pt1[3] = {0,0,0};
              points->GetPoint(cropIndex0, pt0);
              points->GetPoint(cropIndex1, pt1);
              avtVector pt = avtVector(pt0) +
                (avtVector(pt1) - avtVector(pt0)) * cropParam;
              points->InsertPoint(cropIndex, pt[0], pt[1], pt[2]);
              // Scalar
              double s0=0, s1=0;
              scalars->GetTuple(cropIndex0, &s0);
              scalars->GetTuple(cropIndex1, &s1);
              double s = s0 + (s1-s0) * cropParam;
              scalars->InsertTuple1(cropIndex, s);
              // Tangent
              double tan0[3] = {0,0,0}, tan1[3] = {0,0,0};
              tangents->GetTuple(cropIndex0, tan0);
              tangents->GetTuple(cropIndex1, tan1);
              avtVector tan = avtVector(tan0) +
                (avtVector(tan1) - avtVector(tan0)) * cropParam;
              tangents->InsertTuple3(cropIndex, tan[0], tan[1], tan[2]);
              // Theta
              if(displayGeometry == IntegralCurveAttributes::Ribbons)
              {
                double t0=0, t1=0;
                thetas->GetTuple(cropIndex0, &t0);
                thetas->GetTuple(cropIndex1, &t0);
                double t = t0 + (t1-t0) * cropParam;
                thetas->InsertTuple1(cropIndex, t);
              }
              // Secondarys
              for( unsigned int i=0; i<secondaryVariables.size(); ++i )
              {
                double s0=0, s1=0;
                secondarys[i]->GetTuple(cropIndex0, &s0);
                secondarys[i]->GetTuple(cropIndex1, &s1);
                double s = s0 + (s1-s0) * cropParam;
                secondarys[i]->InsertTuple1(cropIndex, s);
              }
            }
          }
        }

        lines->InsertNextCell(line);
        line->Delete();
    }
    
    points->Delete();
    lines->Delete();
    scalars->Delete();
    tangents->Delete();
    if(displayGeometry == IntegralCurveAttributes::Ribbons)
        thetas->Delete();

    for( unsigned int i=0; i<secondaryVariables.size(); ++i )
         secondarys[i]->Delete();

    // if( atts.GetShowLines() )
    // {
      vtkCleanPolyData *clean = vtkCleanPolyData::New();
      clean->ConvertLinesToPointsOff();
      clean->ConvertPolysToLinesOff();
      clean->ConvertStripsToPolysOff();
      clean->PointMergingOn();
      clean->SetInputData(pd);
      clean->Update();
      pd->Delete();

      vtkPolyData *cleanPD = clean->GetOutput();
      append->AddInputData(cleanPD);
      cleanPD->Delete();
    // }
    // else
    //   pd->Delete();

    append->Update();
    vtkPolyData *outPD = append->GetOutput();
    outPD->Register(NULL);
    append->Delete();
    
    avtDataTree *dt = new avtDataTree(outPD, 0);
    SetOutputDataTree(dt);


/*
    if (1)
    {
        char f[51];
        sprintf(f, "streamlines_%03d.txt", PAR_Rank());
        FILE *fp = fopen(f, "w");
        for (int i = 0; i < numICs; i++)
        {
            avtStateRecorderIntegralCurve *ic = dynamic_cast<avtStateRecorderIntegralCurve*>(ics[i]);
            size_t nSamples = (ic ? ic->GetNumberOfSamples() : 0);
            if (nSamples == 0)
                continue;

            fprintf(fp, "%d\n", (int)nSamples);
            for (int j = 0; j < nSamples; j++)
            {
                avtStateRecorderIntegralCurve::Sample s = ic->GetSample(j);
                fprintf(fp, "%lf %lf %lf %lf %lf\n", s.position.x, s.position.y, s.position.z, s.time, s.scalar0);
            
            }
        }
        fflush(fp);
        fclose(fp);
    }
 */
}

// ****************************************************************************
// Method:  avtIntegralCurveFilter::ComputeCorrelationDistance
//
// Purpose: Compute the correlation distance at this point. Defined as
//   the arc length distance from the current point to the next point
//   (greater than minDist away) along the streamilne where the
//   velocity direction is the same (to angTol).
//
// Arguments:
//   
//
// Programmer:  Dave Pugmire
// Creation:    February 21, 2011
//
// ****************************************************************************


float
avtIntegralCurveFilter::ComputeCorrelationDistance(int idx,
                                                   avtStateRecorderIntegralCurve *ic,
                                                   double angTol,
                                                   double minDist)
{
    int nSamps = (int)ic->GetNumberOfSamples();
    
    //Last point...
    if (idx == nSamps-1)
        return 0.0f;
    
    float val = 0; //std::numeric_limits<float>::max();
    
    avtStateRecorderIntegralCurve::Sample s0 = ic->GetSample(idx);
    avtVector curVel = s0.velocity.normalized();
    double dist = 0.0;

    for (int i = idx+1; i < nSamps; i++)
    {
        avtStateRecorderIntegralCurve::Sample s = ic->GetSample(i);
        dist += (s0.position-s.position).length();
        s0 = s;
        
        if (dist < minDist)
            continue;

        avtVector vel = s.velocity.normalized();
        double dot = vel.dot(curVel);

        if (fabs(dot) >= angTol)
        {
            val = dist;
            break;
        }
    }

    return val;
}


static avtStateRecorderIntegralCurve *
icFromID(int id, std::vector<avtIntegralCurve *> &ics)
{
    for (size_t i = 0; i < ics.size(); i++)
    {
        if (ics[i]->id == id)
            return dynamic_cast<avtStateRecorderIntegralCurve*>(ics[i]);
    }

    return NULL;
}

// ****************************************************************************
// Method:  avtIntegralCurveFilter::ProcessVaryTubeRadiusByScalar
//
// Purpose: Unify the radius scaling parameter for streamlines that go in both
//          directions.  Since both dir streamlines are split up, they will
//          be treated separately, resulting in different scaling.
//   
//
// Programmer:  Dave Pugmire
// Creation:    August 24, 2011
//
// ****************************************************************************

void
avtIntegralCurveFilter::ProcessVaryTubeRadiusByScalar(std::vector<avtIntegralCurve *> &ics)
{
    for (size_t i = 0; i < fwdBwdICPairs.size(); i++)
    {
        avtStateRecorderIntegralCurve *ic[2] =
          { icFromID(fwdBwdICPairs[i].first, ics),
            icFromID(fwdBwdICPairs[i].second, ics)};

        if (ic[0] == NULL || ic[1] == NULL)
        {
            EXCEPTION1(ImproperUseException, "Integral curve ID not found.");
        }

        //Get the min/max for each pair of ICs.
        double range[2] = { std::numeric_limits<double>::max(),
                           -std::numeric_limits<double>::max()};

        for (int i = 0; i < 2; i++)
        {
            size_t n = ic[i]->GetNumberOfSamples();
            for (size_t j = 0; j < n; j++)
            {
                avtStateRecorderIntegralCurve::Sample s = ic[i]->GetSample(j);
                if (s.secondarys[tubeVariableIndex] < range[0])
                    range[0] = s.secondarys[tubeVariableIndex];
                if (s.secondarys[tubeVariableIndex] > range[1])
                    range[1] = s.secondarys[tubeVariableIndex];
            }
        }

        double dRange = range[1]-range[0];
        //Scale them into the same range.
        for (int i = 0; i < 2; i++)
        {
            size_t n = ic[i]->GetNumberOfSamples();
            for (size_t j = 0; j < n; j++)
            {
                avtStateRecorderIntegralCurve::Sample s = ic[i]->GetSample(j);
                s.secondarys[tubeVariableIndex] = (s.secondarys[tubeVariableIndex]-range[0]) / dRange;
            }
        }
    }
}
