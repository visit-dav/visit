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
//                             avtPICSFilter.C                               //
// ************************************************************************* //

/**
TODO:
Sort: Bias sorting to "my domain". ICs on this domain will be coming to you.

Consider the leaveDomains ICs and the balancing at the same time.

 **/

#include <avtPICSFilter.h>
#include "avtSerialICAlgorithm.h"
#include "avtParDomICAlgorithm.h"
#include "avtPODICAlgorithm.h"
#include "avtCommDSOnDemandICAlgorithm.h"
#include "avtMasterSlaveICAlgorithm.h"
#include "avtVariableCache.h"
#include <math.h>
#include <string.h>
#include <visitstream.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkLineSource.h>
#include <vtkPlaneSource.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkPointSource.h>
#include <vtkVisItStreamLine.h>
#include <vtkGlyph3D.h>

#include <avtCallback.h>
#include <avtCellLocatorClassic.h>
#include <avtCellLocatorBIH.h>
#include <avtCellLocatorRect.h>
#include <avtCellLocatorRectFace.h>
#include <avtCellLocatorRectEdge.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtDataset.h>
#include <avtDataTree.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtIVPVTKField.h>
#include <avtIVPVTKFaceField.h>
#include <avtIVPVTKEdgeField.h>
#include <avtIVPVTKOffsetField.h>
#include <avtIVPVTKTimeVaryingField.h>
#include <avtIVPDopri5.h>
#include <avtIVPAdamsBashforth.h>
#include <avtIVPEuler.h>
#include <avtIVPRK4.h>
#include <avtIVPLeapfrog.h>
#include <avtIVPM3DC1Integrator.h>
#include <avtIVPM3DC1Field.h>
#include <avtIVPNek5000Field.h>
#ifdef NEKTAR_PLUS_PLUS_FOUND
#include <avtIVPNektar++Field.h>
#include <avtIVPNektar++TimeVaryingField.h>
#endif
#include <avtIVPNIMRODField.h>
#include <avtIVPFlashField.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>
#include <avtVector.h>

#include <DebugStream.h>
#include <MemStream.h>
#include <TimingsManager.h>
#include <InvalidFilesException.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>
#include <VisItStreamUtil.h>
#include <snprintf.h>

#ifdef PARALLEL
#include <time.h> // needed for nanosleep
#include <mpi.h>
#endif

#include <vector>

#ifndef _WIN32
#include <dirent.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

using std::vector;

static const char restartFilename[] = "PICS_Restart";

// ****************************************************************************
//  Method: avtPICSFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//    Hank Childs, Fri Oct  1 20:43:34 PDT 2010
//    Initialize absTolIsFraction.
//
//    Hank Childs, Mon Oct  4 14:50:01 PDT 2010
//    Remove data members for termination.
//
//    Hank Childs, Thu Oct 21 08:54:51 PDT 2010
//    Correctly initialize icAlgo.
//
// ****************************************************************************

avtPICSFilter::avtPICSFilter()
{
    doPathlines = false;
    pathlineOverrideTime = false;
    seedTimeStep0 = 0;
    seedTime0 = 0.0;
    baseTime = 0.0;

    period = 0;
    rollover = false;

    icAlgo = NULL;
    emptyDataset = false;

    fieldType = PICS_FIELD_DEFAULT;
    fieldConstant = 1.0;

    maxStepLength = 0.;
    integrationDirection = VTK_INTEGRATE_FORWARD;
    integrationType = PICS_INTEGRATE_DORMAND_PRINCE;
    relTol = 1e-7;
    absTol = 0;
    absTolIsFraction = false;
    intervalTree = NULL;
    specifyPoint = false;
    solver = NULL;
    dataSpatialDimension = 3;

    InitialIOTime = 0.0;
    InitialDomLoads = 0;
    activeTimeStep = -1;
    MaxID = 0;

    convertToCartesian = false;

    restart = -1;
}


// ****************************************************************************
//  Method: avtPICSFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//   Christoph Garth, Fri Jul 9 12:53:11 PDF 2010
//   Replace vtkVisItCellLocator by avtCellLocator.
// 
//   Hank Childs, Sun Nov 28 12:39:26 PST 2010
//   Make cell locator be reference counted so it can be cached at the database
//   level.
//
// ****************************************************************************

avtPICSFilter::~avtPICSFilter()
{
    ClearDomainToCellLocatorMap();
}

// ****************************************************************************
//  Method: avtPICSFilter::ClearDomainToCellLocatorMap
//
//  Purpose:
//      Clear the map for cell locators.
//
//  Programmer: David Camp
//  Creation:   April 21, 2011
//
//  Modifications:
// ****************************************************************************

void
avtPICSFilter::ClearDomainToCellLocatorMap()
{
    std::map<BlockIDType, avtCellLocator_p>::iterator it;

    for ( it = domainToCellLocatorMap.begin(); it != domainToCellLocatorMap.end(); it++ )
    {
        if (*(it->second))
            it->second = NULL;
    }
    domainToCellLocatorMap.clear();
}


// ****************************************************************************
//  Method: avtPICSFilter::ComputeRankList
//
//  Purpose:
//      Computes the processor for each domain.
//
//  Programmer: Dave Pugmire
//  Creation:   June 23, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

void
avtPICSFilter::ComputeRankList(const vector<int> &domList, 
                                     vector<int> &ranks, vector<int> &doms)
{
    ranks.resize(0);
    
    vector<int> r;
    for (size_t i = 0; i < domList.size(); i++)
    {
        int dom = domList[i];
        BlockIDType d(dom, 0);
        // TODO: Should this be BlockIDType d(dom, activeTimeStep); instead?
        int proc = DomainToRank(d);
        r.push_back(proc);
    }

    //Filter out any duplicates....
    for (size_t i = 0; i < r.size(); i++)
    {
        bool addIt = true;
        for (size_t j = 0; j < ranks.size(); j++)
        {
            if (ranks[j] == r[i])
            {
                addIt = false;
                break;
            }
        }
        
        if (addIt)
        {
            ranks.push_back(r[i]);
            doms.push_back(domList[i]);
        }
    }
}

// ****************************************************************************
// Method:  avtPICSFilter::FindCandidateBlocks
//
// Purpose: 
//   Determine candidate blocks for an IC. *Does no I/O*.
//   
// Programmer:  Dave Pugmire
// Creation:    May 29, 2012
//
//  Modifications:
//
//   Dave Pugmire, Wed Jun  5 16:43:36 EDT 2013
//   Code hardening. Better handling for rectilinear grid corner cases.
//
// ****************************************************************************

void
avtPICSFilter::FindCandidateBlocks(avtIntegralCurve *ic,
                                   BlockIDType *skipBlk)
{
    ic->blockList.clear();
    ic->status.ClearTemporalBoundary();
    ic->status.ClearSpatialBoundary();
    
    int timeStep = GetTimeStep(ic->CurrentTime());

    if (timeStep == -1)
    {
        ic->status.SetExitTemporalBoundary();
        return;
    }
    if (timeStep != curTimeSlice)
    {
        ic->status.SetAtTemporalBoundary();
        return;
    }

    avtVector pt = ic->CurrentLocation();
    double xyz0[3] = {pt.x, pt.y, pt.z};
    double xyz1[3] = {pt.x, pt.y, pt.z};

    /*
    double eps = 1e-4;
    avtVector v = ic->CurrentV();
    if (ic->direction == avtIntegralCurve::DIRECTION_BACKWARD)
        v = -v;
    for (int i = 0; i < 3; i++)
    {
        if (v[i] > 0.0)
            xyz1[i] += eps;
        else if (v[i] < 0.0)
            xyz0[i] -= eps;
    }
    */
    
    std::vector<int> doms;
    intervalTree->GetElementsListFromRange(xyz0, xyz1, doms);
    
    bool blockLoaded = false;
    for (size_t i = 0; i < doms.size(); i++)
    {
        BlockIDType curr(doms[i], timeStep);

        if (skipBlk != NULL && curr == *skipBlk)
        {
            continue;
        }
        else if (BlockLoaded(curr))
        {
            if (ICInBlock(ic, curr))
            {
                ic->blockList.clear();
                ic->blockList.push_back(curr);
                blockLoaded = true;
                break;
            }
        }
        else
        {
            ic->blockList.push_back(curr);
        }
    }

    // std::cerr << (blockLoaded ? "block loaded  " : "no block to load  ")
    //        << timeStep << "  " << skipBlk
    //        << std::endl;

    // No blocks, exited spatial boundary.
    if (ic->blockList.empty())
        ic->status.SetExitSpatialBoundary();
    else if (!blockLoaded)
        ic->status.SetAtSpatialBoundary();
}

// ****************************************************************************
//  Method: avtPICSFilter::GetDomain
//
//  Purpose:
//      Get the VTK domain that contains the particle.  The way we "get" 
//      depends on our execution mode.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 16 15:05:14 EDT 2009
//   Make BlockIDType a const reference.
//
//   Hank Childs, Sun Mar 22 13:31:08 CDT 2009
//   Add support for getting the "domain" by using a point.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//
//   Dave Pugmire, Thu Dec  2 11:27:37 EST 2010
//   Better check for NULL domains. Moved loading of timeslice into new method.
//
// ****************************************************************************

vtkDataSet *
avtPICSFilter::GetDomain(const BlockIDType &domain, const avtVector &pt)
{
    debug1<<"GetDomain() dom= "<<domain<<" pt= "<<pt<<" line= "<<__LINE__<<endl;

    if (domain.domain == -1 || domain.timeStep == -1)
        return NULL;

    vtkDataSet *ds = NULL;
    if (OperatingOnDemand())
    {
        debug1<<"GetDomain() dom= "<<domain<<" pt= "<<pt<<" line= "<<__LINE__<<endl;
        if (specifyPoint)
        {
            ds = avtDatasetOnDemandFilter::GetDataAroundPoint(pt.x, pt.y, pt.z,
                                                              domain.timeStep);
        }
        else
        {
            ds = avtDatasetOnDemandFilter::GetDomain(domain.domain,
                                                     domain.timeStep);
        }
        if (ds && doPathlines)
        {
            // We need to set the active variable to the correct velocity value.
            std::string velocityName, meshName;
            avtDataRequest_p dr = lastContract->GetDataRequest();
            GetPathlineVelocityMeshVariables(dr, velocityName, meshName);

            if (ds->GetPointData()->GetArray(velocityName.c_str()) != NULL)
               ds->GetPointData()->SetActiveAttribute(velocityName.c_str(), vtkDataSetAttributes::VECTORS);
            else if (ds->GetCellData()->GetArray(velocityName.c_str()) != NULL)
               ds->GetCellData()->SetActiveAttribute(velocityName.c_str(), vtkDataSetAttributes::VECTORS);
            else
            {
                if (DebugStream::Level1()) 
                {
                    debug1 << "Unable to locate velocity variable for pathlines.\n";
                }
            }
        }
    }
    else
    {
        debug1<<"GetDomain() dom= "<<domain<<" pt= "<<pt<<" line= "<<__LINE__<<endl;
        ds = dataSets[domain.domain];
    }
    
    debug1<<"GetDomain() dom= "<<domain<<" pt= "<<pt<<" line= "<<__LINE__<<" ds= "<<ds<<endl;
    
    return ds;
}


// ****************************************************************************
// Method:  avtPICSFilter::RestoreICsFilename
//
// Purpose: Create filename for restore ICs files.
//   
// Programmer:  David Camp
// Creation:    March 7, 2012
//
// Modifications:
//
// ****************************************************************************

void
avtPICSFilter::RestoreICsFilename( int timeStep, char *filename, size_t filenameSize )
{
    SNPRINTF( filename, filenameSize, "%s_%d_%03d", restartFilename, PAR_Rank(), timeStep );
}

// ****************************************************************************
// Method:  avtPICSFilter::RestoreICs
//
// Purpose: Restore ICs from file to restart integration or view ICs
//   
// Programmer:  David Camp
// Creation:    March 7, 2012
//
// Modifications:
//
// ****************************************************************************

void
avtPICSFilter::RestoreICs( vector<avtIntegralCurve *> &ics, int timeStep )
{
    int i, icNum;

    // Look at doing a better guess in starting buffer size
    MemStream buff;

    // Load data into buff from file.
    char filename[32];
    RestoreICsFilename( timeStep, filename, 32 );
    buff.LoadFile( filename );

    buff.read(icNum);
    ics.resize( icNum );
    for(i=0; i < icNum ;i++)
    {
        avtIntegralCurve *ic = CreateIntegralCurve();
        ic->Serialize(MemStream::READ, buff, solver, avtIntegralCurve::SERIALIZE_ALL);
        ics[i] = ic;
    }
}

// ****************************************************************************
// Method:  avtPICSFilter::SaveICs
//
// Purpose: Save the ICs to file to restore later.
//   
// Programmer:  David Camp
// Creation:    March 7, 2012
//
// Modifications:
//
// ****************************************************************************

