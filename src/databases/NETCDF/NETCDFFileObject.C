/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <NETCDFFileObject.h>
#include <netcdf.h>
#include <DebugStream.h>
#include <InvalidDBTypeException.h>

#define INVALID_FILE_HANDLE -1

//
// Define functions to free memory.
//

// ****************************************************************************
// Function: free_mem
//
// Purpose: 
//   Template function to free memory.
//
// Arguments:
//   ptr : Pointer to the memory to free.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 13:42:57 PST 2003
//
// Modifications:
//   
// ****************************************************************************

template <class T>
void free_mem(T *ptr)
{
    delete [] ptr;
}

// ****************************************************************************
// Function: free_void_mem
//
// Purpose: 
//   Function to free void* memory.
//
// Arguments:
//   ptr : The memory to free.
//   t   : The type of the memory.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 13:43:33 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
free_void_mem(void *ptr, TypeEnum t)
{
    char   *cptr = (char *)  ptr;
    unsigned char *ucptr = (unsigned char *)  ptr;
    short  *sptr = (short *)   ptr;
    int    *iptr = (int *)   ptr;
    float  *fptr = (float *) ptr;
    double *dptr = (double *)ptr;
    long   *lptr = (long *)  ptr;

    switch(t)
    {
    case CHARARRAY_TYPE:
        delete [] cptr;
        break;
    case UCHARARRAY_TYPE:
        delete [] ucptr;
        break;
    case SHORTARRAY_TYPE:
        delete [] sptr;
        break;
    case INTEGERARRAY_TYPE:
        delete [] iptr;
        break;
    case FLOATARRAY_TYPE:
        delete [] fptr;
        break;
    case DOUBLEARRAY_TYPE:
        delete [] dptr;
        break;
    case LONGARRAY_TYPE:
        delete [] lptr;
        break;
    default:
        break;
    }
}

// ****************************************************************************
// Method: NETCDFFileObject::NETCDFFileObject
//
// Purpose: 
//   Constructor for the NETCDFFileObject class.
//
// Arguments:
//   name : The name of the file to open.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:26:53 PST 2005
//
// Modifications:
//   
// ****************************************************************************

NETCDFFileObject::NETCDFFileObject(const char *name) : filename(name)
{
    ncid = INVALID_FILE_HANDLE;
}

// ****************************************************************************
// Method: NETCDFFileObject::~NETCDFFileObject
//
// Purpose: 
//   Destructor for the NETCDFFileObject class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:27:22 PST 2005
//
// Modifications:
//   
// ****************************************************************************

NETCDFFileObject::~NETCDFFileObject()
{
    Close();
}

// ****************************************************************************
// Method: NETCDFFileObject::IsOpen
//
// Purpose: 
//   Returns whether the file is currently open.
//
// Returns:    True if open; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:27:37 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
NETCDFFileObject::IsOpen() const
{
    return ncid != INVALID_FILE_HANDLE;
}

// ****************************************************************************
// Method: NETCDFFileObject::Open
//
// Purpose: 
//   Opens the file.
//
// Returns:    Returns true if successful; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:28:04 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool 
NETCDFFileObject::Open()
{
    const char *mName = "NETCDFFileObject::Open: ";
    int id = INVALID_FILE_HANDLE;
    int status;
    if((status = nc_open(filename.c_str(), NC_NOWRITE, &id)) == NC_NOERR)
    {
        ncid = id;
        debug4 << mName << filename.c_str() << " was opened." << endl;
    }
    else
    {
        debug4 << mName << "Could not open " << filename.c_str() << ": ";
        HandleError(status);
    }

    return ncid != INVALID_FILE_HANDLE;
}

// ****************************************************************************
// Method: NETCDFFileObject::Close
//
// Purpose: 
//   Closes the file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:28:35 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
NETCDFFileObject::Close()
{
    if(ncid != INVALID_FILE_HANDLE)
    {
        nc_close(ncid);
        ncid = INVALID_FILE_HANDLE;
    }
}

// ****************************************************************************
// Method: NETCDFFileObject::GetName
//
// Purpose: 
//   Gets the filename associated with the file.
//
// Returns:    The name of the file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:28:50 PST 2005
//
// Modifications:
//   
// ****************************************************************************

const std::string &
NETCDFFileObject::GetName() const
{
    return filename;
}

