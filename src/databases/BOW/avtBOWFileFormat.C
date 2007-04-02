/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                            avtBOWFileFormat.C                             //
// ************************************************************************* //

#include <avtBOWFileFormat.h>

#include <string>
#include <algorithm>
#include <set>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <visitstream.h>
#include <Utility.h>
#include <snprintf.h>
#include <DebugStream.h>
#include <TimingsManager.h>

#include <avtStructuredDomainBoundaries.h>

#include <avtDatabase.h>
#include <avtParallel.h>
#include <avtVariableCache.h>

#include <math.h>
#include "bow.h"

using     std::string;

#if defined(__GNUC__) && ((__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 2) || (__GNUC__ == 3 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ == 0))
#define DELOGIFY(V) ((V>=0.0)?1.0:-1.0) * pow(10.0, 16.0 * fabs(V) - 8.0)
#else
#define DELOGIFY(V) ((V>=0.0f)?1.0f:-1.0f) * powf(10.0f, 16.0f * fabsf(V) - 8.0f)
#endif

#define DEBUG_PRINT
#ifdef DEBUG_PRINT
// ****************************************************************************
// Method: print_array
//
// Purpose: 
//   Template function for helping to print arrays.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 11 14:50:25 PST 2006
//
// Modifications:
//   
// ****************************************************************************

template <class T>
void
print_array(ostream &os, const char *name, const T *data, int nElems,
    int space=8, bool comma=true, bool donewline = true)
{
    os << name << " = {";
    if(donewline)
       os << endl;
    int ct = 0;
    bool first = true;
    for(int i = 0; i < nElems; ++i)
    {
        if(comma && !first)
            os << ", ";
        first = false;
        os << data[i];
        if(ct < space-1)
        {
            ++ct;
        }
        else
        {
            if(!comma)
                os << "***end" << endl;
            else
                os << endl;
            first = true;
            ct = 0;
        }
    }
    os << "}" << endl;
}

// ****************************************************************************
// Method: operator <<
//
// Purpose: 
//   Prints out the contents of a bowinfo object.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:46:42 PST 2004
//
// Modifications:
//   
// ****************************************************************************
ostream &
operator << (ostream &os, bowinfo binf)
{
    os << "bowinfo = {" << endl;
    os << "\tversion = " << binf->version << endl;
    os << "\tnumbow = " << binf->numbow << endl;
    os << "\t";
    print_array(os, "size", binf->size, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "offset", binf->offset, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "notran", binf->notran, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "ti", binf->ti, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "bi", binf->bi, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "vi", binf->vi, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "xs", binf->xs, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "ys", binf->ys, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "zs", binf->zs, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "x0", binf->x0, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "y0", binf->y0, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "z0", binf->z0, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "v0", binf->v0, binf->numbow, 8,true,false);
    os << "\t";
    print_array(os, "v1", binf->v1, binf->numbow, 8,true,false);
    os << "}" << endl;

    return os;
}
#endif

// ****************************************************************************
// Method: my_bow_alloc
//
// Purpose: 
//   Called to allocate memory for the bow library.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:46:01 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void *my_bow_alloc(void *opaque, size_t size)
{
    long nLongs = (size / sizeof(long)) + (((size % sizeof(long)) > 0) ? 1 : 0);
    void *retval = (void *)(new long[nLongs]);
//        debug4 << "my_bow_alloc2: alloc " << retval << "(" << size
//               << " bytes,  allocated " << nLongs << " longs)" << endl;

    return retval;
}

// ****************************************************************************
// Method: my_bow_free
//
// Purpose: 
//   Called to deallocate memory for the bow library.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 14:46:01 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void my_bow_free(void *opaque, void *ptr)
{
    if(ptr != 0) 
    {
//        debug4 << "my_bow_free: freeing " << ptr << endl;
        long *lptr =  (long*)ptr;
        delete [] lptr;
    }
}

// ****************************************************************************
//  Method: avtBOWFileFormat constructor
//
//  Arguments:
//    filename : The name of the file that contains the data.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 30 11:27:47 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

avtBOWFileFormat::avtBOWFileFormat(const char *filename)
    : avtMTMDFileFormat(filename) , header(), cycles(), times()
{
    // Read the header file.
    if(header.Read(filename) < 0)
    {
        EXCEPTION1(InvalidFilesException, filename);
    }

    headerRead = true;
    cyclesAndTimesDetermined = false;
    domainConnectivtyComputed = false;
    domainConnectivity = 0;
}

// ****************************************************************************
// Method: avtBOWFileFormat::~avtBOWFileFormat
//
// Purpose: Destructor
//   
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:47:13 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

avtBOWFileFormat::~avtBOWFileFormat()
{
    if(domainConnectivity != 0)
        delete [] domainConnectivity;
}

