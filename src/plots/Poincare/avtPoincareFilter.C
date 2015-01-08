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
//                             avtPoincareFilter.C                           //
// ************************************************************************* //


#define RATIONAL_SURFACE

#include <avtPoincareFilter.h>

#include <avtCallback.h>

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkQuad.h>
#include <vtkSlicer.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtPoincareIC.h>
#include <utility>

#include <sys/stat.h>

#include "FieldlineAnalyzerLib.h"

#ifdef RATIONAL_SURFACE
#include "RationalSurfaceLib.h"
#endif

#ifdef STRAIGHTLINE_SKELETON
#include "skelet.h"
#endif

#define SIGN(x) ((x) < 0.0 ? -1 : 1)

static const int DATA_None = 0;
static const int DATA_SafetyFactorQ = 1;
static const int DATA_SafetyFactorP = 2;
static const int DATA_SafetyFactorQ_NotP = 3;
static const int DATA_SafetyFactorP_NotQ = 4;
static const int DATA_ToroidalWindings = 5;
static const int DATA_PoloidalWindingsQ = 6;
static const int DATA_PoloidalWindingsP = 7;
static const int DATA_FieldlineOrder = 8;
static const int DATA_PointOrder = 9;
static const int DATA_PlaneOrder = 10;
static const int DATA_WindingGroupOrder = 11;
static const int DATA_WindingPointOrder = 12;
static const int DATA_WindingPointOrderModulo = 13;

// ****************************************************************************
//  Method: CreateSphere
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

static vtkPolyData *
CreateSphere(float val, double p[3])
{
    vtkPoints *pt = vtkPoints::New();
    pt->SetNumberOfPoints(1);
    pt->SetPoint(0, p[0], p[1], p[2]);
    
    vtkPolyData *point = vtkPolyData::New();
    point->SetPoints(pt);
    pt->Delete();

    vtkIdType ids[1] = {0};
    point->Allocate(1);
    point->InsertNextCell(VTK_VERTEX, 1, ids);

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetName("colorVar");
    arr->SetNumberOfTuples(1);
    arr->SetTuple1(0, val);
    point->GetPointData()->SetScalars(arr);
    arr->Delete();

    return point;
}


// ****************************************************************************
//  Method: avtPoincareFilter constructor
//
//  Programmer: Dave Pugmire -- generated by xml2avt
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//
//    Dave Pugmire, Wed Feb 25 09:52:11 EST 2009
//    Add terminate by steps, add AdamsBashforth solver,
//    Allen Sanderson's new code.
//
//    Dave Pugmire, Fri Apr 17 11:32:40 EDT 2009
//    Add variables for dataValue var.
//
//    Dave Pugmire, Tue Apr 28 09:26:06 EDT 2009
//    Changed color to dataValue
//
//    Dave Pugmire, Wed May 27 15:03:42 EDT 2009
//    Initialize points.
//
//    Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//    Add ability to restart fieldline integration.
//
// ****************************************************************************

avtPoincareFilter::avtPoincareFilter() :
    maximumToroidalWinding( 0 ),
    overrideToroidalWinding( 0 ),
    overridePoloidalWinding( 0 ),
    windingPairConfidence( 0.90 ),
    rationalSurfaceFactor( 0.10 ),
    adjust_plane(-1),
    overlaps(1),

    is_curvemesh(1),
    dataValue(DATA_SafetyFactorQ),

    showRationalSurfaces( false ),
    rationalSurfaceMaxIterations(2),
    showOPoints( false ),
    OPointMaxIterations(2),
    XPointMaxIterations(2),
    performOLineAnalysis( false ),
    OLineToroidalWinding( 1 ),
    OLineAxisFileName(""),
    showIslands( false ),
    showLines( true ),
    showPoints( false ),
    summaryFlag( true ),
    verboseFlag( false ),
    pointScale(1)
{
    planes.resize(1);
    planes[0] = 0;
    intersectObj = NULL;
    maxIntersections = 0;
}


// ****************************************************************************
//  Method: avtPoincareFilter destructor
//
//  Programmer: Dave Pugmire -- generated by xml2avt
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//
// ****************************************************************************

avtPoincareFilter::~avtPoincareFilter()
{
    if (intersectObj)
        intersectObj->Delete();
}