// ****************************************************************************
// Method: NETCDFFileObject::GetVarId
//
// Purpose: 
//   Looks up a variable's index in the list of variables.
//
// Arguments:
//   name  : The name of the variable to look for.
//   varid : The return value for variable id.
//
// Returns:    True if the variable was found; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:33:48 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
NETCDFFileObject::GetVarId(const char *name, int *varid)
{
    int status;
    if((status = nc_inq_varid(GetFileHandle(), name, varid)) != NC_NOERR)
        HandleError(status);

    return status == NC_NOERR;
}

// ****************************************************************************
// Method: NETCDFFileObject::ReadStringAttribute
//
// Purpose: 
//   Reads a string attribute for a variable.
//
// Arguments:
//   varname : The name of the variable for which to read the attribute.
//   attname : The name of the attribute to read.
//   attval  : The returned attribute value.
//
// Returns:    True if the attribute could be read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:29:10 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool 
NETCDFFileObject::ReadStringAttribute(const char *varname, 
    const char *attname, std::string &attval)
{
    const char *mName = "NETCDFFileObject::ReadStringAttribute: ";
    bool retval = false;
    TypeEnum t = NO_TYPE;
    int ndims = 0;
    int *dims = 0;
    void *value = 0;

    debug4 << mName << "(" << varname << ", " << attname << ")" << endl;
    if(ReadAttribute(varname, attname, &t, &ndims, &dims, &value))
    {
        if(t == CHARARRAY_TYPE && ndims == 1)
        {
            char *c = (char *)value;
            attval = std::string(c);
            retval = true;
        }
        else
            debug4 << mName << "t=" << int(t) << ", ndims=" << ndims << endl;
        delete [] dims;
        free_void_mem(value, t);
    }
    else
    {
        debug4 << mName << "(" << varname << ", "
               << attname << ") failed" << endl;
    }

    return retval;
}

// ****************************************************************************
// Method: NETCDFFileObject::ReadStringAttribute
//
// Purpose: 
//   Reads a global string attribute.
//
// Arguments:
//   attname : The name of the attribute to read.
//   attval  : The returned attribute value.
//
// Returns:    True if the attribute could be read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:29:10 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool 
NETCDFFileObject::ReadStringAttribute(const char *attname, std::string &attval)
{
    const char *mName = "NETCDFFileObject::ReadStringAttribute: ";
    bool retval = false;
    TypeEnum t = NO_TYPE;
    int ndims = 0;
    int *dims = 0;
    void *value = 0;

    debug4 << mName << "(" << attname << ")" << endl;
    if(ReadAttribute(attname, &t, &ndims, &dims, &value))
    {
        if(t == CHARARRAY_TYPE && ndims == 1)
        {
            char *c = (char *)value;
            attval = std::string(c);
            retval = true;
        }

        delete [] dims;
        free_void_mem(value, t);
    }

    return retval;
}

// ****************************************************************************
// Method: NETCDFFileObject::ReadAttribute
//
// Purpose: 
//   Read a global attribute.
//
// Arguments:
//   attname : The name of the attribute.
//   type    : The attribute type.
//   ndims   : The attribute's number of dimensions.
//   dims    : The attribute's dimensions.
//   value   : The attribute's value.
//
// Returns:    True if the attribute could be read; false otherwise.
//
// Note:       dims and value must be freed by the caller if this method
//             returns true. The memory pointed to by value should be freed
//             using the free_void_mem function, passing the type value.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:30:46 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
NETCDFFileObject::ReadAttribute(const char *attname, TypeEnum *type, int *ndims,
                       int **dims, void **value)
{
    return ReadAttribute(0, attname, type, ndims, dims, value);
}