// ****************************************************************************
// Method: avtBOWFileFormat::GetCycles
//
// Purpose: 
//   Returns the cycles.
//
// Arguments:
//   c : The vector that will contain the cycles.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:47:31 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtBOWFileFormat::GetCycles(std::vector<int> &c)
{
    const char *mName = "avtBOWFileFormat::GetCycles: ";
    debug4 << mName << "start" << endl;
    if(DetermineCyclesAndTimes())
        c = cycles;
    else
        avtMTMDFileFormat::GetCycles(c);
    debug4 << mName << "end" << endl;
}

// ****************************************************************************
// Method: avtBOWFileFormat::GetTimes
//
// Purpose: 
//   Returns the times.
//
// Arguments:
//   t : The vector that will contain the times.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:48:03 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtBOWFileFormat::GetTimes(std::vector<double> &t)
{
    const char *mName = "avtBOWFileFormat::GetTimes: ";
    debug4 << mName << "start" << endl;
    if(DetermineCyclesAndTimes())
        t = times;
    else
        avtMTMDFileFormat::GetTimes(t);
    debug4 << mName << "end" << endl;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 30 11:27:47 PDT 2006
//
// ****************************************************************************

int
avtBOWFileFormat::GetNTimesteps(void)
{
    const char *mName = "avtBOWFileFormat::GetNTimesteps: ";
    debug4 << mName << "start" << endl;
    DetermineCyclesAndTimes();
    debug4 << mName << "end" << endl;

    return cycles.size();
}

// ****************************************************************************
// Method: avtBOWFileFormat::DetermineCyclesAndTimes
//
// Purpose: 
//   Determines the list of cycles and times using different methods.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 11 16:36:21 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtBOWFileFormat::DetermineCyclesAndTimes()
{
    const char *mName = "avtBOWFileFormat::DetermineCyclesAndTimes: ";

    debug4 << mName << "start" << endl;

    if(!cyclesAndTimesDetermined)
    {
        int method = header.timeMethod;

        //
        // The user specified visit_timefile (method 0) where a time file
        // was given in the header. We'll use that.
        //
        if(method == 0)
        {
            ifstream ifile;
            ifile.open(header.m1_file);
            if(!ifile.is_open())
            {
                debug1 << mName << "could not read " << header.m1_file << endl;
                debug1 << mName << "Reverting to fallback time "
                    "determination method." << endl;
                method = -1;
            }
            else
            {
                debug4 << mName << "Reading cycles and times from file." << endl;
                char line[1000 + 1];
                while (!ifile.eof())
                {
                    ifile.getline(line, 1000);

                    int itmp;
                    double dtmp;
                    if(line[0] == '#')
                        continue;
                    else if(sscanf(line, "%d %lg", &itmp, &dtmp) == 2)
                    {
                        debug4 << "\tCycle " << itmp << ", Time " << dtmp << endl;
                        cycles.push_back(itmp);
                        times.push_back(dtmp);
                    }
                }
            }
        }  

        //
        // The user specified visit_times so let's calculate the cycles and times.
        //
        if(method == 1)
        {
            debug4 << mName << "Calculating cycles and times." << endl;
            int    c0 = header.m2_startCycle;
            double t0 = header.m2_startTime;
            for(int i = 0; i < header.m2_nStates; ++i)
            {
                debug4 << "\tCycle " << c0 << ", Time " << t0 << endl;
                cycles.push_back(c0);
                times.push_back(t0);

                c0 += header.m2_deltaCycle;
                t0 += header.m2_deltaTime;
            }
        }

        //
        // Fallback HACK. The user did not specify a time file or the information
        // needed to calculate cycles and times. This should really be a full
        // directory search for files that have different cycle numbers but that
        // can come later...
        //
        if(method == -1)
        {
            // Just for now...
            debug1 << mName << "No time method was given. Create some hack values." << endl;
            int index = 0;
            for(int i = 50; i <= 18300; i+=50, index++)
            {
                debug4 << "\tCycle " << i << ", Time " << index << endl;
                cycles.push_back(i);
                times.push_back(index);
            }
        }

        cyclesAndTimesDetermined = true;
    }

    return cyclesAndTimesDetermined;
}


// ****************************************************************************
//  Method: avtBOWFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 30 11:27:47 PDT 2006
//
// ****************************************************************************

void
avtBOWFileFormat::FreeUpResources(void)
{
    debug4 << "avtBOWFileFormat::FreeUpResources" << endl;
}


// ****************************************************************************
//  Method: avtBOWFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 30 11:27:47 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtBOWFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    debug4 << "avtBOWFileFormat::PopulateDatabaseMetaData" << endl;

    //
    // Add the mesh
    //
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "mesh";
    mmd->meshType = AVT_RECTILINEAR_MESH;
    mmd->numBlocks = header.numblocks;
    mmd->blockOrigin = 0;
    mmd->cellOrigin = 0;
    mmd->spatialDimension = 3;
    mmd->topologicalDimension = 3;
    mmd->hasSpatialExtents = false;
    mmd->blockTitle = "blocks";
    mmd->blockPieceName = "block";
    mmd->xUnits = "meters";
    mmd->yUnits = "meters";
    mmd->zUnits = "meters";
    md->Add(mmd);

    //
    // Add the scalar
    //
    AddScalarVarToMetaData(md, header.varname, "mesh", AVT_ZONECENT);
    AddScalarVarToMetaData(md, "delogify", "mesh", AVT_ZONECENT);
}