// ****************************************************************************
//  Method: avtPoincareFilter::PreExecute
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
avtPoincareFilter::PreExecute(void)
{
    avtStreamlineFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtPoincareFilter::PostExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Nov  7 13:01:47 EST 2008
//
//  Modifications:
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
// ****************************************************************************

void
avtPoincareFilter::PostExecute(void)
{
    avtStreamlineFilter::PostExecute();
    
    double range[2];
    avtDataset_p ds = GetTypedOutput();
    avtDatasetExaminer::GetDataExtents(ds, range, "colorVar");

    avtExtents *e;
    e = GetOutput()->GetInfo().GetAttributes().GetThisProcsOriginalDataExtents();
    e->Merge(range);
    e = GetOutput()->GetInfo().GetAttributes().GetThisProcsActualDataExtents();
    e->Merge(range);
}

// ****************************************************************************
//  Method: avtPoincareFilter::CreateIntegralCurve
//
//  Purpose:
//      Create an integral curve and set its properties.
//
//  Programmer: Christoph Garth
//  Creation:   Thu July 15, 2010
//
//  Modifications:
//
//    Hank Childs, Fri Oct  8 23:30:27 PDT 2010
//    Create PoincareICs, not StateRecorderICs.
//
// ****************************************************************************

avtIntegralCurve *
avtPoincareFilter::CreateIntegralCurve( const avtIVPSolver* model,
                                        avtIntegralCurve::Direction dir,
                                        const double& t_start,
                                        const avtVector &p_start,
                                        const avtVector &v_start,
                                        long ID ) 
{
    // need at least these three attributes
    unsigned char attr = avtStateRecorderIntegralCurve::SAMPLE_POSITION;

    avtPoincareIC *rv = new avtPoincareIC( 0, false, 0,
                                           attr, model, dir, 
                                           t_start, p_start, v_start, ID );

    if (intersectObj)
        rv->SetIntersectionCriteria(intersectObj, maxIntersections);

    return rv;
}

// ****************************************************************************
//  Method: avtPoincareFilter::GetFieldlinePoints
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
avtPoincareFilter::GetIntegralCurvePoints(std::vector<avtIntegralCurve *> &ics)
{
    for ( int i=0; i<ics.size(); ++i )
    {
        avtPoincareIC * poincare_ic = (avtPoincareIC *) ics[i];

        // Only move the points needed over to the storage.
        if( poincare_ic->points.size() < poincare_ic->GetNumberOfSamples() )
        {
          unsigned int start = poincare_ic->points.size();
          unsigned int stop  = poincare_ic->GetNumberOfSamples();

          // Get all of the points from the fieldline which are stored
          // as an array and move them into a vector for easier
          // manipulation by the analysis code.
          poincare_ic->points.resize( poincare_ic->GetNumberOfSamples() );

          for( size_t p=start; p<stop; ++p )
            poincare_ic->points[p] = poincare_ic->GetSample( p ).position;
        }

        // If the analysis asked for more points but did not get any
        // then the integration failed. As such, terminate the
        // integration and analysis.
        else if( poincare_ic->properties.analysisState ==
                 FieldlineProperties::ADDING_POINTS &&
                 
                 poincare_ic->points.size() ==
                 poincare_ic->GetNumberOfSamples() )
        {
            poincare_ic->status.SetTerminationMet();
            poincare_ic->properties.analysisState =
            FieldlineProperties::TERMINATED;

//           std::cerr << "Terminated integration for Fieldline: id = "
//                     << poincare_ic->id << "  "
//                  << std::endl;
        }
    }
}

// ****************************************************************************
//  Method: avtPoincareFilter::Execute
//
//  Purpose:
//      Calculate poincare points.
//
//  Programmer: Dave Pugmire -- generated by xml2avt
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//    Dave Pugmire (for Allen Sanderson), Wed Feb 25 09:52:11 EST 2009
//    Add terminate by steps, add AdamsBashforth solver, Allen Sanderson's new code.
//
//    Dave Pugmire, Wed May 27 15:03:42 EDT 2009
//    Re-organization. GetFieldlinePoints removed.
//
//    Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//    Add ability to restart fieldline integration.
//
// ****************************************************************************

void
avtPoincareFilter::Execute()
{
    if( performOLineAnalysis )
    {
        struct stat fileAtt;

        //Use the stat function to get the file information
        if (stat(OLineAxisFileName.c_str(), &fileAtt) != 0)
        {
          std::string msg("Trying to perform O-line analysis but the O-line axis file is not valid.");

          avtCallback::IssueWarning(msg.c_str());
          EXCEPTION1(ImproperUseException, msg);
        }
        else
        {
          FILE *fp = fopen( OLineAxisFileName.c_str(), "r" );

          if( fp == NULL )
          {
            std::string msg("Trying to perform O-line analysis but the O-line axis file can not be openned.");
            
            avtCallback::IssueWarning(msg.c_str());
            EXCEPTION1(ImproperUseException, msg);
          }

          while( !feof(fp) )
          {
            Point nextPt;

            if( fscanf( fp, "%lf %lf %lf", &nextPt.x, &nextPt.y, &nextPt.z ) != 3 &&
                feof(fp) == 0)
            {
              std::string msg("Trying to perform O-line analysis but the O-line axis file can not be read.");
              
              avtCallback::IssueWarning(msg.c_str());
              EXCEPTION1(ImproperUseException, msg);
            }
          }

          fclose (fp);
        }
    }

    avtStreamlineFilter::Execute();

    std::vector<avtIntegralCurve *> ics;
    GetTerminatedIntegralCurves(ics);

    avtDataTree *dt = new avtDataTree();
    
    CreatePoincareOutput( dt, ics );
#ifdef RATIONAL_SURFACE
    CreateRationalOutput( dt, ics );
#endif
    SetOutputDataTree(dt);
}


// ****************************************************************************
//  Method: avtPoincareFilter::ContinueExecute
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
avtPoincareFilter::ContinueExecute()
{
    debug5 << "Continue execute " << std::endl;

    std::vector<avtIntegralCurve *> ics;
    
    GetTerminatedIntegralCurves(ics);
    GetIntegralCurvePoints(ics);

    if (analysis && (! ClassifyFieldlines(ics) || ! ClassifyRationals(ics)))
    {
      std::vector< int > ids_to_delete;

      // Because points are added the size will change so get the
      // inital size so that new seeds are not processed.
      unsigned int nics = ics.size();

      for ( int i=0; i<nics; ++i )
      {
        avtPoincareIC * poincare_ic = (avtPoincareIC *) ics[i];
        FieldlineProperties &properties = poincare_ic->properties;

#ifdef STRAIGHTLINE_SKELETON
        // For Island Chains add in the O Points.
        if( showOPoints )
        {
          // Are O Points present?
          if( properties.type & FieldlineProperties::ISLAND_CHAIN &&
              properties.analysisState == FieldlineProperties::ADD_O_POINTS &&

              (properties.searchState == FieldlineProperties::ISLAND_O_POINT ||
               properties.searchState == FieldlineProperties::NO_SEARCH) )
          {
            // Change the state of the properties to complete now that
            // the seed point has been stripped off.
            poincare_ic->properties.analysisState =
              FieldlineProperties::COMPLETED;
            
            if(verboseFlag )
              std::cerr << "Have island seed  " << properties.seedPoints[0]
                        << std::endl;
            
            if( properties.iteration < OPointMaxIterations )
            {
              std::vector<avtIntegralCurve *> new_ics;
              avtVector vec(0,0,0);
              
              if(verboseFlag )
                std::cerr << "Adding island O point seed  "
                          << properties.seedPoints[0]
                          << std::endl;
              
              AddSeedPoint( properties.seedPoints[0], vec, new_ics );
              
              for( unsigned int j=0; j<new_ics.size(); ++j )
              {
                if(verboseFlag )
                  std::cerr << "New island O point seed ids "
                            << new_ics[j]->id << "  ";

                avtPoincareIC* seed_poincare_ic =
                  (avtPoincareIC *) new_ics[j];

                // Transfer and update properties.
                seed_poincare_ic->properties = properties;
              
                seed_poincare_ic->properties.searchState =
                  FieldlineProperties::ISLAND_O_POINT;
                seed_poincare_ic->properties.analysisState =
                  FieldlineProperties::UNKNOWN_ANALYSIS;
                seed_poincare_ic->properties.source =
                  properties.type;
              
                seed_poincare_ic->properties.iteration =
                  properties.iteration + 1;
              }

              if(verboseFlag )
                std::cerr << std::endl;
              
              // The source was an island_chain which meant the seed was
              // an intermediate seed so delete it.

              // Note only delete the seed if another seed replaces
              // it. If past the maximum iterations the seed will
              // not be deleted.
              if( properties.source & FieldlineProperties::ISLAND_CHAIN )
              {
                if( verboseFlag )
                  std::cerr << "O Point search deleting O Point seed "
                            << poincare_ic->id << std::endl;
                
                ids_to_delete.push_back( poincare_ic->id );
              }
            }
          }
              
          // Landed on an O-point directly. Can not search for the
          // boundary as there is no value for properties.searchDelta.
          else if( properties.type == FieldlineProperties::O_POINT &&
                   properties.analysisState == FieldlineProperties::ADD_BOUNDARY_POINT &&
                   properties.searchState == FieldlineProperties::NO_SEARCH )
          {
            // Change the state of the properties to complete now that
            // the seed point has been stripped off.
            poincare_ic->properties.analysisState =
              FieldlineProperties::COMPLETED;
          }

          // Is a boundary seed point present?
          else if( properties.type == FieldlineProperties::O_POINT &&
                   properties.analysisState == FieldlineProperties::ADD_BOUNDARY_POINT &&

                    (properties.searchState == FieldlineProperties::ISLAND_O_POINT ||
                     properties.searchState == FieldlineProperties::ISLAND_BOUNDARY_SEARCH) )
          {
            // Change the state of the properties to complete now that
            // the seed point has been stripped off.
            poincare_ic->properties.analysisState =
              FieldlineProperties::COMPLETED;

            properties.searchIncrement = 1.0;
            properties.pastFirstSearchFailure = false;

            if( properties.iteration < OPointMaxIterations )
            {
              // First time through the loop.
              if( properties.searchState == FieldlineProperties::ISLAND_O_POINT )
                properties.searchMagnitude = properties.searchIncrement;

              // Ended up back up at the O Point so try again.
              else if( properties.searchState == FieldlineProperties::ISLAND_BOUNDARY_SEARCH )
                properties.searchMagnitude += properties.searchIncrement;

              properties.baseToroidalWinding = properties.toroidalWinding;
              properties.basePoloidalWinding = properties.poloidalWinding;
              
              avtVector seed = properties.lastSeedPoint +
                properties.searchNormal *
                properties.searchMagnitude * properties.searchDelta;
              
              std::cerr << "LINE " << __LINE__ << "  "
                        << properties.iteration << "  "
                        << properties.pastFirstSearchFailure << "  "
                        << properties.searchIncrement << "  "
                        << properties.searchMagnitude << "  "
                        << seed << "  "
                        << properties.searchNormal << "  "
                        << properties.searchDelta << "  "
                        << std::endl;

              std::vector<avtIntegralCurve *> new_ics;
              avtVector vec(0,0,0);
              
              if(verboseFlag )
                std::cerr << "Have island boundary seed  " << seed << std::endl;
              
              AddSeedPoint( seed, vec, new_ics );
              
              for( unsigned int j=0; j<new_ics.size(); ++j )
              {
                if(verboseFlag )
                  std::cerr << "LINE " << __LINE__
                            << " New island boundary seed ids "
                            << new_ics[j]->id << "  ";
                
                avtPoincareIC* seed_poincare_ic =
                  (avtPoincareIC *) new_ics[j];
                
                // Transfer and update properties.
                seed_poincare_ic->properties = properties;
                
                seed_poincare_ic->properties.searchState =
                  FieldlineProperties::ISLAND_BOUNDARY_SEARCH;
                seed_poincare_ic->properties.analysisState =
                  FieldlineProperties::UNKNOWN_ANALYSIS;
                
                seed_poincare_ic->properties.source = properties.type;
                // Save the seed point curve.
                seed_poincare_ic->properties.parentOPointIC = poincare_ic;
              
                // First time through the loop.
                if( properties.searchState == FieldlineProperties::ISLAND_O_POINT )
                {
                  seed_poincare_ic->properties.iteration = 0;
                }

                // Ended up back up at the O Point so try again with a
                // new seed.
                else if( properties.searchState == FieldlineProperties::ISLAND_BOUNDARY_SEARCH )
                {
                  seed_poincare_ic->properties.iteration =
                    properties.iteration + 1;

                  if( verboseFlag )
                    std::cerr << "Island boundary search deleting O Point seed "
                              << poincare_ic->id << std::endl;
                
                  ids_to_delete.push_back( poincare_ic->id );
                }
              }

              if(verboseFlag )
                std::cerr << std::endl;
            }
          }

          // Boundary surfaces
          else if( ( (properties.type & FieldlineProperties::ISLAND_CHAIN &&
                      properties.analysisState == FieldlineProperties::ADD_BOUNDARY_POINT) || 

                     (properties.type & FieldlineProperties::FLUX_SURFACE &&
                      properties.analysisState == FieldlineProperties::COMPLETED) || 
                     
                     (properties.analysisState == FieldlineProperties::TERMINATED) ) &&

                   properties.searchState == FieldlineProperties::ISLAND_BOUNDARY_SEARCH )
          {
            bool terminated = properties.analysisState == FieldlineProperties::TERMINATED;

            // Change the state of the properties to complete.
            poincare_ic->properties.analysisState =
              FieldlineProperties::COMPLETED;

            if( properties.iteration <
                (properties.pastFirstSearchFailure ? 1 : 10) * OPointMaxIterations )
            {
              // If a flux surface is found or the analysis terminates
              // go back a step and then half the increment. 
              if( (properties.type & FieldlineProperties::FLUX_SURFACE) ||
                  terminated )
              {
                if( properties.pastFirstSearchFailure == false )
                {
                  properties.pastFirstSearchFailure = true;
                  properties.iteration = 0;
                }

                properties.searchMagnitude -= properties.searchIncrement;
              }

              if( properties.pastFirstSearchFailure )
                properties.searchIncrement /= 2.0;
              
              // If about to end do not increment so to be assured
              // that an island is found.
              if( properties.iteration + 1 <
                  (properties.pastFirstSearchFailure ? 1 : 10) * OPointMaxIterations )
                properties.searchMagnitude += properties.searchIncrement;

              avtVector seed = properties.lastSeedPoint +
                properties.searchMagnitude * properties.searchDelta *
                properties.searchNormal;
            
              std::cerr << "LINE " << __LINE__ << "  "
                        << properties.iteration << "  "
                        << properties.pastFirstSearchFailure << "  "
                        << properties.searchIncrement << "  "
                        << properties.searchMagnitude << "  "
                        << seed << "  "
                        << properties.searchNormal << "  "
                        << properties.searchDelta << "  "
                        << std::endl;

              std::vector<avtIntegralCurve *> new_ics;
              avtVector vec(0,0,0);
              
              if(verboseFlag )
                std::cerr << "LINE " << __LINE__
                          << " Have additional island boundary seed  " << seed << std::endl;
                          
              AddSeedPoint( seed, vec, new_ics );
              
              for( unsigned int j=0; j<new_ics.size(); ++j )
              {
                if(verboseFlag )
                  std::cerr << "New island boundary seed ids "
                            << new_ics[j]->id << "  ";
              
                avtPoincareIC* seed_poincare_ic =
                  (avtPoincareIC *) new_ics[j];
              
                // Transfer and update properties.
                seed_poincare_ic->properties = properties;
              
                seed_poincare_ic->properties.analysisState =
                  FieldlineProperties::UNKNOWN_ANALYSIS;
              
                seed_poincare_ic->properties.source = properties.type;
              
                seed_poincare_ic->properties.iteration =
                  properties.iteration + 1;
              
                seed_poincare_ic->properties.searchState =
                  FieldlineProperties::ISLAND_BOUNDARY_SEARCH;
              }

              if(verboseFlag )
                std::cerr << std::endl;

              // Note only delete the seed if another seed replaces
              // it. If past the maximum iterations the seed will
              // not be deleted.
//            if( properties.source & FieldlineProperties::ISLAND_CHAIN )
              {
                if( verboseFlag )
                  std::cerr << "Island boundary search deleting boundary seed "
                            << poincare_ic->id << std::endl;
                
                ids_to_delete.push_back( poincare_ic->id );
              }
            }
            else
            {
              poincare_ic->properties.searchState =
                FieldlineProperties::NO_SEARCH;

              std::cerr << "LINE " << __LINE__ << "  "
                        << properties.baseToroidalWinding << "  "
                        << properties.basePoloidalWinding << "  "
                        << properties.toroidalWinding << "  "
                        << properties.poloidalWinding << "  "
                        << std::endl;

              poincare_ic->properties.parentOPointIC->properties.childOPointIC = 
                poincare_ic;
            }
          }

          // Not an O or X point check to see if the source was from
          // an island chain. If so delete.
          else if( properties.type != FieldlineProperties::O_POINT &&
                   properties.type != FieldlineProperties::X_POINT )
          {
            // The source was an island_chain which meant the seed was
            // an intermediate seed that did make it into an O Point
            // so delete it.
            if( properties.source & FieldlineProperties::ISLAND_CHAIN &&
                properties.analysisState == FieldlineProperties::COMPLETED &&
                properties.searchState == FieldlineProperties::ISLAND_O_POINT )
            {
              if( verboseFlag )
                std::cerr << "Deleting an O Point seed that resulted in a surface "
                          << poincare_ic->id << std::endl;
              
              ids_to_delete.push_back( poincare_ic->id );
            }
          }
        }
#endif

#ifdef RATIONAL_SURFACE

        if( showRationalSurfaces )
        {

        /////////////////////////
        // Begin Rational Search
        /////////////////////////
        if ( properties.analysisMethod == FieldlineProperties::RATIONAL_SEARCH &&
             properties.type        == FieldlineProperties::RATIONAL &&
             properties.searchState == FieldlineProperties::ORIGINAL_RATIONAL )
          {       
            // Intentionally empty
          }
        else if( properties.analysisMethod  == FieldlineProperties::DEFAULT_METHOD &&
            properties.type            == FieldlineProperties::RATIONAL &&
            (properties.analysisState  == FieldlineProperties::COMPLETED ||
             properties.analysisState  == FieldlineProperties::TERMINATED) )
        {

          // Here we've caught a rational coming out of analysis
          //Set rational to original_rational, and rational_search and setup children vector
          SetupRational(poincare_ic);

          avtVector point1, point2;
          std::vector<avtVector> seeds = GetSeeds(poincare_ic, point1, point2, MAX_SEED_SPACING);

          for( unsigned int s=0; s<seeds.size(); ++s )
            {   
              if (2 <= RATIONAL_DEBUG)std::cerr << "LINE " << __LINE__ << " New Seed: " 
                                                << VectorToString(seeds[s]);
             
              std::vector<avtIntegralCurve *> new_ics;
              avtVector vec(0,0,0);
              
              seeds[s][1] = Z_OFFSET;
              AddSeedPoint( seeds[s], vec, new_ics );
              
              for( unsigned int j=0; j<new_ics.size(); ++j )
                {
                  avtPoincareIC *seed = (avtPoincareIC *) new_ics[j];

                  SetupNewSeed(seed, poincare_ic, seeds[s], point1, point2);              
                  
                  if (2 <= RATIONAL_DEBUG)std::cerr << " with ID: " << seed->id <<"\n";
                }
              if (2 <= RATIONAL_DEBUG)std::cerr << std::endl;
            }
        }

        //////////////////
        // Grab each seed 
        //////////////////
        else if( properties.analysisMethod == FieldlineProperties::RATIONAL_SEARCH &&
                 properties.searchState    == FieldlineProperties::SEARCHING_SEED &&
                 (properties.analysisState  == FieldlineProperties::COMPLETED ||
                  properties.analysisState  == FieldlineProperties::TERMINATED))
          {
          if (1 <= RATIONAL_DEBUG)std::cerr << "LINE: " << __LINE__ << "  "
                                            << "Found seed ID: " << poincare_ic->id <<",pt: "<< VectorToString(poincare_ic->points[0])<< std::endl;

          if(NO_MINIMIZATION)
            {
              poincare_ic->properties.searchState = FieldlineProperties::WAITING_SEED;
              
            }
          else
            {
              bool needToMinimize = NeedToMinimize(poincare_ic);
              if( !needToMinimize )
                {
                  if (1 <= RATIONAL_DEBUG)std::cerr<< "LINE: " << __LINE__
                                                   << " Got lucky, this seed ID: " <<poincare_ic->id
                                                   <<"  is already very good: " << FindMinimizationDistance(poincare_ic)<<std::endl;
                  properties.searchState = FieldlineProperties::WAITING_SEED;
                }
              else
                {          // Otherwise, we need to bracket the min before we can minimize to it
                  // May as well guess our seed is nearest the minimum, so make it b
                  avtVector zeroVec = avtVector(0,0,0);
              
                  avtVector newA, newB, newC; //newA isn't used, poincare_ic is A
                  if (false == PrepareToBracket(poincare_ic, newA, newB, newC))
                    {
                      poincare_ic->properties.searchState = FieldlineProperties::WAITING_SEED;
                      /* std::vector<avtPoincareIC *> *children = poincare_ic->src_rational_ic->properties.children;
                      if (2 <= RATIONAL_DEBUG)
                        std::cerr << "LINE " << __LINE__ << "  " << "Failed to prepare bracketing, Deleting ID: "
                                  <<poincare_ic->id << std::endl;
                      ids_to_delete.push_back(poincare_ic->id);
                      if(std::find(children->begin(), children->end(), poincare_ic) != children->end())
                      children->erase(std::remove(children->begin(), children->end(), poincare_ic));*/
                    }
                  else
                    {
                      std::vector<avtIntegralCurve *> new_ics;
                      newA[1] =Z_OFFSET;                      
                      AddSeedPoint( newB, zeroVec, new_ics );
                      avtPoincareIC *seed_b;          
                      for( unsigned int k=0; k<new_ics.size(); k++ )
                        {
                          seed_b = (avtPoincareIC *) new_ics[k];
                          SetupNewBracketB(seed_b, poincare_ic, newB);
                        }
                  
                      // Setup 'c' (use the new point just calculated)
                      std::vector<avtIntegralCurve *> new_ics_2;
                      newC[1]=Z_OFFSET;
                      AddSeedPoint( newC, zeroVec, new_ics_2 );
                      avtPoincareIC *seed_c;
                  
                      for( unsigned int k=0; k<new_ics_2.size(); k++ )
                        {
                          seed_c = (avtPoincareIC *) new_ics_2[k];
                          SetupNewBracketC(seed_c, poincare_ic, newC);
                        }

                      // We catch the b curve when it comes back to bracket the minimum
                      seed_b->a_IC = poincare_ic;
                      seed_b->c_IC = seed_c;

                    }     
                }
            }
        }      
        else if (properties.analysisMethod == FieldlineProperties::RATIONAL_SEARCH &&
                 properties.searchState == FieldlineProperties::MINIMIZING_A)
          {
            // Intentionally empty
            if (5 <= RATIONAL_DEBUG)
              cerr << "Minimizing A found" << std::endl;
          }
        else if (properties.analysisMethod == FieldlineProperties::RATIONAL_SEARCH &&
                 properties.searchState == FieldlineProperties::MINIMIZING_C)
          {
            // Intentionally empty
            if (5 <= RATIONAL_DEBUG)
              cerr << "Minimizing C found" << std::endl;
          }
        else if ( properties.analysisMethod == FieldlineProperties::RATIONAL_SEARCH &&
                  properties.searchState == FieldlineProperties::MINIMIZING_B &&
                  poincare_ic->a_IC != NULL && poincare_ic->c_IC != NULL &&
                  std::find(ids_to_delete.begin(), ids_to_delete.end(), poincare_ic->id) == ids_to_delete.end())
        {
          avtPoincareIC *_a = poincare_ic->a_IC;
          avtPoincareIC *_b = poincare_ic;
          avtPoincareIC *_c = poincare_ic->c_IC;
          avtPoincareIC *seed = poincare_ic->src_seed_ic;
          
          Vector xzplane(0,1,0);
          FieldlineLib fieldlib;
          std::vector<avtVector> xa_puncturePoints;
          std::vector<avtVector> xb_puncturePoints;
          std::vector<avtVector> xc_puncturePoints;       
          fieldlib.getPunctures(_a->points,xzplane,xa_puncturePoints);
          fieldlib.getPunctures(_b->points,xzplane,xb_puncturePoints);
          fieldlib.getPunctures(_c->points,xzplane,xc_puncturePoints);
          int xa_i = FindMinimizationIndex(_a);
          int xb_i = FindMinimizationIndex(_b);
          int xc_i = FindMinimizationIndex(_c);

          if (1 <= RATIONAL_DEBUG)
            {
              std::cerr <<"Line: "<<__LINE__<< " Found Bracketing\n"
                        <<"Line: "<<__LINE__<< " A ID: " <<_a->id <<", dist: "<< FindMinimizationDistance(_a) <<"\tpt: " <<VectorToString(xa_puncturePoints[xa_i])<< std::endl;
              std::cerr <<"Line: "<<__LINE__<< " B ID: " <<_b->id <<", dist: "<< FindMinimizationDistance(_b) <<"\tpt: " <<VectorToString(xb_puncturePoints[xb_i])<< std::endl;
              std::cerr <<"Line: "<<__LINE__<< " C ID: "        <<_c->id <<", dist: "<< FindMinimizationDistance(_c) <<"\tpt: " <<VectorToString(xc_puncturePoints[xc_i])<< std::endl;
            }

          if (seed == NULL)
            {
              seed = poincare_ic;
              poincare_ic->src_seed_ic = poincare_ic;
            }

          std::vector<avtPoincareIC *> *children = poincare_ic->src_rational_ic->properties.children;

          if ( !BracketIsValid( poincare_ic ) )
            {         
            if (2 <= RATIONAL_DEBUG)
              {
                std::cerr << "LINE " << __LINE__ << "  " << "Deleting ID: "<<_a->id << std::endl;
                std::cerr << "LINE " << __LINE__ << "  " << "Deleting ID: "<<_b->id << std::endl;
                std::cerr << "LINE " << __LINE__ << "  " << "Deleting ID: "<<_c->id << std::endl;
              }    

            if (_a->id != seed->id)
              ids_to_delete.push_back(_a->id);
            else
              _a->properties.searchState = FieldlineProperties::WAITING_SEED;
            if (_b->id != seed->id)
              ids_to_delete.push_back(_b->id);
            else
              _b->properties.searchState = FieldlineProperties::WAITING_SEED;
            if (_c->id != seed->id)
              ids_to_delete.push_back(_c->id);
            else
              _c->properties.searchState = FieldlineProperties::WAITING_SEED;
          }
          // Pick the best curve, I guess
          else if (_a->properties.iteration > rationalSurfaceMaxIterations ||
                   _b->properties.iteration > rationalSurfaceMaxIterations ||
                   _c->properties.iteration > rationalSurfaceMaxIterations )
            {
              PickBestAndSetupToDraw(_a,_b,_c,NULL,ids_to_delete);
            }
          else
          {
            ///////////////////////////////////
            // If the b_dist is smaller than a or c, then we know a minimum lies between a & c
            // So, we have the min bracketed and here we setup the actual minimization. It requires 4 curves.
            if ( MinimumIsBracketed(poincare_ic) )
            {
              if (2 <= RATIONAL_DEBUG)
                std::cerr << "LINE " << __LINE__ << "  " 
                          << "Got B, minimum is bracketed, setup for minimization" << std::endl;

              avtVector newPt;
              bool cbGTba;
              PrepareToMinimize(poincare_ic, newPt, cbGTba);

              std::vector<avtIntegralCurve *> new_ics;
              avtVector zeroVec = avtVector(0,0,0);
              avtPoincareIC *newIC;
              int j;
              newPt[1]=Z_OFFSET;
              AddSeedPoint( newPt, zeroVec, new_ics );

              if (cbGTba)
                {
                  for( j=0; j<new_ics.size(); j++ )
                    {
                      newIC = (avtPoincareIC*) new_ics[j];
                      newIC->maxIntersections = 8 * properties.toroidalWinding + 2;
                      newIC->properties = poincare_ic->properties;
                      newIC->properties.searchState = FieldlineProperties::MINIMIZING_X2;
                      newIC->properties.type = FieldlineProperties::IRRATIONAL;
                      newIC->properties.analysisMethod = FieldlineProperties::RATIONAL_MINIMIZE;
                      newIC->properties.iteration = poincare_ic->properties.iteration + 1;
                      newIC->src_seed_ic = poincare_ic->src_seed_ic;
                      newIC->src_rational_ic = poincare_ic->src_rational_ic;
                      newIC->properties.srcPt = newPt;
                      if (2 <= RATIONAL_DEBUG)
                        std::cerr << "LINE " << __LINE__ << "  " <<  "Bracketed, New X2 ID: "<<newIC->id << std::endl;
                    }
                  newIC =  (avtPoincareIC*)new_ics[0];
                  _a->GS_x1 = _b;
                  _a->GS_x2 = newIC;
                  _a->GS_x3 = _c;
                  if (2 <= RATIONAL_DEBUG)
                    {
                      std::cerr << "LINE " << __LINE__ << "  " << "1 x0 ID: "<<_a->id << std::endl;
                      std::cerr << "LINE " << __LINE__ << "  " << "1 x1 ID: "<<_b->id << std::endl; 
                      std::cerr << "LINE " << __LINE__ << "  " << "1 x2 ID: "<<newIC->id << std::endl; 
                      std::cerr << "LINE " << __LINE__ << "  " << "1 x3 ID: "<<_c->id << std::endl;
                    }
                }
              else
                {                               
                  for( j=0; j<new_ics.size(); j++ )
                    {
                      newIC = (avtPoincareIC*)new_ics[j];
                      newIC->maxIntersections = 8 * properties.toroidalWinding + 2;
                      newIC->properties = poincare_ic->properties;
                      newIC->properties.searchState = FieldlineProperties::MINIMIZING_X1;
                      newIC->properties.analysisMethod = FieldlineProperties::RATIONAL_MINIMIZE;
                      newIC->properties.type = FieldlineProperties::IRRATIONAL;
                      newIC->properties.iteration = poincare_ic->properties.iteration + 1;
                      newIC->properties.srcPt = newPt;
                      newIC->src_seed_ic = poincare_ic->src_seed_ic;
                      newIC->src_rational_ic = poincare_ic->src_rational_ic;
                      if (2 <= RATIONAL_DEBUG)
                        std::cerr << "LINE " << __LINE__ << "  " << "Bracketed, new X1 ID: "<<newIC->id << std::endl;
                    }
                  newIC =  (avtPoincareIC*)new_ics[0];
                  _a->GS_x1 = newIC;
                  _a->GS_x2 = _b;
                  _a->GS_x3 = _c;
                  if (2 <= RATIONAL_DEBUG)
                    {
                      std::cerr << "LINE " << __LINE__ << "  " << "2 x0 ID: "<<_a->id << std::endl;
                      std::cerr << "LINE " << __LINE__ << "  " << "2 x1 ID: "<<newIC->id << std::endl;
                      std::cerr << "LINE " << __LINE__ << "  " << "2 x2 ID: "<<_b->id << std::endl;
                      std::cerr << "LINE " << __LINE__ << "  " << "2 x3 ID: "<<_c->id << std::endl;
                    }
                }
            }
            ////////////////////////
            // Otherwise, we still need to bracket the minimum
            else
            {
              bool swapped = false;
              avtVector C; // C will always be the new curve, a & c might swap
              bool result = UpdateBracket(poincare_ic, swapped, C);

              if (swapped)
                {
                  if (1 <= RATIONAL_DEBUG)
                    {
                      cerr << "Should swap A & B now " << _a->properties.iteration <<"\n";
                    }
                  _b->a_IC = NULL;
                  _b->c_IC = NULL;

                   avtPoincareIC *temp = _a;
                   _a = _b;
                   _b = temp;
                   
                   _a->properties.searchState =
                     FieldlineProperties::MINIMIZING_A;
                   _b->properties.searchState =
                     FieldlineProperties::MINIMIZING_B;
                   
                   _b->a_IC = _a;
                   _b->c_IC = _c;
                }

              _a->properties.iteration++;
              _b->properties.iteration++;
              _c->properties.iteration++;

              if (result == true) // otherwise, keep the same C (still may have swapped A & B)
                {
                  std::vector<avtIntegralCurve *> new_ics;
                  avtVector zeroVec = avtVector(0,0,0);
                  avtPoincareIC *newIC;
                  AddSeedPoint( C, zeroVec, new_ics );
                  
                  bool success = false;
                  for(int j=0; j<new_ics.size(); j++ )
                    {
                      success = true;
                      newIC = (avtPoincareIC*)new_ics[j];
                      newIC->maxIntersections = 8 * (properties.toroidalWinding + 2);
                      newIC->properties = poincare_ic->properties;
                      newIC->properties.searchState = FieldlineProperties::MINIMIZING_C;
                      newIC->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
                      newIC->properties.type = FieldlineProperties::IRRATIONAL;
                      newIC->properties.iteration = poincare_ic->properties.iteration + 1;
                      newIC->properties.srcPt = C;
                      newIC->src_seed_ic = poincare_ic->src_seed_ic;
                      newIC->src_rational_ic = poincare_ic->src_rational_ic;
                      
                      if (3 <= RATIONAL_DEBUG)
                        {
                          std::cerr << "LINE " << __LINE__ << "  " << "Bracketing Update:\nA ID: "<<_a->id << std::endl;
                          cerr<< "B ID: "<< _b->id << "\n";
                          cerr<< "C ID(old): "<< _c->id <<" , "<<VectorToString(C) << "\n";
                        }
                    }
                  
                  newIC =  (avtPoincareIC*)new_ics[0];
                  _b->c_IC = newIC;
                  
                  if (success)
                    {            
                      if(_c->id != seed->id && 
                         std::find(children->begin(), children->end(), _c) == children->end())
                        {
                          ids_to_delete.push_back(_c->id);
                          if (2 <= RATIONAL_DEBUG)
                            std::cerr << "LINE " << __LINE__ << "  " << "Deleting Old Bracketing Curve ID: "<<_c->id << std::endl;
                        }
                      else
                        {
                          _c->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
                          _c->properties.searchState = FieldlineProperties::WAITING_SEED;
                        }
                    }
                  else
                    {
                      ids_to_delete.push_back(_a->id);
                      ids_to_delete.push_back(_b->id);
                      ids_to_delete.push_back(_c->id);
                      if(std::find(children->begin(), children->end(), _a) != children->end())
                        children->erase(std::remove(children->begin(), children->end(), _a), children->end());
                      if(std::find(children->begin(), children->end(), _b) != children->end())
                        children->erase(std::remove(children->begin(), children->end(), _b), children->end());
                      if(std::find(children->begin(), children->end(), _c) != children->end())
                        children->erase(std::remove(children->begin(), children->end(), _c), children->end());
                      if (2 <= RATIONAL_DEBUG)
                        {
                          std::cerr << "LINE " << __LINE__ << "  " << "Failed to extend search bracket" << std::endl;
                          std::cerr << "LINE " << __LINE__ << "  " << "Deleting Bracketing Curve ID: "<<_a->id << std::endl;
                          std::cerr << "LINE " << __LINE__ << "  " << "Deleting Bracketing Curve ID: "<<_b->id << std::endl;
                          std::cerr << "LINE " << __LINE__ << "  " << "Deleting Bracketing Curve ID: "<<_c->id << std::endl;
                        }
                    }
                }
            }
          }
        }
        
        // Here's the meat of the minimization, once the bracketing stuff is all done
        // Grab X0, and go
        else if (poincare_ic->properties.analysisMethod == FieldlineProperties::RATIONAL_MINIMIZE &&
                 poincare_ic->properties.searchState == FieldlineProperties::MINIMIZING_X1){
            //      cerr << "Minimizing X1 found" << std::endl;
          }
        else if (poincare_ic->properties.analysisMethod == FieldlineProperties::RATIONAL_MINIMIZE &&
                 poincare_ic->properties.searchState == FieldlineProperties::MINIMIZING_X2){
            //      cerr << "Minimizing X2 found" << std::endl;
          }
        else if (poincare_ic->properties.analysisMethod == FieldlineProperties::RATIONAL_MINIMIZE &&
                 poincare_ic->properties.searchState == FieldlineProperties::MINIMIZING_X3){
            //      cerr << "Minimizing X3 found" << std::endl;
          }       
        else if (poincare_ic->properties.analysisMethod == FieldlineProperties::RATIONAL_MINIMIZE &&
                 poincare_ic->properties.searchState == FieldlineProperties::MINIMIZING_X0 &&
                 std::find(ids_to_delete.begin(), ids_to_delete.end(), poincare_ic->id) == ids_to_delete.end()){

          if (4 <= RATIONAL_DEBUG) std::cerr << "LINE " << __LINE__ << "  " 
                                        << "Done bracketing, found minimizing X0 ID: "<< poincare_ic->id 
                                        <<", finding minimum now" << std::endl;
          Vector xzplane(0,1,0);
          FieldlineLib fieldlib;
          
          avtPoincareIC *seed = poincare_ic->src_seed_ic;

          if ( seed == NULL )
            {
              poincare_ic->src_seed_ic = poincare_ic;
              seed = poincare_ic;
            }
          avtPoincareIC *_x0 = poincare_ic;
          avtPoincareIC *_x1 = poincare_ic->GS_x1;
          avtPoincareIC *_x2 = poincare_ic->GS_x2;      
          avtPoincareIC *_x3 = poincare_ic->GS_x3;    

          _x0->properties.iteration++;
          _x1->properties.iteration++;
          _x2->properties.iteration++;
          _x3->properties.iteration++;

          std::vector<avtPoincareIC *> *children = seed->src_rational_ic->properties.children;

          if(!(poincare_ic->GS_x1->points.size() > 0) ||
             !(poincare_ic->GS_x2->points.size() > 0) ||
             !(poincare_ic->GS_x3->points.size() > 0))
            {
              // Have to hang out
              if (3 <= RATIONAL_DEBUG)
                cerr << "Minimization waiting for curves to finish..." <<std::endl;
            }
          else{        
              
              if (3 <= RATIONAL_DEBUG)
                {
                  std::cerr << "Line " << __LINE__ << "  " << "Found Minimizing curves (x0)\n";
                  std::cerr << "LINE " << __LINE__ << "  " << "x0 ID: "<<_x0->id << std::endl;
                  std::cerr << "LINE " << __LINE__ << "  " << "x1 ID: "<<_x1->id << std::endl;
                  std::cerr << "LINE " << __LINE__ << "  " << "x2 ID: "<<_x2->id << std::endl;
                  std::cerr << "LINE " << __LINE__ << "  " << "x3 ID: "<<_x3->id << std::endl;
                }
              
              std::vector<avtVector> x0_puncturePoints;
              std::vector<avtVector> x1_puncturePoints;
              std::vector<avtVector> x2_puncturePoints;
              std::vector<avtVector> x3_puncturePoints;
              
              fieldlib.getPunctures(_x0->points,xzplane,x0_puncturePoints);
              fieldlib.getPunctures(_x1->points,xzplane,x1_puncturePoints);
              fieldlib.getPunctures(_x2->points,xzplane,x2_puncturePoints);
              fieldlib.getPunctures(_x3->points,xzplane,x3_puncturePoints);
              
              // Need to get distances, so get the index first
              int x0_i = FindMinimizationIndex(_x0);
              int x1_i = FindMinimizationIndex(_x1);
              int x2_i = FindMinimizationIndex(_x2);
              int x3_i = FindMinimizationIndex(_x3);
              
              //Sadly, we've come this far but it's over
              if (x0_i == -1 || 
                  x1_i == -1 ||
                  x2_i == -1 || 
                  x3_i == -1)
                {
                  
                  if (2 <= RATIONAL_DEBUG)
                    std::cerr << "LINE " <<  __LINE__ << "  " << "Either maxed out iterations, or Failed to get a minimization index... Fatal error" << std::endl;
                  
                  if (2 <= RATIONAL_DEBUG)
                    {
                      std::cerr << "LINE " << __LINE__ << "  " << "Deleting ID: "<<_x0->id << std::endl;
                      std::cerr <<"LINE " <<  __LINE__ << "  " << "Deleting ID: "<<_x1->id << std::endl;
                      std::cerr <<"LINE " <<  __LINE__ << "  " << "Deleting ID: "<<_x2->id << std::endl;
                      std::cerr <<"LINE " <<  __LINE__ << "  " << "Deleting ID: "<<_x3->id << std::endl;
                      std::cerr <<"LINE " <<  __LINE__ << "  " << "Deleting ID: "<<seed->id << std::endl;
                    }
                  ids_to_delete.push_back(_x0->id);
                  ids_to_delete.push_back(_x1->id);
                  ids_to_delete.push_back(_x2->id);
                  ids_to_delete.push_back(_x3->id);
                  ids_to_delete.push_back(seed->id);
                  
                  // One of these curves isn't showing up & we need all three of them or none
                  if(std::find(children->begin(), children->end(), _x0) != children->end())
                    children->erase(std::remove(children->begin(), children->end(), _x0), children->end());
                  if(std::find(children->begin(), children->end(), _x1) != children->end())
                    children->erase(std::remove(children->begin(), children->end(), _x1), children->end());
                  if(std::find(children->begin(), children->end(), _x2) != children->end())
                    children->erase(std::remove(children->begin(), children->end(), _x2), children->end());
                  if(std::find(children->begin(), children->end(), _x3) != children->end())
                    children->erase(std::remove(children->begin(), children->end(), _x3), children->end());           
                  if(std::find(children->begin(), children->end(), seed) != children->end())
                   children->erase(std::remove(children->begin(), children->end(), seed), children->end());
                }
              // Pick the best curve, I guess
              else if (poincare_ic->properties.iteration > rationalSurfaceMaxIterations ||
                       _x1->properties.iteration > rationalSurfaceMaxIterations ||
                       _x2->properties.iteration > rationalSurfaceMaxIterations ||
                       _x3->properties.iteration > rationalSurfaceMaxIterations)
                {
                  PickBestAndSetupToDraw(_x0,_x1,_x2,_x3,ids_to_delete);
                }
              else{     // Looks like we're in the clear
                if (2 <= RATIONAL_DEBUG)
                  {
                    cerr <<"LINE: "<<__LINE__ << " Our minimizing curves have returned. puncture pts:\n";
                    cerr<<"\t"<<"ID: "<<_x0->id<<"\t("<<FindMinimizationDistance(_x0)<<"), "
                        <<VectorToString(x0_puncturePoints[x0_i])<<std::endl;
                    cerr<<"\t"<<"ID: "<<_x1->id<<"\t("<<FindMinimizationDistance(_x1)<<"), "
                        <<VectorToString(x1_puncturePoints[x1_i])<<std::endl;
                    cerr<<"\t"<<"ID: "<<_x2->id<<"\t("<<FindMinimizationDistance(_x2)<<"), "
                        <<VectorToString(x2_puncturePoints[x2_i])<<std::endl;
                    cerr<<"\t"<<"ID: "<<_x3->id<<"\t("<<FindMinimizationDistance(_x3)<<"), "
                        <<VectorToString(x3_puncturePoints[x3_i])<<std::endl;
                  }

                  avtVector x0 = x0_puncturePoints[x0_i];
                  avtVector x1 = x1_puncturePoints[x1_i];
                  avtVector x2 = x2_puncturePoints[x2_i];
                  avtVector x3 = x3_puncturePoints[x3_i];

                  avtVector newPoint;
                  
                  std::vector<avtIntegralCurve *> new_ics;
                  avtVector zeroVec = avtVector(0,0,0);
                  avtPoincareIC *newIC;
                  int j;     
                  
                  double range1 = FindMinimizationDistance(_x1);
                  double range2 = FindMinimizationDistance(_x2);
                  double range = range1 < range2 ? range1 : range2;

                  if (1 <= RATIONAL_DEBUG)
                    cerr <<"LINE: "<<__LINE__<< " Range: " << range << "\n" ;
                  
                  bool cont = true;
                  if (range > MAX_SPACING){ // Not minimized yet
                      if (range2 < range1)
                        {
                          
                          newPoint = x2 + golden_R * (x3-x2);
                          newPoint[1] = Z_OFFSET;
                          
                          if (newPoint[0] == x0[0] && newPoint[2] == x0[2] ||
                              newPoint[0] == x1[0] && newPoint[2] == x1[2] ||
                              newPoint[0] == x2[0] && newPoint[2] == x2[2] ||
                              newPoint[0] == x3[0] && newPoint[2] == x3[2] )
                            { //We should bail and pick the best
                              PickBestAndSetupToDraw(_x0,_x1,_x2,_x3,ids_to_delete);
                              cont = false;
                            }
                          else
                            {
                              if (3 <= RATIONAL_DEBUG)
                                cerr <<"LINE " <<__LINE__<< "New Rational min search pt: " <<newPoint<<std::endl;
                          
                              AddSeedPoint( newPoint, zeroVec, new_ics );
                              for( j=0; j<new_ics.size(); j++ )
                                {
                                  newIC = (avtPoincareIC*)new_ics[j];
                                  newIC->properties = poincare_ic->properties;
                                  newIC->src_seed_ic = poincare_ic->src_seed_ic;
                                  newIC->src_rational_ic = poincare_ic->src_rational_ic;
                                  newIC->properties.srcPt = newPoint;
                                }
                              newIC =  (avtPoincareIC*)new_ics[0];
                              if(std::find(children->begin(), children->end(), _x0) == children->end())
                                {
                                  ids_to_delete.push_back(_x0->id);     
                                }
                              else
                                {
                                  if (seed->id != _x0->id)
                                    {
                                      if (2 <= RATIONAL_DEBUG)
                                        cerr << "LINE " << __LINE__ << "deleting ID: " << _x0->id << ",ID: " << seed->id << std::endl;  
                                      children->erase(std::remove(children->begin(), children->end(), _x0));
                                      ids_to_delete.push_back(_x0->id);     
                                    }
                                  else
                                    {
                                      _x0->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
                                      _x0->properties.searchState = FieldlineProperties::WAITING_SEED;
                                    }
                                }
                          
                              // Shift pointers to prepare for next run
                              _x0 = _x1;
                              _x1 = _x2;
                              _x2 = newIC;
                            }
                        }
                      else
                        {

                          newPoint = x1 + golden_R * (x0-x1);
                          newPoint[1] = Z_OFFSET;

                          if (newPoint[0] == x0[0] && newPoint[2] == x0[2] ||
                              newPoint[0] == x1[0] && newPoint[2] == x1[2] ||
                              newPoint[0] == x2[0] && newPoint[2] == x2[2] ||
                              newPoint[0] == x3[0] && newPoint[2] == x3[2] )
                            { //We should bail and pick the best
                              PickBestAndSetupToDraw(_x0,_x1,_x2,_x3,ids_to_delete);
                              cont = false;
                            }
                          else
                            {
                              if (3 <= RATIONAL_DEBUG)
                                cerr <<  "LINE " << __LINE__ <<"New Rational min search pt: " <<newPoint<<std::endl;

                              AddSeedPoint( newPoint, zeroVec, new_ics );
                          
                              for( j=0; j<new_ics.size(); j++ )
                                {
                                  newIC = (avtPoincareIC*)new_ics[j];
                                  newIC->properties = poincare_ic->properties;
                                  newIC->src_seed_ic = poincare_ic->src_seed_ic;
                                  newIC->src_rational_ic = poincare_ic->src_rational_ic;
                                  newIC->properties.srcPt = newPoint;
                                }
                          
                              if(std::find(children->begin(), children->end(), _x3) == children->end())
                                {
                                  ids_to_delete.push_back(_x3->id);              
                                }
                              else
                                {
                                  if (seed->id != _x3->id)
                                    {
                                      if (2 <= RATIONAL_DEBUG)
                                        cerr << "LINE " << __LINE__ << " deleting ID: " << _x3->id << ",ID: " << seed->id << std::endl;
                                      children->erase(std::remove(children->begin(), children->end(), _x3));
                                      ids_to_delete.push_back(_x3->id);     
                                    }
                                  else
                                    {
                                      _x3->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
                                      _x3->properties.searchState = FieldlineProperties::WAITING_SEED;
                                    }
                            }
                          
                          // Shift pointers to prepare for next run
                          _x3 = _x2;
                          _x2 = _x1;
                          _x1 = newIC;
                            }
                        }
                      
                      if (cont)
                        {
                          _x0->GS_x1 = _x1;
                          _x0->GS_x2 = _x2;
                          _x0->GS_x3 = _x3;
                          if (2 <= RATIONAL_DEBUG)
                            {
                              bool b = (range2 < range1);
                          
                              std::cerr << "LINE " << __LINE__ 
                                        << " ID: " << _x0->id<<"-> X0 (" 
                                        << FindMinimizationDistance(_x0) << "): " 
                                        <<  VectorToString(_x0->points[0]) << std::endl;

                              b ? std::cerr << "LINE " << __LINE__ 
                                            << " ID: " << _x1->id<< " -> X1 (" 
                                            << FindMinimizationDistance(_x1) << "): "
                                            << VectorToString(_x1->points[0]) << std::endl
                                :
                                std::cerr   << "LINE " << __LINE__ 
                                            << " ID: " << _x1->id<< " -> X1 NEWPT: " 
                                            <<  VectorToString(newPoint) << std::endl;

                              !b ? std::cerr << "LINE " << __LINE__ 
                                             << " ID: " << _x2->id<<" -> X2 ("
                                             << FindMinimizationDistance(_x2) << "): "
                                             << VectorToString(_x2->points[0]) << std::endl
                                : 
                                std::cerr  << "LINE " << __LINE__ 
                                           << " ID: " << _x2->id<<" -> X2 NEWPT: " 
                                           << VectorToString(newPoint) << std::endl;


                              std::cerr << "LINE " << __LINE__ << " ID: " 
                                        << _x3->id<<" -> X3 (" 
                                        << FindMinimizationDistance(_x3) << "): " 
                                        << VectorToString(_x3->points[0])<<"\nIteration Count: "
                                        <<_x3->properties.iteration << std::endl << std::endl;
                            }
                          _x0->properties.searchState = FieldlineProperties::MINIMIZING_X0;
                          _x1->properties.searchState = FieldlineProperties::MINIMIZING_X1;
                          _x2->properties.searchState = FieldlineProperties::MINIMIZING_X2;
                          _x3->properties.searchState = FieldlineProperties::MINIMIZING_X3;
                        }
                  }
                  // We have minimized!!!
                  else{
                      std::vector<avtPoincareIC *>::iterator it = children->begin();
                      if (range1 < range2)
                        {
                          // X1 is our minimum

                          if (1 <= RATIONAL_DEBUG)
                            std::cerr << "LINE " << __LINE__ << "  " << "Found the minimum ID: " << _x1->id << std::endl;
                          _x1->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
                          _x1->properties.searchState = FieldlineProperties::WAITING_SEED;
                          
                          ids_to_delete.push_back(_x0->id);
                          ids_to_delete.push_back(_x2->id);
                          ids_to_delete.push_back(_x3->id);
                          if(_x0->id != seed->id &&
                             std::find(children->begin(), children->end(), _x0) != children->end())
                            {
                              children->erase(std::remove(children->begin(), children->end(), _x0));
                              if (2 <= RATIONAL_DEBUG)
                                std::cerr << "LINE " << __LINE__ << "  " <<" Deleting ID: "<<_x0->id << std::endl;
                            }

                          if(_x2->id != seed->id &&
                             std::find(children->begin(), children->end(), _x2) != children->end())
                            {
                              children->erase(std::remove(children->begin(), children->end(), _x2));
                              if (2 <= RATIONAL_DEBUG)
                                std::cerr << "LINE " << __LINE__ << "  " <<" Deleting ID: "<<_x2->id << std::endl;
                            }
                          if(_x3->id != seed->id &&
                             std::find(children->begin(), children->end(), _x3) != children->end())
                            {
                              children->erase(std::remove(children->begin(), children->end(), _x3));
                              if (2 <= RATIONAL_DEBUG)
                                std::cerr << "LINE " << __LINE__ << "  " <<"Deleting ID: "<<_x3->id << std::endl;              
                            }

                          if (std::find(children->begin(), children->end(), seed) != children->end())
                            {
                              if (seed->id != _x1->id)
                                {
                                  std::replace(children->begin(),children->end(),seed,_x1);
                                  if (2 <= RATIONAL_DEBUG)
                                    std::cerr << "LINE " << __LINE__ << "  " << "x1 id: "<<_x1->id<<" swapped with seed ID: " <<seed->id << std::endl;
                                  ids_to_delete.push_back(seed->id);
                                }
                              else if (2 <= RATIONAL_DEBUG)
                                std::cerr << "LINE " << __LINE__ << "  " << "original seed was already minimum" << std::endl;
                            }
                          else if (2 <= RATIONAL_DEBUG)
                            std::cerr << "LINE " << __LINE__ << " Where's the seed...? id: " << seed->id <<std::endl;

                        }
                      else
                        {
                          // X2 is our guy

                          if (1 <= RATIONAL_DEBUG)
                            std::cerr << "LINE " << __LINE__ << "  " << "Found the minimum ID: " << _x2->id << std::endl;                         
                          _x2->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
                          _x2->properties.searchState = FieldlineProperties::WAITING_SEED;                        
                          
                          ids_to_delete.push_back(_x0->id);
                          ids_to_delete.push_back(_x1->id);
                          ids_to_delete.push_back(_x3->id);
                          if(_x0->id != seed->id &&
                             std::find(children->begin(), children->end(), _x0) != children->end())
                            {
                              children->erase(std::remove(children->begin(), children->end(), _x0));
                              if (2 <= RATIONAL_DEBUG)
                                std::cerr << "LINE " << __LINE__ << "  " <<" Deleting ID: "<<_x0->id << std::endl;
                            }
                          if(_x1->id != seed->id &&
                             std::find(children->begin(), children->end(), _x1) != children->end())
                            {
                              children->erase(std::remove(children->begin(), children->end(), _x1));
                              if (2 <= RATIONAL_DEBUG)
                                std::cerr << "LINE " << __LINE__ << "  " <<" Deleting ID: "<<_x1->id << std::endl;
                            }
                          if(_x3->id != seed->id &&
                             std::find(children->begin(), children->end(), _x3) != children->end())
                            {
                              children->erase(std::remove(children->begin(), children->end(), _x3));
                              if (2 <= RATIONAL_DEBUG)
                                std::cerr << "LINE " << __LINE__ << "  " <<" Deleting ID: "<<_x3->id << std::endl;                      
                            }
                          if (std::find(children->begin(), children->end(), seed) != children->end())
                            {
                              if (seed->id != _x2->id)
                                {
                                  std::replace(children->begin(),children->end(),seed,_x2);
                                  if (2 <= RATIONAL_DEBUG)
                                    std::cerr << "LINE " << __LINE__ << "  " << "x2 ID: "<<_x2->id <<"  swapped with seed ID: "<<seed->id << std::endl;
                                  ids_to_delete.push_back(seed->id);
                                  if(std::find(children->begin(), children->end(), seed) != children->end())
                                    children->erase(std::remove(children->begin(), children->end(), seed));
                                }
                              else if (2 <= RATIONAL_DEBUG)
                                  std::cerr << "LINE " << __LINE__ << "  " << "original seed was already minimum" << std::endl; 
                            }
                          else if (2 <= RATIONAL_DEBUG)
                            std::cerr << "LINE " << __LINE__ << " Where's the seed...? id: " << seed->id <<std::endl;
                        }
                  }
              }
          }
        }
        
        else
          {
            if (3 <= RATIONAL_DEBUG)
              {
                std::cerr << "LINE " << __LINE__ << "  " << "In ContinueExecute - " 
                          << poincare_ic->id << " case fell through" << std::endl;
              }
          }
        }
#endif

      }

      DeleteIntegralCurves( ids_to_delete );

      return true;
    }
    // No analysis requested or analysis complete, no need to
    // continue.
    else
    {
       return false;
    }
}


// ****************************************************************************
//  Method: avtPoincareFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Allows the filter to change its output's data object information, which
//      is a description of the data object.
//
//  Programmer: Dave Pugmire -- generated by xml2avt
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Apr  8 13:23:10 EDT 2009
//    Set topological dimension and normals request appropriately.
//
// ****************************************************************************

void
avtPoincareFilter::UpdateDataObjectInfo(void)
{ 
    avtDatasetOnDemandFilter::UpdateDataObjectInfo();

    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();
    avtDataValidity   &val  = GetOutput()->GetInfo().GetValidity();
    if (is_curvemesh)
    {
        atts.SetTopologicalDimension(1);
        val.SetNormalsAreInappropriate(true);
    }
    else
    {
        atts.SetTopologicalDimension(2);
        val.SetNormalsAreInappropriate(false);
    }

    if (! atts.ValidVariable("colorVar"))
    {
        atts.AddVariable("colorVar");
        atts.SetActiveVariable("colorVar");
        atts.SetVariableDimension(1);
    }
}

// ****************************************************************************
//  Method: avtPoincareFilter::ClassifyFieldlines
//
//  Purpose:
//      Classify the fieldlines (toroidal/poloidal winding).
//
//  Arguments:
//
//  Returns:      void
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//
// ****************************************************************************

bool
avtPoincareFilter::ClassifyFieldlines(std::vector<avtIntegralCurve *> &ics)
{
    FieldlineLib FLlib;
    FLlib.verboseFlag = verboseFlag;

    debug5 << "Classifying Fieldlines " << std::endl;

    bool analysisComplete = true;

    for ( int i=0; i<ics.size(); ++i )
    {
        avtPoincareIC * poincare_ic = (avtPoincareIC *) ics[i];
        FieldlineProperties &properties = poincare_ic->properties;

        // If the analysis is completed then skip it.
        if( properties.analysisMethod != FieldlineProperties::DEFAULT_METHOD ||
            properties.analysisState  == FieldlineProperties::COMPLETED ||
            properties.analysisState  == FieldlineProperties::TERMINATED )
        {
//           std::cerr <<"Skipping Classified Fieldline: id = "
//                     << poincare_ic->id << "  "
//                  << "with "
//                  << poincare_ic->GetNumberOfSamples() << "  "
//                  << "points."
//                  << std::endl;

          continue;
        }

        // Pass the maxPunctures so that fieldlines can be terminated
        // if needed.
        poincare_ic->properties.maxPunctures = maxPunctures;
        
        // Perform the fieldline analysis.
        FLlib.fieldlineProperties( poincare_ic->points,
                                   poincare_ic->properties,
                                   overrideToroidalWinding,
                                   overridePoloidalWinding,
                                   maximumToroidalWinding,
                                   windingPairConfidence,
                                   rationalSurfaceFactor,
                                   showOPoints,
                                   performOLineAnalysis ? OLineToroidalWinding : 0,
                                   OLineAxisFileName );

//      std::cerr << "Analysis of Fieldline: id = "
//                << poincare_ic->id << "  "
//                << "fieldline status " << poincare_ic->status << "  "
//                << "analysis status " << poincare_ic->properties.analysisState
//                << std::endl;

        // Did the analysis but the integration can not continue
        // because it was terminated rather having a normal finish. So
        // regardless of the analysis terminate the fieldline analysis
        // because additional integration steps are not possible.
        if( poincare_ic->status.Terminated() &&
            !poincare_ic->TerminatedBecauseOfMaxIntersections() )
        {
          poincare_ic->properties.nPuncturesNeeded = 0;
          poincare_ic->properties.analysisState =
            FieldlineProperties::TERMINATED;

//           std::cerr << "Terminated Fieldline: id = "
//                     << poincare_ic->id << "  "
//                  << std::endl;
        }

        // Additional puncture points are being requested.
        else if( poincare_ic->properties.analysisState ==
                 FieldlineProperties::ADDING_POINTS )
        {
          poincare_ic->status.ClearTerminationMet();

          // Do not add more points than the user specified.
          if( poincare_ic->properties.nPuncturesNeeded > maxPunctures )
            poincare_ic->properties.nPuncturesNeeded = maxPunctures;

          // Set the number of intersections (punctures) for the curve.

          // Note the number of punctures is 2x because the fieldline
          // analysis uses only the punctures in the same direction as
          // the puncture plane normal while the integral curve uses
          // the plane regardless of the normal.

          poincare_ic->maxIntersections =
            2 * poincare_ic->properties.nPuncturesNeeded;

          // Change the status so more integration steps will be taken.
          poincare_ic->status.SetOK();

          // Make more analysis is done in the poincare filter.
          analysisComplete = false;

//           std::cerr <<"Adding points to Fieldline: id = "
//                     << poincare_ic->id << "  "
//                  << "with "
//                  << poincare_ic->GetNumberOfSamples() << "  "
//                  << "points."
//                  << std::endl;
        }

        // See if O Points from an island need to be added.
        else if( poincare_ic->properties.analysisState ==
                 FieldlineProperties::ADD_O_POINTS )
        {
          poincare_ic->status.ClearTerminationMet();

          // Make sure more analysis is done in the poincare filter
          // once O point seeds are added to the queue.
          analysisComplete = false;
        }

        // See if a seed for finding the island boundary need to be added.
        else if( poincare_ic->properties.analysisState ==
                 FieldlineProperties::ADD_BOUNDARY_POINT ||

                 properties.searchState ==
                 FieldlineProperties::ISLAND_BOUNDARY_SEARCH )
        {
          poincare_ic->status.ClearTerminationMet();

          // Make sure more analysis is done in the poincare filter
          // once boundary seed points are added to the queue.
          analysisComplete = false;
        }
        // Catch all for completed or terminated fieldlines
        else
        {
          // The integration status should FINSIHED but just in case.
          poincare_ic->status.SetTerminationMet();

//           std::cerr << "Finished Fieldline: id = "
//                     << poincare_ic->id << "  "
//                  << "analysis status " << poincare_ic->properties.analysisState
//                  << std::endl;
        }

        double safetyFactor;
        
        if ( poincare_ic->properties.poloidalWinding > 0 )
            safetyFactor = ( (double) poincare_ic->properties.toroidalWinding /
                             (double) poincare_ic->properties.poloidalWinding );
        else
            safetyFactor = 0;

        if( summaryFlag )
         std::cerr << "Classify Fieldline: id = "<< poincare_ic->id
               << "  ptCnt = " << poincare_ic->points.size()
               << "  type = " << poincare_ic->properties.type
               << "  toroidal/poloidal windings = "
               << poincare_ic->properties.toroidalWinding << ","
               << poincare_ic->properties.poloidalWinding
               << "  (" << safetyFactor << ")"
               << "  toroidal/poloidal resonance = "
               << poincare_ic->properties.toroidalResonance << ","
               << poincare_ic->properties.poloidalResonance
               << "  windingGroupOffset = "
               << poincare_ic->properties.windingGroupOffset
               << "  islands = " << poincare_ic->properties.islands
               << "  nodes = " << poincare_ic->properties.nnodes
               << "  nPuncturesNeeded = " << poincare_ic->properties.nPuncturesNeeded
               << "  analysis state = " << poincare_ic->properties.analysisState
               << "  search state = " << poincare_ic->properties.searchState
//               << (poincare_ic->ic->status == avtIntegralCurve::STATUS_FINISHED ? 
//                   0 : poincare_ic->ic->maxIntersections )
               << std::endl << std::endl;
    }

    debug5 << "Classifying fieldlines"
         << (analysisComplete ? "Analysis completed" : "Analysis was not complete")
         << std::endl;

    return analysisComplete;
}

// ****************************************************************************
//  Method: avtPoincareFilter::ClassifyRationals
//
//  Purpose:
//      Analyze rational seeds.
//
//  Arguments:
//
//  Returns:      bool
//                                      True: if 
//
//  Programmer: Jake Van Alstyne
//  Creation:   Sun Jun 12 11:18:52 PDT 2011
//
//  Modifications:
//
// ****************************************************************************

bool
avtPoincareFilter::ClassifyRationals(std::vector<avtIntegralCurve *> &ics)
{
#ifdef RATIONAL_SURFACE

  if( !showRationalSurfaces )
    return true;

  /////////////// JAKE YOU HAVE ACCESS TO THIS MEMBER VARIABLE TO
  /////////////// LIMIT THE NUMBER OF SEARCH ITERATIONS.
  /////////////// rationalSurfaceMaxIterations;

  bool inRationalSearch = false;
  bool haveNewCompletedRational = false;
  bool seedsAreMinimizing = false;
  bool seedSearching = false;
  bool origFlag = false;
  bool origRational = false;
  
  // 2d array - 1st is for each rational, 2nd is the collection of
  // curves belonging to the rational
  std::map<long, std::vector<avtPoincareIC *> > rationalCurves;
  std::map<long, int> rationalCounts; // Number of waiting curves per rational.
  std::map<long, int> waitingCounts;
  
  // Count up waiting fieldlines for each original_rational
  // Keep it organized by original_rational
  for( int i=0; i<ics.size(); ++i )
  {
    avtPoincareIC * poincare_ic = (avtPoincareIC *) ics[i];
    
    FieldlineProperties &properties = poincare_ic->properties;

    // If we have curves involved in the rational search
    if( properties.analysisMethod == FieldlineProperties::RATIONAL_SEARCH ||
        properties.analysisMethod == FieldlineProperties::RATIONAL_MINIMIZE )
    {
      inRationalSearch = true;
      // Seeds
      if( properties.searchState == FieldlineProperties::SEARCHING_SEED ||
          properties.searchState == FieldlineProperties::WAITING_SEED)
      {   
        if (2 <= RATIONAL_DEBUG)
          cerr << "Found a seed ID: " << poincare_ic->id << "\n";
        seedSearching = true;
        rationalCurves[poincare_ic->src_rational_ic->id].push_back(poincare_ic);
        
        // Initialize and update the counts       
        rationalCounts[poincare_ic->src_rational_ic->id]++;
        
        if( properties.searchState == FieldlineProperties::WAITING_SEED)
          waitingCounts[poincare_ic->src_rational_ic->id]++;
      }
      else if( properties.searchState == FieldlineProperties::MINIMIZING_A ||
               properties.searchState == FieldlineProperties::MINIMIZING_B ||
               properties.searchState == FieldlineProperties::MINIMIZING_C ||
               properties.searchState == FieldlineProperties::MINIMIZING_X0 ||
               properties.searchState == FieldlineProperties::MINIMIZING_X1 ||
               properties.searchState == FieldlineProperties::MINIMIZING_X2 ||
               properties.searchState == FieldlineProperties::MINIMIZING_X3 )
      {
        seedsAreMinimizing = true;
        seedSearching = true;
        if(2 <= RATIONAL_DEBUG)
          cerr << "Found a minimizing curve ID: " << poincare_ic->id << "\n";
      }
      else if( properties.searchState == FieldlineProperties::ORIGINAL_RATIONAL)
      {
        if (2 <= RATIONAL_DEBUG)
          cerr << "Found the original rational ID: " << poincare_ic->id << "\n";

        origRational = true;
      } 
    }
    
    // Otherwise, we need to continueExecute if we have a completed rational
    else if( properties.analysisMethod == FieldlineProperties::DEFAULT_METHOD && 
             properties.type           == FieldlineProperties::RATIONAL &&
             properties.searchState != FieldlineProperties::ORIGINAL_RATIONAL &&
             (properties.analysisState  == FieldlineProperties::COMPLETED ||
              properties.analysisState  == FieldlineProperties::TERMINATED) )
    {
      if (2 <= RATIONAL_DEBUG)
        std::cerr << "Found NEW rational to begin search ID: " << poincare_ic->id << "\n";

      haveNewCompletedRational = true;
    }
    else if( properties.searchState == FieldlineProperties::ORIGINAL_RATIONAL)
      {
        if (2 <= RATIONAL_DEBUG)
          std::cerr << "LINE " << __LINE__ << " Found an original rational ID: " << poincare_ic->id << "\n";
        origRational = true;
      }
    else if( properties.searchState == FieldlineProperties::DEAD_SEED)
      {
        if (2 <= RATIONAL_DEBUG)
          std::cerr << "LINE " << __LINE__ << " Found a dead seed ID: " << poincare_ic->id << "\n";     
      }
  }
  
  // We aren't doing anything so return that we are done
  if (!inRationalSearch)
    {
      if (2 <= RATIONAL_DEBUG)
        cerr << "done, not in rational search\n";
      return !haveNewCompletedRational;
    }

  // If we got into rational search but found no seeds, 
  // those rationals (triggering origRational) failed 
  // and all the others are done
  if (!seedSearching && origRational)
    {
      if (2 <= RATIONAL_DEBUG)
        cerr << "No seeds\n";
      return true;
    }
  
  std::map<long, std::vector<avtPoincareIC *> >::iterator itr;
  long long numRationalCurves, numFinishedCurves;
  numRationalCurves = 0, numFinishedCurves = 0;
  for( itr = rationalCurves.begin();  itr != rationalCurves.end(); ++itr )
  { 
    std::vector<avtPoincareIC *> curves = (*itr).second;
    std::vector<avtPoincareIC *>::iterator inneritr;

    for( inneritr = curves.begin(); inneritr != curves.end(); ++inneritr )
    {
      numRationalCurves++;
      // If all the curves for a rational are waiting…
      if( rationalCounts[(*inneritr)->src_rational_ic->id] ==
          waitingCounts[(*inneritr)->src_rational_ic->id] )
      {
        if (2 <= RATIONAL_DEBUG)
          cerr << "Got a finished seed here\n";
        numFinishedCurves++;
        (*inneritr)->properties.searchState = FieldlineProperties::FINISHED_SEED;
      }
    }
  }

  if (seedSearching && origFlag)
    std::cerr << "LINE " << __LINE__ << " Problem!" << std::endl;
  
  // If we've come this far, then we are done unless there are
  // unfinished curves or there is a new rational
  if (2 <= RATIONAL_DEBUG)
    cerr << "Leaving ClassifyRationals with "<<
      numRationalCurves<<", "<<numFinishedCurves<<", "<<haveNewCompletedRational
         <<", "<<seedsAreMinimizing<<", "<<origFlag<<"\n";

  return (numRationalCurves == numFinishedCurves &&
          !haveNewCompletedRational && !seedsAreMinimizing && !origFlag);
#else
  return true;
#endif
}


// ****************************************************************************
//  Method: avtPoincareFilter::misc crap
//
//  Purpose:
//      Create poincare output
//
//  Arguments:
//
//  Returns:      Poincare segments
//
//  Programmer: Allen Sanderson
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
// ****************************************************************************
template< class T > int
pairsortfirst( const std::pair < T, T > s0, const std::pair < T, T > s1 )
{
  return (s0.first > s1.first );
}

template< class T > int
pairsortsecond( const std::pair < T, T > s0, const std::pair < T, T > s1 )
{
  return (s0.second > s1.second );
}

void realDFTamp( std::vector< double > &g, std::vector< double > &G )
{
  unsigned int N = g.size();

  G.resize(N/2);

  for(unsigned int i=0; i<N/2; i++)
  {
    double freq = double(i) / double(N);

    double GRe = 0;
    double GIm = 0;

    for( unsigned int j=0; j<N; j++)
    {
      double a = -2.0 * M_PI * double(j) * freq;
//    if(inverse) a *= -1.0;
      double ca = cos(a);
      double sa = sin(a);
      
      GRe += g[j] * ca; // - in[x][1] * sa;
      GIm += g[j] * sa; // + in[x][1] * ca;
    }

    G[i] = sqrt(GRe*GRe + GIm*GIm);
  }
}


// ****************************************************************************
//  Method: avtPoincareFilter::CreatePoincareOutput
//
//  Purpose:
//      Create poincare output
//
//  Arguments:
//
//  Returns:      Poincare segments
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//    Dave Pugmire (for Allen Sanderson), Wed Feb 25 09:52:11 EST 2009
//    Add terminate by steps, add AdamsBashforth solver, Allen Sanderson's new code.
//
//    Dave Pugmire, Fri Apr 17 11:32:40 EDT 2009
//    Add variables for dataValue var.
//
//    Dave Pugmire, Tue Apr 28 09:26:06 EDT 2009
//    Changed color to dataValue
//
//    Dave Pugmire, Wed May 27 15:03:42 EDT 2009
//    Replacedstd::cerr/cout with debug5.
//
// ****************************************************************************
void
avtPoincareFilter::CreatePoincareOutput( avtDataTree *dt,
                                         std::vector<avtIntegralCurve *> &ic)
{
    FieldlineLib FLlib;
    FLlib.verboseFlag = verboseFlag;

    debug5 << "Creating output " << std::endl;

    if( summaryFlag ) 
       std::cerr << std::endl << std::endl << "count " << ic.size() << std::endl << std::endl;

    for ( size_t i=0; i<ic.size(); ++i )
    {
        avtPoincareIC * poincare_ic = (avtPoincareIC *) ic[i];

        FieldlineProperties &properties = poincare_ic->properties;

        // Skip rational-search curves
        if (properties.analysisMethod != FieldlineProperties::DEFAULT_METHOD)
          continue;

#ifdef RATIONAL_SURFACE
        if (showRationalSurfaces) // don't draw non-rationals
          continue;
#endif

        FieldlineProperties::FieldlineType type = properties.type;
        bool complete =
          (properties.analysisState == FieldlineProperties::COMPLETED);

        unsigned int toroidalWinding    = properties.toroidalWinding;
        unsigned int poloidalWinding    = properties.poloidalWinding;
        unsigned int toroidalWindingP   = properties.toroidalWindingP;
        unsigned int poloidalWindingP   = properties.poloidalWindingP;
        unsigned int toroidalResonance  = properties.toroidalResonance;
        unsigned int poloidalResonance  = properties.poloidalResonance;
        unsigned int windingGroupOffset = properties.windingGroupOffset;
        unsigned int islands            = properties.islands;
        unsigned int islandGroups       = properties.islandGroups;
        unsigned int nnodes             = properties.nnodes;

        std::vector< avtVector > &seedPoints = properties.seedPoints;

        bool completeIslands = true;

        if( summaryFlag )
        {
          double safetyFactor;

          if( poloidalWinding > 0 )
            safetyFactor = (float) toroidalWinding / (float) poloidalWinding;
          else
            safetyFactor = 0;

          std::cerr << "Surface id = " << poincare_ic->id << "  < "
                    << poincare_ic->points[0].x << " "
                    << poincare_ic->points[0].y << " "
                    << poincare_ic->points[0].z << " >  "
                    << toroidalWinding << "," << poloidalWinding << " ("
                    << safetyFactor << ")  ";

          if( type == FieldlineProperties::RATIONAL )
            std::cerr << "rational surface  ";
          
          else if( type == FieldlineProperties::FLUX_SURFACE )
            std::cerr << "flux surface  ";

          else if( type == FieldlineProperties::O_POINT )
            std::cerr << "O Point  ";
          
          else if( type == FieldlineProperties::X_POINT )
            std::cerr << "X Point  ";
          
          else if( type == FieldlineProperties::ISLAND_PRIMARY_CHAIN )
            std::cerr << islands << " island chain with resonances: "
                      << toroidalResonance << "," << poloidalResonance << "  ";
          
          else if( type == FieldlineProperties::ISLAND_SECONDARY_CHAIN )
            std::cerr << islands << " islands total ("
                      << islandGroups << " islandGroups with "
                      << islands/islandGroups << " islands each) with resonances: "
                      << toroidalResonance << "," << poloidalResonance << "  ";

          else if( type == FieldlineProperties::ISLAND_PRIMARY_SECONDARY_AXIS )
          {
            std::cerr << islands << " island chain with a secondary axis: "
                      << toroidalWindingP << "," << poloidalWindingP << " ("
                      << (float) toroidalWindingP / (float) poloidalWindingP << ")  ";
          }
          else if( type == FieldlineProperties::ISLAND_SECONDARY_SECONDARY_AXIS )
          {
            std::cerr << islands << " islands around "
                      << islandGroups << " islandGroups with resonances: "
                      << toroidalResonance << "," << poloidalResonance << "  ";
            std::cerr << islands << " with a secondary axis: "
                      << toroidalWindingP << "," << poloidalWindingP << " ("
                      << (float) toroidalWindingP / (float) poloidalWindingP << ")  ";
          }
          else if( type == FieldlineProperties::CHAOTIC )
            std::cerr << "chaotic  ";
          
          else if( type == FieldlineProperties::UNKNOWN_TYPE )
            std::cerr << "unknown  ";

          std::cerr << "with " << nnodes << " nodes"
                    << (complete ? " (Complete)  " : "  ")
                    << std::endl;
          
          if( type & FieldlineProperties::ISLAND_CHAIN &&
              toroidalWinding != poloidalWinding &&
              islands != toroidalWinding )
            std::cerr << "WARNING - The island count does not match the toroidalWinding count" << std::endl;
        }
        
        if( type == FieldlineProperties::ISLAND_PRIMARY_SECONDARY_AXIS )
        {
            toroidalWinding = islands * toroidalWindingP;
            poloidalWinding = islands * poloidalWindingP;
            windingGroupOffset = islands * windingGroupOffset;
            islands = 0;
        }

        else if( type == FieldlineProperties::ISLAND_SECONDARY_SECONDARY_AXIS )
        {
            toroidalWinding = toroidalWindingP;
            poloidalWinding = poloidalWindingP;
            windingGroupOffset = islands * windingGroupOffset;
            islands = 0;
        }
        else if( toroidalWinding == poloidalWinding )
        {
          toroidalWinding = poloidalWinding = 1;
          windingGroupOffset = 0;
        }

          
        // If toroidal winding is zero, skip it.
        if( type == FieldlineProperties::CHAOTIC )
        {
          if( showChaotic )
          {
            if( toroidalWinding == 0 )
              toroidalWinding = toroidalWindingP = 1;
            if( poloidalWinding == 0 )
              poloidalWinding = poloidalWindingP = 1;
          }
          else
          {
            continue;
          }
        }
        else if( type == FieldlineProperties::UNKNOWN_TYPE ) 
        {
          if( analysis == 0 )
          {
            toroidalWinding = 1;
            poloidalWinding = 1;
            toroidalWindingP = 1;
            poloidalWindingP = 1;
          }
          else
          {
            if( summaryFlag ) 
              std::cerr << " id = " << poincare_ic->id
                        << " SKIPPING UNKNOWN TYPE " << std::endl;
            
            std::pair< unsigned int, unsigned int > topo( 0, 0 );
            
            continue;
          }
        }
        else if( toroidalWinding == 0 ) 
        {
            if( summaryFlag ) 
              std::cerr << " id = " << poincare_ic->id
                        << " SKIPPING TOROIDAL WINDING OF 0" << std::endl;
            
            std::pair< unsigned int, unsigned int > topo( 0, 0 );
            
            continue;
        }
        else if( poloidalWinding == 0 ) 
        {
            if( summaryFlag ) 
              std::cerr << " id = " << poincare_ic->id
                        << " SKIPPING POLOIDAL WINDING OF 0" << std::endl;
            
            std::pair< unsigned int, unsigned int > topo( 0, 0 );
            
            continue;
        }

        // Get the direction of the fieldline toroidalWinding.
        Point lastPt = poincare_ic->points[0];
        Point currPt = poincare_ic->points[1];
        
        bool CCWfieldline = (atan2( lastPt.y, lastPt.x ) <
                              atan2( currPt.y, currPt.x ));
        
        double lastDist, currDist;

        // Put all of the points into the bins for each plane.
        std::vector< std::vector< std::vector < avtVector > > > puncturePts;
        
        puncturePts.resize( planes.size() );

        std::vector < avtVector > distancePts;

        std::vector< std::vector < avtVector > > islandPts;
        
        unsigned int startIndex = 0;
        
        for( unsigned int p=0; p<planes.size(); ++p ) 
        {
            Vector planeN;
            Vector planePt(0,0,0);
            
            if( puncturePlane == 0 ) // Poloidal Plane
            {
              // Go through the planes in the same direction as the fieldline.
              if( CCWfieldline )
              {
                planeN = Vector( cos(planes[p]),
                                 sin(planes[p]),
                                 0 );
              }
              else
              {
                planeN = Vector( cos(planes[planes.size()-1-p]),
                                 sin(planes[planes.size()-1-p]),
                                 0 );
              }
            }

            else //if( puncturePlane == 1 ) // Toroidal Plane
            {
              planeN = Vector( 0, 0, -1 );
            }

            // Set up the plane equation.
            double plane[4];
            
            plane[0] = planeN.x;
            plane[1] = planeN.y;
            plane[2] = planeN.z;
            plane[3] = planePt.dot(planeN);
            
            puncturePts[p].resize( toroidalWinding );
            int bin = 0;
            
            // So to get the winding groups consistant start examining
            // the fieldline in the same place for each plane.
            currPt = poincare_ic->points[startIndex];
            currDist = planeN.dot( currPt ) - plane[3];
            
            for( unsigned int j=startIndex+1; j<poincare_ic->points.size(); ++j )
            {
                lastPt = currPt;
                currPt = Vector(poincare_ic->points[j]);
                
                lastDist = currDist;
                currDist = Dot( planeN, currPt ) - plane[3];
                
                // First look at only points that intersect the plane.
                if( SIGN(lastDist) != SIGN(currDist) ) 
                {
                    Vector dir(currPt-lastPt);
                    
                    double dot = Dot(planeN, dir);
                    
                    // If the segment is in the same direction as the plane then
                    // find where it intersects the plane.
                    if( dot > 0.0 )
                    {
                        // In order to get the winding groups
                        // consistant start examining the fieldline
                        // in the same place for each plane so store
                        // the index of the first puncture point.
                        if( startIndex == 0 )
                            startIndex = j - 1;
                        
                        Vector w = lastPt - planePt;
                        
                        double t = -Dot(planeN, w ) / dot;
                        
                        Point point = Point(lastPt + dir * t);
                        
                        puncturePts[p][bin].push_back( point );
                        
                        if( p == 0 && puncturePts[p][bin].size() > 1 )
                        {
                          int ic = puncturePts[p][bin].size()-2;

                          double len = (puncturePts[p][bin][ic]-
                                        puncturePts[p][bin][ic+1]).length();
                  
                          distancePts.push_back( Point( (float) distancePts.size()/50.0,
                                                    0,
                                                    len) );
                        }

                        bin = (bin + 1) % toroidalWinding;

                    }
                }
            }

            if( p == 0 && islands )
            {
              int offset = nnodes;
              
              islandPts.resize( toroidalWinding );
              
              for( unsigned int j=0; j<toroidalWinding; ++j )
              {
                for( unsigned int k=offset; k<puncturePts[p][j].size(); ++k )
                {
                  double len = (puncturePts[p][j][k-offset]-
                                puncturePts[p][j][k]).length();
                  
                  islandPts[j].push_back( Point( (float) islandPts[j].size()/50.0,
                                                 0,
                                                 -1.5+(float)i*.1+len) );
                  
                }
                
//              std::vector< pair< unsigned int, double > > stats;           
//              FLlib.periodicityStats( islandPts[j], stats, 2 );
              }
            }
        }
        

        // Get the ridgeline points. There is one point between each
        // Z plane puncture.
        Vector planeN( 0, 0, 1 );
        Vector planePt(0,0,0);
        
        // Set up the plane equation.
        double plane[4];

        plane[0] = planeN.x;
        plane[1] = planeN.y;
        plane[2] = planeN.z;
        plane[3] = planePt.dot(planeN);
            
        std::vector < avtVector > ridgelinePts;

        // Start looking for the z max after the first Z plane
        // intersetion is found.
        bool haveFirstIntersection = false;
        double maxZ = 0;

        // To get the winding groups consistant start examining the
        // fieldline in the same place for each plane.
        currPt = poincare_ic->points[0];
        currDist = planeN.dot( currPt ) - plane[3];
            
        for( unsigned int j=startIndex+1;
             j<poincare_ic->points.size();
             ++j )
        {
          lastPt = currPt;
          currPt = Vector(poincare_ic->points[j]);
          
          lastDist = currDist;
          currDist = Dot( planeN, currPt ) - plane[3];
          
          // First look at only points that intersect the plane.
          if( SIGN(lastDist) != SIGN(currDist) ) 
          {
            Vector dir(currPt-lastPt);
            
            double dot = Dot(planeN, dir);
            
            // If the segment is in the same direction as the plane then
            // record the max Z value.
            if( dot > 0.0 )
            {
              if( haveFirstIntersection )
              {
                ridgelinePts.push_back( Point( (float) ridgelinePts.size()/50.0,
                                               0,
                                               maxZ) );
              }
              else
                haveFirstIntersection = true;

              maxZ = 0;
            }
          }

          if( maxZ < currPt.z )
            maxZ = currPt.z;
        }

        // Have the puncture points now draw them ...
        for( unsigned int p=0; p<planes.size(); p++ ) 
        {
            if( type == FieldlineProperties::UNKNOWN_TYPE ||
                type == FieldlineProperties::CHAOTIC ||
                type == FieldlineProperties::ISLAND_PRIMARY_SECONDARY_AXIS ||
                type == FieldlineProperties::ISLAND_SECONDARY_SECONDARY_AXIS )
              nnodes = puncturePts[p][0].size();

            else if( type == FieldlineProperties::FLUX_SURFACE )
            {
                if( overlaps == 1 || overlaps == 3 )
                    FLlib.removeOverlap( puncturePts[p], nnodes,
                                         toroidalWinding, poloidalWinding,
                                         windingGroupOffset, islands );
                if( overlaps == 2 )
                    FLlib.mergeOverlap( puncturePts[p], nnodes,
                                        toroidalWinding, poloidalWinding,
                                        windingGroupOffset, islands );
                else if( overlaps == 3 )
                    FLlib.smoothCurve( puncturePts[p], nnodes,
                                       toroidalWinding, poloidalWinding,
                                       windingGroupOffset, islands );
            }
            else if( type == FieldlineProperties::O_POINT ||
                     type == FieldlineProperties::X_POINT )
            {
              if( overlaps != 0 )
              {
                // Loop through each island.
                for( unsigned int j=0; j<toroidalWinding; j++ )
                {
                  // Erase all of the overlapping points.
                  puncturePts[p][j].erase( puncturePts[p][j].begin()+nnodes,
                                           puncturePts[p][j].end() );
                  
                  // Close the island if it is complete
                  puncturePts[p][j].push_back( puncturePts[p][j][0] );
                }
              }
            }
            else if( type == FieldlineProperties::ISLAND_PRIMARY_CHAIN ||
                     type == FieldlineProperties::ISLAND_SECONDARY_CHAIN )
            {
              if( overlaps != 0 )
              {
                if( properties.analysisState == FieldlineProperties::COMPLETED ||
//                    properties.analysisState == FieldlineProperties::TERMINATED ||
                    0 )
                {
                  // Loop through each island.
                  for( unsigned int j=0; j<toroidalWinding; j++ )
                  {
                    // Erase all of the overlapping points.
                    puncturePts[p][j].erase( puncturePts[p][j].begin()+nnodes,
                                             puncturePts[p][j].end() );
                    
                    // Close the island if it is complete
                    puncturePts[p][j].push_back( puncturePts[p][j][0] );
                  }
                }
                else
                {
                  // If the analysis did result in a complete island try
                  // to find the boundary manually
//                if( verboseFlag )
//                 std::cerr << "Cleaning up island " << std::endl;

//                FLlib.removeOverlap( puncturePts[p], nnodes,
//                                     toroidalWinding, poloidalWinding,
//                                     windingGroupOffset, islands );
                }
              }
            }
            else if( type == FieldlineProperties::ISLAND_PRIMARY_SECONDARY_AXIS ||
                     type == FieldlineProperties::ISLAND_SECONDARY_SECONDARY_AXIS )
            {
              if( overlaps != 0 )
              {
                if( showLines )
                  nnodes = 2;

                bool tmpPoints = showPoints;

                // Loop through each island.
                for( unsigned int j=0; j<toroidalWinding; j++ )
                {
                  // Erase all of the overlapping points.
                  puncturePts[p][j].erase( puncturePts[p][j].begin()+nnodes,
                                           puncturePts[p][j].end() );

                  if( showLines )
                  {
                    unsigned int n = (j+windingGroupOffset) % toroidalWinding;

                    puncturePts[p][j][1] = puncturePts[p][j][0] +
                      0.9 * (puncturePts[p][n][0] - puncturePts[p][j][0]);
                  }
                }
              }
            }
            
            bool VALID = true;
            
            // Sanity check
            for( unsigned int j=0; j<toroidalWinding; ++j ) 
            {
                if( nnodes > puncturePts[p][j].size() )
                    nnodes = puncturePts[p][j].size();
                
                if( puncturePts[p][j].size() < 1 ) 
                {
                    if( verboseFlag ) 
                     std::cerr << "Clean up check failed - Plane " << p
                           << " bin  " << j
                           << " number of points " << puncturePts[p][j].size()
                           << std::endl;
                    
                    VALID = false;
                    
//                  return NULL;
                }
                
//              std::cerr << "Surface " << i
//                           << " plane " << p
//                           << " bin " << j
//                           << " base number of nodes " << nnodes
//                           << " number of points " << puncturePts[p][j].size()
//                           << std::endl;
            }
        }

        if( !showIslands ||
            (showIslands &&
             (type == FieldlineProperties::ISLAND_PRIMARY_CHAIN ||
              type == FieldlineProperties::ISLAND_SECONDARY_CHAIN ||
              type == FieldlineProperties::ISLAND_PRIMARY_SECONDARY_AXIS ||
              type == FieldlineProperties::ISLAND_SECONDARY_SECONDARY_AXIS)) )
        {
            double color_value;

            if( !analysis )
            {
              if( dataValue != DATA_None && 
                  dataValue != DATA_FieldlineOrder && 
                  dataValue != DATA_PointOrder )
              {
                dataValue = DATA_FieldlineOrder;
                color_value = poincare_ic->id;
              }
            }
            else if( dataValue == DATA_FieldlineOrder )
                color_value = poincare_ic->id;
            else if( dataValue == DATA_ToroidalWindings )
                color_value = toroidalWinding;
            else if( dataValue == DATA_PoloidalWindingsQ )
                color_value = poloidalWinding;
            else if( dataValue == DATA_PoloidalWindingsP )
                color_value = poloidalWindingP;
            else if( dataValue == DATA_SafetyFactorQ )
                color_value = (double) toroidalWinding / (double) poloidalWinding;
            else if( dataValue == DATA_SafetyFactorP )
            {
              if( poloidalWindingP )
                color_value = (double) toroidalWinding / (double) poloidalWindingP;
              else
                color_value = (double) toroidalWinding / (double) poloidalWinding;
            }
            else if( dataValue == DATA_SafetyFactorQ_NotP )
            {
              if( type != FieldlineProperties::ISLAND_PRIMARY_SECONDARY_AXIS )
                color_value = (double) toroidalWinding / (double) poloidalWinding;
              else if( type != FieldlineProperties::ISLAND_SECONDARY_SECONDARY_AXIS )
                color_value = (double) toroidalResonance / (double) poloidalWinding;
              else
                continue;
            }
            else if( dataValue == DATA_SafetyFactorP_NotQ )
            {
              if( type == FieldlineProperties::ISLAND_PRIMARY_SECONDARY_AXIS )
                color_value = (double) toroidalWinding / (double) poloidalWindingP;
              else if( type == FieldlineProperties::ISLAND_SECONDARY_SECONDARY_AXIS )
                color_value = (double) toroidalResonance / (double) poloidalWindingP;
              else
                continue;
            }
            else
              color_value = 0;

            // Currently the surface mesh is a structquad so set the dims - it
            // really should be and unstructured surface so multiple surface
            // can be generated.
            if( is_curvemesh ) 
            {
              if( type == FieldlineProperties::UNKNOWN_TYPE ||
                  type == FieldlineProperties::CHAOTIC )
              {
                bool tmpLines  = showLines;
                bool tmpPoints = showPoints;

                if( windingGroupOffset == 0 )
                {
                  showLines  = false;
                  showPoints = true;

                  if( dataValue != DATA_None && 
                      dataValue != DATA_FieldlineOrder && 
                      dataValue != DATA_PointOrder )
                  {
                    dataValue = DATA_FieldlineOrder;
                    color_value = poincare_ic->id;
                  }
                }

                drawIrrationalCurve( dt, puncturePts, nnodes, islands,
                                     windingGroupOffset,
                                     dataValue, color_value, 0, 0 );

                showLines  = tmpLines;
                showPoints = tmpPoints;
              }
              else if( type == FieldlineProperties::O_POINT ||
                       type == FieldlineProperties::X_POINT )
              {
                bool tmpLines  = showLines;
                bool tmpPoints = showPoints;

                if( overlaps != 0 )
                {
                  showLines  = false;
                  showPoints = true;
                }

                drawIrrationalCurve( dt, puncturePts, nnodes, islands,
                                     windingGroupOffset,
                                     dataValue, color_value, 0, 0 );

                showLines  = tmpLines;
                showPoints = tmpPoints;
              }
              else if( type == FieldlineProperties::RATIONAL )
              {
                drawRationalCurve( dt, puncturePts, nnodes, islands,
                                   windingGroupOffset,
                                   dataValue, color_value );
              }
              else if( type & FieldlineProperties::IRRATIONAL )
              {
                drawIrrationalCurve( dt, puncturePts, nnodes, islands,
                                     windingGroupOffset,
                                     dataValue, color_value,
                                     overlaps ? true : false,
                                     dataValue == DATA_WindingPointOrderModulo );
              }

//               if( showOPoints &&
//                   (type == FieldlineProperties::ISLAND_PRIMARY_CHAIN ||
//                    type == FieldlineProperties::ISLAND_SECONDARY_CHAIN ||
//                    type == FieldlineProperties::ISLAND_PRIMARY_SECONDARY_AXIS ||
//                    type == FieldlineProperties::ISLAND_SECONDARY_SECONDARY_AXIS) )
//               {
//                 drawPoints( dt, seedPoints );
//               }
            }
            else
            {
              drawSurface( dt, puncturePts, nnodes, islands,
                           windingGroupOffset,
                           dataValue, color_value );
            }

            if( 0 && show1DPlots )
              drawPeriodicity( dt, distancePts,
                               toroidalResonance,
//                             distancePts.size(),
                               nnodes, islands, poloidalWinding,
                               dataValue, color_value, true );

            
            if( show1DPlots )
              drawPeriodicity( dt, ridgelinePts,
//                               poloidalResonance,
                               (unsigned int)ridgelinePts.size(),
                               nnodes, islands, poloidalWinding,
                               dataValue, color_value, true );
            

            if( islands && show1DPlots )
            {
              for( unsigned int j=0; j<toroidalWinding; ++j )
              {
                drawPeriodicity( dt, islandPts[j],
                                 (unsigned int)islandPts[j].size(),
                                 nnodes, islands, poloidalWinding,
                                 dataValue, color_value, true );
              }
            }
        }
    }
    
    debug5 << "Finished creating poincare output " << std::endl;
}

// ****************************************************************************
//  Method: avtPoincareFilter::CreateRationalOutput
//
//  Purpose:
//      Create poincare output
//
//  Arguments:
//
//  Returns:      Poincare segments
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Oct 7 09:02:52 PDT 2008
//
//  Modifications:
//    Dave Pugmire (for Allen Sanderson), Wed Feb 25 09:52:11 EST 2009
//    Add terminate by steps, add AdamsBashforth solver, Allen Sanderson's new code.
//
//    Dave Pugmire, Fri Apr 17 11:32:40 EDT 2009
//    Add variables for dataValue var.
//
//    Dave Pugmire, Tue Apr 28 09:26:06 EDT 2009
//    Changed color to dataValue
//
//    Dave Pugmire, Wed May 27 15:03:42 EDT 2009
//    Replaced cerr/cout with debug5.
//
// ****************************************************************************
void
avtPoincareFilter::CreateRationalOutput( avtDataTree *dt,
                                         std::vector<avtIntegralCurve *> &ic)
{
#ifdef RATIONAL_SURFACE
  FieldlineLib FLlib;
  FLlib.verboseFlag = verboseFlag;

  // Put all of the points into the bins for each plane.
  std::vector< std::vector< std::vector < avtVector > > > puncturePts;

  for ( int i=0; i<ic.size(); ++i )
  {
    avtPoincareIC * poincare_ic = (avtPoincareIC *) ic[i];
    FieldlineProperties &properties = poincare_ic->properties;

    if( properties.analysisMethod  != FieldlineProperties::RATIONAL_SEARCH ||
        (properties.analysisMethod == FieldlineProperties::RATIONAL_SEARCH &&
         properties.searchState    != FieldlineProperties::ORIGINAL_RATIONAL) )
      continue;
     
    FieldlineProperties::FieldlineType type = properties.type;
    bool complete = (properties.analysisState == FieldlineProperties::COMPLETED);

    unsigned int toroidalWinding    = properties.toroidalWinding;
    unsigned int poloidalWinding    = properties.poloidalWinding;
    unsigned int windingGroupOffset = properties.windingGroupOffset;
    unsigned int poloidalWindingP   = properties.poloidalWindingP;
    unsigned int islands            = properties.islands;
  
    std::vector< avtPoincareIC *> *children = poincare_ic->properties.children;
    unsigned int nnodes = children->size();       

    if (2 <= RATIONAL_DEBUG)
      cerr << "Rational Search Parent: ID: " << poincare_ic->id << "  "
           << VectorToString(poincare_ic->points[0]) << "  with  "
           << poincare_ic->properties.children->size() << " children"
           << std::endl;

    if (children->size() < 1)
      {
        if (2 <= RATIONAL_DEBUG)
          cerr << "Not enough children, skipping...\n";
        continue;
      }

    bool problem = false;
    for (int i = 0; i < children->size(); i++)
      {

        problem = (
                   children->at(i)->properties.searchState > FieldlineProperties::ORIGINAL_RATIONAL && 
                   children->at(i)->properties.searchState <= FieldlineProperties::MINIMIZING_X3 &&
                   children->at(i) &&
                   children->at(i)->id &&
                   children->at(i)->points.size() > 0
                   ) 
          ? problem : true;
        if (2 <= RATIONAL_DEBUG && !problem)
          cerr << "Child ID: " << children->at(i)->id<<" ("
               << FindMinimizationDistance(children->at(i)) << "), "
               << VectorToString(children->at(i)->points[0])
               << std::endl; 
        else if (2 <= RATIONAL_DEBUG)
          cerr << "Child ID Problem\n";
      }

    if (problem)
      {
        if (2 <= RATIONAL_DEBUG)
          cerr << "There is a problem with drawing a child!" << std::endl;
        continue;
      }
    puncturePts.resize( planes.size() );

    problem = false;

    const avtVector zero_vec = avtVector(0,0,0);
    for( unsigned int p=0; p<planes.size(); ++p )
    {
      puncturePts[p].resize( toroidalWinding );

      for( unsigned int t = 0; t<toroidalWinding; ++t )
        {
          puncturePts[p][t].resize(children->size());

          // Initialize
          for (unsigned int s = 0; s < children->size(); s++)
            puncturePts[p][t][s] = zero_vec;
        }
    }

    for( int j=0; j<children->size(); ++j )
    {       
      avtPoincareIC *child_poincare_ic = children->at(j);

      // Get the direction of the fieldline toroidalWinding.
      Point lastPt = child_poincare_ic->points[0];
      Point currPt = child_poincare_ic->points[1];
            
      bool CCWfieldline = (atan2( lastPt.y, lastPt.x ) <
                            atan2( currPt.y, currPt.x ));
            
      double lastDist, currDist;
            
      std::vector < avtVector > distancePts;
            
      std::vector< std::vector < avtVector > > islandPts;
            
      unsigned int startIndex = 0;
            
      for( unsigned int p=0; p<planes.size(); ++p ) 
      {
        Vector planeN;
        Vector planePt(0,0,0);
        
        if( puncturePlane == 0 ) // Poloidal Plane
        {
          // Go through the planes in the same direction as the fieldline.
          if( CCWfieldline )
          {
            planeN = Vector( cos(planes[p]),
                             sin(planes[p]),
                             0 );
          }
          else
          {
            planeN = Vector( cos(planes[planes.size()-1-p]),
                             sin(planes[planes.size()-1-p]),
                             0 );
          }
        }
        
        else //if( puncturePlane == 1 ) // Toroidal Plane
        {
          planeN = Vector( 0, 0, -1 );
        }
        
        // Set up the plane equation.
        double plane[4];
        
        plane[0] = planeN.x;
        plane[1] = planeN.y;
        plane[2] = planeN.z;
        plane[3] = planePt.dot(planeN);
                
        int bin = 0;
        startIndex = 0;
                
        // So to get the winding groups consistent start examining
        // the fieldline in the same place for each plane.
        currPt = child_poincare_ic->points[startIndex];
        currDist = planeN.dot( currPt ) - plane[3];
        
        for( unsigned int k=startIndex+1; k<child_poincare_ic->points.size(); ++k )
        {
          //      cerr << "SIZE: " << child_poincare_ic->points.size() << ", " << k <<std::endl;

          lastPt = currPt;
          currPt = Vector(child_poincare_ic->points[k]);
          
          lastDist = currDist;
          currDist = Dot( planeN, currPt ) - plane[3];
          // First look at only points that intersect the plane.
          if( SIGN(lastDist) != SIGN(currDist) ) 
          {
            //cerr << "Iterating Curve: "<<k<<", "<<startIndex<<std::endl;

            Vector dir(currPt-lastPt);
            
            double dot = Dot(planeN, dir);
            
            // If the segment is in the same direction as the plane then
            // find where it intersects the plane.
            if( dot > 0.0 )
             {
               // In order to get the winding groups
               // consistent start examining the fieldline
               // in the same place for each plane so store
               // the index of the first puncture point.
               if( startIndex == 0 )
                 startIndex = k - 1;
                            
               Vector w = lastPt - planePt;
                            
               double t = -Dot(planeN, w ) / dot;
                            
               Point point = Point(lastPt + dir * t);         
               puncturePts[p][bin][j] = point;

               if (++bin >= toroidalWinding)             
                 break;          
             }
          }       
        }
        problem = bin == toroidalWinding ? problem : true;
      }
    }
 
    if (problem)
      {
        //One of the children didn't have enough points, skip this parent
        if (2 <= RATIONAL_DEBUG)
          cerr << "A child didn't have enough points, skipping...\n";
        continue;
      }

// Here, we are done with all the children but still in the parent
            
    // Have the puncture points now draw them ...
    for( unsigned int p=0; p<planes.size(); p++ ) 
    {
      bool VALID = true;
      
      // Sanity check
      for( unsigned int j=0; j<toroidalWinding && VALID; ++j ) 
      {
        if( nnodes > puncturePts[p][j].size() )
          nnodes = puncturePts[p][j].size();
        
        if( puncturePts[p][j].size() < 1 ) 
        {
          if( verboseFlag ) 
            cerr << "Rational clean up check failed - Plane " << p
                 << " bin  " << j
                 << " number of points " << puncturePts[p][j].size()
                 << endl;
          
          VALID = false;
        }
        
        //      cerr << "Surface " << i << " plane " << p << " bin " << j << " base number of nodes " << nnodes << " number of points " << puncturePts[p][j].size() << endl;
      }

      if( !showIslands )
      {
        double color_value;

        if( dataValue == DATA_FieldlineOrder )
          color_value = poincare_ic->id;
        else if( dataValue == DATA_ToroidalWindings )
          color_value = toroidalWinding;
        else if( dataValue == DATA_PoloidalWindingsQ )
          color_value = poloidalWinding;
        else if( dataValue == DATA_PoloidalWindingsP )
          color_value = poloidalWindingP;
        else if( dataValue == DATA_SafetyFactorQ )
          color_value = (double) toroidalWinding / (double) poloidalWinding;
        else if( dataValue == DATA_SafetyFactorP )
        {
          if( poloidalWindingP )
            color_value = (double) toroidalWinding / (double) poloidalWindingP;
          else
            color_value = (double) toroidalWinding / (double) poloidalWinding;
        }
        else if( dataValue == DATA_SafetyFactorQ_NotP )
        {
          if( poloidalWinding == poloidalWindingP )
            color_value = (double) toroidalWinding / (double) poloidalWinding;
          else
            continue;
        }
        else if( dataValue == DATA_SafetyFactorP_NotQ )
        {
          if( poloidalWindingP != poloidalWinding )
            color_value = (double) toroidalWinding / (double) poloidalWindingP;
          else
            continue;
        }
        else
          color_value = 0;
                
        // Currently the surface mesh is a structquad so set the dims - it
        // really should be and unstructured surface so multiple surface
        // can be generated.
        if( is_curvemesh ) 
        {
          int i = puncturePts.size();
          int j = puncturePts[0].size();
          int k = puncturePts[0][0].size();
          if (2 <= RATIONAL_DEBUG)
            cerr << i << ", " << j << ", " << k << std::endl;

          if (puncturePts[0][0].size() == 2)
            {
              if (2 <= RATIONAL_DEBUG)
                cerr << "Problem: " << PythDist(puncturePts[0][0][0], puncturePts[0][0][1]) << std::endl;
            }
            
          drawIrrationalCurve( dt, puncturePts, nnodes, islands,
                               windingGroupOffset,
                               dataValue, color_value,
                               overlaps ? true : false,
                               dataValue == DATA_WindingPointOrderModulo );
        }
        else
        {
          drawSurface( dt, puncturePts, nnodes, islands,
                       windingGroupOffset,
                       dataValue, color_value );
        }
      }
    }
  }
    
  debug5 << "Finished creating rational output " << endl;
#endif
}


// ****************************************************************************
//  Method: avtPoincareFilter::drawRationalCurve
//
//  Purpose: This method is for RATIONAL surfaces.
//           Creates a curve from the puncture points. Each curve
//           represents one toroidal winding group. 
//
//  Arguments:
//
//  Returns:      void
//
//  Programmer: Allen Sanderson
//  Creation:   Wed Feb 25 09:52:11 EST 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtPoincareFilter::drawRationalCurve( avtDataTree *dt,
                                      std::vector< std::vector < std::vector < avtVector > > > &nodes,
                                      unsigned int nnodes,
                                      unsigned int islands,
                                      unsigned int skip,
                                      unsigned int color,
                                      double color_value ) 
{
    vtkAppendPolyData *append = vtkAppendPolyData::New();
    
    unsigned int nplanes = nodes.size();
    unsigned int toroidalWindings = nodes[0].size();

    // If an island then only points.
    if( showLines && islands == 0 && toroidalWindings > 1 )
    {
      // Loop through each plane
      for( unsigned int p=0; p<nplanes; ++p ) 
      {
        if( color == DATA_PlaneOrder )
            color_value = p;
        
        //Create groups that represent the toroidial groups.
        vtkPoints *points = vtkPoints::New();
        vtkCellArray *cells = vtkCellArray::New();
        vtkFloatArray *scalars = vtkFloatArray::New();
            
        cells->InsertNextCell(toroidalWindings+1);
        scalars->Allocate    (toroidalWindings+1);

        // Loop through each toroidial group taking just the first
        // point from each group.
        for( unsigned int jj=0; jj<=toroidalWindings*skip; jj+=skip ) 
        {
            unsigned int j = jj % toroidalWindings;

            if( color == DATA_WindingGroupOrder )
                color_value = j;
            
            // Use the first point in each toroidial group
            unsigned int i=0;

            points->InsertPoint(j,
                                nodes[p][j][i].x,
                                nodes[p][j][i].y,
                                nodes[p][j][i].z);

            cells->InsertCellPoint(j);

            if( color == DATA_PointOrder )
              color_value = (i*toroidalWindings+j)*nplanes + p;
            else if( color == DATA_WindingPointOrder )
              color_value = i;
            else if( color == DATA_WindingPointOrderModulo )
              color_value = i % nnodes;
                
            scalars->InsertTuple1(j, color_value);
        }
         
        // Create a new VTK polyline.
        vtkPolyData *pd = vtkPolyData::New();
        pd->SetPoints(points);
        pd->SetLines(cells);
        scalars->SetName("colorVar");
        pd->GetPointData()->SetScalars(scalars);
        append->AddInputData(pd);
        
        points->Delete();
        cells->Delete();
        scalars->Delete();
      }   
    }

    if (showPoints || toroidalWindings == 1 )
    {
        for( unsigned int p=0; p<nplanes; ++p ) 
        {
            if( color == DATA_PlaneOrder )
                color_value = p;
            
            // Loop through each toroidial group
            for( unsigned int j=0; j<toroidalWindings; ++j ) 
            {
                if( color == DATA_WindingGroupOrder )
                    color_value = j;

                size_t npts;

                if( toroidalWindings > 1 )
                  npts = 1;
                else
                  npts = nodes[p][j].size();

                // Draw each point in the toroidial group
                for( size_t i=0; i<npts; ++i )
                {      
                    double pt[3] =
                      { nodes[p][j][i].x, nodes[p][j][i].y, nodes[p][j][i].z };
                    
                    if( color == DATA_PointOrder )
                      color_value = (i*toroidalWindings+j)*nplanes + p;
                    else if( color == DATA_WindingPointOrder )
                      color_value = i;
                    else if( color == DATA_WindingPointOrderModulo )
                      color_value = i % nnodes;
                    
                    vtkPolyData *ball = CreateSphere(color_value, pt);
                    
                    append->AddInputData(ball);
                    ball->Delete();
                }
            }
        }
    }
    
    if (0 && showPoints)
    {
        for( unsigned int p=0; p<nplanes; ++p ) 
        {
            if( color == DATA_PlaneOrder )
                color_value = p;
            
            // Loop through each toroidial group
            for( unsigned int j=0; j<toroidalWindings; ++j ) 
            {
                if( color == DATA_WindingGroupOrder )
                    color_value = j;
                
                //Create groups that represent the toroidial groups.
                vtkPoints *points = vtkPoints::New();
                vtkCellArray *cells = vtkCellArray::New();
                vtkFloatArray *scalars = vtkFloatArray::New();

                scalars->Allocate( nodes[p][j].size() );
        
                // Loop through each point in toroidial group
                for( unsigned int i=0; i<nodes[p][j].size(); ++i )
                {      
                    points->InsertNextPoint(nodes[p][j][i].x,
                                            nodes[p][j][i].y,
                                            nodes[p][j][i].z );
                    
                    vtkIdType id = (vtkIdType)i;
                    cells->InsertNextCell(1, &id);
                    
                    if( color == DATA_PointOrder )
                      color_value = (i*toroidalWindings+j)*nplanes + p;
                    else if( color == DATA_WindingPointOrder )
                      color_value = i;
                    else if( color == DATA_WindingPointOrderModulo )
                      color_value = i % nnodes;
                    
                    scalars->InsertTuple1(i, color_value);
                }

                // Create a new VTK point clouds.
                vtkPolyData *pd = vtkPolyData::New();
                pd->SetPoints(points);
                pd->SetVerts(cells);
                scalars->SetName("colorVar");
                pd->GetPointData()->SetScalars(scalars);
                append->AddInputData(pd);
    
                points->Delete();
                cells->Delete();
                scalars->Delete();  
            }
        }
    }
    
    append->Update();
    vtkPolyData *outPD = append->GetOutput();
    outPD->Register(NULL);
    append->Delete();
    
    dt->Merge( new avtDataTree(outPD, 0) );
}