// ****************************************************************************
// Method: NETCDFFileObject::ReadAttribute
//
// Purpose: 
//   Read a variable's attribute.
//
// Arguments:
//   varname : The name of the variable for which we're reading an attribute.
//   attname : The name of the attribute.
//   type    : The attribute type.
//   ndims   : The attribute's number of dimensions.
//   dims    : The attribute's dimensions.
//   value   : The attribute's value.
//
// Returns:    True if the attribute could be read; false otherwise.
//
// Note:       dims and value must be freed by the caller if this method
//             returns true. The memory pointed to by value should be freed
//             using the free_void_mem function, passing the type value.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:30:46 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
NETCDFFileObject::ReadAttribute(const char *varname, const char *attname,
    TypeEnum *type, int *ndims, int **dims, void **value)
{
    const char *mName = "NETCDFFileObject::ReadAttribute: ";
    int varid, status;
    int natts = 0;
    bool varvalid = false;

    if(varname == 0)
    {
        varid = NC_GLOBAL;
        if((status = nc_inq_natts(GetFileHandle(), &natts)) == NC_NOERR)
            varvalid = natts > 0;
        else
        {
            debug4 << mName << "0: ";
            HandleError(status);
        }
    }
    else
    {
        // Look up the variable name and gets its variable id.
        varvalid = GetVarId(varname, &varid);
    }

    if(varvalid)
    {
        nc_type atttype;
        size_t  attsize;
        if((status = nc_inq_att(GetFileHandle(), varid, attname, &atttype,
                                &attsize)) == NC_NOERR)
        {
            void *val = 0;
            TypeEnum t = NO_TYPE;

            // Try and read the attribute.
            if(atttype == NC_CHAR)
            {
                char *arr = new char[attsize+1];
                status = nc_get_att_text(GetFileHandle(), varid, attname, arr);
                if(status != NC_NOERR)
                    delete [] arr;
                else
                {
                    // Trim trailing spaces.
                    arr[attsize] = '\0';
                    char *c2 = arr + attsize - 1;
                    while(c2 >= arr && *c2 == ' ')
                        *c2-- = '\0';

                    val = (void*)arr;
                }
                t = CHARARRAY_TYPE;
            }
            else if(atttype == NC_INT)
            {
                int *arr = new int[attsize];
                status = nc_get_att_int(GetFileHandle(), varid, attname, arr);
                if(status != NC_NOERR)
                    delete [] arr;
                else
                    val = (void*)arr;
                t = INTEGERARRAY_TYPE;
            } 
            else if(atttype == NC_LONG)
            {
                long *arr = new long[attsize];
                status = nc_get_att_long(GetFileHandle(), varid, attname, arr);
                if(status != NC_NOERR)
                    delete [] arr;
                else
                    val = (void*)arr;
                t = LONGARRAY_TYPE;
            } 
            else if(atttype == NC_FLOAT)
            {
                float *arr = new float[attsize];
                status = nc_get_att_float(GetFileHandle(), varid, attname, arr);
                if(status != NC_NOERR)
                    delete [] arr;
                else
                    val = (void*)arr;
                t = FLOATARRAY_TYPE;
            } 
            else if(atttype == NC_DOUBLE)
            {
                double *arr = new double[attsize];
                status = nc_get_att_double(GetFileHandle(), varid, attname, arr);
                if(status != NC_NOERR)
                    delete [] arr;
                else
                    val = (void*)arr;
                t = DOUBLEARRAY_TYPE;
            } 

            if(status == NC_NOERR)
            {
                *type = t;
                *ndims = 1;
                int *d = new int[1];
                d[0] = attsize;
                *dims = d;
                *value = val;
            }
            else
            {
                *type = NO_TYPE;
                *ndims = 0;
                *dims = 0;
                *value = 0;
                debug4 << mName << "3: ";
                HandleError(status);
                varvalid = false;
            }
        }
        else
        {
            varvalid = false;
            debug4 << mName << "4: ";
            HandleError(status);
        }
    }

    return varvalid;
}

