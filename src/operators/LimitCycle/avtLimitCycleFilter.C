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
//                        avtLimitCycleFilter.C                           //
// ************************************************************************* //

#include <avtLimitCycleFilter.h>

#include <vtkSphereSource.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkFieldData.h>
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

#include <avtPoincareIC.h>

#include <vector>
#include <limits>

std::string avtLimitCycleFilter::colorVarArrayName = "colorVar";
std::string avtLimitCycleFilter::thetaArrayName = "theta";
std::string avtLimitCycleFilter::tangentsArrayName = "tangents";


// ****************************************************************************
//  Method: CreateVTKVertex
//
//  Programmer:
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
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

    vtkDoubleArray *arr = vtkDoubleArray::New();
    arr->SetName("colorVar");
    arr->SetNumberOfTuples(1);
    arr->SetTuple1(0, val);

    pd->GetPointData()->SetScalars(arr);
    arr->Delete();

    // secondary scalars
    for( unsigned int i=0; i<secondaryVariables.size(); ++i )
    {
        vtkDoubleArray *secondary = vtkDoubleArray::New();
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
//  Method: avtLimitCycleFilter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

avtLimitCycleFilter::avtLimitCycleFilter() : seedVelocity(0,0,0),
  coordinateSystem(0)

{
    dataValue = LimitCycleAttributes::TimeAbsolute;

    //
    // Initialize source values.
    //
    sourceType = LimitCycleAttributes::Line_;
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
    issueWarningForStepsize = true;
    issueWarningForStiffness = true;
    issueWarningForCriticalPoints = true;
    criticalPointThreshold = 1e-3;
    correlationDistanceAngTol = 0.0;
    correlationDistanceMinDist = 0.0;
    correlationDistanceDoBBox = false;

    intPlane = NULL;
}


// ****************************************************************************
//  Method: avtLimitCycleFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

avtLimitCycleFilter::~avtLimitCycleFilter()
{
    if (intPlane)
        intPlane->Delete();
}

// ****************************************************************************
//  Method:  avtLimitCycleFilter::Create
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
// ****************************************************************************

avtFilter *avtLimitCycleFilter::Create()
{
    return new avtLimitCycleFilter();
}


// ****************************************************************************
//  Method: avtLimitCycleFilter::Equivalent
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
avtLimitCycleFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(LimitCycleAttributes*)a);
}

// ****************************************************************************
//  Method: avtLimitCycleFilter::ExamineContact
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
avtLimitCycleFilter::ExamineContract(avtContract_p in_contract)
{
    avtDataRequest_p in_dr = in_contract->GetDataRequest();

    std::string key( "PseudocolorAttributes::lineType" );
    std::string lineTypeString("");

    if( in_contract->GetAttribute( key ) )
      lineTypeString = in_contract->GetAttribute( key )->AsString();
    else
      lineTypeString = std::string("");

    // Data from all secondary variables need to be added in.
    std::vector<CharStrRef> secondaryVars =
      in_dr->GetSecondaryVariablesWithoutDuplicates();

    secondaryVariables.resize( secondaryVars.size() );

    for( unsigned int i=0; i<secondaryVars.size(); ++i )
      secondaryVariables[i] = std::string( *(secondaryVars[i]) );

    // Call the examine contract function of the super classes first
    avtPluginFilter::ExamineContract(in_contract);
    avtPICSFilter::ExamineContract(in_contract);
}

// ****************************************************************************
//  Method: avtLimitCycleFilter::ModifyContract
//
//  Purpose:
//      Creates a contract the removes the operator-created-expression.
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
//  Modifications:
//
// ****************************************************************************

