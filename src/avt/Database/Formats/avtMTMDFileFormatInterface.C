/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                          avtMTMDFileFormatInterface.C                     //
// ************************************************************************* //

#include <avtMTMDFileFormatInterface.h>

#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtIOInformation.h>
#include <avtMTMDFileFormat.h>

#include <BadIndexException.h>
#include <DebugStream.h>


using std::vector;


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface constructor
//
//  Arguments:
//      f        The MTMD file format that we are an interface for.
//
//  Notes:       The interface owns the file format after this call.
//
//  Programmer:  Hank Childs
//  Creation:    April 4, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

avtMTMDFileFormatInterface::avtMTMDFileFormatInterface(avtMTMDFileFormat **lst,
                                                       int ntsgroups)
{
    chunks = lst;
    nTimestepGroups = ntsgroups;
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface destructor
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

avtMTMDFileFormatInterface::~avtMTMDFileFormatInterface()
{
    if (chunks != NULL)
    {
        for (int i=0; i<nTimestepGroups; i++)
        {
            if (chunks[i] != NULL)
            {
                delete chunks[i];
                chunks[i] = NULL;
            }
        }
        delete[] chunks;
    }
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetMesh
//
//  Purpose:
//      Gets the mesh from the file format.
//
//  Arguments:
//      ts      The desired time step.
//      dom     The domain.
//      mesh    The name of the mesh.
//
//  Returns:    The vtk mesh.
//
//  Progrmamer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

vtkDataSet *
avtMTMDFileFormatInterface::GetMesh(int ts, int dom, const char *mesh)
{
    int tsGroup = GetTimestepGroupForTimestep(ts);
    int localTS = GetTimestepWithinGroup(ts);
    return chunks[tsGroup]->GetMesh(localTS, dom, mesh);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetVar
//
//  Purpose:
//      Gets the var from the file format.
//
//  Arguments:
//      ts      The desired time step.
//      dom     The domain.
//      var     The name of the var.
//
//  Returns:    The vtk variable (field).
//
//  Progrmamer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

vtkDataArray *
avtMTMDFileFormatInterface::GetVar(int ts, int dom, const char *var)
{
    int tsGroup = GetTimestepGroupForTimestep(ts);
    int localTS = GetTimestepWithinGroup(ts);
    return chunks[tsGroup]->GetVar(localTS, dom, var);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetVectorVar
//
//  Purpose:
//      Gets the var from the file format.
//
//  Arguments:
//      ts      The desired time step.
//      dom     The domain.
//      var     The name of the var.
//
//  Returns:    The vtk variable (field).
//
//  Progrmamer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

vtkDataArray *
avtMTMDFileFormatInterface::GetVectorVar(int ts, int dom, const char *var)
{
    int tsGroup = GetTimestepGroupForTimestep(ts);
    int localTS = GetTimestepWithinGroup(ts);
    return chunks[tsGroup]->GetVectorVar(localTS, dom, var);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data from the file format.
//
//  Arguments:
//      var     The variable.
//      ts      The time step.
//      dom     The domain.
//      type    The type of auxiliary data.
//      args    Any additional arguments.
//
//  Returns:    A void * of the auxiliary data.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//
//    Dave Bremer, Wed Apr 23 14:55:31 PDT 2008
//    Allow metadata requests for information about all domains to go through.
//
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

void *
avtMTMDFileFormatInterface::GetAuxiliaryData(const char *var, int ts, int dom,
                          const char *type, void *args, DestructorFunction &df)
{
    int tsGroup = GetTimestepGroupForTimestep(ts);
    int localTS = GetTimestepWithinGroup(ts);
    return chunks[tsGroup]->GetAuxiliaryData(var, localTS, dom, type, args, df);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetFilename
//
//  Purpose:
//      Gets the name of the file we are using.
//
//  Arguments:
//      ts       The timestep
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

const char *
avtMTMDFileFormatInterface::GetFilename(int ts)
{
    int tsGroup = GetTimestepGroupForTimestep(ts);
    return chunks[tsGroup]->GetFilename();
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::SetDatabaseMetaData
//
//  Purpose:
//      Gets the database meta-data.  This is a tricky issue because we don't
//      want to read in the table of contents for every file.  Just read it
//      in for the first one and try to add some time step information.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Brad Whitlock, Wed May 14 09:43:16 PDT 2003
//    Added int argument to conform to new method prototype.
//
//    Brad Whitlock, Mon Oct 13 13:54:06 PST 2003
//    Added code to set the times in the metadata.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added anonymous bool arg satisfy interface. Added logic to populate
//    cycles and times
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Replaced -INT_MAX & -DBL_MAX with INVALID_CYCLE and INVALID_TIME
//
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

void
avtMTMDFileFormatInterface::SetDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState, bool)
{
    int i, j;

    GenerateTimestepCounts();

    //
    // Throw an exception if an invalid time state was requested.
    //
    if (timeState < 0 || timeState >= nTotalTimesteps)
    {
        EXCEPTION2(BadIndexException, timeState, nTotalTimesteps);
    }

    //
    // We know for sure that the number of states is the number of timesteps.
    //
    md->SetNumStates(nTotalTimesteps);

    //
    // Let the format plugin populate as much of database metadata as it can,
    // first. It migth actually set cycles/times too.
    //
    int tsGroup = GetTimestepGroupForTimestep(timeState);
    int localTS = GetTimestepWithinGroup(timeState);
    chunks[tsGroup]->SetDatabaseMetaData(md, localTS);

    //
    // Note: In an MTXX format, a single file has multiple time steps in it
    // So, we don't have the same kinds of semantics we do with STXX databases
    // in, for example, trying to guess cycle numbers from file names
    //
    if (md->AreAllCyclesAccurateAndValid(nTotalTimesteps) != true)
    {
        vector<int> cycles;
        for (i=0; i<nTimestepGroups; i++)
        {
            vector<int> tmp;
            chunks[i]->FormatGetCycles(tmp);
            cycles.insert(cycles.end(),tmp.begin(),tmp.end());
        }
        bool cyclesLookGood = true;
        for (i = 0; i < cycles.size(); i++)
        {
            if ((i != 0) && (cycles[i] <= cycles[i-1]))
            {
                cyclesLookGood = false;
                break;
            }
        }
        if (cycles.size() != nTotalTimesteps)
            cyclesLookGood = false;
        if (cyclesLookGood == false)
        {
            cycles.clear();
            for (i = 0; i < nTotalTimesteps; i++)
            {
                int tsg = GetTimestepGroupForTimestep(i);
                int lts = GetTimestepWithinGroup(i);
                int c = chunks[tsg]->FormatGetCycle(lts);

                cycles.push_back(c);

                if ((c == avtFileFormat::INVALID_CYCLE) ||
                   ((i != 0) && (cycles[i] <= cycles[i-1])))
                {
                    cyclesLookGood = false;
                    break;
                }
            }
        }

        //
        // Ok, now put cycles into the metadata
        //
        if (cyclesLookGood)
        {
            md->SetCycles(cycles);
            md->SetCyclesAreAccurate(true);
        }
        else
        {
            cycles.clear();
            for (j = 0 ; j < nTotalTimesteps ; j++)
            {
                cycles.push_back(j);
            }
            md->SetCycles(cycles);
            md->SetCyclesAreAccurate(false);
        }
    }

    if (md->AreAllTimesAccurateAndValid(nTotalTimesteps) != true)
    {
        // Set the times in the metadata.
        vector<double> times;
        for (i=0; i<nTimestepGroups; i++)
        {
            vector<double> tmp;
            chunks[i]->FormatGetTimes(tmp);
            times.insert(times.end(),tmp.begin(),tmp.end());
        }
        bool timesLookGood = true;
        for (i = 0; i < times.size(); i++)
        {
            if ((i != 0) && (times[i] <= times[i-1]))
            {
                timesLookGood = false;
                break;
            }
        }
        if (times.size() != nTotalTimesteps)
            timesLookGood = false;
        if (timesLookGood == false)
        {
            times.clear();
            for (i = 0; i < nTotalTimesteps; i++)
            {
                int tsg = GetTimestepGroupForTimestep(i);
                int lts = GetTimestepWithinGroup(i);
                double t = chunks[tsg]->FormatGetTime(lts);

                times.push_back(t);

                if ((t == avtFileFormat::INVALID_TIME) ||
                   ((i != 0) && (times[i] <= times[i-1])))
                {
                    timesLookGood = false;
                    break;
                }
            }
        }

        //
        // Ok, now put times into the metadata
        //
        if (timesLookGood)
        {
            md->SetTimes(times);
            md->SetTimesAreAccurate(true);
            md->SetTemporalExtents(times[0], times[times.size() - 1]);
        }
        else
        {
            times.clear();
            for (j = 0 ; j < nTotalTimesteps ; j++)
            {
                times.push_back((double)j);
            }
            md->SetTimes(times);
            md->SetTimesAreAccurate(false);
        }
    }

}

// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::SetCycleTimeInDatabaseMetaData
//
//  Purpose: Set cycles/times in metadata for all times if possible or just
//      current time
//
//  Programmer: Mark C. Miller 
//  Creation:   May 31, 2005 
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

void
avtMTMDFileFormatInterface::SetCycleTimeInDatabaseMetaData(
    avtDatabaseMetaData *md, int timeState)
{
    GenerateTimestepCounts();

    //
    // Throw an exception if an invalid time state was requested.
    //
    if (timeState < 0 || timeState >= nTotalTimesteps)
    {
        EXCEPTION2(BadIndexException, timeState, nTotalTimesteps);
    }

    vector<int> cycles;
    for (int i=0; i<nTimestepGroups; i++)
    {
        vector<int> tmp;
        chunks[i]->FormatGetCycles(tmp);
        cycles.insert(cycles.end(),tmp.begin(),tmp.end());
    }
    bool cyclesLookGood = true;
    for (int i = 0; i < cycles.size(); i++)
    {
        if ((i != 0) && (cycles[i] <= cycles[i-1]))
        {
            cyclesLookGood = false;
            break;
        }
    }
    if (cycles.size() != nTotalTimesteps)
        cyclesLookGood = false;
    if (cyclesLookGood == false)
    {
        int tsg = GetTimestepGroupForTimestep(timeState);
        int lts = GetTimestepWithinGroup(timeState);
        int c = chunks[tsg]->FormatGetCycle(lts);
        if (c != avtFileFormat::INVALID_CYCLE)
        {
            md->SetCycle(timeState, c);
            md->SetCycleIsAccurate(true, timeState);
        }
    }
    else
    {
        md->SetCycles(cycles);
        md->SetCyclesAreAccurate(true);
    }

    vector<double> times;
    for (int i=0; i<nTimestepGroups; i++)
    {
        vector<double> tmp;
        chunks[i]->FormatGetTimes(tmp);
        times.insert(times.end(),tmp.begin(),tmp.end());
    }
    bool timesLookGood = true;
    for (int i = 0; i < times.size(); i++)
    {
        if ((i != 0) && (times[i] <= times[i-1]))
        {
            timesLookGood = false;
            break;
        }
    }
    if (times.size() != nTotalTimesteps)
        timesLookGood = false;
    if (timesLookGood == false)
    {
        int tsg = GetTimestepGroupForTimestep(timeState);
        int lts = GetTimestepWithinGroup(timeState);
        double t = chunks[tsg]->FormatGetTime(lts);
        if (t != avtFileFormat::INVALID_TIME)
        {
            md->SetTime(timeState, t);
            md->SetTimeIsAccurate(true, timeState);
        }
    }
    else
    {
        md->SetTimes(times);
        md->SetTimesAreAccurate(true);
        md->SetTemporalExtents(times[0], times[times.size() - 1]);
    }
}

// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::FreeUpResources
//
//  Purpose:
//      Frees up the resources for the file (memory, file descriptors).
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

void
avtMTMDFileFormatInterface::FreeUpResources(int, int)
{
    for (int i = 0 ; i < nTimestepGroups ; i++)
    {
        chunks[i]->FreeUpResources();
    }
}

// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::ActivateTimestep
//
//  Purpose: Notify the format of our intention to read data for a given
//  timestep. This gives the format an opportunity to do whatever 
//  parallel collective work it might need to for the given timestep
//
//  Programmer: Mark C. Miller 
//  Creation:   February 23, 2004 
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

void
avtMTMDFileFormatInterface::ActivateTimestep(int ts)
{
    GenerateTimestepCounts();

    int tsGroup = GetTimestepGroupForTimestep(ts);
    int localTS = GetTimestepWithinGroup(ts);
    chunks[tsGroup]->ActivateTimestep(localTS);
}

// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::PopulateIOInformation
//
//  Purpose: Populate information regarding domain's assignment to files 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 16, 2004 
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
// ****************************************************************************

void
avtMTMDFileFormatInterface::PopulateIOInformation(int ts, avtIOInformation& ioInfo)
{
    int tsGroup = GetTimestepGroupForTimestep(ts);
    int localTS = GetTimestepWithinGroup(ts);
    chunks[tsGroup]->PopulateIOInformation(localTS, ioInfo);
}



// ****************************************************************************
// Method:  avtMTMDFileFormatInterface::GetTimestepGroupForTimestep
//
// Purpose:
//   Find the timestep group containing the given timestep.
//
// Arguments:
//   ts         the timestep
//
// Programmer:  Jeremy Meredith
// Creation:    January 28, 2010
//
// ****************************************************************************
int
avtMTMDFileFormatInterface::GetTimestepGroupForTimestep(int ts)
{
    int group = 0;
    while (group < tsPerGroup.size() &&
           tsPerGroup[group] <= ts)
    {
        ts -= tsPerGroup[group];
        ++group;
    }
    if (group >= tsPerGroup.size())
    {
        EXCEPTION2(BadIndexException, group, tsPerGroup.size());
    }
    return group;
}

// ****************************************************************************
// Method:  avtMTMDFileFormatInterface::GetTimestepWithinGroup
//
// Purpose:
//   Find the "local timestep", i.e. the index within the timestep-group
//   containing the given timestep.
//
// Arguments:
//   ts         the timestep
//
// Programmer:  Jeremy Meredith
// Creation:    January 28, 2010
//
// ****************************************************************************
int
avtMTMDFileFormatInterface::GetTimestepWithinGroup(int ts)
{
    int group = GetTimestepGroupForTimestep(ts);
    int base = 0;
    for (int i=0; i<group; i++)
        base += tsPerGroup[i];
    return ts - base;
}

// ****************************************************************************
// Method:  avtMTMDFileFormatInterface::GenerateTimestepCounts
//
// Purpose:
//   Generate the local information used to map local<->global time steps.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    January 28, 2010
//
// ****************************************************************************
void
avtMTMDFileFormatInterface::GenerateTimestepCounts()
{
    //
    // Count up the time steps from each timestep group
    //
    tsPerGroup.clear();
    nTotalTimesteps = 0;
    for (int i=0; i<nTimestepGroups; i++)
    {
        int n = chunks[i]->GetNTimesteps();
        tsPerGroup.push_back(n);
        nTotalTimesteps += n;
    }
}