// ****************************************************************************
// Method: NETCDFFileObject::InqVariable
//
// Purpose: 
//   Determines the native type of a variable as well as its real dimensions.
//
// Arguments:
//   varname : The name of the variable for which we want information.
//   type    : Return value for the variable's type.
//   ndims   : Return value for the variable's number of dims.
//   dims    : Return value for the variable's dims, which are the actual sizes
//             and not the indices in the file's dims array.
//
// Returns:    True if the variable was found; false otherwise
//
// Note:       The dims array should be freed by the caller.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:34:57 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
NETCDFFileObject::InqVariable(const char *varname, TypeEnum *type, int *ndims,
    int **dims)
{
    int varid;
    bool retval;
    if((retval = GetVarId(varname, &varid)) == true)
    {
        char    tmp[NC_MAX_NAME+1];
        nc_type vartype;
        int  varndims;
        int  *vardims = new int[NC_MAX_VAR_DIMS];
        int  varnatts;
        int  status;
        if((status = nc_inq_var(GetFileHandle(), varid, tmp, &vartype, &varndims, 
                                vardims, &varnatts)) == NC_NOERR)
        {
            for(int i = 0; i < varndims; ++i)
            {
                size_t realSize;
                status = nc_inq_dimlen(GetFileHandle(), vardims[i], &realSize);
                if(status == NC_NOERR)
                    vardims[i] = realSize;
                else
                    HandleError(status);
            }
            if(vartype == NC_BYTE)
               *type = UCHARARRAY_TYPE;
            else if(vartype == NC_CHAR)
               *type = CHARARRAY_TYPE;
            else if(vartype == NC_SHORT)
               *type = SHORTARRAY_TYPE;
            else if(vartype == NC_INT)
               *type = INTEGERARRAY_TYPE;
            else if(vartype == NC_LONG)
               *type = LONGARRAY_TYPE;
            else if(vartype == NC_FLOAT)
               *type = FLOATARRAY_TYPE;
            else if(vartype == NC_DOUBLE)
               *type = DOUBLEARRAY_TYPE;

            *ndims = varndims;
            *dims = vardims;
        }
        else
        {
            delete [] vardims;
            *type = NO_TYPE;
            *ndims = 0;
            *dims = 0;
            HandleError(status);
            retval = false;
        }
    }
    else
    {
        *type = NO_TYPE;
        *ndims = 0;
        *dims = 0;
    }

    return retval;
}

// ****************************************************************************
// Method: NETCDFFileObject::ReadVariable
//
// Purpose: 
//   Reads the variable into an array that matches its native type.
//
// Arguments:
//   varname : The name of the variable that we want to read.
//   type    : Return value for the variable's type.
//   ndims   : Return value for the variable's number of dims.
//   dims    : Return value for the variable's dims, which are the actual sizes
//             and not the indices in the file's dims array.
//   values  : Return value for the variable's values.
//
// Returns:    True if the variable was found; false otherwise
//
// Note:       The dims array should be freed by the caller. The values array
//             should be freed by the caller using the free_void_mem function
//             using the type that gets returned from this method.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:34:57 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
NETCDFFileObject::ReadVariable(const char *varname, TypeEnum *type, int *ndims,
   int **dims, void **values)
{ 
    if(InqVariable(varname, type, ndims, dims))
    {
        int nElems = 1;
        int *d = *dims;
        for(int i = 0; i < *ndims; ++i)
            nElems *= d[i];

#define READVAR(T) \
        {\
            T *arr = new T[nElems];\
            if(ReadVariableInto(varname, *type, (void*)arr))\
                *values = (void *)arr;\
            else\
                delete [] arr; \
        }

        if(*type == CHARARRAY_TYPE || *type == UCHARARRAY_TYPE)
            READVAR(unsigned char)
        else if(*type == SHORTARRAY_TYPE)
            READVAR(short)
        else if(*type == INTEGERARRAY_TYPE)
            READVAR(int)
        else if(*type == LONGARRAY_TYPE)
            READVAR(long)
        else if(*type == FLOATARRAY_TYPE)
            READVAR(float)
        else if(*type == DOUBLEARRAY_TYPE)
            READVAR(double)
    }
    else
        *values = 0;

    return *values != 0;
}

// ****************************************************************************
// Method: NETCDFFileObject::ReadVariableInto
//
// Purpose: 
//   Reads the specified variable in its into a user-provided array.
//
// Arguments:
//   varname : The variable that we want to read.
//   t       : The representation that we want to use for the variable. The
//             NetCDF library performs conversions as necessary.
//   arr     : The data array into which the variable will be read.
//
// Returns:    True if the variable is read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:38:32 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
NETCDFFileObject::ReadVariableInto(const char *varname, TypeEnum t, void *arr)
{
    bool retval = false;
#define READVARINTO(T, FUNC) \
        {\
            int varId;\
            if(GetVarId(varname, &varId))\
            {\
                int status = FUNC(GetFileHandle(), varId, (T*)arr);\
                if(status == NC_NOERR)\
                    retval = true;\
                else\
                    HandleError(status);\
            }\
        }
    if(t == CHARARRAY_TYPE)
        READVARINTO(char, nc_get_var_text)
    else if(t == UCHARARRAY_TYPE)
        READVARINTO(unsigned char, nc_get_var_uchar)
    else if(t == SHORTARRAY_TYPE)
        READVARINTO(short, nc_get_var_short)
    else if(t == INTEGERARRAY_TYPE)
        READVARINTO(int, nc_get_var_int)
    else if(t == LONGARRAY_TYPE)
        READVARINTO(long, nc_get_var_long)
    else if(t == FLOATARRAY_TYPE)
        READVARINTO(float, nc_get_var_float)
    else if(t == DOUBLEARRAY_TYPE)
        READVARINTO(double, nc_get_var_double)

    return retval;
}