avtContract_p
avtLimitCycleFilter::ModifyContract(avtContract_p in_contract)
{
    avtDataRequest_p in_dr = in_contract->GetDataRequest();
    avtDataRequest_p out_dr = NULL;
    std::string var = in_dr->GetOriginalVariable();

    in_dr->SetUsesAllDomains(true);

    if( strncmp(var.c_str(), "operators/LimitCycle/",
                strlen("operators/LimitCycle/")) == 0)
    {
        std::string justTheVar = var.substr(strlen("operators/LimitCycle/"));

        outVarName = justTheVar;

        out_dr = new avtDataRequest(in_dr, justTheVar.c_str());
    }

    else if( strncmp(var.c_str(), "operators/LCS/",
                strlen("operators/LCS/")) == 0)
    {
        std::string justTheVar = var.substr(strlen("operators/LCS/"));

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
    {
        out_dr = new avtDataRequest(in_dr);
    }

    if (dataValue == LimitCycleAttributes::Variable)
        out_dr->AddSecondaryVariable(dataVariable.c_str());

    avtContract_p out_contract;

    if ( *out_dr )
        out_contract = new avtContract(in_contract, out_dr);
    else
        out_contract = new avtContract(in_contract);

    return avtPICSFilter::ModifyContract(out_contract);
}

// ****************************************************************************
//  Method: avtLimitCycleFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Tells output that we have a new variable.
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
//  Modifications:
//
// ****************************************************************************

void
avtLimitCycleFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();

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

//    avtPluginFilter::UpdateDataObjectInfo();
    avtPICSFilter::UpdateDataObjectInfo();

    out_atts.AddFilterMetaData("LimitCycle");
}


// ****************************************************************************
//  Method: avtLimitCycleFilter::SetAtts
//
//  Purpose:
//      Sets the atts for the LimitCycle plot.
//
//  Arguments:
//      atts    The attributes for this LimitCycle plot.
//
//  Programmer: Dave Pugmire -- generated by xml2avt
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtLimitCycleFilter::SetAtts(const AttributeGroup *a)
{
    const LimitCycleAttributes *newAtts = (const LimitCycleAttributes *)a;

    // See if any attributes that require the plot to be regenerated were
    // changed and copy the state object.
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;

    //
    // Set the filter's attributes based on the plot attributes.
    //
    switch (atts.GetSourceType())
    {
      case LimitCycleAttributes::Line_:
        SetLineSource(atts.GetLineStart(), atts.GetLineEnd(),
                      atts.GetSampleDensity0(), atts.GetRandomSamples(),
                      atts.GetRandomSeed(), atts.GetNumberOfRandomSamples());
        break;
      case LimitCycleAttributes::Plane:
        SetPlaneSource(atts.GetPlaneOrigin(), atts.GetPlaneNormal(),
                       atts.GetPlaneUpAxis(), atts.GetSampleDensity0(),
                       atts.GetSampleDensity1(), atts.GetSampleDistance0(),
                       atts.GetSampleDistance1(), atts.GetFillInterior(),
                       atts.GetRandomSamples(), atts.GetRandomSeed(),
                       atts.GetNumberOfRandomSamples());
        break;
    }

    int CMFEType = (atts.GetPathlinesCMFE() ==
                    LimitCycleAttributes::CONN_CMFE
                    ? PICS_CONN_CMFE : PICS_POS_CMFE);

    SetPathlines(atts.GetPathlines(),
                 atts.GetPathlinesOverrideStartingTimeFlag(),
                 atts.GetPathlinesOverrideStartingTime(),
                 atts.GetPathlinesPeriod(),
                 CMFEType);

    SetIntegrationDirection(atts.GetIntegrationDirection());
    SetIntersectionCriteria();

    SetFieldType(atts.GetFieldType());
    SetFieldConstant(atts.GetFieldConstant());
    SetVelocitySource(atts.GetVelocitySource());

    SetIntegrationType(atts.GetIntegrationType());

    SetParallelizationAlgorithm(atts.GetParallelizationAlgorithmType(), 
                                atts.GetMaxProcessCount(),
                                atts.GetMaxDomainCacheSize(),
                                atts.GetWorkGroupSize());

    if (atts.GetIntegrationType() == LimitCycleAttributes::DormandPrince)
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
    bool doBBox = (atts.GetAbsTolSizeType() == LimitCycleAttributes::FractionOfBBox);
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
    IssueWarningForStepsize(atts.GetIssueStepsizeWarnings());
    IssueWarningForCriticalPoints(atts.GetIssueCriticalPointsWarnings(),
                                  atts.GetCriticalPointThreshold());

    SetDataValue(int(atts.GetDataValue()), atts.GetDataVariable());

    if (atts.GetDataValue() == LimitCycleAttributes::CorrelationDistance)
    {
        bool doBBox = (atts.GetCorrelationDistanceMinDistType() ==
                       LimitCycleAttributes::FractionOfBBox);

        double minDist = (doBBox ? atts.GetCorrelationDistanceMinDistBBox() :
                                   atts.GetCorrelationDistanceMinDistAbsolute());
        double angTol = atts.GetCorrelationDistanceAngTol();

        SetCorrelationDistanceTol(angTol, minDist, doBBox);
    }

    SetVelocitiesForLighting(1);
}