// ****************************************************************************
// Method: avtBOWFileFormat::ActivateTimestep
//
// Purpose: 
//   This method is called when we change time steps.
//
// Arguments:
//   ts : The time step to which we're changing.
//
// Note:       We calculate domain connectivity if we've not already done so.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:40:18 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtBOWFileFormat::ActivateTimestep(int ts)
{
    // Read the domain connectivity if we've not already read it in.
    debug1 << "avtBOWFileFormat::ActivateTimeStep: ts=" << ts << endl;
    if(!avtDatabase::OnlyServeUpMetaData())
        ReadDomainConnectivity();
}

// ****************************************************************************
// Method: avtBOWFileFormat::ComputeStrideAdjustedSize
//
// Purpose: 
//   Adjusts domain size for stride.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 14 18:26:44 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtBOWFileFormat::ComputeStrideAdjustedSize(const int *domainSize,
    int *adjustedSize) const
{
    adjustedSize[0] = domainSize[0] / header.stride[0];
    adjustedSize[1] = domainSize[1] / header.stride[1];
    adjustedSize[2] = domainSize[2] / header.stride[2];
    adjustedSize[0] = (adjustedSize[0] < 1) ? 1 : adjustedSize[0];
    adjustedSize[1] = (adjustedSize[1] < 1) ? 1 : adjustedSize[1];
    adjustedSize[2] = (adjustedSize[2] < 1) ? 1 : adjustedSize[2];
}

