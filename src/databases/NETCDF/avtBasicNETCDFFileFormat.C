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

#include <vector>
#include <snprintf.h>
#include <netcdf.h>

#include <avtBasicNETCDFFileFormat.h>
#include <NETCDFFileObject.h>
#include <avtDatabaseMetaData.h>
#include <DebugStream.h>
#include <avtBasicNETCDFReader.h>

#include <avtMTSDFileFormatInterface.h>
#include <avtSTSDFileFormatInterface.h>

// ****************************************************************************
// Method: avtBasic_MTSD_NETCDFFileFormat::Identify
//
// Purpose: 
//   Identifies the file as having a time dimension.
//
// Arguments:
//   fileObject : The file object used to perform the query.
//
// Returns:    True if the file looks like MT, false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 12:12:45 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtBasic_MTSD_NETCDFFileFormat::Identify(NETCDFFileObject *fileObject)
{
    bool isMT = false;

    // Get the time dimension.
    int timedim = -1, time_nts = 0;
    std::string timedimname;
    if(avtNETCDFReaderBase::GetTimeDimension(fileObject, timedim, time_nts, timedimname))
        isMT = time_nts > 1;

    return isMT;
}

// ****************************************************************************
//  Method: CCSMCommonPluginInfo::SetupCCSMDatabase
//
//  Purpose:
//      Sets up a CCSM database.
//
//  Arguments:
//      list    A list of file names.
//      nList   The number of timesteps in list.
//      nBlocks The number of blocks in the list.
//
//  Returns:    A CCSM database from list.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 18:26:32 PST 2007
//
// ****************************************************************************

avtFileFormatInterface *
avtBasic_MTSD_NETCDFFileFormat::CreateInterface(NETCDFFileObject *f,
    const char *const *list, int nList, int nBlock)
{
    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
    {
        if(f != 0)
        {
            ffl[i] = new avtBasic_MTSD_NETCDFFileFormat(list[i], f);
            f = 0;
        }
        else
            ffl[i] = new avtBasic_MTSD_NETCDFFileFormat(list[i]);
    }
    return new avtMTSDFileFormatInterface(ffl, nList);
}

avtBasic_MTSD_NETCDFFileFormat::avtBasic_MTSD_NETCDFFileFormat(const char *filename) : 
    avtMTSDFileFormat(&filename, 1)
{
    reader = new avtBasicNETCDFReader(filename);
}

avtBasic_MTSD_NETCDFFileFormat::avtBasic_MTSD_NETCDFFileFormat(const char *filename, NETCDFFileObject *obj) : 
    avtMTSDFileFormat(&filename, 1)
{
    reader = new avtBasicNETCDFReader(filename, obj);
}

avtBasic_MTSD_NETCDFFileFormat::~avtBasic_MTSD_NETCDFFileFormat()
{
    delete reader;
}

void
avtBasic_MTSD_NETCDFFileFormat::GetCycles(std::vector<int> &cycles)
{
    reader->GetCycles(cycles);
}

void
avtBasic_MTSD_NETCDFFileFormat::GetTimes(std::vector<double> &times)
{
    reader->GetTimes(times);
}

int
avtBasic_MTSD_NETCDFFileFormat::GetNTimesteps(void)
{
    return reader->GetNTimesteps();
}

void
avtBasic_MTSD_NETCDFFileFormat::FreeUpResources(void)
{
    //reader->FreeUpResources();
}

vtkDataSet *
avtBasic_MTSD_NETCDFFileFormat::GetMesh(int timeState, const char *meshname)
{
    return reader->GetMesh(timeState, meshname);
}

vtkDataArray *
avtBasic_MTSD_NETCDFFileFormat::GetVar(int timeState, const char *varname)
{
    return reader->GetVar(timeState, varname);
}

void
avtBasic_MTSD_NETCDFFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    reader->PopulateDatabaseMetaData(timeState, md);
    if(md != 0)
         md->SetDatabaseComment(std::string("Read using Basic NETCDF MT reader\n") + md->GetDatabaseComment());
}

