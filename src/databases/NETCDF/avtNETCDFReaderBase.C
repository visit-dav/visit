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
#include <avtNETCDFReaderBase.h>

#include <cstring>

#include <netcdf.h>

#include <avtDatabaseMetaData.h>
#include <DebugStream.h>
#include <NETCDFFileObject.h>

// ****************************************************************************
//  Method: avtCCSM constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
// ****************************************************************************

avtNETCDFReaderBase::avtNETCDFReaderBase(const char *filename, NETCDFFileObject *f)
{
    fileObject = f;
}

avtNETCDFReaderBase::avtNETCDFReaderBase(const char *filename)
{
    fileObject = new NETCDFFileObject(filename);
}

// ****************************************************************************
// Method: avtNETCDFReaderBase::~avtNETCDFReaderBase
//
// Purpose: 
//   Destructor for the avtNETCDFReaderBase class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 12:17:36 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

avtNETCDFReaderBase::~avtNETCDFReaderBase()
{
    delete fileObject;
}

// ****************************************************************************
//  Method: avtNETCDFReaderBase::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
// ****************************************************************************

void
avtNETCDFReaderBase::FreeUpResources()
{
    debug4 << "avtNETCDFReaderBase::FreeUpResources" << endl;
    fileObject->Close();
}

// ****************************************************************************
// Method: avtNETCDFReaderBase::GetTimeDimension
//
// Purpose: 
//   Gets the ncdim, size, and name of the time axis.
//
// Arguments:
//   ncdim : The netcdf dimension index that identifies the dimension.
//   nts   : The size of the time dimension, or -1
//   name  : The name of the time dimension.
//
// Returns:    True if there is a time dimension; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 30 14:33:01 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
avtNETCDFReaderBase::GetTimeDimension(NETCDFFileObject *fileObject, int &ncdim, int &nts, std::string &name)
{
    const char *mName = "avtNETCDFReaderBase::GetTimeDimension: ";
    bool retval = false;

    ncdim = -1;
    nts = -1;
    name = "";

    int status, nDims, nVars, nGlobalAtts, unlimitedDimension;
    status = nc_inq(fileObject->GetFileHandle(), &nDims, &nVars, &nGlobalAtts,
                    &unlimitedDimension);
    // Check the dimension size.
    size_t sz = 0;
    char dimName[NC_MAX_NAME+1];
    if(status == NC_NOERR && unlimitedDimension >= 0)
    {
        ncdim = unlimitedDimension;
        debug4 << mName << "unlimitedDimension = " << unlimitedDimension << endl;

        if((status = nc_inq_dim(fileObject->GetFileHandle(), unlimitedDimension, dimName,
            &sz)) == NC_NOERR)
        {
            nts = sz;
            name = dimName;
            retval = true;
            debug4 << mName << "unlimitedDimension name=" << name << ", size=" << nts << endl;
        }
    }

    if(nts == -1)
    {
        debug4 << mName << "No unlimited dimension so look for suitable time dimensions" << endl;
        const char *timedims[] = {"time", "Time", "T"};
        for(int i = 0; i < 3; ++i)
        {
            if(fileObject->GetDimensionInfo(timedims[i], &sz))
            {
                // Look up which nc dimension this is too.
                for(int j = 0; j < nDims && ncdim == -1; ++j)
                {
                    if((status = nc_inq_dim(fileObject->GetFileHandle(), j, dimName,
                                            &sz)) == NC_NOERR)
                    {
                        if(strcmp(dimName, timedims[i]) == 0)
                            ncdim = j;
                    }
                }

                nts = sz;
                name = timedims[i];
                retval = true;
                debug4 << mName << timedims[i] << " dimension size=" << nts << endl;
                break;
            }
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
// ****************************************************************************

int
avtNETCDFReaderBase::GetNTimesteps(void)
{
    int ncdim = -1, nts = 1;
    std::string name;
    GetTimeDimension(fileObject, ncdim, nts, name);
    return nts;
}

void
avtNETCDFReaderBase::GetCycles(std::vector<int> &cycles)
{
    int nts = GetNTimesteps();
    for(int i = 0; i < nts; ++i)
        cycles.push_back(i);
}

void
avtNETCDFReaderBase::GetTimes(std::vector<double> &times)
{
    int ncdim = -1, nts = 1;
    std::string name;
    float *times_array = 0;
    if(GetTimeDimension(fileObject, ncdim, nts, name))
    {
        // Read the times from the DB...
        times_array = ReadArray(name.c_str());
    }

    if(times_array != 0)
    {
        for(int i = 0; i < nts; ++i)
            times.push_back((double)times_array[i]);
        delete [] times_array;
    }
    else 
    {
        for(int i = 0; i < nts; ++i)
            times.push_back((double)i);
    }
}

// ****************************************************************************
// Method: avtCCSMReader::ReadArray
//
// Purpose: 
//   Reads a variable into a float array.
//
// Arguments:
//   varname : The name of the variable to read.
//
// Returns:    A float array or NULL.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 12:18:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

float *
avtNETCDFReaderBase::ReadArray(const char *varname)
{
    const char *mName = "avtCCSMReader::ReadArray: ";
    TypeEnum t;
    int ndims, *dims = 0;
    float *arr = 0;

    if(fileObject->InqVariable(varname, &t, &ndims, &dims))
    {
        debug4 << mName << "Got information for " << varname << endl;

        // Determine the size of the variable.
        int nelems = 1;
        for(int i = 0; i < ndims; ++i)
            nelems *= dims[i];
        arr = new float[nelems];
        delete [] dims;

        // Read the variable.
        if(!fileObject->ReadVariableIntoAsFloat(varname, arr))
        {
            debug4 << mName << "Could not read variable as float." << endl;
            delete [] arr;
            arr = 0;
        }
        else
            debug4 << mName << "Variable " << varname << " was read." << endl;
    }

    return arr;
}