// ****************************************************************************
// Method: NETCDFFileObject::ReadVariableInto
//
// Purpose: 
//   Reads the specified variable in its into a user-provided array.
//
// Arguments:
//   varname : The variable that we want to read.
//   t       : The representation that we want to use for the variable. The
//             NetCDF library performs conversions as necessary.
//   starts  : starting node index on each axis of hyperslab to read
//   counts  : number of nodes on each axis of hyperslab to read
//   arr     : The data array into which the variable will be read.
//
// Returns:    True if the variable is read; false otherwise.
//
// Programmer: Mark C. Miller 
// Creation:   August 14, 2006 
//   
// ****************************************************************************

bool
NETCDFFileObject::ReadVariableInto(const char *varname, TypeEnum t,
    const int *const starts, const int *const counts, void *arr)
{
    int i, ndims, *dims = 0;
    TypeEnum type;
    if(!InqVariable(varname, &type, &ndims, &dims))
    {
        if (dims) delete [] dims;
        return false;
    }
    delete [] dims;

    size_t *startz = new size_t[ndims];
    size_t *countz = new size_t[ndims];
    debug4 << "Doing a partial read with nc_get_vara..." << endl;
    for (i = 0; i < ndims; i++)
    {
        startz[i] = (size_t) starts[i];
        countz[i] = (size_t) counts[i];
        debug4 << "   dimension " << i << ": start = " << startz[i]
               << ", count = " << countz[i] << endl;
    }

    bool retval = false;
#define READVARAINTO(T, FUNC) \
        {\
            int varId;\
            if(GetVarId(varname, &varId))\
            {\
                int status = FUNC(GetFileHandle(), varId,\
                                  startz, countz, (T*)arr);\
                if(status == NC_NOERR)\
                    retval = true;\
                else\
                    HandleError(status);\
            }\
        }
    if(t == CHARARRAY_TYPE)
        READVARAINTO(char, nc_get_vara_text)
    else if(t == UCHARARRAY_TYPE)
        READVARAINTO(unsigned char, nc_get_vara_uchar)
    else if(t == SHORTARRAY_TYPE)
        READVARAINTO(short, nc_get_vara_short)
    else if(t == INTEGERARRAY_TYPE)
        READVARAINTO(int, nc_get_vara_int)
    else if(t == LONGARRAY_TYPE)
        READVARAINTO(long, nc_get_vara_long)
    else if(t == FLOATARRAY_TYPE)
        READVARAINTO(float, nc_get_vara_float)
    else if(t == DOUBLEARRAY_TYPE)
        READVARAINTO(double, nc_get_vara_double)

    delete [] startz;
    delete [] countz;

    return retval;
}

// ****************************************************************************
// Method: NETCDFFileObject::ReadVariableIntoAsFloat
//
// Purpose: 
//   Reads the specified variable as a float.
//
// Arguments:
//   varname : The variable that we want to read.
//   fvalues : The float array that was allocated to contain the values.
//
// Returns:    True if the variable was read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:40:59 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
NETCDFFileObject::ReadVariableIntoAsFloat(const char *varname, float *fvalues)
{
    return ReadVariableInto(varname, FLOATARRAY_TYPE, (void *)fvalues);
}

// ****************************************************************************
// Method: NETCDFFileObject::AutoOpen
//
// Purpose: 
//   Automatically opens the file if it has not yet been opened.
//
// Returns:    True if the file was opened; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:41:45 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
NETCDFFileObject::AutoOpen()
{
    bool retval = true;
    if(ncid == INVALID_FILE_HANDLE)
    {
        debug4 << "NETCDFFileObject::AutoOpen: need to open " << filename.c_str()
               << ". Calling Open." << endl;
        retval = Open();
    }
    return retval;
}