////////////////////////////////////////////////////////////////////////////////
///                            ST FILE FORMAT
////////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::CreateInterface
//
// Purpose: 
//   Creates the file format interface for this reader.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:03:39 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtFileFormatInterface *
avtBasic_STSD_NETCDFFileFormat::CreateInterface(NETCDFFileObject *f, 
    const char *const *list, int nList, int nBlock)
{
    avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[nList];
    int nTimestep = nList / nBlock;

    for (int i = 0 ; i < nTimestep ; i++)
    {
        ffl[i] = new avtSTSDFileFormat*[nBlock];
        for (int j = 0 ; j < nBlock ; j++)
        {
            if(f != 0)
            {
                ffl[i][j] = new avtBasic_STSD_NETCDFFileFormat(list[i*nBlock + j], f);
                f = 0;
            }
            else
                ffl[i][j] = new avtBasic_STSD_NETCDFFileFormat(list[i*nBlock + j]);
        }
    }

    return new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::avtBasic_STSD_NETCDFFileFormat
//
// Purpose: 
//   Constructor for the avtBasic_STSD_NETCDFFileFormat class.
//
// Arguments:
//   filename : The name of the file being read.
//   f        : The file object associated with the file being read.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:03:59 PST 2005
//
// Modifications:
//
// ****************************************************************************

avtBasic_STSD_NETCDFFileFormat::avtBasic_STSD_NETCDFFileFormat(const char *filename) :
    avtSTSDFileFormat(filename)
{
    reader = new avtBasicNETCDFReader(filename);
}

avtBasic_STSD_NETCDFFileFormat::avtBasic_STSD_NETCDFFileFormat(const char *filename,
    NETCDFFileObject *f) : avtSTSDFileFormat(filename)
{
    reader = new avtBasicNETCDFReader(filename, f);
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::~avtBasic_STSD_NETCDFFileFormat
//
// Purpose: 
//   Destructor for the avtBasic_STSD_NETCDFFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:04:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtBasic_STSD_NETCDFFileFormat::~avtBasic_STSD_NETCDFFileFormat()
{
    delete reader;
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::FreeUpResources
//
// Purpose: 
//   Frees up the resources.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:04:58 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtBasic_STSD_NETCDFFileFormat::FreeUpResources()
{
    reader->FreeUpResources();
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::GetCycleFromFilename
//
// Purpose: 
//   Make it guess the cycle from the filename.
//
// Returns:    The cycle.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 16 14:00:37 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
avtBasic_STSD_NETCDFFileFormat::GetCycleFromFilename(const char *f) const
{
    return GuessCycle(f);
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::GetCycle
//
// Purpose: 
//   Get the cycle from the file
//
// Returns:    The time
//
// Programmer: Eric Brugger
// Creation:   Fri Nov 13 16:33:08 PST 2009
//
// Modifications:
//   
// ****************************************************************************

int
avtBasic_STSD_NETCDFFileFormat::GetCycle()
{
    intVector cycles;
    reader->GetCycles(cycles);
    return (cycles.size() > 0) ? cycles[0] : avtFileFormat::INVALID_CYCLE;
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::GetTime
//
// Purpose: 
//   Get the time from the file
//
// Returns:    The time
//
// Programmer: Brad Whitlock
// Creation:   Tue May 16 14:00:37 PST 2006
//
// Modifications:
//   Eric Brugger, Fri Nov 13 16:33:08 PST 2009
//   I modified the routine to return INVALID_TIME instead of 0 if there
//   were no times in the file.
//   
// ****************************************************************************

double
avtBasic_STSD_NETCDFFileFormat::GetTime()
{
    doubleVector times;
    reader->GetTimes(times);
    return (times.size() > 0) ? times[0] : avtFileFormat::INVALID_TIME;
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::ActivateTimestep
//
// Purpose: 
//   Activates the time step.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:05:14 PST 2005
//
// Modifications:
//
// ****************************************************************************

void
avtBasic_STSD_NETCDFFileFormat::ActivateTimestep()
{
    debug4 << "avtBasic_STSD_NETCDFFileFormat::ActivateTimestep" << endl;
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::PopulateDatabaseMetaData
//
// Purpose: 
//   Populates the metadata from information in the file.
//
// Arguments:
//   md : The metadata object to populate.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:05:29 PST 2005
//
// Modifications:
//
// ****************************************************************************

void
avtBasic_STSD_NETCDFFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader->PopulateDatabaseMetaData(0, md);
    if(md != 0)
         md->SetDatabaseComment(std::string("Read using Basic NETCDF ST reader\n") + md->GetDatabaseComment());
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::GetMesh
//
// Purpose: 
//   Returns the specified mesh.
//
// Arguments:
//   var : The name of the mesh to create.
//
// Returns:    A vtkDataSet containing the mesh or 0.
// 
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:05:59 PST 2005
//
// Modifications:
// ****************************************************************************

vtkDataSet *
avtBasic_STSD_NETCDFFileFormat::GetMesh(const char *var)
{
    return reader->GetMesh(0, var);
}

// ****************************************************************************
// Method: avtBasic_STSD_NETCDFFileFormat::GetVar
//
// Purpose: 
//   Returns the data for the specified variable.
//
// Arguments:
//   var : The name of the variable to read.
//
// Returns:    The data or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:06:49 PST 2005
//
// Modifications:
//
// ****************************************************************************

vtkDataArray *
avtBasic_STSD_NETCDFFileFormat::GetVar(const char *var)
{
    return reader->GetVar(0, var);
}