// ****************************************************************************
//  Method: avtPoincareFilter::drawIrrationalCurve
//
//  Purpose: Creates a curve from the puncture points. Each curve
//           represents one toroidal winding group.
//
//  Arguments:
//
//  Returns:      void
//
//  Programmer: Allen Sanderson
//  Creation:   Wed Feb 25 09:52:11 EST 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtPoincareFilter::drawIrrationalCurve( avtDataTree *dt,
                                        std::vector< std::vector < std::vector < avtVector > > > &nodes,
                                        unsigned int nnodes,
                                        unsigned int islands,
                                        unsigned int skip,
                                        unsigned int color,
                                        double color_value,
                                        bool connect,
                                        bool modulo ) 
{
    vtkAppendPolyData *append = vtkAppendPolyData::New();
    
    unsigned int nplanes = nodes.size();
    unsigned int toroidalWindings = nodes[0].size();
    connect = 0;
    if (showLines)
    {
      if( modulo && islands )
      {
        unsigned int nSegments = nnodes;
        
        Vector intra = nodes[0][0][0] - nodes[0][0][nSegments];
        Vector inter = nodes[0][0][0] - nodes[0][0][1];

        int offset = Dot( intra, inter ) ? skip : -skip;

        offset = 0;

        // Loop through each plane
        for( unsigned int p=0; p<nplanes; ++p ) 
        {
          if( color == DATA_PlaneOrder )
            color_value = p;
          
          // Loop through each toroidial group
          for( unsigned int j=0; j<toroidalWindings; ++j ) 
          {
//          unsigned int bb = 0;

            if( color == DATA_WindingGroupOrder )
              color_value = j;

            // There is one segment for each node.
            for( unsigned int n=0; n<nSegments; ++n ) 
            {
              //Create groups that represent the toroidial groups.
              vtkPoints *points = vtkPoints::New();
              vtkCellArray *cells = vtkCellArray::New();
              vtkFloatArray *scalars = vtkFloatArray::New();

              unsigned int npts =
                ceil((nodes[p][j].size()-n) / (float) nSegments);
            
              cells->InsertNextCell(npts+(offset?1:0));
              scalars->Allocate    (npts+(offset?1:0));
            
              unsigned int cc = 0;

              // Loop through each point in toroidial group
              for( unsigned int i=n; i<nodes[p][j].size(); i+=nSegments ) 
              {
                points->InsertPoint(cc,
                                    nodes[p][j][i].x,
                                    nodes[p][j][i].y,
                                    nodes[p][j][i].z);

                cells->InsertCellPoint(cc);

                if( color == DATA_PointOrder )
                  color_value = (i*toroidalWindings+j)*nplanes + p;
                else if( color == DATA_WindingPointOrder )
                  color_value = i;
                else if( color == DATA_WindingPointOrderModulo )
                  color_value = i % nSegments;
                
//              color_value = bb++;
                
                scalars->InsertTuple1(cc++, color_value);
              }

              if( offset )
              {
                // Add one point in from the previous neighbor to create
                // a complete boundary.
                unsigned int i = (n+offset+nSegments) % nSegments;
                
                points->InsertPoint(cc,
                                    nodes[p][j][i].x,
                                    nodes[p][j][i].y,
                                    nodes[p][j][i].z);
                
                cells->InsertCellPoint(cc);
                
                if( color == DATA_PointOrder )
                  color_value = (i*toroidalWindings+j)*nplanes + p;
                else if( color == DATA_WindingPointOrder )
                  color_value = i;
                else if( color == DATA_WindingPointOrderModulo )
                  color_value = i % nSegments;

//              color_value = bb++;
                
                scalars->InsertTuple1(cc++, color_value);
              }

              // Create a new VTK polyline.
              vtkPolyData *pd = vtkPolyData::New();
              pd->SetPoints(points);
              pd->SetLines(cells);
              scalars->SetName("colorVar");
              pd->GetPointData()->SetScalars(scalars);
              append->AddInputData(pd);
            
              points->Delete();
              cells->Delete();
              scalars->Delete();       
            }
          }
        }
      }
      else //if( !modulo )
      {
        // Determine if the winding group order matches the point
        // ordering. This is only needed when building surfaces.
        Vector intra = nodes[0][   0][1] - nodes[0][0][0];
        Vector inter = nodes[0][skip][0] - nodes[0][0][0];

        int offset;

        if( !islands && connect )
          offset = (Dot( intra, inter ) < 0 ) ? toroidalWindings-skip : skip;
        else
          offset = 0;

        // Loop through each plane
        for( unsigned int p=0; p<nplanes; ++p ) 
        {
            if( color == DATA_PlaneOrder )
              color_value = p;
        
            // Loop through each toroidial group
            for( unsigned int j=0; j<toroidalWindings; ++j ) 
            {
              //Create groups that represent the toroidial groups.
              vtkPoints *points = vtkPoints::New();
              vtkCellArray *cells = vtkCellArray::New();
              vtkFloatArray *scalars = vtkFloatArray::New();
            
              cells->InsertNextCell(nodes[p][j].size()+(offset?1:0));
              scalars->Allocate    (nodes[p][j].size()+(offset?1:0));

              if( color == DATA_WindingGroupOrder )
                color_value = j;
            
              // Loop through each point in toroidial group
              for( unsigned int i=0; i<nodes[p][j].size(); ++i ) 
              {
                  points->InsertPoint(i,
                                      nodes[p][j][i].x,
                                      nodes[p][j][i].y,
                                      nodes[p][j][i].z);

                  cells->InsertCellPoint(i);

                  if( color == DATA_PointOrder )
                    color_value = (i*toroidalWindings+j)*nplanes + p;
                  else if( color == DATA_WindingPointOrder )
                    color_value = i;
                  else if( color == DATA_WindingPointOrderModulo )
                    color_value = i % nnodes;

                  scalars->InsertTuple1(i, color_value);
              }

              if( offset )
              {
                // Add one point in from the previous neighbor to create
                // a complete boundary.

                unsigned int ii = nodes[p][j].size();
                unsigned int jj = (j+offset) % toroidalWindings;
                
                points->InsertPoint(ii,
                                    nodes[p][jj][0].x,
                                    nodes[p][jj][0].y,
                                    nodes[p][jj][0].z);
                
                cells->InsertCellPoint(ii);

                if( color == DATA_PointOrder )
                  color_value = (ii*toroidalWindings+j)*nplanes + p;
                else if( color == DATA_WindingPointOrder )
                  color_value = ii;
                else if( color == DATA_WindingPointOrderModulo )
                  color_value = ii % nnodes;
                
                scalars->InsertTuple1(ii, color_value);
              }

            
              // Create a new VTK polyline.
              vtkPolyData *pd = vtkPolyData::New();
              pd->SetPoints(points);
              pd->SetLines(cells);
              scalars->SetName("colorVar");
              pd->GetPointData()->SetScalars(scalars);
              append->AddInputData(pd);
            
              points->Delete();
              cells->Delete();
              scalars->Delete();
            }
        }
      }
    }
    
    if (showPoints)
    {
        for( unsigned int p=0; p<nplanes; ++p ) 
        {
            if( color == DATA_PlaneOrder )
                color_value = p;

//          std::cerr << nnodes << std::endl;
            
            // Loop through each toroidial group
            for( unsigned int j=0; j<toroidalWindings; ++j ) 
            {
                if( color == DATA_WindingGroupOrder )
                    color_value = j;

//              std::cerr << nodes[p][j].size() << "  ";

                // Loop through each point in toroidial group
                for( unsigned int i=0; i<nodes[p][j].size(); ++i )
                { 
                    double pt[3] =
                      { nodes[p][j][i].x, nodes[p][j][i].y, nodes[p][j][i].z };
                    
                    if( color == DATA_PointOrder )
                      color_value = (i*toroidalWindings+j)*nplanes + p;
                    else if( color == DATA_WindingPointOrder )
                      color_value = i;
                    else if( color == DATA_WindingPointOrderModulo )
                      color_value = i % nnodes;
                    
                    vtkPolyData *ball = CreateSphere(color_value, pt);

                    append->AddInputData(ball);
                    ball->Delete();
                }
            }

//          std::cerr << std::endl;
        }
    }
    
    if (0 && showPoints)
    {
        for( unsigned int p=0; p<nplanes; ++p ) 
        {
            if( color == DATA_PlaneOrder )
                color_value = p;
            
            // Loop through each toroidial group
            for( unsigned int j=0; j<toroidalWindings; ++j ) 
            {
                if( color == DATA_WindingGroupOrder )
                    color_value = j;

                //Create groups that represent the toroidial groups.
                vtkPoints *points = vtkPoints::New();
                vtkCellArray *cells = vtkCellArray::New();
                vtkFloatArray *scalars = vtkFloatArray::New();

                scalars->Allocate( nodes[p][j].size() );
        
                // Loop through each point in toroidial group
                for( unsigned int i=0; i<nodes[p][j].size(); ++i )
                {      
                    points->InsertNextPoint(nodes[p][j][i].x,
                                            nodes[p][j][i].y,
                                            nodes[p][j][i].z );
                  
                    vtkIdType id = (vtkIdType)i;
                    cells->InsertNextCell(1, &id);
                  
                    if( color == DATA_PointOrder )
                      color_value = (i*toroidalWindings+j)*nplanes + p;
                    else if( color == DATA_WindingPointOrder )
                      color_value = i;
                    else if( color == DATA_WindingPointOrderModulo )
                      color_value = i % nnodes;
                  
                    scalars->InsertTuple1(i, color_value);
                }

                // Create a new VTK point clouds.
                vtkPolyData *pd = vtkPolyData::New();
                pd->SetPoints(points);
                pd->SetVerts(cells);
                scalars->SetName("colorVar");
                pd->GetPointData()->SetScalars(scalars);
                append->AddInputData(pd);
    
                points->Delete();
                cells->Delete();
                scalars->Delete();  
            }
        } 
    }
    
    append->Update();
    vtkPolyData *outPD = append->GetOutput();
    outPD->Register(NULL);
    append->Delete();

    
    dt->Merge( new avtDataTree(outPD, 0) );
}



