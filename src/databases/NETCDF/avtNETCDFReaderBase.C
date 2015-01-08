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
#include <avtNETCDFReaderBase.h>

#include <cstring>

#include <netcdf.h>

#include <avtDatabaseMetaData.h>
#include <DebugStream.h>
#include <NETCDFFileObject.h>

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

// ****************************************************************************
//  Method: avtNETCDFReaderBase constructor
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
            nts = (int)sz;
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

                nts = (int)sz;
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
//  Method: avtNETCDFReaderBase::GetNTimesteps
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
    if (nts <= 1)
    {
        cycles.push_back(ReadCycleAttribute());
    }
    else
    {
        for(int i = 0; i < nts; ++i)
            cycles.push_back(i);
    }
}

void
avtNETCDFReaderBase::GetTimes(std::vector<double> &times)
{
    int ncdim = -1, nts = 1;
    std::string name;
    float *times_array = 0;
    if(GetTimeDimension(fileObject, ncdim, nts, name))
    {
        // Read the times from the DB.
        times_array = ReadArray(name.c_str());
    }
    else
    {
        // Read the time attribute.
        times_array = ReadTimeAttribute();
        if (times_array != 0)
            nts = 1;
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
// Method: avtNETCDFReaderBase::ReadTimeAttribute
//
// Purpose: 
//   Reads the global attribute Time into a float array.
//
// Returns:    A float array or NULL.
//
// Note:       
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 16 08:01:05 PST 2009
//
// Modifications:
//   
// ****************************************************************************

float *
avtNETCDFReaderBase::ReadTimeAttribute()
{
    float *times_array = 0;

    //
    // See if the global attribute time exists. If it does and is
    // either a float or double, then return the first value of it
    // as the time array.
    //
    int     status;
    nc_type atttype;
    size_t  attsize;
    if((status = nc_inq_att(fileObject->GetFileHandle(), NC_GLOBAL,
        "Time", &atttype, &attsize)) == NC_NOERR)
    {
        if (atttype == NC_FLOAT)
        {
            times_array = new float[1];
            float *value = new float[attsize];
            nc_get_att_float(fileObject->GetFileHandle(), NC_GLOBAL,
                "Time", value); 
            times_array[0] = value[0];
            delete [] value;
        }
        else if (atttype == NC_DOUBLE)
        {
            times_array = new float[1];
            double *value = new double[attsize];
            nc_get_att_double(fileObject->GetFileHandle(), NC_GLOBAL,
                "Time", value); 
            times_array[0] = value[0];
            delete [] value;
        }
    }

    return times_array;
}

// ****************************************************************************
// Method: avtNETCDFReaderBase::ReadCycleAttribute
//
// Purpose: 
//   Returns the global attribute Cycle as an int.
//
// Returns:    The global attribute Cycle.
//
// Note:       
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 16 08:01:05 PST 2009
//
// Modifications:
//   
// ****************************************************************************

int
avtNETCDFReaderBase::ReadCycleAttribute()
{
    int cycle = 0;

    //
    // See if the global attribute cycle exists. If it does and is
    // an int, then return the first value of it as the cycle.
    //
    int     status;
    nc_type atttype;
    size_t  attsize;
    if((status = nc_inq_att(fileObject->GetFileHandle(), NC_GLOBAL,
        "Cycle", &atttype, &attsize)) == NC_NOERR)
    {
        if (atttype == NC_INT)
        {
            int *value = new int[attsize];
            nc_get_att_int(fileObject->GetFileHandle(), NC_GLOBAL,
                "Cycle", value); 
            cycle = value[0];
            delete [] value;
        }
    }

    return cycle;
}

// ****************************************************************************
// Method: avtNETCDFReaderBase::ReadArray
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
    const char *mName = "avtNETCDFReaderBase::ReadArray: ";
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

// ****************************************************************************
// Method: avtNETCDFReaderBase::HandleMissingData
//
// Purpose: 
//   Look for attributes that will inform the metadata for this variable of
//   the missing data values and style.
//
// Arguments:
//   varname : The name of the variable whose attributes we'll check.
//   smd     : The scalar metadata whose missing data we're setting.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan  4 15:43:55 PST 2012
//
// Modifications:
//   
//   Hank Childs, Thu Jul 26 13:50:25 PDT 2012
//   Make missing data supercede range information.  Despite online 
//   documentation stating that range info should inform missing data, some
//   files have range info that should *not* be used for missing data.  By
//   superceding, the range info is ignored.
//
// ****************************************************************************

bool
avtNETCDFReaderBase::HandleMissingData(const std::string &varname, avtScalarMetaData *smd)
{
#define VALID_MIN    1
#define VALID_MAX    2
#define MISSING_DATA 4

    // note that the ordering is important ... the values array is overwritten
    // with each successive iteration ... missing_value trumps valid_min, for example.
    const char *fill_value[] = {"valid_range", "valid_min", "valid_max",
        "missing_value", "fill_value", "_FillValue", "_Fill_Value_"};

    const int operation[] = { VALID_MIN | VALID_MAX, VALID_MIN, VALID_MAX, MISSING_DATA, 
                              MISSING_DATA, MISSING_DATA, MISSING_DATA};
    int flags = 0;
    double values[2] = {0., 0.};
    for(int i = 0; i < 7; ++i)
    {
        double *missingData = NULL;
        int nMissingData = 0;
        TypeEnum t = NO_TYPE;
        if(fileObject->ReadAttributeAsDouble(varname.c_str(), fill_value[i],
            &t, &missingData, &nMissingData))
        {
            debug4 << "HandleMissingData for " << varname
                   << " attribute=" << fill_value[i] << " {";
            for(int j = 0; j < nMissingData; ++j)
                debug4 << " " << missingData[j];
            debug4 << "}" << endl;

            // If we have a missing_value with the following types then let's see
            // if we need to scale the data. If we scale the data then we'll scale
            // our missing data value too. NETCDF recommends checking the missing
            // value before the data are scaled but we don't do our missing data
            // checks until later in the generic db, which means we'll need to
            // scale the missing value too if it is to match the scaled data values.
            if(operation[i] == MISSING_DATA && 
               (t == CHARARRAY_TYPE || t == UCHARARRAY_TYPE || t == SHORTARRAY_TYPE))
            {
                double scale = 1., offset = 0.;
                if(ReadScaleAndOffset(varname, &t, &scale, &offset))
                {
                    debug4 << "Scaling missing data values" << endl;
                    for(vtkIdType j = 0; j < nMissingData; ++j)
                        missingData[j] = missingData[j] * scale + offset;
                }
            }

            if(operation[i] == VALID_MIN)
            {
                values[0] = missingData[0];
                flags |= VALID_MIN;
            }
            else if(operation[i] == VALID_MAX)
            {
                values[1] = missingData[0];
                flags |= VALID_MAX;
            }
            else if(operation[i] == (VALID_MIN | VALID_MAX))
            {
                values[0] = missingData[0];
                values[1] = missingData[1];
                flags |= (VALID_MIN | VALID_MAX);
            }
            else
            {
                values[0] = missingData[0];
                flags = MISSING_DATA;
            }
            delete [] missingData;
        }
    }

    double arr[2] = {0., 0.};
    if(flags == VALID_MIN)
    {
        arr[0] = values[0];
        smd->SetMissingData(arr);
        smd->SetMissingDataType(avtScalarMetaData::MissingData_Valid_Min);
    }
    else if(flags == VALID_MAX)
    {
        arr[0] = values[1];
        smd->SetMissingData(arr);
        smd->SetMissingDataType(avtScalarMetaData::MissingData_Valid_Max);
    }
    else if(flags == (VALID_MIN | VALID_MAX))
    {
        arr[0] = values[0];
        arr[1] = values[1];
        smd->SetMissingData(arr);
        smd->SetMissingDataType(avtScalarMetaData::MissingData_Valid_Range);
    }
    else if(flags > 0)
    {
        arr[0] = values[0];
        smd->SetMissingData(arr);
        smd->SetMissingDataType(avtScalarMetaData::MissingData_Value);
    }

    return (flags > 0);
}

// ****************************************************************************
// Method: avtNETCDFReaderBase::ReadScaleAndOffset
//
// Purpose: 
//   Read the scale and the offset (if there are any) for the variable.
//
// Arguments:
//   var : The variable name.
//   t   : The type of the attributes.
//   scale : The return scale.
//   offset : The return offset.
//
// Returns:    True on success and false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan  6 16:50:10 PST 2012
//
// Modifications:
//   
// ****************************************************************************

bool
avtNETCDFReaderBase::ReadScaleAndOffset(const std::string &var,
    TypeEnum *t, double *scale, double *offset)
{
    bool retval = false;   
 
    *t = NO_TYPE;
    *scale = 1.;
    *offset = 0.;

    // Let's see if there is a "scale_factor" attribute.
    double *scale_factor = NULL;
    int nscale_factor = 0;
    if(fileObject->ReadAttributeAsDouble(var.c_str(), "scale_factor", 
       t, &scale_factor, &nscale_factor))
    {
        *scale = scale_factor[0];
        delete [] scale_factor;

        // See if there is an offset to add.
        double *offset_array = NULL;
        int noffset_array = 0;
        TypeEnum t2 = NO_TYPE;
        if(fileObject->ReadAttributeAsDouble(var.c_str(), "add_offset", 
            &t2, &offset_array, &noffset_array))
        {
            *offset = offset_array[0];
            delete [] offset_array;
        }

        retval = true;
    }

    return retval;
}

// ****************************************************************************
// Method: avtNETCDFReaderBase::ApplyScalingAndOffset
//
// Purpose: 
//   Look for scale_factor and add_offset attributes on the data and apply them.
//   They are typically used to turn shorts into float or double.
//
// Arguments:
//   realvar : The variable.
//   arr     : The input data array to convert.
//
// Returns:    A new data array (or input with incremented refcount) that has
//             the data converted (if the attributes are found).
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan  6 11:34:44 PST 2012
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
avtNETCDFReaderBase::ApplyScaleAndOffset(const std::string &realvar,
    vtkDataArray *arr)
{
    vtkDataArray *retval = NULL;

    TypeEnum t = NO_TYPE;
    double scale = 1.;
    double offset = 0.;
    if(ReadScaleAndOffset(realvar, &t, &scale, &offset))
    {
        // Convert the data to its final form.
        debug5 << "Multiply " << realvar << " data by " << scale
               << " and add offset " << offset << endl;
        vtkDoubleArray *newarr = vtkDoubleArray::New();
        newarr->SetNumberOfTuples(arr->GetNumberOfTuples());
        for(vtkIdType id = 0; id < arr->GetNumberOfTuples(); ++id)
            newarr->SetTuple1(id, arr->GetTuple1(id) * scale + offset);
        retval = newarr;      
    }
    else
    {
        arr->Register(NULL);
        retval = arr;
    }

    return retval;
}