// ****************************************************************************
// Method: avtBOWFileFormat::ReadDomainConnectivity
//
// Purpose: 
//   Read in the domain connectivity, which tells VisIt where domains are
//   in space and how they are connected.
//
// Note:       This routine contains collective communication.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:38:17 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtBOWFileFormat::ReadDomainConnectivity()
{
    const char *mName = "avtBOWFileFormat::ReadDomainConnectivity: ";

    if(domainConnectivtyComputed)
        return;

    int total = visitTimer->StartTimer();

    //
    // Partition the workload of reading information from the BOW files.
    //
    debug4 << mName << "Procs: " << PAR_Size()
           << ", Rank: " << PAR_Rank() << endl;
    int start, stop;
    int doms_per_proc = header.numblocks / PAR_Size();
    if(doms_per_proc < 0)
    {
        // If there are more processors than domains then only let the
        // first N processors read BOW files. The others' arrays will
        // contain zeroes to ensure that things sum properly.
        if(PAR_Rank() < header.numblocks)
        {
            start = stop = PAR_Rank();
        }
        else
        {
            start = 1;
            stop = 0;
        }
    }
    else
    {
        start = PAR_Rank() * doms_per_proc;
        if(start > header.numblocks-1)
            start = 0;
        stop = start + doms_per_proc-1;
        if(PAR_Rank() == PAR_Size() - 1)
            stop = header.numblocks-1;
    }
    debug4 << mName << "Total domains: " << header.numblocks << endl;
    debug4 << mName << "Domains per processor: " << doms_per_proc << endl;
    debug4 << mName << "Working on domains [" << start << ", " << stop
           << "]" << endl;

    // Make sure that we have the proper list of cycles.
    DetermineCyclesAndTimes();

    //
    // Read the BOW files assigned to this processor.
    //
    int stage1 = visitTimer->StartTimer();
    int ts = cycles[0];
    int dom, errorCount = 0;
    int *thisConnectivity = new int[header.numblocks * 6];
    memset(thisConnectivity, 0, sizeof(int) * header.numblocks * 6);
    for(dom = start; dom <= stop; ++dom)
    {
        bool success = false;
        for(int count = 0; count < 3 && !success; ++count)
        {
            int *vals = thisConnectivity + 6 * dom;
            success = ReadSingleBOFDimensions(dom, ts, vals);
            if(success)
            {
                for(int j = 0; j < 6; ++j)
                    success &= vals[j] >= 0;
            }
            if(!success && count < 3-1)
                debug4 << "Retrying the read since there were errors" << endl;
        }

        if(!success)
        {
            ++errorCount;
            debug1 << mName << "Could not read info for domain " << dom << endl;
        }

        const int *size = thisConnectivity + dom * 6;
        const int *offset = thisConnectivity + dom * 6 + 3;
        debug4 << mName << "ThisDomain " << dom
               << " size[" << size[0]
               << ", " << size[1]
               << ", " << size[2] << "]"
               << " offset[" << offset[0]
               << ", " << offset[1]
               << ", " << offset[2] << "]" << endl;
    }
    visitTimer->StopTimer(stage1, "stage1");

    // Sum the errorCheck array across all processors. If it does not
    // equal header.numblocks then there were errors reading the
    // bow info and we need to throw an exception.
    int stage2 = visitTimer->StartTimer();
    SumIntAcrossAllProcessors(errorCount);
    visitTimer->StopTimer(stage2, "stage2");
    if(errorCount > 0)
    { 
        visitTimer->StopTimer(total, mName);
        delete [] thisConnectivity;
        EXCEPTION1(InvalidFilesException, "The domain connectivity could not "
           "be created because some bow files could not be opened.");
    }

    //
    // There were no errors so sum the connectivity arrays.
    //
    int stage3 = visitTimer->StartTimer();
    domainConnectivity = new int[header.numblocks * 6];
    memset(domainConnectivity, 0, header.numblocks * 6 * sizeof(int));
    SumIntArrayAcrossAllProcessors(thisConnectivity, domainConnectivity,
        header.numblocks * 6);
    debug4 << mName << endl;
    for(dom = 0; dom < header.numblocks; ++dom)
    {
        const int *size = domainConnectivity + dom * 6;
        const int *offset = domainConnectivity + dom * 6 + 3;
        debug4 << "\tDomain " << dom
               << " size[" << size[0]
               << ", " << size[1]
               << ", " << size[2] << "]"
               << " offset[" << offset[0]
               << ", " << offset[1]
               << ", " << offset[2] << "]" << endl;
    }
    visitTimer->StopTimer(stage3, "stage3");

    //
    // If we're doing strides then we'll need to figure out new domain
    // origins because dividing the origins by a stride that is not divisible
    // will produce gaps or worse.
    //
    typedef std::map<int, int> IntIntMap;
    IntIntMap X_OriginToSmallSize, Y_OriginToSmallSize, Z_OriginToSmallSize;
    for(int dom = 0; dom < header.numblocks; ++dom)
    {
        int *domainOrigin = &domainConnectivity[dom*6+3];
        int *domainSize = &domainConnectivity[dom*6];

        int adjustedSize[3];
        ComputeStrideAdjustedSize(domainSize, adjustedSize);

        X_OriginToSmallSize[domainOrigin[0]] = adjustedSize[0];
        Y_OriginToSmallSize[domainOrigin[1]] = adjustedSize[1];
        Z_OriginToSmallSize[domainOrigin[2]] = adjustedSize[2];
    }
    IntIntMap X_OriginToOrigin, Y_OriginToOrigin, Z_OriginToOrigin;
    int xorg = 0, yorg = 0, zorg = 0;
    for(IntIntMap::const_iterator xi = X_OriginToSmallSize.begin();
        xi != X_OriginToSmallSize.end(); ++xi)
    {
        X_OriginToOrigin[xi->first] = xorg;
        xorg += xi->second;
    }
    for(IntIntMap::const_iterator yi = Y_OriginToSmallSize.begin();
        yi != Y_OriginToSmallSize.end(); ++yi)
    {
        Y_OriginToOrigin[yi->first] = yorg;
        yorg += yi->second;
    }
    for(IntIntMap::const_iterator zi = Z_OriginToSmallSize.begin();
        zi != Z_OriginToSmallSize.end(); ++zi)
    {
        Z_OriginToOrigin[zi->first] = zorg;
        zorg += zi->second;
    }

    //
    // Populate the domain connectivity structure.
    //
    int stage4 = visitTimer->StartTimer();
    avtRectilinearDomainBoundaries *rdb = new avtRectilinearDomainBoundaries(true);
    rdb->SetNumDomains(header.numblocks);
    debug4 << "Creating domain connectivity taking stride into account" << endl;
    for(int dom = 0; dom < header.numblocks; ++dom)
    {
        int *domainOrigin = &domainConnectivity[dom*6+3];
        int *domainSize = &domainConnectivity[dom*6];

        int adjustedSize[3];
        ComputeStrideAdjustedSize(domainSize, adjustedSize);

        int extents[6];
        extents[0] = X_OriginToOrigin[domainOrigin[0]];
        extents[1] = extents[0] + adjustedSize[0];
        extents[2] = Y_OriginToOrigin[domainOrigin[1]];
        extents[3] = extents[2] + adjustedSize[1];
        extents[4] = Z_OriginToOrigin[domainOrigin[2]];
        extents[5] = extents[4] + adjustedSize[2];
        rdb->SetIndicesForRectGrid(dom, extents);

        debug4 << "\tDomain " << dom
               << " size[" << adjustedSize[0]
               << ", " << adjustedSize[1]
               << ", " << adjustedSize[2] << "]"
               << " offset[" << extents[0]
               << ", " << extents[2]
               << ", " << extents[4] << "]" << endl;
    }
    rdb->CalculateBoundaries();
    void_ref_ptr vr = void_ref_ptr(rdb,
                           avtStructuredDomainBoundaries::Destruct);
    cache->CacheVoidRef("any_mesh",
               AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);
    visitTimer->StopTimer(stage4, "stage4");

    // Indicate that we've computed the connectivity.
    domainConnectivtyComputed = true;

    debug4 << mName << "end" << endl;

    visitTimer->StopTimer(total, mName);
}