// ****************************************************************************
//  Method: avtPoincareFilter::drawSurface
//
//  Purpose: Creates a surface from a series the puncture points. The
//           surface is sweep from each toroidal winding group around
//           each plane in the torus. Each is connected together to
//           form a circular cross section.
//
//  Arguments:
//
//  Returns:      void
//
//  Programmer: Allen Sanderson
//  Creation:   Wed Feb 25 09:52:11 EST 2009
//
//  Modifications:
//    Brad Whitlock, Fri Apr 20 16:03:09 PDT 2012
//    Use SetPoint so we can have multiple point precisions.
//
// ****************************************************************************

void
avtPoincareFilter::drawSurface( avtDataTree *dt,
                                std::vector< std::vector < std::vector < avtVector > > > &nodes,
                                unsigned int nnodes,
                                unsigned int islands,
                                unsigned int skip,
                                unsigned int color,
                                double color_value,
                                bool modulo ) 
{
    unsigned int nplanes = nodes.size();
    unsigned int toroidalWindings = nodes[0].size();
    
    int dims[2];
    
    // Add one to the first dimension to create a closed cylinder. Add
    // one to the second dimension to form a torus.
    dims[0] = nnodes + 1;
    dims[1] = nplanes * toroidalWindings + 1;
    
    // Create an unstructured quad for the island surface.
    vtkUnstructuredGrid *grid = vtkUnstructuredGrid::New();
    vtkQuad *quad = vtkQuad::New();
    vtkPoints *points = vtkPoints::New();
    vtkFloatArray *scalars = vtkFloatArray::New();
    
    points->SetNumberOfPoints(dims[0]*dims[1]);
    scalars->Allocate(dims[0]*dims[1]);
    
    // Determine if the winding group order matches the point
    // ordering. This is only needed when building surfaces.
    Vector intra = nodes[0][   0][1] - nodes[0][0][0];
    Vector inter = nodes[0][skip][0] - nodes[0][0][0];

    int offset = (Dot( intra, inter ) < 0 ) ? -skip : skip;

    // Loop through each toroidial group
    for( unsigned int j=0; j<toroidalWindings; ++j )
    {
        if( color == DATA_WindingGroupOrder )
            color_value = j;

        // Loop through each plane.
        for( unsigned int p=0; p<nplanes; ++p ) 
        {
            // Normally each toroidial winding group can be displayed
            // in the order received. Except for the last plane where
            // it needs to be adjusted by one group. That is if the
            // fieldline started in the "correct" place. This is not
            // always the case so it may be necessary to adjust the
            // toroidal winding group location by one.
            unsigned int k;
            
            if( p == adjust_plane )
            {
                k = (j-1 + toroidalWindings) % toroidalWindings;
            }
            else
            {
                k = j;
            }

            unsigned int jj = nplanes * j + p;
            
            if( color == DATA_PlaneOrder )
                color_value = jj;
            
            // Loop through each point in toroidial group.
            for(unsigned int i=0; i<nnodes; ++i )
            {
                unsigned int n1 = jj * dims[0] + i;

                points->SetPoint(n1, nodes[p][k][i].x,
                                     nodes[p][k][i].y,
                                     nodes[p][k][i].z);

                if( color == DATA_PointOrder )
                    color_value = (i*toroidalWindings+j)*nplanes + p;
                else if( color == DATA_WindingPointOrder )
                    color_value = i;
                else if( color == DATA_WindingPointOrderModulo )
                    color_value = i % nnodes;
                
                scalars->InsertTuple1(n1, color_value);

                // Create the quad.
                quad->GetPointIds()->SetId( 0,   jj    * dims[0] + i );
                quad->GetPointIds()->SetId( 1,  (jj+1) * dims[0] + i );
                quad->GetPointIds()->SetId( 2,  (jj+1) * dims[0] + i + 1);
                quad->GetPointIds()->SetId( 3,   jj    * dims[0] + i + 1);
                
                grid->InsertNextCell( quad->GetCellType(),
                                      quad->GetPointIds() );                
            }

            // For a surface add in the first point from the adjacent
            // toroidial group. Otherwise for an island add in the
            // first point from the current toroidal group.
            if( !islands )
                k = (k+offset+toroidalWindings) % toroidalWindings;

            unsigned int i = nnodes;

            unsigned int n1 = jj * dims[0] + i;
            points->SetPoint(n1, nodes[p][k][0].x,
                                 nodes[p][k][0].y,
                                 nodes[p][k][0].z);
            
            if( color == DATA_PointOrder )
              color_value = (i*toroidalWindings+j)*nplanes + p;
            else if( color == DATA_WindingPointOrder )
              color_value = i;
            else if( color == DATA_WindingPointOrderModulo )
              color_value = i % nnodes;
            
            scalars->InsertTuple1(n1, color_value);
        }
    }
    
    // Add in the first toroidal group from the first plane to complete
    // the torus.
    unsigned int j = 0;
    
    if( color == DATA_WindingGroupOrder )
        color_value = j;
    
    // Add in the first toroidal group from the first plane to complete
    // the torus.
    unsigned int p = 0;
    
    // Normally each toroidial group can be displayed in the order
    // received. Except for the last plane where it needs to be
    // adjusted by one group. That is if the fieldline started in
    // the "correct" place. This is not always the case so it may be
    // necessary to adjust the winding group location by one.
    unsigned int k;
    
    if( p == adjust_plane )
    {
        k = (j-1 + toroidalWindings) % toroidalWindings;
    }
    else
    {
        k = j;
    }
    
    unsigned int jj = nplanes * toroidalWindings;
    
    if( color == DATA_PlaneOrder )
        color_value = jj;
    
    // Loop through each point in toroidial group.
    for(unsigned int i=0; i<nnodes; ++i )
    {
      // Normally each point in a toroidial group can be displayed in
      // the order received. Except when dealing with 1:1 surfaces for
      // the last plane where it needs to be adjusted by one
      // location. That is if the fieldline started in the "correct"
      // place. This is not always the case so it may be necessary to
      // adjust the point ordering by one.
      unsigned int ii;

      if( p == adjust_plane && toroidalWindings == 1) 
        ii = (i-1 + nnodes) % nnodes;
      else
        ii = i;

        unsigned int n1 = jj * dims[0] + ii;

        points->SetPoint(n1, nodes[p][k][i].x,
                             nodes[p][k][i].y,
                             nodes[p][k][i].z);

        if( color == DATA_PointOrder )
            color_value = (i*toroidalWindings+j)*nplanes + p;
        else if( color == DATA_WindingPointOrder )
          color_value = i;
        else if( color == DATA_WindingPointOrderModulo )
          color_value = i % nnodes;
        
        scalars->InsertTuple1(n1, color_value);
    }

    // For a surface add in the first point from the adjacent
    // toroidial group. Otherwise for an island add in the
    // first point from the current toroidal group.
    if( !islands )
        k = (k+offset+toroidalWindings) % toroidalWindings;

    unsigned int i = nnodes;
    unsigned int n1 = jj * dims[0] + i;

    points->SetPoint(n1, nodes[p][k][0].x,
                         nodes[p][k][0].y,
                         nodes[p][k][0].z);
    
    if( color == DATA_PointOrder )
      color_value = (i*toroidalWindings+j)*nplanes + p;
    else if( color == DATA_WindingPointOrder )
      color_value = i;
    else if( color == DATA_WindingPointOrderModulo )
      color_value = i % nnodes;
    
    scalars->InsertTuple1(n1, color_value);


    // Stuff the points and scalars into the VTK unstructure grid.
    grid->SetPoints(points);
    scalars->SetName("colorVar");
    grid->GetPointData()->SetScalars(scalars);
    dt->Merge( new avtDataTree(grid, 0) );
    
    quad->Delete();
    points->Delete();
    scalars->Delete();
}


