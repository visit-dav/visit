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
#include <avtCCSMFileFormat.h>
#include <NETCDFFileObject.h>
#include <avtCCSMReader.h>
#include <avtDatabaseMetaData.h>

#include <avtMTSDFileFormatInterface.h>
#include <avtSTSDFileFormatInterface.h>

// ****************************************************************************
// Method: avtCCSM_MTSD_FileFormat::Identify
//
// Purpose: 
//   Identifies the file as CCSM.
//
// Arguments:
//   fileObject : The file object used to perform the query.
//
// Returns:    True if the file looks like CCSM, false otherwise.
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
avtCCSM_MTSD_FileFormat::Identify(NETCDFFileObject *fileObject)
{
    // Make sure it's the right type of file.
    bool isCCSM = false;
    size_t sz;
    int tmp = 0;
    if(fileObject->GetDimensionInfo("time", &sz))
    {
        size_t latSize = 0, lonSize = 0;
        bool hasLongitude = fileObject->GetDimensionInfo("lon", &lonSize) ||
                            fileObject->GetDimensionInfo("longitude", &lonSize) ||
                            fileObject->GetDimensionInfo("lsmlon", &lonSize);
        bool hasLatitude = fileObject->GetDimensionInfo("lat", &latSize) ||
                           fileObject->GetDimensionInfo("latitude", &latSize) ||
                           fileObject->GetDimensionInfo("lsmlat", &latSize);

        isCCSM = (sz > 1 && 
                  (hasLongitude && lonSize > 1) &&
                  (hasLatitude && latSize > 1));
    }
    return isCCSM;
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
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 12:28:07 EST 2010
//    MTSD now accepts grouping multiple files into longer sequences, so
//    its interface has changed to accept both a number of timestep groups
//    and a number of blocks.
// ****************************************************************************

avtFileFormatInterface *
avtCCSM_MTSD_FileFormat::CreateInterface(NETCDFFileObject *f,
    const char *const *list, int nList, int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTSDFileFormat ***ffl = new avtMTSDFileFormat**[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
    {
        ffl[i] = new avtMTSDFileFormat*[nBlock];
        for (int j = 0 ; j < nBlock ; j++)
        {
            ffl[i][j] = new avtCCSM_MTSD_FileFormat(list[i*nBlock+j],
                                                    (i==0) ? f : NULL);
        }
    }

    return new avtMTSDFileFormatInterface(ffl, nTimestepGroups, nBlock);
}

avtCCSM_MTSD_FileFormat::avtCCSM_MTSD_FileFormat(const char *filename) : 
    avtMTSDFileFormat(&filename, 1)
{
    reader = new avtCCSMReader(filename);
}

avtCCSM_MTSD_FileFormat::avtCCSM_MTSD_FileFormat(const char *filename, NETCDFFileObject *obj) : 
    avtMTSDFileFormat(&filename, 1)
{
    reader = new avtCCSMReader(filename, obj);
}

avtCCSM_MTSD_FileFormat::~avtCCSM_MTSD_FileFormat()
{
    delete reader;
}

void
avtCCSM_MTSD_FileFormat::GetCycles(std::vector<int> &cycles)
{
    reader->GetCycles(cycles);
}

void
avtCCSM_MTSD_FileFormat::GetTimes(std::vector<double> &times)
{
    reader->GetTimes(times);
}

int
avtCCSM_MTSD_FileFormat::GetNTimesteps(void)
{
    return reader->GetNTimesteps();
}

void
avtCCSM_MTSD_FileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

vtkDataSet *
avtCCSM_MTSD_FileFormat::GetMesh(int timeState, const char *meshname)
{
    return reader->GetMesh(timeState, meshname);
}

vtkDataArray *
avtCCSM_MTSD_FileFormat::GetVar(int timeState, const char *varname)
{
    return reader->GetVar(timeState, varname);
}

void
avtCCSM_MTSD_FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    reader->PopulateDatabaseMetaData(timeState, md);
    if(md != 0)
         md->SetDatabaseComment("Read as CCSM MT data");
}
////////////////////////////////////////////////////////////////////////////////
///                            ST FILE FORMAT
////////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: avtCCSM_STSD_FileFormat::Identify
//
// Purpose: 
//   Identifies the file format as ST CCSM.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 27 14:33:43 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
avtCCSM_STSD_FileFormat::Identify(NETCDFFileObject *fileObject)
{
    // look for lat and lon dimensions.
    size_t latSize = 0, lonSize = 0;
    bool hasLongitude = fileObject->GetDimensionInfo("lon", &lonSize) ||
                        fileObject->GetDimensionInfo("longitude", &lonSize) ||
                        fileObject->GetDimensionInfo("lsmlon", &lonSize);
    bool hasLatitude = fileObject->GetDimensionInfo("lat", &latSize) ||
                       fileObject->GetDimensionInfo("latitude", &latSize) ||
                       fileObject->GetDimensionInfo("lsmlat", &latSize);
    bool isCCSM = (hasLongitude && lonSize > 1) &&
                  (hasLatitude && latSize > 1);
    
    return isCCSM;
}

// ****************************************************************************
// Method: avtCCSM_STSD_FileFormat::CreateInterface
//
// Purpose: 
//   Create an STSD file format interface for this file format.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 27 14:35:42 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

avtFileFormatInterface *
avtCCSM_STSD_FileFormat::CreateInterface(NETCDFFileObject *f, 
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
                ffl[i][j] = new avtCCSM_STSD_FileFormat(list[i*nBlock + j], f);
                f = 0;
            }
            else
                ffl[i][j] = new avtCCSM_STSD_FileFormat(list[i*nBlock + j]);
        }
    }

    return new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);
}

avtCCSM_STSD_FileFormat::avtCCSM_STSD_FileFormat(const char *filename) :
    avtSTSDFileFormat(filename)
{
    reader = new avtCCSMReader(filename);
}

avtCCSM_STSD_FileFormat::avtCCSM_STSD_FileFormat(const char *filename, 
    NETCDFFileObject *fileObject) : avtSTSDFileFormat(filename)
{
    reader = new avtCCSMReader(filename, fileObject);
}

avtCCSM_STSD_FileFormat::~avtCCSM_STSD_FileFormat()
{
    delete reader;
}

int
avtCCSM_STSD_FileFormat::GetCycle()
{
    intVector cycles;
    reader->GetCycles(cycles);
    return (cycles.size() > 0) ? cycles[0] : 0;
}

double
avtCCSM_STSD_FileFormat::GetTime()
{
    doubleVector times;
    reader->GetTimes(times);
    return (times.size() > 0) ? times[0] : 0;
}

void
avtCCSM_STSD_FileFormat::ActivateTimestep(void)
{
}

void
avtCCSM_STSD_FileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

vtkDataSet *
avtCCSM_STSD_FileFormat::GetMesh(const char *meshname)
{
    return reader->GetMesh(0, meshname);
}

vtkDataArray *
avtCCSM_STSD_FileFormat::GetVar(const char *varname)
{
    return reader->GetVar(0, varname);
}

int
avtCCSM_STSD_FileFormat::GetCycleFromFilename(const char *filename) const
{
    return 0; // figure out what to do here.
}

void
avtCCSM_STSD_FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader->PopulateDatabaseMetaData(0, md);
    if(md != 0)
         md->SetDatabaseComment("Read as CCSM ST data");
}