void
avtPICSFilter::SaveICs( std::vector<avtIntegralCurve *> &ics, int timeStep )
{
    int i, icNum = ics.size();

    // Look at doing a better guess in starting buffer size
    MemStream buff( icNum * 100 );

    buff.write(icNum);

    for(i=0; i < icNum ;i++)
    {
        ics[i]->Serialize(MemStream::WRITE, buff, solver, avtIntegralCurve::SERIALIZE_ALL);
    }

    // Save MemStream to file.
    char filename[32];
    RestoreICsFilename( timeStep, filename, 32 );
    buff.SaveFile( filename );
}

// ****************************************************************************
// Method:  avtPICSFilter::CheckIfRestart
//
// Purpose: Check if there is a restart file to load.
//   
// Programmer:  David Camp
// Creation:    March 7, 2012
//
// Modifications:
//
// ****************************************************************************

bool
avtPICSFilter::CheckIfRestart( int &timeStep )
{
    bool found = false;
#ifndef _WIN32
    DIR *pDIR;
    struct dirent *entry;

    if( (pDIR = opendir(".")) )
    {
        while( (entry = readdir(pDIR)) )
        {
            if( strncmp(entry->d_name, restartFilename, sizeof(restartFilename)-1) == 0 )
            {
                found = true;

                char *s = entry->d_name + sizeof(restartFilename); // skip the filename
                s = strstr( s, "_" ) + 1;                          // skip the rank.
                int num = atoi( s );                               // Get time step
                if( num > timeStep )
                    timeStep = num;
            }
        }
        closedir( pDIR );
    }
#else
    WIN32_FIND_DATA fd;
    HANDLE dirHandle = FindFirstFile(".\\*", &fd);
    if (dirHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (strncmp(fd.cFileName, restartFilename, sizeof(restartFilename)-1) == 0)
            {
                found = true;
                char *s = fd.cFileName + sizeof(restartFilename);
                s = strstr(s, "_") +1;
                int num = atoi(s);
                if (num > timeStep)
                    timeStep = num;
            }
        } while(FindNextFile(dirHandle, &fd));
        FindClose(dirHandle);
    } 
#endif

    return( found );
}

// ****************************************************************************
// Method:  avtPICSFilter::LoadNextTimeSlice
//
// Purpose: Load next time slice.
//
// Programmer:  Dave Pugmire
// Creation:    December 2, 2010
//
// Modifications:
//
//   Hank Childs, Fri Mar  9 16:50:48 PST 2012
//   Add support for reverse pathlines.
//
// ****************************************************************************

bool
avtPICSFilter::LoadNextTimeSlice()
{
    ClearDomainToCellLocatorMap();
    EmptyQueue();  // Clear the avtDatasetOnDemandFilter data queue

    if( period == 0 )
    {
      if (integrationDirection == VTK_INTEGRATE_BACKWARD)
      {
        // When going backwards can not use the first time slice.
        if ((curTimeSlice-1) <= 0)
            return false;
      }
      else
      {
        // When going forwards can not use the lasst time slice.
        if ((curTimeSlice+1) >= domainTimeIntervals.size())
            return false;
      }
    }

    rollover = false;

    // Reset the timeout for the next iteration.
    avtCallback::ResetTimeout(60*60);
    
    if (integrationDirection == VTK_INTEGRATE_BACKWARD)
    {
        curTimeSlice--;

        // When going backwards can not use the first time slice.
        if (period > 0 && curTimeSlice <= 0)
        {
            curTimeSlice = domainTimeIntervals.size();
            rollover = true;
        }
    }
    else
    {
        curTimeSlice++;

        // When going forwards can not use the lasst time slice.
        if ( period > 0 && curTimeSlice >= (int)domainTimeIntervals.size())
        {
            curTimeSlice = 0;
            rollover = true;
        }
    }

    // std::cerr << "LoadNextTimeSlice() " << curTimeSlice << " tsMax= " << domainTimeIntervals.size() <<"  " << period << std::endl;


    if (DebugStream::Level5()) 
    {
        debug5<<"LoadNextTimeSlice() "<<curTimeSlice<<" tsMax= "<<domainTimeIntervals.size()<<endl;
    }
    avtContract_p new_contract;
    if (OperatingOnDemand())
    {
        // The last contract has the domains we wanted for the previous time
        // slice.  We may want different domains in the next time slice.
        // Turn off all domains and let on-demand loading handle it.
        new_contract = new avtContract(lastContract);
        new_contract->GetDataRequest()->GetRestriction()->TurnOffAll();

        // There is no reason the other MPI tasks need to be loading the same
        // time slices at the same time.  But that's what will happen
        // if we don't set on demand streaming to "true", because
        // the database might do collective communication.
        new_contract->SetOnDemandStreaming(true);
    }
    else
    {
        // The last contract has the right domain list for the previous
        // time slice.  But the next time slice may have a different 
        // number of domains.  So we need to go to the database and get
        // the correct domain list.
        std::string db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
        ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
        if (*dbp == NULL)
            EXCEPTION1(InvalidFilesException, db.c_str());

        std::string mesh = GetInput()->GetInfo().GetAttributes().GetMeshname();
        avtDataObject_p dob = dbp->GetOutput(mesh.c_str(), curTimeSlice);
        avtContract_p gc = dob->GetOriginatingSource()->GetGeneralContract();
        avtSILRestriction_p silr = gc->GetDataRequest()->GetRestriction();

        avtDataRequest_p new_dr = new avtDataRequest(lastContract->GetDataRequest(), silr);
        new_contract = new avtContract(lastContract, new_dr);
    }
    new_contract->GetDataRequest()->SetTimestep(curTimeSlice);
    GetInput()->Update(new_contract);

    // Need to make sure we have the right active variable for pathlines.
    std::string velocityName, meshName;
    avtDataRequest_p dr = lastContract->GetDataRequest();
    GetPathlineVelocityMeshVariables(dr, velocityName, meshName);
    GetTypedInput()->SetActiveVariable(velocityName.c_str());

    UpdateIntervalTree(curTimeSlice);
    if (intervalTree == NULL)
        return false;

    if (! OperatingOnDemand())
    {
        GetAllDatasetsArgs ds_list;
        bool dummy = false;
        GetInputDataTree()->Traverse(CGetAllDatasets, (void*)&ds_list, dummy);

        // Release all the old dataSets.
        for (size_t i = 0; i < dataSets.size(); i++)
        {
            if(dataSets[i])
            {
                dataSets[i]->UnRegister(NULL);
                dataSets[i] = NULL;
            }
        }

        // Load the dataSets map with the new datasets for the next time step.
        numDomains = intervalTree->GetNLeaves();
        dataSets.resize(numDomains, NULL);
        for (size_t i = 0; i < ds_list.domains.size(); i++)
        {
            vtkDataSet *ds = ds_list.datasets[i];
            ds->Register(NULL);
            dataSets[ ds_list.domains[i] ] = ds;
        }
    }

    // Need to update the domain to rank mapping because the
    // domain numbers have changed.
    ComputeDomainToRankMapping();

    // The mesh may have changed and the ICs need to update their
    // domain.  If a pos_cmfe, then update the domains. If a
    // conn_cmfe, this expensive step is not needed
    if (pathlineCMFE == PICS_POS_CMFE)
        icAlgo->UpdateICsDomain( curTimeSlice );

    return true;
}

// ****************************************************************************
//  Method: avtPICSFilter::GetTimeStep
//
//  Purpose:
//      Determine the time step from a t value.
//
//  Programmer: Dave Pugmire
//  Creation:   March 4, 2009
//
//  Modifications:
//    Gunther H. Weber, Thu Apr  2 10:59:47 PDT 2009
//    Return activeTimeStep obtained from contract instead of 0 when doing
//    integral curves.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
//
//    Hank Childs, Sat Mar 10 14:10:47 PST 2012
//    Add support for reverse pathlines.
//
// ****************************************************************************

int
avtPICSFilter::GetTimeStep(double t) const
{
    if (doPathlines)
    {
        for (size_t i = 0; i < domainTimeIntervals.size(); i++)
        {
            if (integrationDirection == VTK_INTEGRATE_BACKWARD)
            {
                if (t >  domainTimeIntervals[i][0] &&
                    t <= domainTimeIntervals[i][1])
                {
                    return i+1;
                }
            }
            else
            {
                if (t >= domainTimeIntervals[i][0] &&
                    t <  domainTimeIntervals[i][1])
                {
                    return i;
                }
            }
        }
        return -1;
    }
    else
    {
        return activeTimeStep;
    }
}

// ****************************************************************************
//  Method: avtPICSFilter::DomainLoaded
//
//  Purpose:
//      Report back as to whether the domain can be loaded.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

bool
avtPICSFilter::BlockLoaded(BlockIDType &domain) const
{
    bool val = false;
#ifdef PARALLEL
    if (OperatingOnDemand())
    {
        val = avtDatasetOnDemandFilter::DomainLoaded(domain.domain, domain.timeStep);
    }
    else
        val = (PAR_Rank() == domainToRank[domain.domain]);
#else
    val = true;
#endif
    debug1<<"BlockLoaded("<<domain<<")= "<<val<<endl;
    return val;
}


// ****************************************************************************
// Method: avtPICSFilter::SetFieldType
//
// Purpose: 
//   Sets value for specialized fields.
//
// Arguments:
//   field : The new field type.
//
// Programmer: Christoph Garth
// Creation:   Mon Feb 25 16:14:44 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
avtPICSFilter::SetFieldType(int field)
{
    fieldType = field;
}


// ****************************************************************************
// Method: avtPICSFilter::SetFieldConstant
//
// Purpose: 
//   Sets a constant value for specialized fields.
//
// Arguments:
//   val : The value of the constant.
//
// Programmer: Christoph Garth
// Creation:   Mon Feb 25 16:14:44 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
avtPICSFilter::SetFieldConstant(double val)
{
    fieldConstant = val;
}


// ****************************************************************************
// Method: avtPICSFilter::SetMaxStepLength
//
// Purpose: 
//   Sets the filter's integration maximum step length.
//
// Arguments:
//   len : The new step length.
//
// Programmer: Christoph Garth
// Creation:   Mon Feb 25 16:14:44 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
avtPICSFilter::SetMaxStepLength(double len)
{
    maxStepLength = len;
}


// ****************************************************************************
// Method: avtPICSFilter::SetIntegrationType
//
// Purpose: 
//   Sets the filter's integration type.
//
// Arguments:
//   type : The type of integrator to use.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 15:28:31 EDT 2008
//
// Modifications:
//
// ****************************************************************************

void
avtPICSFilter::SetIntegrationType(int type)
{
    integrationType = type;
}


// ****************************************************************************
// Method: avtPICSFilter::SetPathlines
//
// Purpose: 
//   Turns pathlines on and off.
//
// Arguments:
//   pathlines : true if user wants to use pathlines.
//   overrideTime : true if user wants to use time0 and not the slider time.
//   time0 : Time value user wants to start with if overrideTime is true.
//
// Programmer: Dave Pugmire
// Creation:   Thu Mar  5 09:51:00 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Initialize seedTime0.
//
//   Hank Childs, Sat Mar 10 09:48:32 PST 2012
//   Add check for non-supported case (pathlines + integrating both directions).
//
// ****************************************************************************

void
avtPICSFilter::SetPathlines(bool pathlines,
                            bool overrideTime, double time0, double _period,
                            int _pathlineCMFE)
{
    doPathlines = pathlines;
    pathlineOverrideTime = overrideTime;
    seedTime0 = time0;
    period = _period;
    pathlineCMFE = _pathlineCMFE;

    if (doPathlines && (integrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS))
    {
        EXCEPTION1(VisItException, "VisIt is not capable of doing pathlines "
                     "calculations both forwards and backwards.  Please contact "
                     "a developer if this capability is needed.");
    }
}


// ****************************************************************************
// Method: avtPICSFilter::SetParallelizationAlgorithm
//
// Purpose: 
//   Sets the parallelization algorithm and parameters.
//
// Arguments:
//   algo : Type of algorithm
//   maxCnt : maximum number of integral curves to process before distributing.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 12:46:32 EDT 2008
//
// Modifications:
//
//   Dave Pugmire, Thu Feb  5 12:23:33 EST 2009
//   Add workGroupSize for masterSlave algorithm.
//
// ****************************************************************************

void
avtPICSFilter::SetParallelizationAlgorithm(int algo,
                                           int maxCnt,
                                           int domCache,
                                           int workGrpSz)
{
    method = algo;
    maxCount = maxCnt;
    cacheQLen = domCache;
    workGroupSz = workGrpSz;
}