// ****************************************************************************
// Method: avtBOWFileFormat::GetFilenameForDomain
//
// Purpose: 
//   Gets the name of the BOW file for the specified domain and time step.
//
// Arguments:
//   filename : The array that will contain the filename.
//   len      : The maximum number of characters in filename.
//   dom      : The domain.
//   ts       : The time state.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:25:37 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtBOWFileFormat::GetFilenameForDomain(char *filename, int len, int dom, int ts)
{
    // Determine the file to use.
    int pp = dom;
    int tt = ts;
    SNPRINTF(filename, len, header.filenamepat, 
        pp % header.topdirmod, pp, (tt/500)*500, tt, pp);
    debug4 << "avtBOWFileFormat::GetFilenameForDomain: filename for dom="
           << dom << ", ts=" << ts << " is: " << filename << endl;
}

// ****************************************************************************
// Method: avtBOWFileFormat::GetBOWBytesForDomainAtTime
//
// Purpose: 
//   Returns an array of chars containing bytes from the BOW file for the
//   specified domain and time step.
//
// Arguments:
//   dom : The domain.
//   ts  : The time state.
//   infoOnly : If true then just read a small part of the start of the file.
//              If false then read the whole file into memory.
//
// Returns:    An array of chars containing the bytes from the BOW file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:22:25 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

char *
avtBOWFileFormat::GetBOWBytesForDomainAtTime(int dom, int ts, bool infoOnly)
{
    const char *mName = "avtBOWFileFormat::GetBOWBytesForDomainAtTime: ";

    // Get the BOW filename.
    char filename[2000];
    GetFilenameForDomain(filename, 2000, dom, ts);

    // Open the BOW file and read it.
    FILE *fp = fopen(filename, "rb");
    char *data = 0;
    if(fp != 0)
    {
        long size = 500;
        if(!infoOnly)
        {
            fseek(fp, 0, SEEK_END);
            size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            debug4 << mName << "Determined the file is " << size
                   << " bytes long" << endl;
        }
        debug4 << mName << "Allocating " << size << " bytes." << endl;
        data = new char[size];
        fread(data, size, 1, fp);
        fclose(fp);
    }

    return data;    
}

// ****************************************************************************
// Method: avtBOWFileFormat::ReadSingleBOFDimensions
//
// Purpose: 
//   Reads the size and offset of the BOW for the specified domain and timestate.
//
// Arguments:
//   dom     : The domain
//   ts      : The time state.
//   dimsize : Return array of 6 ints containing the size and offset.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:20:14 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtBOWFileFormat::ReadSingleBOFDimensions(int dom, int ts, int *dimsize)
{
    const char *mName = "avtBOWFileFormat::ReadSingleBOFDimensions: ";

    //   
    // Read the first part of the file so we can get to the bow header info.
    //
    bool retval = false;
    char *data = GetBOWBytesForDomainAtTime(dom, ts, true);
    if(data != 0)
    {
        //
        // Get the BOW info out of the data array.
        //
        bowglobal bg = bowglobal_create(my_bow_alloc, my_bow_free, 0);
        bowinfo binf = bow_getbowinfo(bg, data);
        if(binf == 0)
        {
            debug4 << mName << "bow_getbowinfo returned 0!"
                   << endl;
        }
        else
        {
            dimsize[0] = binf->xs[0];
            dimsize[1] = binf->ys[0];
            dimsize[2] = binf->zs[0];
            dimsize[3] = binf->x0[0];
            dimsize[4] = binf->y0[0];
            dimsize[5] = binf->z0[0];
            retval = true;
            debug4 << mName << filename << "BOW info = \n" << binf << endl;
        }

        delete [] data;
        bowglobal_destroy(bg);
    }
    else
    {
        debug4 << mName << "Failed to get bow bytes!" << endl;
    }

    return retval;
}