void
avtPoincareFilter::drawPeriodicity( avtDataTree *dt,
                                    std::vector < Point  > &nodes,
                                    unsigned int period,
                                    unsigned int nnodes,
                                    unsigned int islands,
                                    unsigned int poloidalWindings,
                                    unsigned int color,
                                    double color_value,
                                    bool ptFlag )
{
  if( period <= 1 )
    period = (unsigned int)nodes.size();

  unsigned int colorMax = 0;

  vtkAppendPolyData *append = vtkAppendPolyData::New();

  if( islands )
    poloidalWindings *= nnodes;
  
  if (showLines)
  {
    //Create groups that represent the toroidial groups.
    vtkPoints *points;
    vtkCellArray *cells;
    vtkFloatArray *scalars;
    
    unsigned int cc = 0;
  
    // Loop through each point in poloidal group
    for( unsigned int i=0; i<nodes.size(); ++i )
    {      
      if( i % period == 0 )
      {
        //Create groups that represent the toroidial groups.
        points = vtkPoints::New();
        cells = vtkCellArray::New();
        scalars = vtkFloatArray::New();

        unsigned int npts = period < (nodes.size()-i) ?
          period : (nodes.size()-i);
      
        cells->InsertNextCell( npts );
        scalars->Allocate    ( npts );
      
        cc = 0;
      }

      if( ptFlag )
        points->InsertPoint(cc,
                            (float) (i % period) / 50.0,
                            nodes[i].y,
                            nodes[i].z);
      else
        points->InsertPoint(cc, nodes[i].x, nodes[i].y, nodes[i].z);
    
      cells->InsertCellPoint(cc);

      if( color == DATA_PointOrder )
        color_value = i;
      else if( color == DATA_WindingGroupOrder )
        color_value = i / poloidalWindings;
      else if( color == DATA_WindingPointOrder )
        color_value = i % poloidalWindings;
      else if( color == DATA_WindingPointOrderModulo )
        color_value = (i % poloidalWindings) % nnodes;
          
      scalars->InsertTuple1(cc, color_value);
        
      ++cc;
            
      if( i % period == 0 )
      {
        // Create a new VTK polyline.
        vtkPolyData *pd = vtkPolyData::New();
        pd->SetPoints(points);
        pd->SetLines(cells);
        scalars->SetName("colorVar");
        pd->GetPointData()->SetScalars(scalars);
        append->AddInputData(pd);
        
        points->Delete();
        cells->Delete();
        scalars->Delete();       
      }
    }
  }

  if (showPoints)
  {
    // Loop through each poloidal group
    // Loop through each point in poloidial group
    for( unsigned int i=0; i<nodes.size(); ++i )
    {
      double pt[3] = { nodes[i].x, nodes[i].y, nodes[i].z };
      
      if( ptFlag )
        pt[0] = (float) (i % period) / 50.0;
      else
        pt[0] = nodes[i].x;
          
      if( color == DATA_PointOrder )
        color_value = i;
      else if( color == DATA_WindingGroupOrder )
        color_value = i / poloidalWindings;
      else if( color == DATA_WindingPointOrder )
        color_value = i % poloidalWindings;
      else if( color == DATA_WindingPointOrderModulo )
        color_value = (i % poloidalWindings) % nnodes;

      if( colorMax < color_value )
        colorMax = color_value;
      
      vtkPolyData *ball = CreateSphere(color_value, pt);
      append->AddInputData(ball);
      ball->Delete();
    }
  }

  if (0 && showPoints)
  {
    //Create groups that represent the toroidial groups.
    vtkPoints *points = vtkPoints::New();
    vtkCellArray *cells = vtkCellArray::New();
    vtkFloatArray *scalars = vtkFloatArray::New();

    scalars->Allocate( nodes.size() );

    // Loop through each poloidal group
    // Loop through each point in poloidial group
    for( unsigned int i=0; i<nodes.size(); ++i )
    {      
      if( ptFlag )
        points->InsertNextPoint( (float) (i % period) / 50.0,
                                 nodes[i].y,
                                 nodes[i].z);
      else
        points->InsertNextPoint(nodes[i].x, nodes[i].y, nodes[i].z);

      vtkIdType id = (vtkIdType)i;
      cells->InsertNextCell(1, &id);
      
      if( color == DATA_PointOrder )
        color_value = i;
      else if( color == DATA_WindingGroupOrder )
        color_value = i / poloidalWindings;
      else if( color == DATA_WindingPointOrder )
        color_value = i % poloidalWindings;
      else if( color == DATA_WindingPointOrderModulo )
        color_value = (i % poloidalWindings) % nnodes;

      if( colorMax < color_value )
        colorMax = color_value;

      scalars->InsertTuple1(i, color_value);
    }
    
    // Create a new VTK point clouds.
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->SetVerts(cells);
    scalars->SetName("colorVar");
    pd->GetPointData()->SetScalars(scalars);
    append->AddInputData(pd);
    
    points->Delete();
    cells->Delete();
    scalars->Delete();       
  }

  append->Update();
  vtkPolyData *outPD = append->GetOutput();
  outPD->Register(NULL);
  append->Delete();
  
  dt->Merge( new avtDataTree(outPD, 0) );
}