// ****************************************************************************
// Method: avtPICSFilter::SetTolerances
//
// Purpose: 
//   Sets the filter's integration tolerances
//
// Arguments:
//   reltol : The new relative tolerance.
//   abstol : The new absolute tolerance.
//   isBBox : Is abstol a fraction of the bounding box.
//
// Programmer: Christoph Garth
// Creation:   Mon Feb 25 16:14:44 PST 2008
//
// Modifications:
//
//   Hank Childs, Fri Oct  1 20:35:21 PDT 2010
//   Add option for absTol that is a fraction of the bbox.
//
// ****************************************************************************

void
avtPICSFilter::SetTolerances(double reltol, double abstol, bool isFraction)
{
    relTol = reltol;
    absTol = abstol;
    absTolIsFraction = isFraction;
}


// ****************************************************************************
// Method: avtPICSFilter::SetIntegrationDirection
//
// Purpose: 
//   Sets the integral cruve integration direction
//
// Arguments:
//   dir : The new direction
//
// Programmer: Dave Pugmire
// Creation:   Thu Nov 15 12:09:08 EST 2007
//
// Modifications:
//   
//   Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//   Renamed to SetIntegrationDirection.
//
//   Hank Childs, Sat Mar 10 09:48:32 PST 2012
//   Add check for non-supported case (pathlines + integrating both directions).
//
// ****************************************************************************