// ****************************************************************************
//  Method: avtBOWFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 30 11:27:47 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtBOWFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    const char *mName = "avtBOWFileFormat::GetMesh: ";

    debug4 << mName << "ts=" << timestate << ", dom=" << domain << endl;

    const int *sizes = &domainConnectivity[domain * 6];
    const int *offsets = &domainConnectivity[domain * 6 + 3];

    //
    // Calculate the extents of the domain.
    //
    float extents[6];
    // for now...
    extents[0] = offsets[0] * header.dx;
    extents[1] = (offsets[0] + sizes[0]) * header.dx;
    extents[2] = offsets[1] * header.dy;
    extents[3] = (offsets[1] + sizes[1]) * header.dy;
    extents[4] = offsets[2] * header.dz;
    extents[5] = (offsets[2] + sizes[2]) * header.dz;

    //
    // The number of nodes in each dimension.
    //
    int adjustedSize[3];
    ComputeStrideAdjustedSize(sizes, adjustedSize);

    int size[3];
    size[0] = adjustedSize[0] + 1;
    size[1] = adjustedSize[1] + 1;
    size[2] = adjustedSize[2] + 1;

    //
    // Populate the coordinates.
    //
    vtkFloatArray *coords[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(size[i]);
        int i2 = i * 2;
        int i21 = i2 + 1;
        for (int j = 0 ; j < size[i] ; j++)
        {
            float t = float(j) / float(size[i] - 1);
            float c = (1.-t)*extents[i2] + t*extents[i21];
            coords[i]->SetComponent(j, 0, c);
        }
    }
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New(); 
    grid->SetDimensions(size);
    grid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    grid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    grid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    debug4 << mName << "end" << endl;

    return grid;
}

// ****************************************************************************
//  Method: avtBOWFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 30 11:27:47 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

typedef struct {size_t size; void *ptr; int count;} VTK_Memory;

void *vtk_bow_alloc(void *opaque, size_t size)
{
    void *retval = 0;

    VTK_Memory *m = (VTK_Memory *)opaque;
    if(m->size == size && ++m->count == 1)
    {
        retval = m->ptr;
    }
    else
    {
        long nLongs = (size / sizeof(long)) + (((size % sizeof(long)) > 0) ? 1 : 0);
        retval = (void *)(new long[nLongs]);
    }

    return retval;
}

void vtk_bow_free(void *opaque, void *ptr)
{
    VTK_Memory *m = (VTK_Memory *)opaque;
    if(ptr != m->ptr)
    {
        delete [] ((long *)ptr);
    }
}