// ****************************************************************************
// Method: NETCDFFileObject::GetFileHandle
//
// Purpose: 
//   Returns the netcdf handle to the file.
//
// Returns:    The netcdf handle to the file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:42:18 PST 2005
//
// Modifications:
//   
// ****************************************************************************

int
NETCDFFileObject::GetFileHandle()
{
    AutoOpen();
    return ncid;
}

// ****************************************************************************
// Method: NETCDFFileObject::HandleError
//
// Purpose: 
//   Prints an error message to the debug logs.
//
// Arguments:
//   status : The value returned from any netcdf function.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:42:43 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
NETCDFFileObject::HandleError(int status) const
{
    debug1 << nc_strerror(status) << endl;

    if(status == NC_ENOTNC)
    {
        EXCEPTION1(InvalidDBTypeException, filename.c_str());
    }
}

// ****************************************************************************
// Method: NETCDFFileObject::PrintFileContents
//
// Purpose: 
//   Prints the contents of the file to an ostream.
//
// Arguments:
//   os  : The stream to which the file contents will be written.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 19 13:43:21 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
NETCDFFileObject::PrintFileContents(ostream &os)
{
    const char *mName = "NETCDFFileObject::PrintFileContents: ";

    // Inquire about the number of dims, attributes, vars.
    int i, status, nDims, nVars, nGlobalAtts, unlimitedDimension;
    status = nc_inq(GetFileHandle(), &nDims, &nVars, &nGlobalAtts, &unlimitedDimension);
    if(status != NC_NOERR)
    {
        HandleError(status);
        return;
    }

    //
    // Print the dimensions to the debug logs.
    //    
    debug4 << mName << "nDims = " << nDims << endl;
    for(i = 0; i < nDims; ++i)
    {
        char   dimName[NC_MAX_NAME+1];
        size_t dimSize;
        if((status = nc_inq_dim(GetFileHandle(), i, dimName, &dimSize)) == NC_NOERR)
            debug4 << "\tname=" << dimName << ", size=" << dimSize << endl;
        else
            HandleError(status);
    }

    //
    // Print the attributes to the debug logs.
    //
    debug4 << mName << "nGlobalAtts = " << nGlobalAtts << endl;
    for(i = 0; i < nGlobalAtts; ++i)
    {
        char    attname[NC_MAX_NAME+1];
        nc_type atttype;
        size_t  attsize;
        if((status = nc_inq_attname(GetFileHandle(), NC_GLOBAL, i, attname))
            == NC_NOERR)
        {
            if((status = nc_inq_att(GetFileHandle(), NC_GLOBAL, attname, &atttype,
                                    &attsize)) == NC_NOERR)
            {
                debug4 << "\tname=" << attname;
                debug4 << ", type=";
                if(atttype == NC_BYTE)
                   debug4 << "NC_BYTE";
                else if(atttype == NC_CHAR)
                    debug4 << "NC_CHAR";
                else if(atttype == NC_SHORT)
                   debug4 << "NC_SHORT";
                else if(atttype == NC_INT)
                   debug4 << "NC_INT";
                else if(atttype == NC_FLOAT)
                   debug4 << "NC_FLOAT";
                else if(atttype == NC_DOUBLE)
                   debug4 << "NC_DOUBLE";
                debug4 << ", size=" << attsize;

#define PRINT_ATTR_VALUES(T, FUNC) \
                        {\
                            T *value = new T[attsize];\
                            FUNC(GetFileHandle(), i, attname, value);\
                            if(attsize > 1)\
                            {\
                                debug4 << ", value={";\
                                for(int k = 0; k < attsize; ++k)\
                                {\
                                    if(k > 0)\
                                        debug4 << ", ";\
                                    debug4 << value[k];\
                                }\
                                debug4 << "}";\
                            }\
                            else\
                                debug4 << ", value=" << value[0];\
                            delete [] value;\
                            debug4 << endl;\
                        }

                if(atttype == NC_CHAR)
                {
                    char *value = new char[attsize+1];
                    nc_get_att_text(GetFileHandle(), NC_GLOBAL, attname, value);
                    value[attsize] = '\0';
                    char *c2 = value + attsize - 1;
                    while(c2 >= value && *c2 == ' ')
                        *c2-- = '\0';
                    debug4 << ", value=\"" << value << "\"\n";
                    delete [] value;
                }
                else if(atttype == NC_INT)
                    PRINT_ATTR_VALUES(int, nc_get_att_int)
                else if(atttype == NC_FLOAT)
                    PRINT_ATTR_VALUES(float, nc_get_att_float)
                else if(atttype == NC_DOUBLE)
                    PRINT_ATTR_VALUES(double, nc_get_att_double)
            }
            else
                HandleError(status);
        }
        else
            HandleError(status);
    }

    //
    // Print the variables to the debug logs.
    //    
    debug4 << mName << "nVars = " << nVars << endl;
    for(i = 0; i < nVars; ++i)
    {
        char varname[NC_MAX_NAME+1];
        nc_type vartype;
        int  varndims;
        int  vardims[NC_MAX_VAR_DIMS];
        int  varnatts;
        if((status = nc_inq_var(GetFileHandle(), i, varname, &vartype, &varndims, 
                                vardims, &varnatts)) == NC_NOERR)
        {
            debug4 << "\tname=" << varname << endl;
            debug4 << "\t\ttype=";
            if(vartype == NC_BYTE)
               debug4 << "NC_BYTE";
            else if(vartype == NC_CHAR)
               debug4 << "NC_CHAR";
            else if(vartype == NC_SHORT)
               debug4 << "NC_SHORT";
            else if(vartype == NC_INT)
               debug4 << "NC_INT";
            else if(vartype == NC_FLOAT)
               debug4 << "NC_FLOAT";
            else if(vartype == NC_DOUBLE)
               debug4 << "NC_DOUBLE";
            debug4 << endl;
            debug4 << "\t\tndims=" << varndims << endl;
            debug4 << "\t\tdims = {";
            int j;
            for(j = 0; j < varndims; ++j)
            {
                char   dimName[NC_MAX_NAME+1];
                size_t dimSize;
                if(j > 0)
                    debug4 << ", ";
                if((status = nc_inq_dim(GetFileHandle(), vardims[j], dimName, &dimSize)) == NC_NOERR)
                {
                    debug4 << dimName << "=" << dimSize;
                }
                else
                {
                    HandleError(status);
                    debug4 << vardims[j];
                }
            }
            debug4 << "}" << endl;
            debug4 << "\t\tnatts=" << varnatts << endl;
            debug4 << "\t\tatts = {" << endl;
            for(j = 0; j < varnatts; ++j)
            {
                char    attname[NC_MAX_NAME+1];
                nc_type atttype;
                size_t  attsize;
                if((status = nc_inq_attname(GetFileHandle(), i, j, attname))
                    == NC_NOERR)
                {
                    if((status = nc_inq_att(GetFileHandle(), i, attname, &atttype,
                                            &attsize)) == NC_NOERR)
                    {
                        debug4 << "\t\t\tname=" << attname;
                        debug4 << ", type=";
                        if(atttype == NC_BYTE)
                           debug4 << "NC_BYTE";
                        else if(atttype == NC_CHAR)
                           debug4 << "NC_CHAR";
                        else if(atttype == NC_SHORT)
                           debug4 << "NC_SHORT";
                        else if(atttype == NC_INT)
                           debug4 << "NC_INT";
                        else if(atttype == NC_FLOAT)
                           debug4 << "NC_FLOAT";
                        else if(atttype == NC_DOUBLE)
                           debug4 << "NC_DOUBLE";
                        debug4 << ", size=" << attsize;

                        if(atttype == NC_CHAR)
                        {
                            char *value = new char[attsize+1];
                            nc_get_att_text(GetFileHandle(), i, attname, value);
                            value[attsize] = '\0';
                            char *c2 = value + attsize - 1;
                            while(c2 >= value && *c2 == ' ')
                                *c2-- = '\0';
                            debug4 << ", value=\"" << value << "\"\n";
                            delete [] value;
                        }
                        else if(atttype == NC_INT)
                            PRINT_ATTR_VALUES(int, nc_get_att_int)
                        else if(atttype == NC_FLOAT)
                            PRINT_ATTR_VALUES(float, nc_get_att_float)
                        else if(atttype == NC_DOUBLE)
                            PRINT_ATTR_VALUES(double, nc_get_att_double)
                    }
                    else
                        HandleError(status);
                }
                else
                    HandleError(status);
            }
            debug4 << "\t\t}" << endl;
        }
        else
            HandleError(status);
    }

    debug4 << mName << "unlimitedDimension = " << unlimitedDimension << endl;
}