void
avtPICSFilter::SetIntegrationDirection(int dir)
{
    integrationDirection = dir;

    if (doPathlines && (integrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS))
    {
        EXCEPTION1(VisItException, "VisIt is not capable of doing pathlines "
                     "calculations both forwards and backwards.  Please contact "
                     "a developer if this capability is needed.");
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::CheckOnDemandViability
//
//  Purpose:
//      Checks to see if on demand processing is viable.  Some generic checks
//      are made by the base class.  This check is to see if interval trees
//      are available, as interval trees are important to this module to do
//      on demand processing.
//
//  Programmer: Hank Childs
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Don't use on demand if user has not requested it.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Reverse the logic to check for on demand.
//
//   Hank Childs, Fri Mar 12 12:25:11 PST 2010
//   Don't use the interval tree if another filter has invalidated it
//   (i.e. displace, reflect)
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Fix bug with variable scoping.
//
// ****************************************************************************

bool
avtPICSFilter::CheckOnDemandViability(void)
{
    // If we don't want on demand, don't provide it.
    if (method == PICS_PARALLEL_OVER_DOMAINS)
    {
        if (DebugStream::Level1()) 
        {
            debug1 << "avtPICSFilter::CheckOnDemandViability(): = 0\n";
        }
        return false;
    }
    
    bool val = false;
    if (GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
    {
        avtIntervalTree *it = GetMetaData()->GetSpatialExtents(curTimeSlice);
        val = (it == NULL ? val : true);
    }
    if (DebugStream::Level1()) 
    {
        debug1 << "avtPICSFilter::CheckOnDemandViability(): = " << val <<endl;
    }
    return val;
}


// ****************************************************************************
//  Method: avtPICSFilter::Execute
//
//  Purpose:
//      Calculates an integral curve.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Jun 12 11:49:10 PDT 2008
//    Make our own copy of the interval tree to make sure it doesn't get 
//    deleted out from underneath us.
//
//    Hank Childs, Mon Jun 16 12:19:20 PDT 2008
//    Calculate a new interval tree when in non-on-demand mode.
//
//    Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//    In serial mode, set the cacheQLen to be the total number of domains.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add MasterSlave method.
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Initialize the initial domain load count and timer.
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of integral curves.
//
//   Dave Pugmire, Thu Dec  3 13:28:08 EST 2009
//   New methods for seedpoint generation.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//   
//   Dave Pugmire, Tue Jul 13 09:24:57 EDT 2010
//   Move icAlgo cleanup from Execute() to PostExecute(). The poincare plot
//   analysis was using IC data after Execute() had been called.
//  
//   Hank Childs, Thu Oct 21 08:54:51 PDT 2010
//   Detect when we have an empty data set and issue a warning (not crash).
//
//   Dave Pugmire, Thu Dec  2 11:29:47 EST 2010
//   Support for pathlines.
//
//   Hank Childs, Fri Mar 16 19:00:26 PDT 2012
//   Switch to Dave's new PODIC and change default algorithm to POS.
//
//   Hank Childs, Tue Apr 10 19:39:37 PDT 2012
//   Increase timeout when the number of particles is high.
//
// ****************************************************************************

void
avtPICSFilter::Execute(void)
{
    Initialize();
    if (emptyDataset)
    {
        avtCallback::IssueWarning("There was no data to advect over.");
        if (DebugStream::Level1()) 
        {
            debug1 << "No data for PICS filter.  Bailing out early." << endl;
        }
        return;
    }

    SetMaxQueueLength(cacheQLen);

#ifdef PARALLEL
    if (method == PICS_SERIAL)
        icAlgo = new avtSerialICAlgorithm(this);
    else if (method == PICS_PARALLEL_OVER_DOMAINS)
        icAlgo = new avtPODICAlgorithm(this, maxCount);
    /*
    else if (method == PICS_PARALLEL_COMM_DOMAINS)
        icAlgo = new avtCommDSOnDemandICAlgorithm(this, cacheQLen);
    else if (method == PICS_PARALLEL_MASTER_SLAVE)
    {
        icAlgo = avtMasterSlaveICAlgorithm::Create(this,
                                                   maxCount,
                                                   PAR_Rank(),
                                                   PAR_Size(),
                                                   workGroupSz);
    }
    */
#else
    icAlgo = new avtSerialICAlgorithm(this);
#endif

    InitialIOTime = visitTimer->LookupTimer("Reading dataset");
    
    // Check if we have a restart condition.
    if( restart != -1 )
    {
        RestoreICs(_ics, restart);
        icAlgo->RestoreInitialize(_ics, curTimeSlice);
    }
    else
    {
        GetIntegralCurvesFromInitialSeeds(_ics);
        icAlgo->Initialize(_ics);
    }

    if (_ics.size() > 10000)
    {
        // Lots of particles ... a timeout is more than likely the wrong
        // thing to do, so set it really high.
        avtCallback::ResetTimeout(60*1200);
    }

    if (doPathlines)
    {
        // for (int i = 0; i < domainTimeIntervals.size(); i++)
        // {
        //     while (1)
        //     {
        //         icAlgo->Execute();
                
        //         if (ContinueExecute())
        //             icAlgo->ResetIntegralCurvesForContinueExecute();
        //         else
        //             break;
        //     }

        //     if (icAlgo->CheckNextTimeStepNeeded(curTimeSlice) &&
        //      LoadNextTimeSlice())
        //     {
        //         icAlgo->ActivateICsForNextTimeStep();
                
        //      if( rollover )
        //        i = -1;
        //     }
        //     else
        //     {
        //         break;
        //     }
        // }

      while (1)
      {
        for (size_t i = 0; i < domainTimeIntervals.size(); i++)
        {
            icAlgo->Execute();
                
            if (icAlgo->CheckNextTimeStepNeeded(curTimeSlice) &&
                LoadNextTimeSlice())
            {
                icAlgo->ActivateICsForNextTimeStep();
                
                if( rollover )
                  i = -1;
            }
            else
            {
                break;
            }
        }

        if (ContinueExecute())
          icAlgo->ResetIntegralCurvesForContinueExecute();
        else
          break;
      }
    }
    else
    {
        while (1)
        {
            icAlgo->Execute();

            if (ContinueExecute())
                icAlgo->ResetIntegralCurvesForContinueExecute();
            else
                break;
        }
    }
}


// ****************************************************************************
//  Function: AlgorithmToString
//
//  Purpose:
//      Gets the name of an algorithm
//
//  Programmer: Hank Childs
//  Creation:   September 29, 2010
//
//  Modifications:
//
//    David Camp, Thu Jul 17 12:55:02 PDT 2014
//    Changed the names of the parallel algorithms to match the GUI.
//
// ****************************************************************************

const char *
AlgorithmToString(int algo)
{
    if (algo == PICS_PARALLEL_OVER_DOMAINS)
    {
        static const char *s = "Parallelize over domains";
        return s;
    }
    if (algo == PICS_PARALLEL_COMM_DOMAINS)
    {
        static const char *s = "Communicate domains";
        return s;
    }
    if (algo == PICS_PARALLEL_MASTER_SLAVE)
    {
        static const char *s = "Parallelize over curves and domains";
        return s;
    }
    if (algo == PICS_SERIAL)
    {
        static const char *s = "Parallelize over curves";
        return s;
    }
    if (algo == PICS_VISIT_SELECTS)
    {
        static const char *s = "VisIt Selects Best Algo";
        return s;
    }

    static const char *s = "Unknown Algorithm";
    return s;
}


// ****************************************************************************
//  Method: avtPICSFilter::Initialize
//
//  Modifications:
//
//   Hank Childs, Mon Jul 21 13:09:13 PDT 2008
//   Remove the "area code" from the initialization so it will compile on
//   my box.
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add statusMsgSz.
//
//   Hank Childs, Tue Jan 20 13:06:33 CST 2009
//   Add support for file formats that do their own domain decomposition.
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Initialize the initial domain load count and timer.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Hank Childs, Mon Mar 23 11:02:55 CDT 2009
//   Add handling for the case where we load data on demand using point
//   selections.
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Set seedTimeStep0 from input time value.
//
//   Dave Pugmire, Thu Apr  2 10:59:42 EDT 2009
//   Properly bound seedTime0 search.
//
//   Gunther H. Weber, Fri Apr  3 16:01:48 PDT 2009
//   Initialize seedTimeStep0 even when integral curves are computed
//   otherwise seed points get created for the wrong time step. 
//
//   Gunther H. Weber, Mon Apr  6 19:19:31 PDT 2009
//   Initialize seedTime0 for integral curve mode. 
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//   
//   Hank Childs, Thu Feb 18 13:01:31 PST 2010
//   Only set seedTime0 to the simulation time for pathlines and not 
//   streamlines.
//
//   Dave Pugmire, Mon Jun 14 14:16:57 EDT 2010
//   Allow serial algorithm to be run in parallel on single domain datasets.
//
//   Hank Childs, Thu Sep  2 10:50:05 PDT 2010
//   Deal with case where domain IDs are not unique.
//
//   Hank Childs, Sun Sep 19 11:04:32 PDT 2010
//   Parallel support for case where domain IDs are not unique.
//
//   Hank Childs, Wed Sep 29 19:25:06 PDT 2010
//   Add support for the "VisIt Selects" algorithm.
//
//   Hank Childs, Thu Oct 21 08:54:51 PDT 2010
//   Detect when we have an empty data set and issue a warning (not crash).
//
//   Dave Pugmire, Mon Feb  7 13:45:54 EST 2011
//   Ensure spatial meta data is ok. If not, recompute.
//
// ****************************************************************************

void
avtPICSFilter::Initialize()
{
    // Need to make sure we have the right active variable for pathlines.
    std::string velocityName, meshName;
    avtDataRequest_p dr = lastContract->GetDataRequest();
    GetPathlineVelocityMeshVariables(dr, velocityName, meshName);
    GetTypedInput()->SetActiveVariable(velocityName.c_str());

    emptyDataset = false;
    dataSpatialDimension = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

    // Get/Compute the interval tree.
    avtIntervalTree *it_tmp = GetMetaData()->GetSpatialExtents( curTimeSlice );

    bool dontUseIntervalTree = false;
    if (GetInput()->GetInfo().GetAttributes().GetDynamicDomainDecomposition() ||
        !GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
    {
        // The reader returns an interval tree with one domain (for everything).
        // This is not what we want.  So forget about this one, as we will be 
        // better off calculating one.
        dontUseIntervalTree = true;
    }

    if (it_tmp == NULL || dontUseIntervalTree)
    {
        UpdateIntervalTree(curTimeSlice);
        if( intervalTree == NULL )
            return;
    }
    else
    {
        // Make a copy so it doesn't get deleted out from underneath us.
        intervalTree = new avtIntervalTree(it_tmp);
    }

    //Set domain/ds info.
    numDomains = intervalTree->GetNLeaves();
    domainToRank.resize(numDomains,0);
    dataSets.resize(numDomains,NULL);

#ifdef PARALLEL
    int rank = PAR_Rank();
    int nProcs = PAR_Size();
#endif
    
    // Assign domains to processors, if needed.
    // For load on demand, just give some reasonable default 
    // domainToRank mapping for now.
    if (OperatingOnDemand())
    {
#ifdef PARALLEL
        int amountPer = numDomains / nProcs;
        int oneExtraUntil = numDomains % nProcs;
        int lastDomain = 0;
    
        for (int p = 0; p < nProcs; p++)
        {
            int extra = (p < oneExtraUntil ? 1 : 0);
            int num = amountPer + extra;
            for (int i = 0; i < num; i++)
                domainToRank[lastDomain+i] = p;
            lastDomain += num;
        }
#endif
    }
    else
    {
        // See what I have.
        GetAllDatasetsArgs ds_list;
        bool dummy = false;
        GetInputDataTree()->Traverse(CGetAllDatasets, (void*)&ds_list, dummy);

        // Set and communicate all the domains.
#ifdef PARALLEL
        if (numDomains > 1)
        {
            vector<int> myDoms;
            myDoms.resize(numDomains, 0);
            for (size_t i = 0; i < ds_list.domains.size(); i++)
                myDoms[ ds_list.domains[i] ] = rank;
            SumIntArrayAcrossAllProcessors(&myDoms[0],&domainToRank[0],numDomains);
            if (DebugStream::Level5()) 
            {
                debug5<<"numdomains= "<<numDomains<<" myDoms[0]= "<<myDoms[0]<<endl;
            }
        }
        else
            domainToRank[0] = rank;
#endif
        for (size_t i = 0; i < ds_list.domains.size(); i++)
        {
            vtkDataSet *ds = ds_list.datasets[i];
            ds->Register(NULL);
            dataSets[ ds_list.domains[i] ] = ds;
        }
        InitialDomLoads = (int)ds_list.domains.size();
    }

#ifdef PARALLEL
    // If not operating on demand, the method *has* to be parallel
    // static domains.
    int actualMethod = method;
    if (actualMethod == PICS_VISIT_SELECTS)
        actualMethod = PICS_SERIAL; // "SERIAL" means parallelize over
                                          // seeds.
    
    if ( ! OperatingOnDemand() )
    {
        if (DebugStream::Level1()) 
        {
            debug1 << "Not operating on demand, using parallel static domains instead." << endl;
        }
        actualMethod = PICS_PARALLEL_OVER_DOMAINS;
    }

    // Parallel and one domains, use the serial algorithm.
    if (numDomains == 1)
    {
        if (DebugStream::Level1()) 
        {
            debug1 << "Forcing load-on-demand because there is only one domain." << endl;
        }
        actualMethod = PICS_SERIAL;
    }

    if ((method != PICS_VISIT_SELECTS) && (method != actualMethod))
    {
        char str[1024];
        SNPRINTF(str, 1024,
                 "Warning: the selected algorithm \"%s\" could not be used, "
                 "instead the following algorithm was used \"%s\".\n",
                 AlgorithmToString(method), AlgorithmToString(actualMethod));
        avtCallback::IssueWarning(str);
    }
    method = actualMethod;
#else
    // for serial, it's all load on demand.
    method = PICS_SERIAL;
#endif

    if (DebugStream::Level5())
    {
        debug5<< "method: " << method << endl;
        debug5<< "Domain/Data setup:\n";
        for (int i = 0; i < numDomains; i++)
            debug5<<i<<": rank= "<< domainToRank[i]<<" ds= "<<dataSets[i]<<endl;
    }

    // Some methods need random number generator.
    srand(2776724);
}


// ****************************************************************************
//  Method: avtPICSFilter::InitializeTimeInformation
//
//  Purpose:
//      Gets time information needed to execute pathlines.
//
//  Programmer: Hank Childs (extracted from Initialize method)
//  Creation:   March 9, 2012
//
// ****************************************************************************

void
avtPICSFilter::InitializeTimeInformation(int currentTimeSliderIndex)
{
    if (doPathlines)
    {
        std::string db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
        ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
        if (*dbp == NULL)
          EXCEPTION1(InvalidFilesException, db.c_str());
        avtDatabaseMetaData *md = dbp->GetMetaData(0, 1);

        if (md->AreAllTimesAccurateAndValid() != true)
        {
            avtCallback::IssueWarning("Pathlines - The time data does not "
                                      "appear to be accurate and valid. "
                                      "Will continue.");
        }

        if (! pathlineOverrideTime)
            seedTime0 = md->GetTimes()[currentTimeSliderIndex];

        int numTimes = md->GetTimes().size() - 1;
        if (numTimes == 0)
        {
            if (DebugStream::Level5()) 
            {
                debug5 << "Pathlines - Only one time slice in the data set, "
                       << "two or more are needed\n";
            }
            EXCEPTION1(VisItException,
                       "Pathlines - Only one time slice in the data set, "
                       "two or more are needed.");
        }

        timeSliceInterval = md->GetTimes()[1] - md->GetTimes()[0];

        if( period > 0 )
        {
          // The base time will always be at the current time slice
          if (integrationDirection == VTK_INTEGRATE_BACKWARD)
            baseTime = md->GetTimes()[currentTimeSliderIndex] - period;
          else
            baseTime = md->GetTimes()[currentTimeSliderIndex];

          // std::cerr << baseTime << "  " << md->GetTimes()[0] << "  "
          //        << md->GetTimes()[numTimes] << "  " << baseTime+period
          //        << std::endl;

          // Period checks make sure there are enough time slices
          if( numTimes < 2 )
          {
            EXCEPTION1(VisItException, "Periodic Pathlines - "
                       "Cannot advect in time periodically "
                       "because there are less than three time slices." );
          }

          if( baseTime                 < md->GetTimes()[0] ||
              md->GetTimes()[numTimes] < baseTime+period )
          {
            EXCEPTION1(VisItException, "Periodic Pathlines - "
                       "cannot advect in time because the specified "
                       "period time is outside the time slices available" );
          }

          if( seedTime0 < baseTime || baseTime+period < seedTime0 )
          {
            EXCEPTION1(VisItException, "Periodic Pathlines - "
                       "cannot advect in time because the specified time "
                       "is outside of the time period specified." );
          }

          if( period < md->GetTimes()[2] - md->GetTimes()[0] )
          {
            EXCEPTION1(VisItException, "Periodic Pathlines - "
                       "the period must be greater than twice the "
                       "time slice interval.");
          }

          double intPart, fracPart = modf(period / timeSliceInterval, &intPart);

          // std::cerr << period << "  " << timeSliceInterval << "  "
          //        << period / timeSliceInterval << "  "
          //        << intPart << "  " << fracPart << "  "
          //        << FLT_EPSILON << "  " << DBL_EPSILON
          //        << std::endl;

          if( fracPart > FLT_EPSILON )
          {
            EXCEPTION1(VisItException, "Periodic Pathlines - "
                       "the period must be an integer multiple of the "
                       "time slice interval .");
          }
        }
          
        if (DebugStream::Level5()) 
        {
            debug5<<"Times: [";
        }


        for (int i = 0; i < numTimes; i++)
        {
            // Save only the needed intervals.
            if( period )
            {
                if( baseTime > md->GetTimes()[i] )
                    continue;

                if( baseTime+period < md->GetTimes()[i+1] )
                    break;
            }

            vector<double> intv(2);
            intv[0] = md->GetTimes()[i];
            intv[1] = md->GetTimes()[i+1];
            
            if (DebugStream::Level5()) 
            {
                debug5<<" ("<<intv[0]<<", "<<intv[1]<<")";
            }
            if (intv[0] >= intv[1])
            {
                EXCEPTION1(VisItException,
                           "Pathlines - Found two adjacent steps that are not "
                           "increasing or equal in time.");
            }

            if (period && fabs((intv[1]-intv[0]) - timeSliceInterval) > FLT_EPSILON )
            {
                EXCEPTION1(VisItException, "Periodic Pathlines - "
                           "Found two adjacent time steps that do not have the "
                           "same interval as the others.");
            }

            domainTimeIntervals.push_back(intv);
        }
        if (DebugStream::Level5()) 
        {
            debug5<<"]"<<endl;
        }
        
        // Check if we have a restart.
        if( restart == -1 )
        {
            if (integrationDirection == VTK_INTEGRATE_BACKWARD)
            {
                if (seedTime0 == md->GetTimes()[0])
                    EXCEPTION1(VisItException,
        "Pathlines - cannot advect backward in time because the specified "
        "starting time is already at the beginning of the simulation time.  "
        "The plot can successfully execute by selecting a later time "
        "step or by overriding the pathline start time.");
            }
            else
            {
                if (seedTime0 == md->GetTimes()[numTimes])
                    EXCEPTION1(VisItException,
        "Pathlines - cannot advect forward in time because the specified "
        "starting time is already at the end of the simulation time.  "
        "The plot can successfully execute by selecting an earlier time "
        "step or by overriding the pathline start time.");
            }

            // No restart, so set seedTimeStep0.
            seedTimeStep0 = -1;
            for (size_t i = 0; i < domainTimeIntervals.size(); i++)
            {
                if (integrationDirection == VTK_INTEGRATE_BACKWARD)
                {
                    if (seedTime0 > domainTimeIntervals[i][0] &&
                        seedTime0 <= domainTimeIntervals[i][1])
                    {
                        seedTimeStep0 = i+1;
                        curTimeSlice = seedTimeStep0;
                        break;
                    }
                }
                else
                {
                    if (seedTime0 >= domainTimeIntervals[i][0] &&
                        seedTime0 < domainTimeIntervals[i][1])
                    {
                        seedTimeStep0 = i;
                        curTimeSlice = seedTimeStep0;
                        break;
                    }
                }
            }
        }

        if (seedTimeStep0 == -1)
        {
            if (DebugStream::Level5()) 
            {
                debug5 << "Pathlines - Did not find starting interval for "
                       << "seedTime0: " << seedTime0 << endl;
            }
            EXCEPTION1(VisItException, "Invalid pathline starting time value.");
        }
    }
    else
    {
        seedTime0 = 0.0;
        seedTimeStep0 = activeTimeStep;
    }
}


void 
avtPICSFilter::UpdateIntervalTree(int timeSlice)
{
    if (OperatingOnDemand())
    {
        // Get/Compute the interval tree.
        avtIntervalTree *it_tmp = GetMetaData()->GetSpatialExtents(timeSlice);

// TODO: The code below can be simplified. Move duplicate code out side of the if statement.
        if (GetInput()->GetInfo().GetAttributes().GetDynamicDomainDecomposition())
        {
            // We are going to assume that the format that operates on
            // demand can accept hints about where the data lies and return
            // that data.
            // (This was previously an exception, so we haven't taken too
            //  far of a step backwards with this assumption.)
            if (DebugStream::Level1())
            {
                debug1 << "Pathlines - This file format reader does dynamic "
                       << "decomposition. Assuming it can handle hints about "
                       << "what data to read." << endl;
            }
            specifyPoint = true;

            if (intervalTree)
                delete intervalTree;

            // Use the dummy interval tree, so we have something that fits
            // the existing interface.
            // Make a copy so it doesn't get deleted out from underneath us.
            intervalTree = new avtIntervalTree(it_tmp);
        }
        else
        {
            // It should be there, or else we would have precluded 
            // OnDemand processing in the method CheckOnDemandViability.
            if (intervalTree)
                delete intervalTree;
            intervalTree = new avtIntervalTree(it_tmp);
        }
    }
    else 
    {
        bool dataIsReplicated = GetInput()->GetInfo().GetAttributes().
                                         DataIsReplicatedOnAllProcessors();
        bool performCalculationsOverAllProcs = true;
        if (dataIsReplicated)
            performCalculationsOverAllProcs = false;
        GetTypedInput()->RenumberDomainIDs(performCalculationsOverAllProcs);
        TRY
        {
            if (intervalTree)
            {
                delete intervalTree;
                intervalTree = NULL;
            }

            intervalTree = GetTypedInput()->CalculateSpatialIntervalTree(
                                           performCalculationsOverAllProcs);
        }
        CATCH(VisItException)
        {
            emptyDataset = true;
            if (intervalTree)
            {
                delete intervalTree;
                intervalTree = NULL;
            }
            return;
        }
        ENDTRY
    }
}

// ****************************************************************************
//  Method: avtPICSFilter::InitializeLocators
//
//  Purpose:
//      Initializes the locators.  Note that some locators don't need to be
//      initialized ... this really only makes sense for parallel static
//      domains.  For PSL, if we don't initialize, then we end up serializing
//      the initialization, as each processor busywaits and then initializes
//      when they actually get something to do.
//
//  Programmer: Hank Childs
//  Creation:   February 19, 2010
//
//  Modifications:
//
//   Christoph Garth, Fri Jul 9 12:53:11 PDF 2010
//   Replace vtkVisItCellLocator by avtCellLocator.
// 
// ****************************************************************************

void
avtPICSFilter::InitializeLocators(void)
{
    if (doPathlines || OperatingOnDemand() || specifyPoint)
        return;  // maybe this makes sense; haven't thought about it

    int t1 = visitTimer->StartTimer();
    for (int i = 0 ; i < numDomains ; i++)
    {
        BlockIDType dom;
        dom.domain = i;
        dom.timeStep = seedTimeStep0;
        if (OwnDomain(dom))
        {
            std::map<BlockIDType,avtCellLocator_p>::iterator cli = 
                domainToCellLocatorMap.find( dom );

            if( cli == domainToCellLocatorMap.end() )
            {
                avtVector pt(0,0,0);
                vtkDataSet *ds = GetDomain(dom, pt);
                SetupLocator(dom, ds);
            } 
        }
    }
    visitTimer->StopTimer(t1, "Initializing locators");
}

// ****************************************************************************
//  Method: avtPICSFilter::ReleaseData
//
//  Purpose:
//      Release data.
//
//  Programmer: David Camp
//  Creation:   August 25, 2010
//
//  Modifications:
//
//    Hank Childs, Mon Jul 11 10:52:56 PDT 2011  
//    Fix dangling pointer.
//
// ****************************************************************************

void
avtPICSFilter::ReleaseData(void)
{
    avtDatasetOnDemandFilter::ReleaseData();
    avtDatasetToDatasetFilter::ReleaseData();

    for (size_t i = 0; i < dataSets.size(); i++)
    {
        if (dataSets[i] != NULL)
        {
            dataSets[i]->UnRegister(NULL);
            dataSets[i] = NULL;
        }
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Copies data attributes from the input to the output.
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2010
//
// ****************************************************************************

void
avtPICSFilter::UpdateDataObjectInfo(void)
{
    avtDatasetToDatasetFilter::UpdateDataObjectInfo();
    avtDatasetOnDemandFilter::UpdateDataObjectInfo();

    GetOutput()->GetInfo().GetAttributes().SetDataIsReplicatedOnAllProcessors(true);
}


// ****************************************************************************
//  Method: avtPICSFilter::SetupLocator
//
//  Purpose:
//      Sets up a locator for a specific domain.
//
//  Programmer: Hank Childs
//  Creation:   February 19, 2010
//
//  Modifications:
//
//   Christoph Garth, Fri Jul 9 12:53:11 PDF 2010
//   Replace vtkVisItCellLocator by avtCellLocator. Return cached locator
//   or create one for this domain.
// 
//   Hank Childs, Sun Nov 28 12:39:26 PST 2010
//   Make cell locator be reference counted so it can be cached at the database
//   level.
//
// ****************************************************************************

avtCellLocator_p
avtPICSFilter::SetupLocator( const BlockIDType &domain, vtkDataSet *ds )
{
    avtCellLocator_p locator;

    bool isFace = false;
    bool isEdge = false;
    vtkDataArray* velData = ds->GetPointData()->GetVectors();

    if (velData)
        CheckStagger( ds, isEdge, isFace );

    if (!velData)
        velData = ds->GetCellData()->GetVectors();


    std::map<BlockIDType, avtCellLocator_p>::iterator it = 
        domainToCellLocatorMap.find( domain );
    
    if( it == domainToCellLocatorMap.end() )
    {
        int timer = visitTimer->StartTimer();

        if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        {
            if ( isFace ) 
            {
              debug5 << "avtPICSFilter::SetupLocator: creating rectilinear 'face' cell locator\n";
                locator = new avtCellLocatorRectFace( ds );
            }
            else if ( isEdge ) 
            {
                debug5 << "avtPICSFilter::SetupLocator: creating rectilinear 'edge' cell locator\n";
                locator = new avtCellLocatorRectEdge( ds );
            }
            else 
            {
                // nodal 
                locator = new avtCellLocatorRect( ds );
            }
        }
        else
        {
            int timeSliceForLocator;
            if (doPathlines && (pathlineCMFE == PICS_CONN_CMFE))
                timeSliceForLocator = -1; // share between time slices
            else // if (doPathlines && (pathlineCMFE == PICS_POS_CMFE))
                timeSliceForLocator = curTimeSlice; // share between time slices

            std::string velocityName, meshName;

            if (CacheLocators())
            {
                avtDataRequest_p dr = lastContract->GetDataRequest();
                GetPathlineVelocityMeshVariables(dr, velocityName, meshName);

                void_ref_ptr vrp =
                  FetchArbitraryRefPtr(SPATIAL_DEPENDENCE,
                                       velocityName.c_str(), domain.domain, 
                                       timeSliceForLocator, "BIH_CELL_LOCATOR");
                if (*vrp != NULL)
                {
                    locator = ref_ptr<avtCellLocator>((avtCellLocator*) (*vrp),
                                                      vrp.GetN());
                    locator->SetDataSet( ds );
                }
            }

            // If it wasn't in the cache, we have to build it.
            if (*locator == NULL)
            {
                locator = new avtCellLocatorBIH( ds );
                if (CacheLocators())
                {
                    void_ref_ptr vrp(*locator, avtCellLocator::Destruct, 
                                     locator.GetN());

                    StoreArbitraryRefPtr(SPATIAL_DEPENDENCE,
                                         velocityName.c_str(), domain.domain,
                                         timeSliceForLocator,
                                         "BIH_CELL_LOCATOR", vrp);
                }
            }
        }

        domainToCellLocatorMap[domain] = locator;

        visitTimer->StopTimer( timer, "SetupLocator" );
    }
    else
    {
        locator = it->second;
        locator->SetDataSet( ds );
    }

    return locator;
}

// ****************************************************************************
//  Method: avtPICSFilter::GetFieldForDomain
//
//  Purpose:
//      Construct a field instance for a specific domain.
//
//  Programmer: Christoph Garth
//  Creation:   July 13, 2010
//
//  Modifications:
//
//   Hank Childs, Sun Nov 28 12:39:26 PST 2010
//   Make cell locator be reference counted so it can be cached at the database
//   level.
//
//   Hank Childs, Fri Mar  9 16:50:48 PST 2012
//   Add support for reverse pathlines.
//
// ****************************************************************************

avtIVPField* 
avtPICSFilter::GetFieldForDomain(const BlockIDType &domain, vtkDataSet *ds)
{
    avtCellLocator_p locator = SetupLocator(domain, ds);

    std::vector<avtVector> offsets(3);
    bool haveOffsets = false;
    bool isFace = false;
    bool isEdge = false;
    vtkDataArray* velData = ds->GetPointData()->GetVectors();

    if (velData)
      CheckStagger(ds, isEdge, isFace);

    if (!velData)
        velData = ds->GetCellData()->GetVectors();
    if (velData)
    {
        vtkInformation* info = velData->GetInformation();
        if (info->Has(avtVariableCache::OFFSET_3_COMPONENT_0()))
        {
            double* vals = info->Get(avtVariableCache::OFFSET_3_COMPONENT_0());
            offsets[0].x = vals[0];
            offsets[0].y = vals[1];
            offsets[0].z = vals[2];

            if ((vals[0] != 0) || (vals[1] != 0) || (vals[2] != 0))
                haveOffsets = true;

            debug5 << "avtPICSFilter::GetFieldForDomain: field has offsets " 
                   << vals[0] << vals[1] << vals[2] << std::endl;
        }

        if (info->Has(avtVariableCache::OFFSET_3_COMPONENT_1()))
        {
            double* vals = info->Get(avtVariableCache::OFFSET_3_COMPONENT_1());
            offsets[1].x = vals[0];
            offsets[1].y = vals[1];
            offsets[1].z = vals[2];

            if ((vals[0] !=0) || (vals[1] !=0) || (vals[2] !=0))
                haveOffsets = true;
        } 
        
        if (info->Has(avtVariableCache::OFFSET_3_COMPONENT_2()))
        {
            double* vals = info->Get(avtVariableCache::OFFSET_3_COMPONENT_2());
            offsets[2].x = vals[0];
            offsets[2].y = vals[1];
            offsets[2].z = vals[2];
            
            if ((vals[0] !=0) || (vals[1] !=0) || (vals[2] !=0))
                haveOffsets = true;
        }
          
    }
    
    if (doPathlines)
    {
      if( fieldType == PICS_FIELD_DEFAULT )
      {
        if (integrationDirection == VTK_INTEGRATE_BACKWARD)
            return new avtIVPVTKTimeVaryingField(ds, *locator, 
                                       domainTimeIntervals[curTimeSlice-1][1],
                                       domainTimeIntervals[curTimeSlice-1][0]);
        else
            return new avtIVPVTKTimeVaryingField(ds, *locator, 
                                       domainTimeIntervals[curTimeSlice][0], 
                                       domainTimeIntervals[curTimeSlice][1]);
      }
      else if( fieldType == PICS_FIELD_NEKTARPP )
      {
#ifdef NEKTAR_PLUS_PLUS_FOUND
        if (integrationDirection == VTK_INTEGRATE_BACKWARD)
            return new avtIVPNektarPPTimeVaryingField(ds, *locator, 
                                       domainTimeIntervals[curTimeSlice-1][1],
                                       domainTimeIntervals[curTimeSlice-1][0]);
        else
            return new avtIVPNektarPPTimeVaryingField(ds, *locator, 
                                       domainTimeIntervals[curTimeSlice][0], 
                                       domainTimeIntervals[curTimeSlice][1]);
#else
        EXCEPTION1(ImproperUseException, "Requesting Nektar++ interpolation but VisIt has not been built with Nektar++ support.");

#endif
      }
      else
      {
        EXCEPTION1(ImproperUseException, "Can not do pathlines with "
                   "higher order elements at this time. "
                   "Use the default field type for linear interpolation.");
      }
    }
    else
    {
      if( fieldType == PICS_FIELD_FLASH )
        return new avtIVPFlashField(ds, *locator, fieldConstant );

      else if( fieldType == PICS_FIELD_M3D_C1_2D )
        return new avtIVPM3DC1Field(ds, *locator, fieldConstant );

      else if( fieldType == PICS_FIELD_M3D_C1_3D )
      {
        avtIVPM3DC1Field *field = new avtIVPM3DC1Field(ds, *locator, 1.0);
        field->reparameterize = true;
        return field;
      }
      else if( fieldType == PICS_FIELD_NEK5000 )
         return new avtIVPNek5000Field(ds, *locator);

      else if( fieldType == PICS_FIELD_NEKTARPP )
      {
#ifdef NEKTAR_PLUS_PLUS_FOUND
         return new avtIVPNektarPPField(ds, *locator);
#else
        EXCEPTION1(ImproperUseException, "Requesting Nektar++ interpolation but VisIt has not been built with Nektar++ support.");

#endif
      }
      else if( fieldType == PICS_FIELD_NIMROD )
         return new avtIVPNIMRODField(ds, *locator);

      else if (isFace) 
      {
        debug5 <<"avtPICSFilter::GetFieldForDomain() - using 'face' field interpolator." <<std::endl;
        avtIVPVTKFaceField* field = new avtIVPVTKFaceField(ds, *locator);
        return field;
      }

      else if (isEdge) 
      {
        debug5 <<"avtPICSFilter::GetFieldForDomain() - using 'edge' field interpolator." <<std::endl;
        avtIVPVTKEdgeField* field = new avtIVPVTKEdgeField(ds, *locator);
        return field;
      }

      else if (haveOffsets) 
      {
        debug5 <<"avtPICSFilter::GetFieldForDomain() - using offset field interpolator." <<std::endl;
        avtIVPVTKOffsetField* field = new avtIVPVTKOffsetField(ds, *locator);
        debug5 <<"avtPICSFilter::GetFieldForDomain() - offsets[0] = " <<offsets[0].x <<", " <<offsets[0].y <<", " <<offsets[0].z <<std::endl;
        debug5 <<"avtPICSFilter::GetFieldForDomain() - offsets[1] = " <<offsets[1].x <<", " <<offsets[1].y <<", " <<offsets[1].z <<std::endl;
        debug5 <<"avtPICSFilter::GetFieldForDomain() - offsets[2] = " <<offsets[2].x <<", " <<offsets[2].y <<", " <<offsets[2].z <<std::endl;
        field->SetNodeOffsets( offsets );
        return field;
      } else {
        debug5 <<"avtPICSFilter::GetFieldForDomain() - no offsets present, using regular interpolator." <<std::endl;
        return new avtIVPVTKField(ds, *locator);
      }
    }
}

// ****************************************************************************
//  Method: avtPICSFilter::ICInBlock
//
//  Purpose:
//      Determine if a point lies in a block.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension and optimization for reclinear grids.
//
//   Dave Pugmire,Thu Dec 18 13:24:23 EST 2008
//   Fix to rectilinear optimization. If there are ghost zones, need to do the
//   full check. Otherwise, points in ghost zones are reported as inside the
//   domain.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 23 18:33:10 EDT 2009
//   Make changes for point decomposed domain databases.
//
//   Hank Childs, Tue Mar 31 12:43:05 CDT 2009
//   Early return for 0 cells.
//
//   Hank Childs, Fri Apr  3 13:51:30 CDT 2009
//   Fixed a problem where on demand with point-based lookups could not
//   support multiple seedpoints.
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//
//   Hank Childs, Fri Feb 19 17:47:04 CST 2010
//   Use a separate routine to generate a cell locator.
//
//   Dave Pugmire, Tue Mar 23 11:11:11 EDT 2010
//   Make sure we ignore ghost zones with using cell locator.
//
//   Christoph Garth, Fri Jul 9 12:53:11 PDF 2010
//   Replace vtkVisItCellLocator by avtCellLocator
//
//   Hank Childs, Sun Nov 28 12:39:26 PST 2010
//   Make cell locator be reference counted so it can be cached at the database
//   level.
//
//   Dave Pugmire, Wed Jun  5 16:43:36 EDT 2013
//   Code hardening. Better handling for rectilinear grid corner cases.
//
// ****************************************************************************

bool
avtPICSFilter::ICInBlock(const avtIntegralCurve *ic, const BlockIDType &block)
{
    avtVector pt = ic->CurrentLocation();
    
    vtkDataSet *ds = GetDomain(block, pt);

    if (ds == NULL || ds->GetNumberOfCells() == 0)
        return false;

    //Rectilinear dataset.
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        return ICInRectilinearBlock(ic, block, ds);
    

    // check if we have a locator
    std::map<BlockIDType,avtCellLocator_p>::iterator cli = domainToCellLocatorMap.find(block);
    if (cli != domainToCellLocatorMap.end() && specifyPoint)
    {
        double bbox[6];
        cli->second->GetDataSet()->GetBounds(bbox);

        if (pt.x < bbox[0] || pt.x > bbox[1] ||
            pt.y < bbox[2] || pt.y > bbox[3] ||
            pt.z < bbox[4] || pt.z > bbox[5])
        {
            // We are getting data in a point based way and the point changed
            // and now we have a new "domain 0".  Remove the locator for the
            // old one.
            cli->second = NULL;
            domainToCellLocatorMap.erase(block);
        }
    }

    avtCellLocator_p locator = SetupLocator(block, ds);
    vtkIdType cell = locator->FindCell(&pt.x, NULL, true);

    if (cell != -1)
    {
        // check if this is perchance a ghost cell; 
        // if it is, we do not want this domain
        if (vtkDataArray* ghosts = ds->GetCellData()->GetArray("avtGhostZones"))
        {
            int gflags = ghosts->GetComponent(cell, 0);
            if (gflags)
                cell = -1;
        }
    }

    return (cell != -1);
}

//****************************************************************************
// Method:  avtPICSFilter::ICInRectilinearBlock
//
// Purpose:
//   Determines if the IC is inside this rectilinear block.
//
//
// Programmer:  Dave Pugmire
// Creation:    June  5, 2013
//
// Modifications:
//
//****************************************************************************

bool
avtPICSFilter::ICInRectilinearBlock(const avtIntegralCurve *ic, 
                                    const BlockIDType &block,
                                    vtkDataSet *ds)
{
    double bbox[6];
    avtVector pt = ic->CurrentLocation();

    intervalTree->GetElementExtents(block.domain, bbox);
    if (pt.x < bbox[0] || pt.x > bbox[1] || pt.y < bbox[2] || pt.y > bbox[3])
        return false;
    
    if (dataSpatialDimension == 3 && (pt.z < bbox[4] || pt.z > bbox[5]))
        return false;

    // If we're on a face, we want to avoid cases where the next step will move
    // the point outside the block.
    if (OnFaceAndPushedOut(ic, block, ds, bbox))
        return false;
    if (OnFaceAndPushedIn(ic, block, ds, bbox))
        return true;
    
    // If no ghost zones, the pt is in dataset.
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    if (ghosts == NULL)
        return true;
    else
    {
        avtCellLocator_p locator = SetupLocator(block, ds);
        vtkIdType cell = locator->FindCell(&pt.x, NULL, true);

        if (cell == -1)
            return false;
        
        // Check if pt in a ghost cell.
        else if (ghosts->GetComponent(cell, 0) != 0)
            return false;
    }

    return true;
}

//****************************************************************************
// Method:  avtPICSFilter::OnFaceAndPushedOut
//
// Purpose:
//   Determines if the IC is on a rectilinear face, but is pushed out
//   of the block.
//
//
// Programmer:  Dave Pugmire
// Creation:    June  5, 2013
//
// Modifications:
//  Kevin Bensema Thu Aug 1 20:51 PDT 
//  freed avtIVPField* field pointer to fix memory leak.
//
//****************************************************************************

bool
avtPICSFilter::OnFaceAndPushedOut(const avtIntegralCurve *ic,
                                  const BlockIDType &block,
                                  vtkDataSet *ds,
                                  double *bbox)
{
    avtVector pt = ic->CurrentLocation();
    double time = ic->CurrentTime();
    double t[3] = {(pt.x-bbox[0]) / (bbox[1]-bbox[0]),
                   (pt.y-bbox[2]) / (bbox[3]-bbox[2]),
                   0.0};
    if (dataSpatialDimension == 3)
        t[2] = (pt.z-bbox[4]) / (bbox[5]-bbox[4]);
    
    //avtVector v = ic->CurrentV();
    avtIVPField *field = GetFieldForDomain(block, ds);
    avtVector vec;
    (*field)(time, pt, vec);
    //vec = v;
    if (ic->direction == avtIntegralCurve::DIRECTION_BACKWARD)
        vec = -vec;

    delete field;

    if (t[0] < 0.01 && vec[0] < 0.0)
        return true;
    if (t[0] > 0.99 && vec[0] > 0.0)
        return true;

    if (t[1] < 0.01 && vec[1] < 0.0)
        return true;
    if (t[1] > 0.99 && vec[1] > 0.0)
        return true;
    
    if (dataSpatialDimension == 3)
    {
        if (t[2] < 0.01 && vec[2] < 0.0)
            return true;
        if (t[2] > 0.99 && vec[2] > 0.0)
            return true;
    }

    return false;
}

//****************************************************************************
// Method:  avtPICSFilter::OnFaceAndPushedIn
//
// Purpose:
//   Determines if the IC is on a rectilinear face, but is pushed out
//   of the block.
//
//
// Programmer:  Dave Pugmire
// Creation:    June  5, 2013
//
// Modifications:
//  Kevin Bensema Thu Aug 1 20:51 PDT 
//  freed avtIVPField* field pointer to fix memory leak.
//
//
//****************************************************************************

bool
avtPICSFilter::OnFaceAndPushedIn(const avtIntegralCurve *ic,
                                  const BlockIDType &block,
                                  vtkDataSet *ds,
                                  double *bbox)
{
    avtVector pt = ic->CurrentLocation();
    double time = ic->CurrentTime();
    double t[3] = {(pt.x-bbox[0]) / (bbox[1]-bbox[0]),
                   (pt.y-bbox[2]) / (bbox[3]-bbox[2]),
                   0.0};
    if (dataSpatialDimension == 3)
        t[2] = (pt.z-bbox[4]) / (bbox[5]-bbox[4]);
    
    //avtVector v = ic->CurrentV();
    avtIVPField *field = GetFieldForDomain(block, ds);
    avtVector vec;
    (*field)(time, pt, vec);
    //vec = v;
    if (ic->direction == avtIntegralCurve::DIRECTION_BACKWARD)
        vec = -vec;
    
    delete field;

    if (t[0] < 0.01 && vec[0] > 0.0)
        return true;
    if (t[0] > 0.99 && vec[0] < 0.0)
        return true;

    if (t[1] < 0.01 && vec[1] > 0.0)
        return true;
    if (t[1] > 0.99 && vec[1] < 0.0)
        return true;
    
    if (dataSpatialDimension == 3)
    {
        if (t[2] < 0.01 && vec[2] > 0.0)
            return true;
        if (t[2] > 0.99 && vec[2] < 0.0)
            return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtPICSFilter::OwnDomain
//
//  Purpose:
//      Reports whether or not this processor owns a given domain.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

bool
avtPICSFilter::OwnDomain(BlockIDType &domain)
{
#ifdef PARALLEL
    if (OperatingOnDemand())
        return true;
    return PAR_Rank() == DomainToRank(domain);
#else
    return true;
#endif
}


// ****************************************************************************
//  Method: avtPICSFilter::ComputeDomainToRankMapping
//
//  Purpose:
//      Compute a mapping of which domains live on which processors.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

void
avtPICSFilter::ComputeDomainToRankMapping()
{
#ifdef PARALLEL
    int rank = PAR_Rank();
    int nProcs = PAR_Size();
#endif

    domainToRank.resize(numDomains, 0);
    
    // Compute a balanced layout of domains to ranks.
    if (OperatingOnDemand())
    {
#ifdef PARALLEL
        int amountPer = numDomains / nProcs;
        int oneExtraUntil = numDomains % nProcs;
        int lastDomain = 0;
    
        for (int p = 0; p < nProcs; p++)
        {
            int extra = (p < oneExtraUntil ? 1 : 0);
            int num = amountPer + extra;
            for (int i = 0; i < num; i++)
                domainToRank[lastDomain+i] = p;
            lastDomain += num;
        }
#endif
    }
    else
    {
        // See what I have.
        GetAllDatasetsArgs ds_list;
        bool dummy = false;
        GetInputDataTree()->Traverse(CGetAllDatasets, (void*)&ds_list, dummy);

#ifdef PARALLEL
        if (numDomains > 1)
        {
            vector<int> myDoms;
            myDoms.resize(numDomains, 0);
            for (size_t i = 0; i < ds_list.domains.size(); i++)
                myDoms[ ds_list.domains[i] ] = rank;
            SumIntArrayAcrossAllProcessors(&myDoms[0], &domainToRank[0], numDomains);
            if (DebugStream::Level5()) 
            {
                debug5<<"numdomains= "<<numDomains<<" myDoms[0]= "<<myDoms[0]<<endl;
            }
        }
        else
            domainToRank[0] = rank;
#endif
    }

    if (DebugStream::Level5())
    {
        for (int i = 0; i < numDomains; i++)
            debug5<<"dom: "<<i<<": rank= "<<domainToRank[i]<<" ds= "<<dataSets[i] << endl;
    }
}

// ****************************************************************************
//  Modifications:
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change ".size() == 0" test with empty, as empty has much better 
//   performance.
//
// ****************************************************************************

int
avtPICSFilter::DomainToRank(BlockIDType &domain)
{
    if (domain.domain < 0 || (size_t)domain.domain >= domainToRank.size())
        EXCEPTION1(ImproperUseException, "Domain out of range.");

    return domainToRank[domain.domain];
}


// ****************************************************************************
//  Method: avtPICSFilter::AdvectParticle
//
//  Purpose:
//      The toplevel routine that actually integrates an integral curve
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Pass domain extents into integration for ghost zone handling.
//
//   Hank Childs, Tue Aug 19 14:41:44 PDT 2008
//   Make sure we initialize the bounds, especially if we are in 2D.
//
//   Dave Pugmire, Mon Mar 23 18:33:10 EDT 2009
//   Make changes for point decomposed domain databases.
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//   
//   Hank Childs, Thu Jun  3 10:22:16 PDT 2010
//   Use new name "GetCurrentLocation".
//
//   Hank Childs, Fri Jun  4 05:13:49 PDT 2010
//   Remove call for collecting statistics that was a no-op.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Dave Pugmire, Mon Dec 15 11:00:23 EST 2014
//   Return number of steps taken.
//
// ****************************************************************************

int
avtPICSFilter::AdvectParticle(avtIntegralCurve *ic)
{
    int numStepsTaken = 0;
    
    //If no blockList, see if we can set it.
    if (ic->blockList.empty())
        FindCandidateBlocks(ic);
    
    if (!ic->status.Integrateable())
        return numStepsTaken;

    bool haveBlock = false;
    BlockIDType blk;
    avtIVPField *field = NULL;
    while (!ic->blockList.empty())
    {
        avtVector pt = ic->CurrentLocation();
        blk = ic->blockList.front();
        ic->blockList.pop_front();
        vtkDataSet *ds = GetDomain(blk, pt);
        field = GetFieldForDomain(blk, ds);

        if (field->IsInside(ic->CurrentTime(),
                            ic->CurrentLocation()) == avtIVPField::OK)
        {
            haveBlock = true;
            break;
        }
        else
            delete field;
    }

    // std::cerr << (haveBlock ? "have block" : "no block") << std::endl;
    if (!haveBlock)
    {
        ic->status.ClearTemporalBoundary();
        ic->status.ClearSpatialBoundary();

        if (ic->blockList.empty())
            ic->status.SetExitSpatialBoundary();
        else
            ic->status.SetAtSpatialBoundary();
        return numStepsTaken;
    }

    numStepsTaken = ic->Advance(field);
    delete field;

    // double dt = ((double) ((int) (ic->CurrentTime()*100.0)) / 100.0);

    // std::cerr << (ic->CurrentTime()-dt) << "  "
    //        << dt << "  "
    //        << ic->CurrentTime() << "  "
    //        << ic->CurrentLocation() << "  "
    //        << ic->status << "  "
    //        << std::endl;

    if (!ic->status.Terminated())
        FindCandidateBlocks(ic, &blk);
    return numStepsTaken;
}


// ****************************************************************************
//  Method: avtPICSFilter::GetLengthScale
//
//  Purpose:
//      Gets the length scale of the volume.
//
//  Programmer: Hank Childs
//  Creation:   October 1, 2010
//
// ****************************************************************************

double
avtPICSFilter::GetLengthScale(void)
{
    double bbox[6];
    bool   gotBounds = false;
    if (GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
    {
        avtIntervalTree *it = GetMetaData()->GetSpatialExtents();
        if (it != NULL)
        {
            it->GetExtents(bbox);
            gotBounds = true;
        }
    }

    if (!gotBounds)
    {
        GetSpatialExtents(bbox);
    }

    double vol = 1;
    int    numDims = 0;
    if (bbox[1] > bbox[0])
    {
        vol *= (bbox[1]-bbox[0]);
        numDims++;
    }
    if (bbox[3] > bbox[2])
    {
        vol *= (bbox[3]-bbox[2]);
        numDims++;
    }
    if (bbox[5] > bbox[4])
    {
        vol *= (bbox[5]-bbox[4]);
        numDims++;
    }

    double length = pow(vol, 1.0/numDims);
    return length;
}


// ****************************************************************************
//  Method: avtPICSFilter::PreExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Hank Childs
//  Creation:   March 3, 2007
//
//  Modifications:
//
//    Dave Pugmire, Tue Aug 12 13:44:10 EDT 2008
//    Moved the box extents code to the seed point generation function.
//
//    Hank Childs, Fri Oct  1 20:43:34 PDT 2010
//    Add an option for absTol to be a fraction of the bbox.
//
// ****************************************************************************

void
avtPICSFilter::PreExecute(void)
{
    avtDatasetOnDemandFilter::PreExecute();

    double absTolToUse = absTol;
    if (absTolIsFraction)
    {
        double l = GetLengthScale();
        absTolToUse = l*absTol;
    }
    // Create the solver. --Get from user prefs.
    if (integrationType == PICS_INTEGRATE_EULER)
    {
        solver = new avtIVPEuler;
        solver->SetMaximumStepSize(maxStepLength);
    }
    else if (integrationType == PICS_INTEGRATE_LEAPFROG)
    {
        solver = new avtIVPLeapfrog;
        solver->SetMaximumStepSize(maxStepLength);
    }
    else if (integrationType == PICS_INTEGRATE_DORMAND_PRINCE)
    {
        solver = new avtIVPDopri5;
        solver->SetMaximumStepSize(maxStepLength);
        solver->SetTolerances(relTol, absTolToUse);
    }
    else if (integrationType == PICS_INTEGRATE_ADAMS_BASHFORTH)
    {
        solver = new avtIVPAdamsBashforth;
        solver->SetMaximumStepSize(maxStepLength);
        solver->SetTolerances(relTol, absTolToUse);
    }
    else if (integrationType == PICS_INTEGRATE_RK4)
    {
        solver = new avtIVPRK4;
        solver->SetMaximumStepSize(maxStepLength);
    }
    else if (integrationType == PICS_INTEGRATE_M3D_C1_2D)
    {
        solver = new avtIVPM3DC1Integrator;
        solver->SetMaximumStepSize(maxStepLength);
        solver->SetTolerances(relTol, absTolToUse);
    }

    if( solver )
    {
      solver->SetPeriod( period );
      solver->SetBaseTime( baseTime );
      solver->SetToCartesian( convertToCartesian );
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::PostExecute
//
//  Purpose:
//      Delete the solver.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//   Dave Pugmire, Tue Jul 13 09:24:57 EDT 2010
//   Move icAlgo cleanup from Execute() to PostExecute(). The poincare plot
//   analysis was using IC data after Execute() had been called.
//
//   Hank Childs, Thu Oct 21 08:54:51 PDT 2010
//   Account for the case where there's not data to advect over. 
//
//   Hank Childs/David Camp, Tue Mar 13 11:32:12 PDT 2012
//   Clear cache in queue before creating output.  (memory saver)
//
// ****************************************************************************

void
avtPICSFilter::PostExecute(void)
{
    avtDatasetOnDemandFilter::PostExecute();

    // If we are loading data on demand, we should clear the queue before 
    // creating our output (free up memory before more memory is needed).
    EmptyQueue();

    if (solver)
    {
        delete solver;
        solver = NULL;
    }

    if (intervalTree != NULL)
    {
        delete intervalTree;
        intervalTree = NULL;
    }

    if (icAlgo != NULL)
    {
        icAlgo->PostExecute();
        delete icAlgo;
        icAlgo = NULL;
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::GetIntegralCurvesFromInitialSeeds
//
//  Purpose:
//      Get the seed points out of the attributes.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//    Dave Pugmire, Tue Aug 12 13:44:10 EDT 2008
//    Moved the box extents code from PreExecute to here.
//    Attempt to slightly adjust seed points not in the DS.
//
//    Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//    Add dataSpatialDimension.
//
//    Hank Childs, Tue Aug 19 14:41:44 PDT 2008
//    Make sure we initialize the bounds, especially if we are in 2D.
//
//    Dave Pugmire, Wed Aug 20 10:37:24 EST 2008
//    Bug fix. The loop index "i" was being changed when trying to "wiggle"
//    seed points into domains.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add a seed point id attribute to each integral curve.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add 3 point density vars.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Initialize time step in domain and start time of integral curves.
//
//   Hank Childs, Mon Apr  6 17:42:55 PDT 2009
//   Change seedTimeStep0 to seedTime0 (integers were mistakenly being
//   send in as doubles).
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//
//   Hank Childs, Sun May  3 12:32:13 CDT 2009
//   Added support for point list sources.
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Set what scalars to compute on the avtStreamline object.
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of integral curves.
//
//   Dave Pugmire, Tue Nov  3 09:15:41 EST 2009
//   Bug fix. Seed points with multiple domains need to be given a separate ID.
//
//   Dave Pugmire, Thu Dec  3 13:28:08 EST 2009
//   Renamed this method.
//
//   Dave Pugmire (for Christoph Garth), Wed Jan 20 09:28:59 EST 2010
//   Add circle source.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sat Jun  5 16:30:00 PDT 2010
//   Have derived type set up point list.
//
// ****************************************************************************

void
avtPICSFilter::GetIntegralCurvesFromInitialSeeds(std::vector<avtIntegralCurve *> &curves)
{
    std::vector<avtVector> seedPts  = GetInitialLocations();
    std::vector<avtVector> seedVels = GetInitialVelocities();

    // Make sure the velocity list size matches the point list size.
    for( size_t i=seedVels.size(); i<seedPts.size(); ++i )
      // Use the last velocity if available or initalize to be zero.
      seedVels.push_back( seedVels.empty() ? avtVector(0,0,0) : seedVels[i-1]);

    vector<vector<int> > ids;
    CreateIntegralCurvesFromSeeds(seedPts, seedVels, curves, ids);
}

// ****************************************************************************
//  Method: avtPICSFilter::AddSeedpoint
//
//  Purpose:
//      Add additional seed points.
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
// ****************************************************************************

void
avtPICSFilter::AddSeedPoint(avtVector &pt,
                            avtVector &vel,
                            std::vector<avtIntegralCurve *> &ics)
{
    if (icAlgo == NULL)
        EXCEPTION1(ImproperUseException,
                   "Improper call of avtPICSFilter::AddSeedpoints");

    std::vector<std::vector<int> > ids;

    std::vector<avtVector> pts;
    pts.push_back( pt );

    std::vector<avtVector> vels;
    vels.push_back( vel );

    CreateIntegralCurvesFromSeeds(pts, vels, ics, ids);
    icAlgo->AddIntegralCurves(ics);
}

// ****************************************************************************
//  Method: avtPICSFilter::AddSeedpoints
//
//  Purpose:
//      Add additional seed points.
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//  
//
//  Modifications:
//
//  Dave Pugmire, Mon Mar 14 01:56:00 EDT 2011
//  Use AddSeedPoint() to add each point.
//  
// ****************************************************************************

void
avtPICSFilter::AddSeedPoints(std::vector<avtVector> &pts,
                             std::vector<avtVector> &vels,
                             std::vector<std::vector<avtIntegralCurve *> > &ics)
{
    for (size_t i = 0; i < pts.size(); i++)
    {
        vector<avtIntegralCurve *> icsFromPt;
        AddSeedPoint(pts[i], vels[i], icsFromPt);
        
        ics.push_back(icsFromPt);
    }
}

// ****************************************************************************
//  Method: avtPICSFilter::CreateIntegralCurvesFromSeeds
//
//  Purpose:
//      Create integral curves from seed points.
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Removed avtStreamlineWrapper:Dir
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sat Jun  5 16:26:12 CDT 2010
//   Use avtIntegralCurves.
//
//   Hank Childs, Fri Oct  8 23:30:27 PDT 2010
//   Don't do initialization of distance for integral curves.  That happens in
//   the derived type of integral curve now.
//
//   Dave Pugmire, Thu Sep  1 07:44:48 EDT 2011
//   Keep track of fwd/bwd pair IDs, as they might need to be unified later.
//
//   Hank Childs, Tue Dec  6 19:01:30 PST 2011
//   Have the particle ID always match the index in the seed list.
//
// ****************************************************************************

void
avtPICSFilter::CreateIntegralCurvesFromSeeds(std::vector<avtVector> &pts,
                                             std::vector<avtVector> &vels,
                                             std::vector<avtIntegralCurve *> &curves,
                                             std::vector<std::vector<int> > &ids)
{
    if (DebugStream::Level5())
    {
        debug5<<"number of IC to generate: "<<pts.size()<<endl;
        for (size_t i = 0; i < pts.size(); i++)
        {
            debug5 << "point: " << pts[i] << endl;
        }
    }

    for (size_t i = 0; i < pts.size(); i++)
    {
        avtVector seedPt;
        avtVector seedVel = vels[i];

        // Transform the seed into the correct coordinate systems so
        // it passes the domain tests.
        if( fieldType == PICS_FIELD_M3D_C1_2D )
        {
          // Convert the seed to cylindrical coordiantes.
          seedPt.x = sqrt(pts[i].x*pts[i].x+pts[i].y*pts[i].y);
          seedPt.y = 0; //atan2( pts[i].y, pts[i].x );
          seedPt.z = pts[i].z;
        }
        else if( fieldType == PICS_FIELD_M3D_C1_3D )
        {
          // Convert the seed to cylindrical coordinates
          seedPt.x = sqrt(pts[i].x*pts[i].x+pts[i].y*pts[i].y);
          seedPt.y = atan2( pts[i].y, pts[i].x );
          seedPt.z = pts[i].z;
        }
        else
          seedPt = pts[i];
        
        vector<int> seedPtIds;
        // Need a single ID for the IC even if there are many domains.
        int currentID = i;
        int nextID = -1;

        currentID = GetNextCurveID();
        if (integrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS)
        {
            currentID = 2*i;
            nextID = 2*i+1;

            nextID = GetNextCurveID();
        }

        if (integrationDirection == VTK_INTEGRATE_FORWARD)
        {
            solver->SetDirection( avtIVPSolver::DIRECTION_FORWARD );
            avtIntegralCurve *ic =
                CreateIntegralCurve(solver,
                                    avtIntegralCurve::DIRECTION_FORWARD,
                                    seedTime0, seedPt, seedVel, 
                                    currentID);
            FindCandidateBlocks(ic);
            curves.push_back(ic);
            seedPtIds.push_back(ic->id);
        }
        else if (integrationDirection == VTK_INTEGRATE_BACKWARD)
        {
            solver->SetDirection( avtIVPSolver::DIRECTION_BACKWARD );
            avtIntegralCurve *ic =
                CreateIntegralCurve(solver,
                                    avtIntegralCurve::DIRECTION_BACKWARD,
                                    seedTime0, seedPt, seedVel,
                                    currentID);
            FindCandidateBlocks(ic);
            curves.push_back(ic);
            seedPtIds.push_back(ic->id);
        }
        else if (integrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS)
        {
            solver->SetDirection( avtIVPSolver::DIRECTION_FORWARD );
            avtIntegralCurve *ic0 =
                CreateIntegralCurve(solver,
                                    avtIntegralCurve::DIRECTION_FORWARD,
                                    seedTime0, seedPt, seedVel,
                                    currentID);
            FindCandidateBlocks(ic0);
            curves.push_back(ic0);
            seedPtIds.push_back(ic0->id);
            
            solver->SetDirection( avtIVPSolver::DIRECTION_BACKWARD );
            avtIntegralCurve *ic1 =
                CreateIntegralCurve(solver,
                                    avtIntegralCurve::DIRECTION_BACKWARD,
                                    seedTime0, seedPt, seedVel,
                                    nextID);
            FindCandidateBlocks(ic1);
            curves.push_back(ic1);
            seedPtIds.push_back(ic1->id);
            
            fwdBwdICPairs.push_back(std::pair<int,int> (ic0->id, ic1->id));
        }
        
// TODO: what happens if we get 0 domains returned. We will still add the seed point to the list.
        ids.push_back(seedPtIds);
    }

//    MaxID = pts.size();

    // Sort them on domain.
    std::sort(curves.begin(), curves.end(), avtIntegralCurve::DomainCompare);

    if (DebugStream::Level5())
    {
        debug5<<"curves.size(): "<<curves.size()<<endl;
        for (size_t i = 0; i < curves.size(); i++)
        {
            avtIntegralCurve *ic = curves[i];
            avtVector loc = ic->CurrentLocation();
            debug5<<"Create seed: id= "<<ic->id<<", dom= "<<ic->blockList
                  <<", loc= " << loc <<endl;
        }
    }
}

// ****************************************************************************
//  Method: avtPICSFilter::ModifyContract
//
//  Purpose:
//      Modifies the contract for the PICS filter.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//   Dave Pugmire, Mon Jun 14 14:16:57 EDT 2010
//   Allow serial algorithm to be run in parallel on single domain datasets.
//
//   Hank Childs, Fri Sep  3 12:10:47 PDT 2010
//   Make sure that we tell upstream filters that we want continuous velocity
//   fields ... this is important for the Reflect operator.
//
//   Hank Childs, Tue Sep  7 23:29:40 PDT 2010
//   Fix problem with previous change regarding removing the color variable.
//
//   Hank Childs, Fri Nov 26 15:14:46 PST 2010
//   More robust test for removing the presence of "colorVar".
//
//   Hank Childs, Fri Mar  9 16:50:48 PST 2012
//   Add support for reverse pathlines.
//
// ****************************************************************************

avtContract_p
avtPICSFilter::ModifyContract(avtContract_p in_contract)
{
    // See if we can set pathlines.
    if (doPathlines)
    {
        std::string db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
        ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
        if (*dbp == NULL)
            EXCEPTION1(InvalidFilesException, db.c_str());
        avtDatabaseMetaData *md = dbp->GetMetaData(0);
        if (md->GetTimes().size() == 1)
            doPathlines = false;
    }
    InitializeTimeInformation(in_contract->GetDataRequest()->GetTimestep());

    avtDataRequest_p out_dr = new avtDataRequest(in_contract->GetDataRequest());
    out_dr->SetVelocityFieldMustBeContinuous(true);

    if ( fieldType == PICS_FIELD_M3D_C1_2D )
    {
        // Add in the other fields that the M3D 2D Interpolation needs
        // for doing their Newton's Metod.

        // Assume the user has selected B as the primary variable.
        // Which is ignored.

        // The mesh - N elements x 7
        out_dr->AddSecondaryVariable("hidden/elements"); // /time_000/mesh/elements

        // Variables on the mesh - N elements x 20
        out_dr->AddSecondaryVariable("hidden/equilibrium/f");  // /equilibrium/fields/f
        out_dr->AddSecondaryVariable("hidden/equilibrium/psi");// /equilibrium/fields/psi


        out_dr->AddSecondaryVariable("hidden/f");      // /time_XXX/fields/f
        out_dr->AddSecondaryVariable("hidden/f_i");    // /time_XXX/fields/f_i
        out_dr->AddSecondaryVariable("hidden/psi");    // /time_XXX/fields/psi
        out_dr->AddSecondaryVariable("hidden/psi_i");  // /time_XXX/fields/psi_i
    }
    else if ( fieldType == PICS_FIELD_M3D_C1_3D )
    {
        // Add in the other fields that the M3D 3D Interpolation needs.

        // Assume the user has selected B as the primary variable.
        // Which is ignored.

        // The mesh - N elements x 9
        out_dr->AddSecondaryVariable("hidden/elements"); // /time_000/mesh/elements

        // Variables on the mesh - N elements x 80
        out_dr->AddSecondaryVariable("hidden/equilibrium/f");  // /equilibrium/fields/f
        out_dr->AddSecondaryVariable("hidden/equilibrium/psi");// /equilibrium/fields/psi
        out_dr->AddSecondaryVariable("hidden/equilibrium/I");  // /equilibrium/fields/I

        out_dr->AddSecondaryVariable("hidden/f");    // /time_XXX/fields/f
        out_dr->AddSecondaryVariable("hidden/psi");  // /time_XXX/fields/psi
        out_dr->AddSecondaryVariable("hidden/I");    // /time_XXX/fields/I
    }
    else if ( fieldType == PICS_FIELD_NEK5000 )
    {
        // Add in the other fields that the NEK 5000 Interpolation needs

        // Assume the user has selected velocity as the primary variable.
    }
    else if ( fieldType == PICS_FIELD_NEKTARPP )
    {
        // Add in the other fields that the Nektar++ Interpolation needs

        // Assume the user has selected velocity as the primary variable.
    }
    else if ( fieldType == PICS_FIELD_NIMROD )
    {
        // Add in the other fields that the NIMROD Interpolation needs

        // Assume the user has selected B as the primary variable.
        // Which is ignored.

        // Fourier series grid and data stored on the original mesh
//        out_dr->AddSecondaryVariable("hidden/grid_fourier_series");  // grid
//        out_dr->AddSecondaryVariable("hidden/data_fourier_series");  // data
    }
    else if ( fieldType == PICS_FIELD_FLASH )
    {
        // Add in the other fields that the Flash Interpolation needs

        // Assume the user has selected B as the primary variable.
        // Which is ignored.

        out_dr->AddSecondaryVariable("B");  // Magnetic Field
        out_dr->AddSecondaryVariable("E");  // Eletric Field
    }

    // Create a CMFE expression that will contain the vector values at
    // the next time step. These values will be added to to current
    // time as a secondary variable.
    if (doPathlines)
    {
        ExpressionList *elist = ParsingExprList::Instance()->GetList();

        // Remove a previous expression, as it might have the wrong definition.
        for (int i = 0; i < elist->GetNumExpressions(); i++)
        {
            if (elist->GetExpressions(i).GetName() ==
                avtIVPVTKTimeVaryingField::NextTimePrefix)
            {
                elist->RemoveExpressions(i);
                break;
            }
        }

        std::string pathlineName, meshName;
        GetPathlineVelocityMeshVariables(out_dr, pathlineName, meshName);

        int timeOffset;
        if (integrationDirection == VTK_INTEGRATE_FORWARD)
            timeOffset = +1;
        else //if (integrationDirection == VTK_INTEGRATE_BACKWARD)
            timeOffset = -1;

        // Same mesh between the two time steps
        char defn[1024];
        if( pathlineCMFE == PICS_CONN_CMFE )
        {
            SNPRINTF(defn, 1024, "conn_cmfe(<[%d]id:%s>, <%s>)",
                     timeOffset, pathlineName.c_str(), meshName.c_str());
        }
        // Different mesh between the two time steps
        else //if( pathlineCMFE == PICS_POS_CMFE )
        {
            SNPRINTF(defn, 1024, "pos_cmfe(<[%d]id:%s>, <%s>, <%s>)",
                     timeOffset, pathlineName.c_str(), meshName.c_str(),
                     pathlineName.c_str());
        }

        Expression *e = new Expression();
        e->SetName(avtIVPVTKTimeVaryingField::NextTimePrefix);
        e->SetDefinition(defn);
        e->SetType(Expression::VectorMeshVar);
        elist->AddExpressions(*e);
        delete e;


        // Ask for a secondary var that will be the vector values at the
        // next time step.
        out_dr->AddSecondaryVariable(avtIVPVTKTimeVaryingField::NextTimePrefix);
    }
    
    avtContract_p out_contract;
    if ( *out_dr )
        out_contract = new avtContract(in_contract, out_dr);
    else
        out_contract = new avtContract(in_contract);

    out_contract->GetDataRequest()->SetDesiredGhostDataType(NO_GHOST_DATA);
    //out_contract->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);

#ifdef PARALLEL
    out_contract->SetReplicateSingleDomainOnAllProcessors(true);
#endif

    if( CheckIfRestart(restart) )
    {
        curTimeSlice = seedTimeStep0 = restart + 1;
        out_contract->GetDataRequest()->SetTimestep(curTimeSlice);
    }
    if (doPathlines)
    {
        out_contract->GetDataRequest()->SetTimestep(curTimeSlice);
    }

    lastContract = out_contract;

    return avtDatasetOnDemandFilter::ModifyContract(out_contract);
}


// ****************************************************************************
//  Method: avtPICSFilter::ExamineContract
//
//  Purpose:
//      Retrieve active time step from current contract.
//
//  Programmer: Gunther H. Weber
//  Creation:   April 2, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtPICSFilter::ExamineContract(avtContract_p in_contract)
{
    avtDatasetOnDemandFilter::ExamineContract(in_contract);
    activeTimeStep = in_contract->GetDataRequest()->GetTimestep();
    curTimeSlice = activeTimeStep;
}


// ****************************************************************************
//  Method: avtPICSFilter::GetTerminatedIntegralCurves
//
//  Purpose:
//      Return list of terminated integral curves.
//
//  Programmer: Dave Pugmire
//  Creation:   Mon Aug 17 09:23:32 EDT 2009
//
//  Modifications:
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
// ****************************************************************************

void
avtPICSFilter::GetTerminatedIntegralCurves(vector<avtIntegralCurve *> &ics)
{
    ics.resize(0);
    if (icAlgo)
        icAlgo->GetTerminatedICs(ics);
}


// ****************************************************************************
//  Method: avtPICSFilter::DeleteIntegralCurves
//
//  Purpose:
//      Delete integral curves
//
//  Programmer: Dave Pugmire
//  Creation:   Tue May 25 10:15:35 EDT 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtPICSFilter::DeleteIntegralCurves(vector<int> &icIDs)
{
    icAlgo->DeleteIntegralCurves(icIDs);
}


// ****************************************************************************
//  Method: avtPICSFilter::GetPathlineVelocityMeshVariables
//
//  Purpose:
//      This method is here for classes that inherit from avtPICSFilter to be
//  able to override the default values for the Pathlines Velocity and Mesh
//  fields.
//
//  Programmer: David Camp
//  Creation:   Wed Nov  3 11:04:19 PDT 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtPICSFilter::GetPathlineVelocityMeshVariables(avtDataRequest_p &dataRequest,
                                                std::string &velocity,
                                                std::string &mesh)
{
    mesh = dataRequest->GetVariable();
    velocity = mesh;
}


// ****************************************************************************
//  Method: avtPICSFilter::CacheLocators
//
//  Purpose:
//      Determines whether or not locators we calculate should be cached in
//      the database.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2010
//
// ****************************************************************************

bool
avtPICSFilter::CacheLocators(void)
{
#ifdef PARALLEL
    if (OperatingOnDemand())
        return false;
    if (method == PICS_PARALLEL_OVER_DOMAINS)
        return true;

    return false;
#else
    // Always true for serial.
    return true;
#endif
}

// ****************************************************************************
// Method:  avtPICSFilter::PostStepCallback()
//
// Purpose: Callback after each integration step is taken.
//   
//
// Programmer:  Dave Pugmire
// Creation:    January 14, 2011
//
// ****************************************************************************

bool
avtPICSFilter::PostStepCallback()
{
    return (icAlgo ? icAlgo->PostStepCallback() : false);
}

// ****************************************************************************
// Method:  avtPICSFilter::PurgeDomain()
//
// Purpose: The avtDatasetOnDemandFilter is purging a data set and we need
//          to remove the cell locator.
//   
//
// Programmer:  David Camp
// Creation:    May 1, 2011
//
// ****************************************************************************

void
avtPICSFilter::PurgeDomain( const int domain, const int timeStep )
{
    BlockIDType dom(domain, timeStep);
    std::map<BlockIDType,avtCellLocator_p>::iterator it =
      domainToCellLocatorMap.find( dom );

    if( it != domainToCellLocatorMap.end() )
    {
        it->second = NULL;
        domainToCellLocatorMap.erase( it );
    } 
}

// ****************************************************************************
//  Method: avtPICSFilter::CheckStagger
//
//  Purpose:
//      Determine if the attached vector field to a dataset is staggered.
//
//  Programmer: Alexander Pletzer
//  Creation:   Tue Dec  3 19:31:02 MST 2013
//
//  Modifications:
//
// ****************************************************************************
void
avtPICSFilter::CheckStagger( vtkDataSet *ds, bool &isEdge, bool &isFace )
{
    isFace = false;
    isEdge = false;

    // staggered data are always defined on the nodal sized mesh
    vtkDataArray* velData = ds->GetPointData()->GetVectors();
    if (velData)
    {
        vtkInformation* info = velData->GetInformation();
        debug5 << "avtPICSFilter::CheckStagger: checking if vector field " 
               << velData << " has STAGGER\n";
        if (info->Has(avtVariableCache::STAGGER()))
        {
            const char* stagger = info->Get(avtVariableCache::STAGGER());
            debug5 << "avtPICSFilter::CheckStagger: field has stagger " << stagger << std::endl;
            if (strcmp(stagger, "face") == 0) 
            {
              isFace = true;
            }
            else if (strcmp(stagger, "edge") == 0) 
            {
              isEdge = true;
            }
        }
    }
}