vtkDataArray *
avtBOWFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    const char *mName = "avtBOWFileFormat::GetVar: ";
    vtkDataArray *retval = 0;

    debug4 << mName << "start" << endl;
    bool delogify = strcmp(varname, "delogify") == 0;

    //
    // Read the BOW bytes for the specified timestate and domain from
    // disk into the bow array.
    //
    int ts = cycles[timestate];
    debug4 << mName << "Time state["<< timestate << "] is really " << ts << endl;
    char *bow = GetBOWBytesForDomainAtTime(domain, ts, false);
    if(bow != 0)
    {
        //
        // Get the BOW info out of the data array.
        //
        VTK_Memory vtkmem;
        vtkmem.size = 0;
        vtkmem.ptr = 0;
        vtkmem.count = 0;
        bowglobal bg;

        bool directDecompress = true;
        bool dataStride = (header.stride[0] != 1 ||
                           header.stride[1] != 1 ||
                           header.stride[2] != 1);
        if(dataStride)
        {
            bg = bowglobal_create(my_bow_alloc, my_bow_free, 0);
            directDecompress = false;
        }
        else
        {
            bg = bowglobal_create(vtk_bow_alloc, vtk_bow_free,
                (void *)&vtkmem);
        }


        bowinfo binf = bow_getbowinfo(bg, bow);
        if(binf == 0)
        {
            debug4 << mName << "bow_getbowinfo returned 0!"
                   << endl;
        }
        else
        {
            size_t size = binf->xs[0] * binf->ys[0] * binf->zs[0];
            debug4 << mName << "Allocated " << size
                   << " floats in which to decompress bow data" << endl;
            vtkFloatArray *fscalars = vtkFloatArray::New();

            if(directDecompress)
            {
                debug4 << mName << "Setting up buffer for direct decompression"
                       << endl;

                fscalars->SetNumberOfTuples(size);
                vtkmem.size = size * sizeof(float);
                vtkmem.ptr = fscalars->GetVoidPointer(0);
            }

            //
            // Decompress the BOW into floats.
            //
            debug4 << mName << "Calling bow2bof" << endl;
            float *f = bow2bof(bg, (char *)bow, 0);

            if(directDecompress)
            {
                if(delogify)
                {
                    debug4 << mName << "Delogifying data" << endl;
                    float *src = (float *)fscalars->GetVoidPointer(0);
                    for(int i = 0; i < size; ++i)
                    {
                        float tmp = *src;
                        *src++ = DELOGIFY(tmp);
                    }
                }

                vtkmem.size = 0;
                vtkmem.ptr = 0;
            }
            else if(dataStride)
            {
                // Figure the size of the strided brick
                int domSize[3];
                domSize[0] = binf->xs[0];
                domSize[1] = binf->ys[0];
                domSize[2] = binf->zs[0];
                int strideSize[3];
                ComputeStrideAdjustedSize(domSize, strideSize);
                size = strideSize[0] * strideSize[1] * strideSize[2];
                fscalars->SetNumberOfTuples(size);

                debug4 << mName << "Strided data size: "
                       << strideSize[0] << ", "
                       << strideSize[1] << ", "
                       << strideSize[2] << endl;

                debug4 << mName << "Copying strided data into smaller brick."
                       << endl;
                float *dest = (float *)fscalars->GetVoidPointer(0);
                int nx = binf->xs[0];
                int nxy = nx * binf->ys[0];
                for(int k = 0; k < strideSize[2]; ++k)
                {
                    int zidx = k * header.stride[2];
                    for(int j = 0; j < strideSize[1]; ++j)
                    {
                        int yidx = j * header.stride[1];
                        for(int i = 0; i < strideSize[0]; ++i)
                        {
                            int xidx = i * header.stride[0];
                            *dest++ = f[zidx*nxy + yidx*nx + xidx];
                        }
                    }
                }

                delete [] f;

                if(delogify)
                {
                    debug4 << mName << "Delogifying data" << endl;
                    float *val = (float *)fscalars->GetVoidPointer(0);
                    for(int i = 0; i < size; ++i)
                    {
                        float tmp = *val;
                        *val++ = DELOGIFY(tmp);
                    }
                }
            }
            else
            {
                // We did not decompress directly into VTK memory so copy
                // the data into the VTK data array.
                fscalars->SetNumberOfTuples(size);

                float *src = f;
                float *dest = (float *)fscalars->GetVoidPointer(0);
                if(delogify)
                {
                    debug4 << mName << "Delogifying data" << endl;
                    for(int i = 0; i < size; ++i)
                    {
                        float tmp = *src++;
                        *dest++ = DELOGIFY(tmp);
                    }
                }
                else
                {
                    for(int i = 0; i < size; ++i)
                        *dest++ = *src++;
                }
                delete [] f;
            }

            retval = fscalars;
        }

        delete [] bow;
        bowglobal_destroy(bg);
    }
    else
    {
        debug4 << mName << "Could not get BOW bytes for var at domain="
               << domain << " timestate=" << timestate << endl;
    }

    debug4 << mName << "end" << endl;

    return retval;
}


// ****************************************************************************
//  Method: avtBOWFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 30 11:27:47 PDT 2006
//
// ****************************************************************************

vtkDataArray *
avtBOWFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}

//
//avtBOWFileFormat::HeaderData class
//


// ****************************************************************************
// Method: avtBOWFileFormat::HeaderData::HeaderData
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:15:27 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

avtBOWFileFormat::HeaderData::HeaderData()
{
    nx = ny = nx = 0;
    dx = dy = dx = 0;
    topdirmin = 0;
    topdirmod = 0;
    numblocks = 0;
    q = 0;
    filenamepat = 0;
    strcpy(varname, "var");
    delogify = false;

    stride[0] = stride[1] = stride[2] = 1;

    timeMethod = -1;
    m1_file = 0;
    m2_nStates = 0;
    m2_startCycle = 0;
    m2_deltaCycle = 0;
    m2_startTime = 0.;
    m2_deltaTime = 0.;
}

// ****************************************************************************
// Method: avtBOWFileFormat::HeaderData::~HeaderData
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:15:27 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

avtBOWFileFormat::HeaderData::~HeaderData()
{
    if(filenamepat != 0)
        delete [] filenamepat;

    if(m1_file != 0)
        delete [] m1_file;
}

// ****************************************************************************
// Method: avtBOWFileFormat::HeaderData::Read
//
// Purpose: 
//   Reads the WPP header information from a file.
//
// Arguments:
//   filename : The file from which to read the header information.
//
// Returns:    -1 on error.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 31 10:04:18 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

