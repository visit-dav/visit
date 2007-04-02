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
// ****************************************************************************

avtMTMDFileFormatInterface::avtMTMDFileFormatInterface(avtMTMDFileFormat *f)
{
    format = f;
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface destructor
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

avtMTMDFileFormatInterface::~avtMTMDFileFormatInterface()
{
    if (format != NULL)
    {
        delete format;
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
// ****************************************************************************

vtkDataSet *
avtMTMDFileFormatInterface::GetMesh(int ts, int dom, const char *mesh)
{
    return format->GetMesh(ts, dom, mesh);
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
// ****************************************************************************

vtkDataArray *
avtMTMDFileFormatInterface::GetVar(int ts, int dom, const char *var)
{
    return format->GetVar(ts, dom, var);
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
// ****************************************************************************

vtkDataArray *
avtMTMDFileFormatInterface::GetVectorVar(int ts, int dom, const char *var)
{
    return format->GetVectorVar(ts, dom, var);
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
// ****************************************************************************

void *
avtMTMDFileFormatInterface::GetAuxiliaryData(const char *var, int ts, int dom,
                          const char *type, void *args, DestructorFunction &df)
{
    if (dom == -1)
    {
        debug5 << "Auxiliary data was requested of multiple timestep, "
               << "multiple domain file format.  Since the data was requested "
               << "for all domains, returning NULL." << endl;
        return NULL;
    }

    return format->GetAuxiliaryData(var, ts, dom, type, args, df);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetFilename
//
//  Purpose:
//      Gets the name of the file we are using.
//
//  Arguments:
//      <unused>   The timestep (does not really apply to this interface).
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

const char *
avtMTMDFileFormatInterface::GetFilename(int)
{
    return format->GetFilename();
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
// ****************************************************************************

void
avtMTMDFileFormatInterface::SetDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState, bool)
{
    int i, j;

    //
    // Throw an exception if an invalid time state was requested.
    //
    int nTimesteps = format->GetNTimesteps();
    if (timeState < 0 || timeState >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, timeState, nTimesteps);
    }

    //
    // We know for sure that the number of states is the number of timesteps.
    //
    md->SetNumStates(nTimesteps);

    //
    // Let the format plugin populate as much of database metadata as it can,
    // first. It migth actually set cycles/times too.
    //
    format->SetDatabaseMetaData(md, timeState);

    if (md->AreAllCyclesAccurateAndValid(nTimesteps) != true)
    {
        //
        // Note: In an MTXX format, a single file has multiple time steps in it
        // So, we don't have the same kinds of semantics we do with STXX databases
        // in, for example, trying to guess cycle numbers from file names
        //
        vector<int> cycles;
        format->FormatGetCycles(cycles);
        bool cyclesLookGood = true;
        for (i = 0; i < cycles.size(); i++)
        {
            if ((i != 0) && (cycles[i] <= cycles[i-1]))
            {
                cyclesLookGood = false;
                break;
            }
        }
        if (cycles.size() != nTimesteps)
            cyclesLookGood = false;
        if (cyclesLookGood == false)
        {
            cycles.clear();
            for (i = 0; i < nTimesteps; i++)
            {
                int c = format->FormatGetCycle(i);

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
            for (j = 0 ; j < nTimesteps ; j++)
            {
                cycles.push_back(j);
            }
            md->SetCycles(cycles);
            md->SetCyclesAreAccurate(false);
        }
    }

    if (md->AreAllTimesAccurateAndValid(nTimesteps) != true)
    {
        // Set the times in the metadata.
        vector<double> times;
        format->FormatGetTimes(times);
        bool timesLookGood = true;
        for (i = 0; i < times.size(); i++)
        {
            if ((i != 0) && (times[i] <= times[i-1]))
            {
                timesLookGood = false;
                break;
            }
        }
        if (times.size() != nTimesteps)
            timesLookGood = false;
        if (timesLookGood == false)
        {
            times.clear();
            for (i = 0; i < nTimesteps; i++)
            {
                double t = format->FormatGetTime(i);

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
            for (j = 0 ; j < nTimesteps ; j++)
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
// ****************************************************************************

void
avtMTMDFileFormatInterface::SetCycleTimeInDatabaseMetaData(
    avtDatabaseMetaData *md, int timeState)
{
    int i;

    //
    // Throw an exception if an invalid time state was requested.
    //
    int nTimesteps = format->GetNTimesteps();
    if (timeState < 0 || timeState >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, timeState, nTimesteps);
    }

    vector<int> cycles;
    format->FormatGetCycles(cycles);
    bool cyclesLookGood = true;
    for (i = 0; i < cycles.size(); i++)
    {
        if ((i != 0) && (cycles[i] <= cycles[i-1]))
        {
            cyclesLookGood = false;
            break;
        }
    }
    if (cycles.size() != nTimesteps)
        cyclesLookGood = false;
    if (cyclesLookGood == false)
    {
        int c = format->FormatGetCycle(timeState);
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
    format->FormatGetTimes(times);
    bool timesLookGood = true;
    for (i = 0; i < times.size(); i++)
    {
        if ((i != 0) && (times[i] <= times[i-1]))
        {
            timesLookGood = false;
            break;
        }
    }
    if (times.size() != nTimesteps)
        timesLookGood = false;
    if (timesLookGood == false)
    {
        double t = format->FormatGetTime(timeState);
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
// ****************************************************************************

void
avtMTMDFileFormatInterface::FreeUpResources(int, int)
{
    format->FreeUpResources();
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
// ****************************************************************************

void
avtMTMDFileFormatInterface::ActivateTimestep(int ts)
{
    format->ActivateTimestep(ts);
}

// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::PopulateIOInformation
//
//  Purpose: Populate information regarding domain's assignment to files 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 16, 2004 
//
// ****************************************************************************

void
avtMTMDFileFormatInterface::PopulateIOInformation(int ts, avtIOInformation& ioInfo)
{
    format->PopulateIOInformation(ts, ioInfo);
}