// ****************************************************************************
//  Method: avtPoincareFilter::drawPoints
//
//  Purpose: Draws a bunch of points.
//
//  Arguments:
//
//  Returns:      void
//
//  Programmer: Allen Sanderson
//  Creation:   Wed Feb 25 09:52:11 EST 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtPoincareFilter::drawPoints( avtDataTree *dt,
                               std::vector < avtVector > &nodes ) 
{
  vtkAppendPolyData *append = vtkAppendPolyData::New();

  if (showPoints)
  {
    //Create groups that represent the toroidial groups.
    vtkPoints *points = vtkPoints::New();
    vtkCellArray *cells = vtkCellArray::New();
    vtkFloatArray *scalars = vtkFloatArray::New();

    scalars->Allocate( nodes.size() );

    for( unsigned int i=0; i<nodes.size(); ++i )
    {      
      points->InsertNextPoint(nodes[i].x, nodes[i].y, nodes[i].z);

      vtkIdType id = (vtkIdType)i;
      cells->InsertNextCell(1, &id);
      
      scalars->InsertTuple1(i, 0);
    }
    
    // Create a new VTK point clouds.
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->SetVerts(cells);
    scalars->SetName("colorVar");
    pd->GetPointData()->SetScalars(scalars);
    append->AddInputData(pd);
    
    points->Delete();
    cells->Delete();
    scalars->Delete();       
  }

  append->Update();
  vtkPolyData *outPD = append->GetOutput();
  outPD->Register(NULL);
  append->Delete();
  
  dt->Merge( new avtDataTree(outPD, 0) );
}


// ****************************************************************************
// Method: avtPoincareFilter::SetIntersectionCriteria
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
avtPoincareFilter::SetIntersectionCriteria(vtkObject *obj, int mi)
{
    if (obj)
    {
        intersectObj = obj;
        intersectObj->Register(NULL);
    }
    maxIntersections = mi;
}