int
avtBOWFileFormat::HeaderData::Read(const char *filename)
{
    const char *mName = "";
    debug4 << mName << "reading " << filename << endl;

    ifstream ifile;
    ifile.open(filename);
    if(!ifile.is_open())
    {
        debug4 << mName << "could not read " << filename;
        return -1;
    }

    char line[1000 + 1];
    while (!ifile.eof())
    {
        ifile.getline(line, 1000);

        if(line[0] == '#')
            continue;
        else if(strncmp(line, "nx ", 3) == 0)
            nx = atoi(line+3);
        else if(strncmp(line, "ny ", 3) == 0)
            ny = atoi(line+3);
        else if(strncmp(line, "nz ", 3) == 0)
            nz = atoi(line+3);
        else if(strncmp(line, "dx ", 3) == 0)
            dx = atoi(line+3);
        else if(strncmp(line, "dy ", 3) == 0)
            dy = atoi(line+3);
        else if(strncmp(line, "dz ", 3) == 0)
            dz = atoi(line+3);
        else if(strncmp(line, "topdirmin ", 10) == 0)
            topdirmin = atoi(line+10);
        else if(strncmp(line, "topdirmod ", 10) == 0)
            topdirmod = atoi(line+10);
        else if(strncmp(line, "numblocks ", 9) == 0)
            numblocks = atoi(line+9);
        else if(strncmp(line, "q ", 2) == 0)
            q = atof(line+2);
        else if(strncmp(line, "filenamepat ", 12) == 0)
        {
            if(filenamepat != 0)
                delete [] filenamepat;
            filenamepat = new char[strlen(line+12) + 1];
            strcpy(filenamepat, line+12);
        }
        else if(strncmp(line, "varname ", 8) == 0)
        {
            int tmp;
            sscanf(line + 8, "%d %s", &tmp, varname);
        }
        else if(strncmp(line, "delogify ", 9) == 0)
        {
            delogify = true;
        }

        // These are VisIt-specific.
        else if(strncmp(line, "visit_timefile ", 15) == 0)
        {
            if(m1_file != 0)
                delete [] m1_file;
            m1_file = new char[strlen(line+15) + 1];
            strcpy(m1_file, line+15);
            timeMethod = 0;
        }
        else if(strncmp(line, "visit_times ", 12) == 0)
        {
            if(sscanf(line+12,"%d %d %d %lg %lg",
               &m2_nStates, &m2_startCycle, &m2_deltaCycle,
               &m2_startTime, &m2_deltaTime) == 5)
            {
                timeMethod = 1;
            }
            else
            {
                EXCEPTION1(InvalidFilesException, filename);
            }
        }
        else if(strncmp(line, "visit_stride ", 13) == 0)
        {
            if(sscanf(line+13,"%d %d %d",
               &stride[0], &stride[1], &stride[2]) != 3)
            {
                EXCEPTION1(InvalidFilesException, filename);
            }
        }
    }

    if(nx == 0 || ny == 0 || nz == 0)
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
    if(dx == 0 || dy == 0 || dz == 0)
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
    if(filenamepat == 0)
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
    if(numblocks == 0)
    {
        EXCEPTION1(InvalidFilesException, filename);
    }

    debug4 << "avtBOWFileFormat::HeaderData::Read: " << endl;
    debug4 << "\tnx = " << nx << endl;
    debug4 << "\tny = " << ny << endl;
    debug4 << "\tnz = " << nz << endl;
    debug4 << "\tdx = " << dx << endl;
    debug4 << "\tdy = " << dy << endl;
    debug4 << "\tdz = " << dz << endl;
    debug4 << "\ttopdirmin = " << topdirmin << endl;
    debug4 << "\ttopdirmod = " << topdirmod << endl;
    debug4 << "\tnumblocks = " << numblocks << endl;
    debug4 << "\tq = " << q << endl;
    debug4 << "\tfilenamepat = " << filenamepat << endl;
    debug4 << "\tvarname = " << varname << endl;
    debug4 << "\tdelogify = " << (delogify?"true":"false") << endl;

    debug4 << "\ttimeMethod = " << timeMethod << endl;
    debug4 << "\tm1_file = \"" << ((m1_file==0)?"null":m1_file) << "\"" << endl;
    debug4 << "\tm2_nStates = " << m2_nStates << endl;
    debug4 << "\tm2_startCycle = " << m2_startCycle << endl;
    debug4 << "\tm2_deltaCycle = " << m2_deltaCycle << endl;
    debug4 << "\tm2_startTime = " << m2_startTime << endl;
    debug4 << "\tm2_deltaTime = " << m2_deltaTime << endl;

    return 1;
}