// ****************************************************************************
//  Method: avtLimitCycleFilter::PreExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Nov  7 13:01:47 EST 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtLimitCycleFilter::PreExecute(void)
{
    SetActiveVariable(outVarName.c_str());
    avtPICSFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtLimitCycleFilter::PostExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Nov  7 13:01:47 EST 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtLimitCycleFilter::PostExecute(void)
{
    avtPICSFilter::PostExecute();

    if (dataValue == LimitCycleAttributes::SeedPointID ||
        dataValue == LimitCycleAttributes::Vorticity ||
        dataValue == LimitCycleAttributes::Speed ||
        dataValue == LimitCycleAttributes::ArcLength ||
        dataValue == LimitCycleAttributes::TimeAbsolute ||
        dataValue == LimitCycleAttributes::TimeRelative ||
        dataValue == LimitCycleAttributes::AverageDistanceFromSeed ||
        dataValue == LimitCycleAttributes::Difference ||
        dataValue == LimitCycleAttributes::Variable)
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
//  Method: avtLimitCycleFilter::ContinueExecute
//
//  Purpose:
//      See if execution needs to continue.
//
//  Programmer: Dave Pugmire
//  Creation:   Mon Aug 17 08:30:06 EDT 2009
//
//  Modifications:
//
//    Hank Childs, Sun Jun  6 11:53:33 CDT 2010
//    Use new names that have integral curves instead of fieldlines.
//
// ****************************************************************************

bool
avtLimitCycleFilter::ContinueExecute()
{
    debug5 << "Continue execute " << std::endl;

    std::vector<avtIntegralCurve *> ics;
    
    GetTerminatedIntegralCurves(ics);
    GetIntegralCurvePoints(ics);

    if( ics.size() == 0 )
      return false;

    bool newSeeds = false;
    int iteration, maxIterations = atts.GetMaxIterations();
    double tolerance = atts.GetCycleTolerance();

    std::vector< int >   ids_to_delete;
    std::vector< std::pair<int,int> > pairs_to_delete;
      
    // Because points are added the size will change so get the
    // inital size so that new seeds are not processed.
    size_t npairs = ICPairs.size();
    
    for (size_t i = 0; i < npairs; i++)
    {
      avtPoincareIC *ic0 = (avtPoincareIC *) icFromID(ICPairs[i].first,  ics );
      avtPoincareIC *ic1 = (avtPoincareIC *) icFromID(ICPairs[i].second, ics );

      // Exception check
      if (ic0 == NULL && ic1 == NULL)
      {
        continue;
      }

      // Exception check
      else if (ic0 == NULL || ic1 == NULL)
      {
        EXCEPTION1(ImproperUseException, "Integral curve ID not found.");
      }

      // Only check points that have actually intersected the plane
      else if( ic0->TerminatedBecauseOfMaxIntersections() && 
               ic1->TerminatedBecauseOfMaxIntersections() )
      {
        // Get the intersecting point.
        avtVector intPt0 =
          GetIntersectingPoint( ic0->points[ic0->points.size()-2],
                                ic0->points[ic0->points.size()-1] );

        avtVector intPt1 =
          GetIntersectingPoint( ic1->points[ic1->points.size()-2],
                                ic1->points[ic1->points.size()-1] );

        // Calculate the vector between the first and last point.
        avtVector vec0 = ic0->points[0] - intPt0;
        avtVector vec1 = ic1->points[0] - intPt1;


        iteration = ic0->properties.iteration;

        // std::cerr << ic0->properties.iteration << "  v0 "
        //           << vec0.length() << "  v1 "
        //           << vec1.length() << "  dot "
        //           << vec0.dot( vec1 ) << "  distance "
        //           << (ic0->points[0] - ic1->points[0]).length() << "  ";

        // If the vectors are in the opposite direction then there is
        // a zero crossing.
        if( vec0.dot( vec1 ) < 0 )
        {
          // std::cerr << " zero crossing, " << std::endl;

          // If the vector length is really small then a cycle has been found.
          if( vec0.length() < tolerance && vec1.length() < tolerance )
          {
            // std::cerr << " small vectors, ";

            // If the seeds are close to each other then keep one curve
            // and delete the other.
            if( (ic0->points[0] - ic1->points[0]).length() < tolerance )
            {
              ids_to_delete.push_back( ic0->id );
              pairs_to_delete.push_back( std::pair<int,int>(ic0->id, ic1->id) );
              // std::cerr << " cycle found " << std::endl;
              continue;
            }
          }

          // Split the interval and search the two smaller intervals
          // for the crossing.
          if( ic0->properties.iteration < maxIterations &&
              ic1->properties.iteration < maxIterations )
          {
            std::vector< std::vector< avtIntegralCurve * > > new_ics;

            avtVector seed0 = ic0->points[0];
            avtVector seed = (ic0->points[0] + ic1->points[0]) * 0.5;
            avtVector seed1 = ic1->points[0];

            std::vector< avtVector > vecs;
            std::vector< avtVector > seeds;

            vecs.push_back( planeN );
            vecs.push_back( planeN );
            vecs.push_back( planeN );
            vecs.push_back( planeN );

            seeds.push_back( seed0 );
            seeds.push_back( seed );
            seeds.push_back( seed );
            seeds.push_back( seed1 );

            AddSeedPoints( seeds, vecs, new_ics );
            for( unsigned int j=0; j<new_ics.size(); ++j )
            {
              for( unsigned int k=0; k<new_ics[j].size(); ++k )
              {
                avtPoincareIC* seed_poincare_ic =
                  (avtPoincareIC *) new_ics[j][k];
                
                // Transfer and update properties.
                seed_poincare_ic->properties = ic0->properties;
                
                seed_poincare_ic->properties.iteration =
                  ic0->properties.iteration + 1;
              }
            }

            ICPairs.push_back(std::pair<int,int> (new_ics[0][0]->id,
                                                  new_ics[1][0]->id));
            ICPairs.push_back(std::pair<int,int> (new_ics[2][0]->id,
                                                  new_ics[3][0]->id));

            pairs_to_delete.push_back( std::pair<int,int>(ic0->id, ic1->id) );
            ids_to_delete.push_back( ic0->id );
            ids_to_delete.push_back( ic1->id );

            newSeeds = true;
            // std::cerr << " splitting " << std::endl;
          }
          else
          {
            // std::cerr << " max iterations " << std::endl;
          }
        }

        // No zero crossing so delete the pair.
        else
        {
          pairs_to_delete.push_back( std::pair<int,int>(ic0->id, ic1->id) );
          ids_to_delete.push_back( ic0->id );
          ids_to_delete.push_back( ic1->id );
          // std::cerr << " no zero crosing found" << std::endl;
        }
      }

      // Curves one or both curves did not terminate with two Poincae
      // intersection.
      else
      {
        pairs_to_delete.push_back( std::pair<int,int>(ic0->id, ic1->id) );
        ids_to_delete.push_back( ic0->id );
        ids_to_delete.push_back( ic1->id );
      }
    }

    // Remove the seeds from the pair list.
    for (int i=0; i<pairs_to_delete.size(); ++i)
    {
      for (int j=0; j<ICPairs.size(); ++j)
      {
        if( ICPairs[j] == pairs_to_delete[i] )
        {
          ICPairs.erase( ICPairs.begin()+j );
          break;
        }
      }
    }

    DeleteIntegralCurves( ids_to_delete );

    // GetTerminatedIntegralCurves(ics);
    // std::cerr << "Iteration  " << iteration << "  "
    //           << "number of curves " << ics.size() << "  "
    //           << "number of pairs " << ICPairs.size() << "  "
    //           << std::endl;

    return newSeeds;
}


// ****************************************************************************
//  Method: avtLimitCycleFilter::Execute
//
//  Purpose:
//      Executes the Integral Curve
//
//  Programmer: Allen Sanderson
//  Creation:   20 August 2013
//
// ****************************************************************************

void
avtLimitCycleFilter::Execute(void)
{
    avtPICSFilter::Execute();

    std::vector<avtIntegralCurve *> ics;
    GetTerminatedIntegralCurves(ics);

    ReportWarnings( ics );
}


// ****************************************************************************
//  Method: avtLimitCycleFilter::GetIntegralCurvePoints
//
//  Purpose:
//      Gets the points from the fieldline and changes them in to a Vector.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Dec  23 12:51:29 EST 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtLimitCycleFilter::GetIntegralCurvePoints(std::vector<avtIntegralCurve *> &ics)
{
    for ( size_t i=0; i<ics.size(); ++i )
    {
        avtPoincareIC * poincare_ic = (avtPoincareIC *) ics[i];

        // Only move the points needed over to the storage.
        if( poincare_ic->points.size() < poincare_ic->GetNumberOfSamples() )
        {
          size_t start = poincare_ic->points.size();
          size_t stop  = poincare_ic->GetNumberOfSamples();

          // Get all of the points from the fieldline which are stored
          // as an array and move them into a vector for easier
          // manipulation by the analysis code.
          poincare_ic->points.resize( poincare_ic->GetNumberOfSamples() );
          poincare_ic->times.resize( poincare_ic->GetNumberOfSamples() );

          for( size_t p=start; p<stop; ++p )
          {
            poincare_ic->points[p] = poincare_ic->GetSample( p ).position;
            poincare_ic->times[p]  = poincare_ic->GetSample( p ).time;
          }
        }
    }
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
// Method:  avtLimitCycleFilter::GetCommunicationPattern()
//
// Programmer:  Dave Pugmire
// Creation:    September  1, 2011
//
// ****************************************************************************

avtPICSFilter::CommunicationPattern
avtLimitCycleFilter::GetCommunicationPattern()
{
//    return avtPICSFilter::RestoreSequenceAssembleUniformly;
    return avtPICSFilter::ReturnToOriginatingProcessor;
}

// ****************************************************************************
// Method:  avtLimitCycleFilter::GenerateAttributeFields() const
//
// Programmer:  Dave Pugmire
// Creation:    November  5, 2010
//
// ****************************************************************************

unsigned int
avtLimitCycleFilter::GenerateAttributeFields() const
{

    // need at least these three attributes
    unsigned int attr = avtStateRecorderIntegralCurve::SAMPLE_POSITION;

    if (storeVelocitiesForLighting)
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VELOCITY;

    // data scalars
    switch( dataValue )
    {
      case LimitCycleAttributes::Speed:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VELOCITY;
        break;
      case LimitCycleAttributes::TimeAbsolute:
      case LimitCycleAttributes::TimeRelative:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_TIME;
        break;
      case LimitCycleAttributes::Vorticity:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VORTICITY;
        break;
      case LimitCycleAttributes::ArcLength:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_ARCLENGTH;
        break;
      case LimitCycleAttributes::Variable:
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

    return attr;
}


// ****************************************************************************
//  Method: avtLimitCycleFilter::SetTermination
//
//  Purpose:
//      Sets the termination criteria for a streamline.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2010
//
// ****************************************************************************

void
avtLimitCycleFilter::SetTermination(int maxSteps_, bool doDistance_,
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
//  Method: avtLimitCycleFilter::CreateIntegralCurve
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
// ****************************************************************************

avtIntegralCurve *
avtLimitCycleFilter::CreateIntegralCurve()
{
    avtPoincareIC *ic = new avtPoincareIC();
    ic->historyMask = GenerateAttributeFields();
    return ic;
}


// ****************************************************************************
//  Method: avtLimitCycleFilter::CreateIntegralCurve
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
// ****************************************************************************

avtIntegralCurve *
avtLimitCycleFilter::CreateIntegralCurve( const avtIVPSolver* model,
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

    avtPoincareIC *ic = 
      new avtPoincareIC(maxSteps, doTime, t_end,
                        attr, model, dir, t_start, p_start, v_start, ID);

    if (intPlane)
        ic->SetIntersectionCriteria(intPlane, maxIntersections);

    return ic;
}


// ****************************************************************************
// Method: avtLimitCycleFilter::SetDataValue
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
// ****************************************************************************

void
avtLimitCycleFilter::SetDataValue(int m, const std::string &var)
{
    dataValue = m;
    dataVariable = var;
}

// ****************************************************************************
// Method: avtLimitCycleFilter::SetVelocitySource
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
avtLimitCycleFilter::SetVelocitySource(const double *p)
{
  seedVelocity.set(p);
}


// ****************************************************************************
// Method: avtLimitCycleFilter::SetLineSource
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
// ****************************************************************************

void
avtLimitCycleFilter::SetLineSource(const double *p0, const double *p1,
                                      int den, bool rand, int seed, int numPts)
{
    sourceType = LimitCycleAttributes::Line_;
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
// Method: avtLimitCycleFilter::SetPlaneSource
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
// ****************************************************************************

void
avtLimitCycleFilter::SetPlaneSource(double O[3], double N[3], double U[3],
                                       int den1, int den2, double dist1, double dist2,
                                       bool f, 
                                       bool rand, int seed, int numPts)
{
    sourceType = LimitCycleAttributes::Plane;
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
// Method: avtLimitCycleFilter::SeedInfoString
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
// ****************************************************************************

std::string
avtLimitCycleFilter::SeedInfoString() const
{
    char buff[256];
    if (sourceType == LimitCycleAttributes::Line_)
        sprintf(buff, "Line [%g %g %g] [%g %g %g] D: %d",
                points[0].x, points[0].y, points[0].z,
                points[1].x, points[1].y, points[1].z, sampleDensity[0]);
    else if (sourceType == LimitCycleAttributes::Plane)
        sprintf(buff, "Plane O[%g %g %g] N[%g %g %g] D: %d %d",
                points[0].x, points[0].y, points[0].z,
                vectors[0].x, vectors[0].y, vectors[0].z,
                sampleDensity[0], sampleDensity[1]);
    else
        sprintf(buff, "%s", "UNKNOWN");
    
    std::string str = buff;
    return str;
}


// ****************************************************************************
//  Method: avtLimitCycleFilter::GetInitialVelocities
//
//  Purpose:
//      Get the seed velocities out of the attributes.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2008
//
// ****************************************************************************

std::vector<avtVector>
avtLimitCycleFilter::GetInitialVelocities(void)
{
    std::vector<avtVector> seedVels;

    seedVels.push_back( planeN );
//    seedVels.push_back( seedVelocity );

    return seedVels;
}


// ****************************************************************************
//  Method: avtLimitCycleFilter::GetInitialLocations
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
// ****************************************************************************

std::vector<avtVector>
avtLimitCycleFilter::GetInitialLocations(void)
{
    std::vector<avtVector> seedPts;
    
    if (randomSamples)
        srand(randomSeed);

    // Add seed points based on the source.
    if(sourceType == LimitCycleAttributes::Line_)
        GenerateSeedPointsFromLine(seedPts);
    else if(sourceType == LimitCycleAttributes::Plane)
        GenerateSeedPointsFromPlane(seedPts);

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
//  Method: avtLimitCycleFilter::GenerateSeedPointsFromLine
//
//  Purpose: Create a series of pairs of curves to check.
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtLimitCycleFilter::GenerateSeedPointsFromLine(std::vector<avtVector> &pts)
{
    if (randomSamples)
    {
        avtVector dv = points[1] - points[0];

        for (int i = 0; i < numSamplePoints; i++)
        {
            avtVector p = points[0] + random01() * dv;

            avtVector ddv = 0.05 * random01() * dv;

            pts.push_back(p-ddv);
            pts.push_back(p+ddv);
        }
    }
    else
    {
        avtVector dv = (points[1] - points[0]) / (double) sampleDensity[0];
    
        for (int i = 0; i <= sampleDensity[0]; i++)
        {
            avtVector p = points[0] + (double) i * dv;

            pts.push_back(p);

            if( 0 < i && i < sampleDensity[0] )
              pts.push_back(p);
        }
    }

    for (int i = 0; i < pts.size(); i+=2)
      ICPairs.push_back(std::pair<int,int> (i, i+1));
}

// ****************************************************************************
//  Method: avtLimitCycleFilter::GenerateSeedPointsFromPlane
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtLimitCycleFilter::GenerateSeedPointsFromPlane(std::vector<avtVector> &pts)
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
//  Method: avtLimitCycleFilter::GetFieldForDomain
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
// ****************************************************************************

avtIVPField* 
avtLimitCycleFilter::GetFieldForDomain(const BlockIDType& dom, vtkDataSet* ds)
{
    avtIVPField* field = avtPICSFilter::GetFieldForDomain( dom, ds );

    //  The dataValue variable must always be after all of the
    //  secondary variables.
    if( dataValue == LimitCycleAttributes::Variable && 
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
//  Method: avtLimitCycleFilter::ReportWarnings() 
//
//  Purpose:
//      Reports any potential integration warnings
//
//  Programmer: Allen Sanderson
//  Creation:   20 August 2013
//
//  Modifications:
//
// ****************************************************************************

void
avtLimitCycleFilter::ReportWarnings(std::vector<avtIntegralCurve *> &ics)
{
    int numICs = (int)ics.size();

    int numEarlyTerminators = 0;
    int numStepSize = 0;
    int numStiff = 0;
    int numCritPts = 0;

    if (DebugStream::Level5())
    {
        debug5 << "::ReportWarnings " << ics.size() << endl;
    }

    //See how many pts, ics we have so we can preallocate everything.
    for (int i = 0; i < numICs; i++)
    {
        avtPoincareIC *ic = dynamic_cast<avtPoincareIC*>(ics[i]);

        if (ic->CurrentVelocity().length() <= criticalPointThreshold)
            numCritPts++;

        if (ic->TerminatedBecauseOfMaxSteps())
            numEarlyTerminators++;

        if (ic->status.StepSizeUnderflow())
            numStepSize++;

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

    if (issueWarningForStepsize)
    {
        SumIntAcrossAllProcessors(numStepSize);
        if (numStepSize > 0)
        {
            SNPRINTF(str, 4096, 
                     "%s\n%d of your integral curves were unable to advect because of the \"stepsize\".  "
                     "Often the step size becomes too small when appraoching a spatial "
                     "or temporal boundary. This especially happens when the step size matches "
                     "the temporal spacing. This condition is referred to as stepsize underflow and "
                     "VisIt stops advecting in this case.  If you want to disable this message, "
                     "you can do this under the Advanced tab.\n", str, numStepSize);
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
//  Method: avtLimitCycleFilter::CreateIntegralCurveOutput
//
//  Purpose:
//      Create the VTK poly data output from the streamline.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
// ****************************************************************************
void
avtLimitCycleFilter::CreateIntegralCurveOutput(std::vector<avtIntegralCurve *> &ics)
{
    if (ics.size() == 0)
        return;

    int numICs = (int)ics.size(), numPts = 0;

    if (DebugStream::Level5())
    {
        debug5 << "::CreateLimitCycleOutput " << ics.size() << endl;
    }

    vtkAppendPolyData *append = vtkAppendPolyData::New();

    //Make a polydata.
    vtkPoints     *points   = vtkPoints::New();
    vtkCellArray  *lines    = vtkCellArray::New();
    vtkDoubleArray *scalars  = vtkDoubleArray::New();
    vtkDoubleArray *tangents = vtkDoubleArray::New();
    vtkDoubleArray *thetas   = NULL;

    std::vector< vtkDoubleArray * > secondarys;
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

    // secondary scalars
    for( unsigned int i=0; i<secondaryVariables.size(); ++i )
    {
        secondarys[i] = vtkDoubleArray::New();
        secondarys[i]->Allocate(numPts);
        secondarys[i]->SetName(secondaryVariables[i].c_str());
        pd->GetPointData()->AddArray(secondarys[i]);
    }

    double correlationDistMinDistToUse = correlationDistanceMinDist;
    double correlationDistAngTolToUse = 0.0;

    if (dataValue == LimitCycleAttributes::CorrelationDistance)
    {
        if (correlationDistanceDoBBox)
            correlationDistMinDistToUse *= GetLengthScale();
        correlationDistAngTolToUse = cos(correlationDistanceAngTol *M_PI/180.0);
    }

    vtkIdType pIdx = 0;

    for (int i = 0; i < numICs; i++)
    {
        avtStateRecorderIntegralCurve *ic =
          dynamic_cast<avtStateRecorderIntegralCurve*>(ics[i]);

        size_t nSamples = (ic ? ic->GetNumberOfSamples() : 0);
        if (nSamples <= 1)
            continue;

        int  beginIndex = 0;
        int  endIndex   = (int) nSamples - 1;

        int totalSamples = (int) nSamples;

        if( totalSamples < 2 )
          continue;
        
        // Create the new vtkPolyline
        vtkPolyLine *line = vtkPolyLine::New();
        line->GetPointIds()->SetNumberOfIds(totalSamples);

        float theta = 0.0, lastTime = 0.0;

        avtStateRecorderIntegralCurve::Sample s, s0 = ic->GetSample(0);

        double startTime = s0.time;
        double distance = 0;

        if( //dataValue == LimitCycleAttributes::ArcLength ||
            dataValue == LimitCycleAttributes::AverageDistanceFromSeed ||
            dataValue == LimitCycleAttributes::Difference )
        {
          for (size_t j = 0; j < nSamples; j++)
          {
            s = ic->GetSample(j);

//          if( dataValue == LimitCycleAttributes::ArcLength )
//            distance += s.arclength;

            if( dataValue == LimitCycleAttributes::AverageDistanceFromSeed )
              distance += (s.position - s0.position).length();
            else if( dataValue == LimitCycleAttributes::Difference )
//            distance += fabs(s.position.y - 100);
              distance += s.arclength;
          }

          if( dataValue == LimitCycleAttributes::AverageDistanceFromSeed )
            distance /= nSamples;
        }
        
        for (int j = beginIndex; j <= endIndex; j++)
        {
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
              case LimitCycleAttributes::Solid:
                data_value = 0.0f;
                break;
              case LimitCycleAttributes::SeedPointID:
                data_value = ic->id;
                break;
              case LimitCycleAttributes::Speed:
                data_value = speed;
                break;
              case LimitCycleAttributes::Vorticity:
                data_value = s.vorticity;
                break;
              case LimitCycleAttributes::ArcLength:
                data_value = s.arclength;
//                data_value = distance;
                break;
              case LimitCycleAttributes::TimeAbsolute:
                data_value = s.time;
                break;
              case LimitCycleAttributes::TimeRelative:
                data_value = s.time - startTime;
                break;
              case LimitCycleAttributes::AverageDistanceFromSeed:
//              data_value = (s.position - s0.position).length();
                data_value = distance;
                break;
              case LimitCycleAttributes::Variable:
                data_value = s.variable;
                break;
              case LimitCycleAttributes::CorrelationDistance:
                data_value =
                  ComputeCorrelationDistance(j, ic,
                                             correlationDistAngTolToUse,
                                             correlationDistMinDistToUse);
                break;
              case LimitCycleAttributes::Difference:
                data_value = distance;
                break;
            }

            scalars->InsertTuple1(pIdx, data_value);

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

        lines->InsertNextCell(line);
        line->Delete();
    }
    
    points->Delete();
    lines->Delete();
    scalars->Delete();
    tangents->Delete();

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
// Method:  avtLimitCycleFilter::ComputeCorrelationDistance
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
avtLimitCycleFilter::ComputeCorrelationDistance(int idx,
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


// ****************************************************************************
// Method: avtLimitCycleFilter::SetIntersectionCriteria
//
// Purpose:
//   Sets the intersection object.
//
// Arguments:
//   obj : Intersection object.
//
// Programmer: Dave Pugmire
// Creation:   11 August 2009
//
// Modifications:
//
// ****************************************************************************

void
avtLimitCycleFilter::SetIntersectionCriteria()
{
    if( sourceType == LimitCycleAttributes::Line_ )
    {
        planePt = avtVector(points[0] + points[1]);
        planePt *= 0.5;
        
        avtVector tangent(points[0] - points[1]);
        tangent.normalize();
        
        planeN = avtVector(tangent[1],-tangent[0],tangent[2]);
        planeN.normalize();
    }
    else if( sourceType == LimitCycleAttributes::Plane )
    {
        planePt = points[0];
        planeN = vectors[0];
        planeN.normalize();
    }
    
    intPlane = vtkPlane::New();
    intPlane->SetOrigin( planePt[0], planePt[1], planePt[2] );
    intPlane->SetNormal( planeN [0], planeN [1], planeN [2] );
    intPlane->Register(NULL);
    
    maxIntersections = 2;
}



avtVector
avtLimitCycleFilter::GetIntersectingPoint( avtVector pt0, avtVector pt1 )
{
  avtVector dir(pt1-pt0);
              
  double dot = Dot(planeN, dir); 
              
  avtVector w = pt1 - planePt;
                
  double t = -Dot(planeN, w ) / dot;
                
  return avtVector(pt1 + dir * t);
}
