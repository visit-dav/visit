/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                           avtExodusFileFormat.C                           //
// ************************************************************************* //

#include <avtExodusFileFormat.h>
#include <netcdf.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <vtkBitArray.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkCharArray.h>
#include <vtkCompositeDataIterator.h>
#include <vtkConfigure.h>
#include <vtkDataObject.h>
#include <vtkDoubleArray.h>
#include <vtkDataArray.h>
#include <vtkDataArrayTemplate.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#ifdef HAVE_VTK_SIZEOF___INT64
#include <vtk__Int64Array.h>
#include <vtkUnsigned__Int64Array.h>
#endif
#include <vtkUnsignedShortArray.h>
#include <vtkInformation.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtVariableCache.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <UnexpectedValueException.h>
#include <Utility.h> // CXX_strndup

#include <string.h>
#include <boost/cstdint.hpp>

#if defined(_WIN32)
#define STRNCASECMP _strnicmp
#else
#define STRNCASECMP strncasecmp
#endif

using     std::map;
using     std::sort;
using     std::string;
using     std::vector;
using     boost::int64_t;

static int VisItNCErr;
static map<string, int> messageCounts;
#define supressMessage "further warnings regarding this error will be supressed"
#define CheckNCError2(ERR, FN, THELINE, THEFILE)                                                \
    if (ERR != NC_NOERR)                                                                        \
    {                                                                                           \
        char msg[1024];                                                                         \
        SNPRINTF(msg, sizeof(msg), "Encountered netCDF error (%d, \"%s\") after call to \"%s\"" \
            " at line %d in file \"%s\"\n", ERR, nc_strerror(ERR), #FN, THELINE, THEFILE);      \
        if (messageCounts.find(msg) == messageCounts.end())                                     \
            messageCounts[msg] = 1;                                                             \
        else                                                                                    \
            messageCounts[msg]++;                                                               \
        if (messageCounts[msg] < 6)                                                             \
        {                                                                                       \
            if (!avtCallback::IssueWarning(msg))                                                \
                cerr << msg << endl;                                                            \
        }                                                                                       \
        else if (messageCounts[msg] == 6)                                                       \
        {                                                                                       \
            if (!avtCallback::IssueWarning(supressMessage))                                     \
                cerr << supressMessage << endl;                                                 \
        }                                                                                       \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        debug5 << "Made it past call to \"" << #FN << "\" at line "                             \
               << THELINE << " in file " << THEFILE << endl;                                    \
    }
#define CheckNCError(FN) CheckNCError2(VisItNCErr, FN, __LINE__, __FILE__)

vector< vector<string> >   *avtExodusFileFormat::globalFileLists = NULL;

static void FreeStringListFromExodusIINCvar(char **list)
{
    int i = 0;
    while (list[i])
        free(list[i++]);
    free(list);
}

static char **GetStringListFromExodusIINCvar(int exfid, char const *var_name)
{
    int ncerr;
    char **retval = (char**) malloc(1 * sizeof(char*));
    retval[0] = 0;

    int len_string_dimid;
    ncerr = nc_inq_dimid(exfid, "len_string", &len_string_dimid);
    if (ncerr != NC_NOERR) return retval;

    int len_name_dimid = -1;
    ncerr = nc_inq_dimid(exfid, "len_name", &len_name_dimid);
    if (ncerr != NC_NOERR) len_name_dimid = -1;

    int varid;
    ncerr = nc_inq_varid(exfid, var_name, &varid);
    if (ncerr != NC_NOERR) return retval;

    nc_type type;
    int ndims, dimids[NC_MAX_VAR_DIMS];
    ncerr = nc_inq_var(exfid, varid, 0, &type, &ndims, dimids, 0);
    if (ncerr != NC_NOERR) return retval;
    if (ndims != 2) return retval;

    size_t dlen0, dlen1;
    ncerr = nc_inq_dimlen(exfid, dimids[0], &dlen0);
    if (ncerr != NC_NOERR) return retval;
    ncerr = nc_inq_dimlen(exfid, dimids[1], &dlen1);
    if (ncerr != NC_NOERR) return retval;

    int nstrings;
    int len_string;
    if (dimids[0] == len_string_dimid || dimids[0] == len_name_dimid)
    {
        nstrings = (int) dlen1;
        len_string = (int) dlen0;
    }
    else if (dimids[1] == len_string_dimid || dimids[1] == len_name_dimid)
    {
        nstrings = (int) dlen0;
        len_string = (int) dlen1;
    }
    else
    {
        return retval;
    }

    char *buf = (char*) malloc(nstrings * len_string * sizeof(char));
    memset(buf, 0, nstrings * len_string);
    ncerr = nc_get_var_text(exfid, varid, buf);
    if (ncerr != NC_NOERR)
    {
        free(buf);
        return retval;
    }
    free(retval);

    int i;
    char *p;
    retval = (char**) malloc((nstrings + 1) * sizeof(char*));
    for (i = 0, p = buf; i < nstrings; i++, p += len_string)
        retval[i] = CXX_strndup(p,len_string);
    retval[i] = 0;
    free(buf);

    return retval;
}

static int AreSuccessiveStringsRelatedComponentNames(char const * const *list, int i,
    int *ncomps, char *compositeName)
{
    if (!list[i+1]) return 0;
    size_t maxlen = strlen(list[i]);
    while (maxlen)
    {
        if (!strncmp(list[i],list[i+1],maxlen)) break;
        maxlen--;
    }
    if (maxlen < 2) return 0;
    *ncomps = 1;
    while (list[i+1])
    {
        if (strncmp(list[i],list[i+1],maxlen)) break;
        i++;
        *ncomps = *ncomps + 1;
    }
    strncpy(compositeName, list[i], maxlen);
    compositeName[maxlen] = '\0';
    return 1;
}

static int SizeOfNCType(int type)
{
    switch (type)
    {
        case NC_BYTE:
        case NC_UBYTE:
        case NC_CHAR:
            return sizeof(char);
        case NC_SHORT:
        case NC_USHORT:
            return sizeof(short);
        case NC_INT:
        case NC_UINT:
            return sizeof(int);
        case NC_INT64:
        case NC_UINT64:
            return sizeof(int64_t);
        case NC_FLOAT:
            return sizeof(float);
        case NC_DOUBLE:
            return sizeof(double);
    }
    return 0;
}

static void
GetData(int exncfid, int ts, const char *visit_varname, int numBlocks, nc_type *rettype,
    int *retnum_comps, int *retnum_vals, void **retbuf)
{
    int ncerr;
    int i;
    avtCentering cent = AVT_NODECENT;
    void *buf;
    int num_vals;
    int num_comps;
    nc_type type;
    size_t len = strlen(visit_varname);
    bool isCoord = !strncmp(visit_varname, "coord", 5);
    int exvaridx = -1;

    if (isCoord)
    {
        if (!strncmp(visit_varname, "coordx", 6) ||
            !strncmp(visit_varname, "coordy", 6) ||
            !strncmp(visit_varname, "coordz", 6))
            num_comps = 1;
        else
        {
            int num_dim_dimid;
            size_t dlen;
            exvaridx = 0;
            nc_inq_dimid(exncfid, "num_dim", &num_dim_dimid);
            nc_inq_dimlen(exncfid, num_dim_dimid, &dlen);
            num_comps = (int) dlen;
        }
    }
    else
    {
        // First, search for this variable's name in node_var_names
        char **node_var_names = GetStringListFromExodusIINCvar(exncfid, "name_nod_var");
        i = 0;
        num_comps = 0;
        while (node_var_names[i])
        {
            if (!strncmp(visit_varname, node_var_names[i], len))
            {
                if (exvaridx == -1)
                {
                    exvaridx = i;
                    num_comps = 1;
                }
                else
                {
                    num_comps++;
                }
            }
            i++;
        }
        FreeStringListFromExodusIINCvar(node_var_names);
        if (exvaridx < 0)
        {
           // Now, try looking in elem var names 
            cent = AVT_ZONECENT;
            char **elem_var_names = GetStringListFromExodusIINCvar(exncfid, "name_elem_var");
            i = 0;
            while (elem_var_names[i])
            {
                if (!strncmp(visit_varname, elem_var_names[i], len))
                {
                    if (exvaridx < 0)
                    {
                        exvaridx = i;
                        num_comps = 1;
                    }
                    else
                    {
                        num_comps++;
                    }
                }
                i++;
            }
            FreeStringListFromExodusIINCvar(elem_var_names);
        }
        if (exvaridx < 0)
            EXCEPTION1(InvalidVariableException, visit_varname);
    }

    if (cent == AVT_NODECENT)
    {
        // Nodal variable data is either in an nc array of 3 dimensions,
        // time_step, num_nod_var, num_nodes named "vals_nod_var" or in
        // individual nc arrays of 2 dimensions, vals_nod_varK (where K
        // is the variables index in name_nod_var) time_step, num_nodes

        int varid;
        char ex_var_name[NC_MAX_NAME];
        int ndims, dimids[NC_MAX_VAR_DIMS], num_nodes;
        size_t starts[3], counts[3], dlen;

        // First, we'll attempt to find the second form of storage 
        SNPRINTF(ex_var_name, sizeof(ex_var_name), "vals_nod_var%d", exvaridx+1);
        ncerr = nc_inq_varid(exncfid, ex_var_name, &varid);
        if (ncerr == NC_NOERR && !isCoord)
        {
            ncerr = nc_inq_var(exncfid, varid, 0, &type, &ndims, dimids, 0);
            if (ncerr != NC_NOERR)
                EXCEPTION1(InvalidVariableException, visit_varname);
            nc_inq_dimlen(exncfid, dimids[1], &dlen);
            num_nodes = (int) dlen;
            num_vals = num_nodes;
            starts[0] = ts;       counts[0] = 1;
            starts[1] = 0;        counts[1] = num_nodes;
            buf = (void*) malloc(num_comps * num_nodes * SizeOfNCType(type));
            for (i = 0; i < num_comps; i++)
            {
                SNPRINTF(ex_var_name, sizeof(ex_var_name), "vals_nod_var%d", exvaridx+1+i);
                nc_inq_varid(exncfid, ex_var_name, &varid);
                char *p = (char *) buf + i * num_nodes * SizeOfNCType(type);
                ncerr = nc_get_vara(exncfid, varid, starts, counts, p);
                CheckNCError2(ncerr, nc_get_vara, __LINE__, __FILE__)      
                if (ncerr != NC_NOERR)
                {
                    free(buf);
                    EXCEPTION1(InvalidVariableException, visit_varname);
                }
            }
        }
        else
        {
            ncerr = nc_inq_varid(exncfid, isCoord?visit_varname:"vals_nod_var", &varid);
            if (ncerr != NC_NOERR)
                EXCEPTION1(InvalidVariableException, visit_varname);
            nc_inq_var(exncfid, varid, 0, &type, &ndims, dimids, 0);
            int dimidx_for_len = 2;
            if (isCoord)
            {
                if (ndims == 1) dimidx_for_len = 0;
                else dimidx_for_len = 1;
            }
            nc_inq_dimlen(exncfid, dimids[dimidx_for_len], &dlen);
            num_nodes = (int) dlen;
            num_vals = num_nodes;
            if (isCoord)
            {
                if (num_comps == 1)
                {
                    starts[0] = 0;       counts[0] = num_nodes;
                }
                else
                {
                    starts[0] = 0;       counts[0] = num_comps;
                    starts[1] = 0;       counts[1] = num_nodes;
                }
            }
            else
            {
                starts[0] = ts;       counts[0] = 1;
                starts[1] = exvaridx; counts[1] = num_comps;
                starts[2] = 0;        counts[2] = num_nodes;
            }
            buf = (void*) malloc(num_comps * num_nodes * SizeOfNCType(type));
            ncerr = nc_get_vara(exncfid, varid, starts, counts, buf);
            CheckNCError2(ncerr, nc_get_vara, __LINE__, __FILE__)      
            if (ncerr != NC_NOERR)
            {
                free(buf);
                EXCEPTION1(InvalidVariableException, visit_varname);
            }
        }
    }
    else
    {
        // Element variable data is in multiple nc arrays of 2 dimensions,
        // time_step, num_el_in_blkK, for each element block as in...
        // double vals_elem_var1eb1(time_step, num_el_in_blk1) ;
        // double vals_elem_var2eb1(time_step, num_el_in_blk1) ;
        int num_elem = 0;
        buf = 0;
        for (int pass = 0; pass < 2; pass++)
        {
            int ndims, dimids[NC_MAX_VAR_DIMS];
            size_t dlen;
            char *p = (char *) buf; // p not relevant on first pass
            for (int i = 0; i < numBlocks; i++)
            {
                int elem_varid;
                char elem_varname[NC_MAX_NAME+1];
//warning THIS LOGIC DOESNT WORK FOR ZONAL VARS WITH MULTIPLE COMPONENTS
                SNPRINTF(elem_varname, sizeof(elem_varname), "vals_elem_var%deb%d", exvaridx+1, i+1);
                ncerr = nc_inq_varid(exncfid, elem_varname, &elem_varid);
                if (ncerr != NC_NOERR) continue;

                ncerr = nc_inq_var(exncfid, elem_varid, 0, &type, &ndims, dimids, 0);
                nc_inq_dimlen(exncfid, dimids[1], &dlen);
                num_elem += (int) dlen;

                if (pass == 1)
                {
                    size_t starts[2] = {ts, 0};
                    size_t counts[2] = {1, dlen};
                    ncerr = nc_get_vara(exncfid, elem_varid, starts, counts, p);
                    CheckNCError2(ncerr, nc_get_vara, __LINE__, __FILE__)      
                    if (ncerr != NC_NOERR)
                    {
                        free(buf);
                        EXCEPTION1(InvalidVariableException, visit_varname);
                    }
                    p += (((int) dlen) * SizeOfNCType(type));
                }

            }

            if (pass == 0)
            {
                if (num_elem == 0)
                    EXCEPTION1(InvalidVariableException, visit_varname);
                num_vals = num_elem;
                buf = (void*) malloc(num_comps * num_elem * SizeOfNCType(type));
            }
        }
    }
    *rettype = type;
    *retnum_comps = num_comps;
    *retnum_vals = num_vals;
    *retbuf = buf;
}

template <class T>
static T* Interleave(T const * ibuf, int ncomps, int nvals)
{
    T* obuf = (T*) malloc(ncomps * nvals * sizeof(T));
    for (int v = 0; v < nvals; v++)
    {
        for (int c = 0; c < ncomps; c++)
            obuf[v*ncomps+c] = ibuf[c*nvals+v];
    }
    return obuf;
}

static vtkDataArray *
MakeVTKDataArrayByTakingOwnershipOfNCVarData(nc_type type,
    int num_comps, int num_vals, void *buf)
{
    const int SAVE_ARRAY = 0;
    const int VTK_DA_FREE = vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE;

    switch (type)
    {
        case NC_BYTE:
        case NC_UBYTE:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<unsigned char>((unsigned char*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtkUnsignedCharArray *arr = vtkUnsignedCharArray::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((unsigned char*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
        case NC_CHAR:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<char>((char*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtkCharArray *arr = vtkCharArray::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((char*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
        case NC_SHORT:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<short>((short*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtkShortArray *arr = vtkShortArray::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((short*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
        case NC_USHORT:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<unsigned short>((unsigned short*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtkUnsignedShortArray *arr = vtkUnsignedShortArray::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((unsigned short*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
        case NC_INT:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<int>((int*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtkIntArray *arr = vtkIntArray::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((int*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
        case NC_UINT:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<unsigned int>((unsigned int*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtkUnsignedIntArray *arr = vtkUnsignedIntArray::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((unsigned int*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
#ifdef HAVE_VTK_SIZEOF___INT64
        case NC_INT64:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<int64_t>((int64_t*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtk__Int64Array *arr = vtk__Int64Array::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((int64_t*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
        case NC_UINT64:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<unsigned int64_t>((unsigned int64_t*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtkUnsigned__Int64Array *arr = vtkUnsigned__Int64Array::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((unsigned int64_t*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
#endif
        case NC_FLOAT:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<float>((float*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtkFloatArray *arr = vtkFloatArray::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((float*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
        case NC_DOUBLE:
        {
            if (num_comps > 1)
            {
                void *newbuf = (void*) Interleave<double>((double*) buf, num_comps, num_vals);
                free(buf);
                buf = newbuf;
            }
            vtkDoubleArray *arr = vtkDoubleArray::New();
            arr->SetNumberOfComponents(num_comps);
            arr->SetArray((double*)buf, num_comps * num_vals, SAVE_ARRAY, VTK_DA_FREE);
            return arr;
        }
    }
    return NULL;
}


static void
GetExodusSetIDs(int exncfid, string stype, vector<int>& retval)
{
    int ncerr;
    int num_set = 0;

    if (stype != "elblk")
    {
        int num_set_dimid;
        ncerr = nc_inq_dimid(exncfid, string("num_" + stype + "_sets").c_str(), &num_set_dimid);
        if (ncerr != NC_NOERR) return;

        size_t dlen = 0;
        ncerr = nc_inq_dimlen(exncfid, num_set_dimid, &dlen);
        if (ncerr != NC_NOERR) return;
        num_set = (int) dlen;
        if (!num_set) return;
    }

    // Populate retval with default ids (1...num_sets)
    retval.resize(num_set);
    for (int i = 1; i <= num_set; i++)
        retval[i-1] = i;

    string pnm = "ns";
    if (stype == "side") pnm = "ss";
    else if (stype == "elblk") pnm = "eb";
    int prop1_varid;
    ncerr = nc_inq_varid(exncfid, string(pnm + "_prop1").c_str(), &prop1_varid);
    if (ncerr != NC_NOERR) return;

    nc_type type;
    int ndims, dimids[NC_MAX_VAR_DIMS], natts;
    ncerr = nc_inq_var(exncfid, prop1_varid, 0, &type, &ndims, dimids, &natts);
    if (ncerr != NC_NOERR) return;
//warning WHAT ABOUT 64-BIT INTEGER TYPE
    if (type != NC_INT || ndims != 1 || natts != 1) return;
    ncerr = nc_inq_att(exncfid, prop1_varid, "name", 0, 0);
    if (ncerr != NC_NOERR) return;

    // Read integer data directly into STL vector obj's buffer
    ncerr = nc_get_var_int(exncfid, prop1_varid, &retval[0]);
}

static vtkBitArray*
GetExodusSetsVar(int exncfid, int ts, char const *var, int numNodes, int numElems)
{
    int ncerr;
    int num_set = 0;
    string stype = string(var)=="Nodesets"?"node":"side";
    vtkBitArray *retval = 0;

    int num_set_dimid;
    ncerr = nc_inq_dimid(exncfid, string("num_" + stype + "_sets").c_str(), &num_set_dimid);
    if (ncerr != NC_NOERR) return retval;

    size_t dlen = 0;
    nc_inq_dimlen(exncfid, num_set_dimid, &dlen);
    num_set = (int) dlen;
    if (!num_set) return 0;

    const int bpuc = sizeof(unsigned char)*8;
    retval = vtkBitArray::New();
    retval->SetNumberOfComponents(((num_set+bpuc-1)/bpuc)*bpuc);
    //retval->SetNumberOfComponents(num_set);
    retval->SetNumberOfTuples(stype=="node"?numNodes:numElems);
    memset(retval->GetVoidPointer(0), 0, retval->GetSize()/bpuc);

    string pnm = "ns";
    if (stype == "side") pnm = "ss";
    else if (stype == "elblk") pnm = "eb";
    int status_varid;
    ncerr = nc_inq_varid(exncfid, string(pnm + "_status").c_str(), &status_varid);
    if (ncerr != NC_NOERR) return retval;
    nc_type type;
    int ndims;
    ncerr = nc_inq_var(exncfid, status_varid, 0, &type, &ndims, 0, 0);
    if (ncerr != NC_NOERR) return retval;
//warning WHAT ABOUT 64-BIT INTEGER TYPE
    if (type != NC_INT || ndims != 1) return retval;

    int *status = new int[num_set * sizeof(int)]; 
    ncerr = nc_get_var_int(exncfid, status_varid, status);
    if (ncerr != NC_NOERR)
    {
        delete [] status;
        return retval;
    }

    for (int i = 0; i < num_set; i++)
    {
        if (!status[i]) continue;

        char tmp[32];
        if (stype == "node")
            SNPRINTF(tmp, sizeof(tmp), "node_ns%d", i+1);
        else
            SNPRINTF(tmp, sizeof(tmp), "elem_ss%d", i+1);
 
        int setlist_varid;
        ncerr = nc_inq_varid(exncfid, tmp, &setlist_varid);
        if (ncerr != NC_NOERR) continue;

        int dimids[NC_MAX_VAR_DIMS];
        ncerr = nc_inq_var(exncfid, setlist_varid, 0, &type, &ndims, dimids, 0);
        if (ncerr != NC_NOERR) continue;
//warning WHAT ABOUT 64-BIT INTEGER TYPE
        if (type != NC_INT || ndims != 1) continue;

        size_t dlen;
        ncerr = nc_inq_dimlen(exncfid, dimids[0], &dlen);
        if (ncerr != NC_NOERR) continue;
        if (!dlen) continue;

        int *setids = new int[dlen];
        ncerr = nc_get_var_int(exncfid, setlist_varid, setids);
        if (ncerr != NC_NOERR)
        {
            delete [] setids;
            continue;
        }

        for (int j = 0; j < (int) dlen; j++)
            retval->SetComponent(setids[j]-1, i, 1);

        delete [] setids;
    }

    delete [] status;

    return retval;
}

template <class T>
static void ReadBlockIds(int fid, int vid, int numBlocks, vector<int>& blockId, int (*NcRdFunc)(int,int,int*))
{
    int ncerr;
    T *buf = new T [numBlocks];
    ncerr = NcRdFunc(fid, vid, buf);
    CheckNCError2(ncerr, nc_get_var, __LINE__, __FILE__);
    for (int k = 0; k < numBlocks; k++)
    {
        if (ncerr == NC_NOERR)
            blockId[k] = (int) buf[k];
        else
            blockId[k] = k;
    }
    delete [] buf;
}
#define READ_BLOCK_IDS(F,V,N,BIDS,TYPE) ReadBlockIds<TYPE>(F,V,N,BIDS,nc_get_var_ ## TYPE);

static void
GetElementBlockNamesAndIds(int ncExIIId, int numBlocks,
    vector<string>& blockName, vector<int>& blockId)
{
    int ncerr;

    blockId.clear();
    blockId.resize(numBlocks);
    for (int i = 0; i < numBlocks; i++) blockId[i] = i;
    blockName.clear();
    blockName.resize(numBlocks);
    char **element_block_names = GetStringListFromExodusIINCvar(ncExIIId, "eb_names");
    if (element_block_names)
    {
        int i = 0;
        while (element_block_names[i])
        {
            blockName[i] = string(element_block_names[i]);
            i++;
        }
        FreeStringListFromExodusIINCvar(element_block_names);
    }

    int num_vars = 0;
    ncerr = nc_inq(ncExIIId, 0, &num_vars, 0, 0);
    if (ncerr != NC_NOERR) return;

    int eb_blockid_varid = -1;
    for (int i = 0; i < num_vars && eb_blockid_varid == -1; i++)
    {
        char tmpvname[NC_MAX_NAME+1];
        ncerr = nc_inq_varname(ncExIIId, i, tmpvname);
        if (ncerr != NC_NOERR) continue;
        if (strncmp(tmpvname, "eb_prop", 7)) continue;

        int natts = 0;
        ncerr = nc_inq_varnatts(ncExIIId, i, &natts);
        if (ncerr != NC_NOERR) continue; // loop over vars
        for (int j = 0; j < natts; j++)
        {
            char tmpattname[NC_MAX_NAME+1];
            ncerr = nc_inq_attname(ncExIIId, i, j, tmpattname);
            if (ncerr != NC_NOERR) continue; // loop over atts
            if (strncmp(tmpattname, "name", 4)) continue;
            eb_blockid_varid = i;
            break;
        }
    }
    if (eb_blockid_varid == -1)
    {
       return;
    }

    nc_type vtype;
    nc_inq_vartype(ncExIIId, eb_blockid_varid, &vtype);
    switch (vtype)
    {
        case NC_INT:   READ_BLOCK_IDS(ncExIIId, eb_blockid_varid, numBlocks, blockId, int); break;
//        case NC_INT64: READ_BLOCK_IDS(ncExIIId, eb_blockid_varid, numBlocks, blockId, int64_t); break;
    }
}

// ****************************************************************************
//  Method: avtExodusFileFormat::RegisterFileList
//
//  Purpose:
//      Registers a file list with this format.  The file list is the "real"
//      list of file names in the Nemesis file.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2004
//
// ****************************************************************************

int
avtExodusFileFormat::RegisterFileList(const char *const *list, int nlist)
{
    if (globalFileLists == NULL)
    {
        globalFileLists = new vector< vector<string> >;
    }

    vector<string> thisList(nlist);
    for (int i = 0 ; i < nlist ; i++)
        thisList[i] = list[i];

    globalFileLists->push_back(thisList);

    return globalFileLists->size()-1;
}

// ****************************************************************************
//  Method: avtExodusFileFormat constructor
//
//  Arguments:
//      name    The name of the exodus file.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Defer reading in the file.
//
//    Hank Childs, Thu Jul 22 14:28:10 PDT 2004
//    Add support for registering file lists.
//
//    Hank Childs, Mon Aug 23 13:41:54 PDT 2004
//    Rename cache object to avoid namespace conflict with base class.
//
// ****************************************************************************

avtExodusFileFormat::avtExodusFileFormat(const char *name)
   : avtMTSDFileFormat(&name, 1)
{
    fileList = -1;
    ncExIIId = -1;
    numBlocks = -1;
    numNodes = -1;
    numElems = -1;
}

// ****************************************************************************
//  Method: avtExodusFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 23 13:41:54 PDT 2004
//    Rename cache object to avoid namespace conflict with base class.
//
// ****************************************************************************

avtExodusFileFormat::~avtExodusFileFormat()
{
    FreeUpResources();
}

int
avtExodusFileFormat::GetFileHandle()
{
    if (ncExIIId == -1)
    {
        VisItNCErr = nc_open(filenames[0], NC_NOWRITE, &ncExIIId);
        CheckNCError(nc_open);
    }
    return ncExIIId;
}

// ****************************************************************************
//  Method: avtExodusFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources, like memory and file descriptors.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

void
avtExodusFileFormat::FreeUpResources(void)
{
    if (ncExIIId != -1)
    {
        VisItNCErr = nc_close(ncExIIId);
        CheckNCError(nc_close);
    }
    ncExIIId = -1;
}

// ****************************************************************************
//  Method: avtExodusFileFormat::GetTimes
//
//  Purpose:
//      Get the times for each timestep.
//
//  Arguments:
//      times  A place to put the times numbers.
//
//  Programmer: Hank Childs
//  Creation:   April 17, 2004
//
//  Modifications:
//
//    Hank Childs, Wed Jul 14 07:35:25 PDT 2004
//    Explicitly tell the reader to load the times -- it turns out that this
//    is a costly operation and we should only do it when necessary -- hence
//    we explicitly tell the reader when we want them.
//
// ****************************************************************************

void
avtExodusFileFormat::GetTimes(vector<double> &times)
{
    GetTimesteps(0, &times);
}

int
avtExodusFileFormat::GetNTimesteps(void)
{
    int ntimes;
    GetTimesteps(&ntimes, 0);
    return ntimes;
}

void
avtExodusFileFormat::GetTimesteps(int *ntimes, vector<double> *times)
{
    GetFileHandle();

    int timesVarId;
    VisItNCErr = nc_inq_varid(ncExIIId, "time_whole", &timesVarId);
    if (VisItNCErr == NC_ENOTVAR) // handle the no-time info case
    {
        if (ntimes)
        {
            *ntimes = 1;
        }
        if (times)
        {
            times->resize(1);
            times->operator[](0) = 0.0;
        }
        return;
    }
    CheckNCError(nc_inq_varid);

    nc_type vtype;
    int vndims, vdimids[NC_MAX_VAR_DIMS];
    VisItNCErr = nc_inq_var(ncExIIId, timesVarId, 0, &vtype, &vndims, vdimids, 0);
    CheckNCError(nc_inq_var);

    // vndims should be 1
    if (vndims != 1)
    {
        EXCEPTION2(UnexpectedValueException, vndims, 1);
    }

    // Compute length of times array
    size_t len = 1;
    for (int i = 0; i < vndims; i++)
    {
        size_t dlen;
        VisItNCErr = nc_inq_dim(ncExIIId, vdimids[i], 0, &dlen);
        CheckNCError(nc_inq_dim);
        if (dlen == 0) dlen = 1;
        len *= dlen;
    }

    if (len > (size_t) INT_MAX) len = (size_t) INT_MAX;

    if (ntimes) *ntimes = (int) len;

    if (times)
    {
        times->resize(len);

        if (vtype == NC_FLOAT)
        {
            double *atimes = &(times->operator[](0));
            float *vals = new float[len];
            VisItNCErr = nc_get_var_float(ncExIIId, timesVarId, vals);
            CheckNCError(nc_inq_dimid);
            for (int i = 0; i < len; i++)
                atimes[i] = vals[i];
            delete [] vals;
        }
        else if (vtype == NC_DOUBLE)
        {
            VisItNCErr = nc_get_var_double(ncExIIId, timesVarId, &(times->operator[](0)));
            CheckNCError(nc_inq_dimid);
        }
        else
        {
            EXCEPTION2(UnexpectedValueException, "NC_FLOAT || NC_DOUBLE", vtype);
        }
    }
}

template <class T, typename N>
static T* ExtendCoordsTemplate(T *rhs)
{
    T* retval = T::New();
    retval->SetNumberOfComponents(3);
    retval->SetNumberOfTuples(rhs->GetNumberOfTuples());

    for (int t = 0; t < retval->GetNumberOfTuples(); t++)
    {
        N cval[5], rval[3] = {0,0,0};
        rhs->GetTupleValue(t, rval);
        cval[0] = rval[0];
        cval[1] = rval[1];
        cval[2] = 0.0;
        retval->SetTupleValue(t, cval);
    }
    return retval;
}

#define ECT(T,t) ExtendCoordsTemplate<vtk ## T ## Array, t>((vtk ## T ## Array *)rhs)
static vtkDataArray * ExtendCoords(vtkDataArray *rhs)
{
    switch (rhs->GetDataType())
    {
        case VTK_CHAR:           return ECT(Char,char);
        case VTK_UNSIGNED_CHAR:  return ECT(UnsignedChar,unsigned char);
        case VTK_SHORT:          return ECT(Short,short);
        case VTK_UNSIGNED_SHORT: return ECT(UnsignedShort,unsigned short);
        case VTK_INT:            return ECT(Int,int);
        case VTK_UNSIGNED_INT:   return ECT(UnsignedInt,unsigned int);
        case VTK_FLOAT:          return ECT(Float,float);
        case VTK_DOUBLE:         return ECT(Double,double);
    }
    return NULL;
}

template <class T, typename N>
static T* ComposeCoordsTemplate(T *rhsx, T *rhsy, T* rhsz)
{
    T* retval = T::New();
    retval->SetNumberOfComponents(3);
    retval->SetNumberOfTuples(rhsx->GetNumberOfTuples());

    for (int t = 0; t < retval->GetNumberOfTuples(); t++)
    {
        N cval[5], xval[5], yval[5] = {0,0,0,0,0}, zval[5] = {0,0,0,0,0};
        rhsx->GetTupleValue(t, xval);
        if (rhsy)
            rhsy->GetTupleValue(t, yval);
        if (rhsz)
            rhsz->GetTupleValue(t, zval);
        cval[0] = xval[0];
        cval[1] = yval[0];
        cval[2] = zval[0];
        retval->SetTupleValue(t, cval);
    }
    return retval;
}

#define CCT(T,t) ComposeCoordsTemplate<vtk ## T ## Array, t>((vtk ## T ## Array *)rhsx,(vtk ## T ## Array *)rhsy,(vtk ## T ## Array *)rhsz)
static vtkDataArray * ComposeCoords(vtkDataArray *rhsx, vtkDataArray *rhsy, vtkDataArray *rhsz)
{
    switch (rhsx->GetDataType())
    {
        case VTK_CHAR:           return CCT(Char,char);
        case VTK_UNSIGNED_CHAR:  return CCT(UnsignedChar,unsigned char);
        case VTK_SHORT:          return CCT(Short,short);
        case VTK_UNSIGNED_SHORT: return CCT(UnsignedShort,unsigned short);
        case VTK_INT:            return CCT(Int,int);
        case VTK_UNSIGNED_INT:   return CCT(UnsignedInt,unsigned int);
        case VTK_FLOAT:          return CCT(Float,float);
        case VTK_DOUBLE:         return CCT(Double,double);
    }
    return NULL;
}

template <class T, typename N>
static void PlusEqualTemplate(T *lhs, T *rhs)
{
    for (int t = 0; t < lhs->GetNumberOfTuples(); t++)
    {
        N lhstuple[32];
        N rhstuple[32];
        lhs->GetTupleValue(t, lhstuple);
        rhs->GetTupleValue(t, rhstuple);
        for (int c = 0; c < lhs->GetNumberOfComponents(); c++)
            lhstuple[c] += rhstuple[c];
        lhs->SetTupleValue(t, lhstuple);
    }
}

#define PUT(T,t) PlusEqualTemplate<vtk ## T ## Array, t>((vtk ## T ## Array *)lhs,(vtk ## T ## Array *)rhs)
static void PlusEqual(vtkDataArray *lhs, vtkDataArray *rhs)
{
    switch (lhs->GetDataType())
    {
        case VTK_CHAR:           PUT(Char,char); break;
        case VTK_UNSIGNED_CHAR:  PUT(UnsignedChar,unsigned char); break;
        case VTK_SHORT:          PUT(Short,short); break;
        case VTK_UNSIGNED_SHORT: PUT(UnsignedShort,unsigned short); break;
        case VTK_INT:            PUT(Int,int); break;
        case VTK_UNSIGNED_INT:   PUT(UnsignedInt,unsigned int); break;
        case VTK_FLOAT:          PUT(Float,float); break;
        case VTK_DOUBLE:         PUT(Double,double); break;
    }
}

static void 
DecodeExodusElemTypeAttText(const char *ex_elem_type_att, int *tdim, int *vtk_celltype)
{
    if (!STRNCASECMP(ex_elem_type_att, "circle", strlen("circle")))
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype) *vtk_celltype = VTK_VERTEX;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "sphere", strlen("sphere")))
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype) *vtk_celltype = VTK_VERTEX;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "quad4", strlen("quad4")))
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype) *vtk_celltype = VTK_QUAD;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "quad", strlen("quad")))
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype) *vtk_celltype = VTK_QUAD;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "tri3", strlen("tri3")))
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype) *vtk_celltype = VTK_TRIANGLE;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "triangle", strlen("triangle")))
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype) *vtk_celltype = VTK_TRIANGLE;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "shell4", strlen("shell4")))
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype) *vtk_celltype = VTK_QUAD;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "shell", strlen("shell")))
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype) *vtk_celltype = VTK_QUAD;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "hex8", strlen("hex8")))
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype) *vtk_celltype = VTK_HEXAHEDRON;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "hex", strlen("hex")))
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype) *vtk_celltype = VTK_HEXAHEDRON;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "tetra", strlen("tetra")))
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype) *vtk_celltype = VTK_TETRA;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "wedge", strlen("wedge")))
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype) *vtk_celltype = VTK_WEDGE;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "pyramid", strlen("pyramid")))
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype) *vtk_celltype = VTK_PYRAMID;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "beam", strlen("beam")))
    {
        if (tdim) *tdim = 1;
        if (vtk_celltype) *vtk_celltype = VTK_LINE;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "bar", strlen("bar")))
    {
        if (tdim) *tdim = 1;
        if (vtk_celltype) *vtk_celltype = VTK_LINE;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "edge", strlen("edge")))
    {
        if (tdim) *tdim = 1;
        if (vtk_celltype) *vtk_celltype = VTK_LINE;
        return;
    }
    else if (!STRNCASECMP(ex_elem_type_att, "null", strlen("null")))
    {
        if (tdim) *tdim = 0;
        if (vtk_celltype) *vtk_celltype = VTK_EMPTY_CELL;
        return;
    }
    if (tdim) *tdim = -1;
    if (vtk_celltype) *vtk_celltype = -1;
}

static int
ExodusElemTypeAtt2TopoDim(const char *ex_elem_type_att)
{
    int tdim;
    DecodeExodusElemTypeAttText(ex_elem_type_att, &tdim, 0);
    return tdim;
}

static int
ExodusElemTypeAtt2VTKCellType(const char *ex_elem_type_att)
{
    int ctype;
    DecodeExodusElemTypeAttText(ex_elem_type_att, 0, &ctype);
    return ctype;
}

void
avtExodusFileFormat::AddVar(avtDatabaseMetaData *md, char const *vname,
    int topo_dim, int ncomps, avtCentering centering)
{
    if (ncomps == 1)
    {
        avtScalarMetaData *smd = new avtScalarMetaData(vname, "Mesh", centering);
        md->Add(smd);
    }
    else if (2 <= ncomps && ncomps <= 3)
    {
        avtVectorMetaData *vmd = new avtVectorMetaData(vname, "Mesh", centering, ncomps);
        md->Add(vmd);
    }
    else if (ncomps == 4)
    {
        if (topo_dim == 2)
        {
            avtTensorMetaData *tmd = new avtTensorMetaData(vname, "Mesh", centering, ncomps);
            md->Add(tmd);
        }
        else
        {
            avtVectorMetaData *vmd = new avtVectorMetaData(vname, "Mesh", centering, ncomps);
            md->Add(vmd);
        }
    }
    else if (ncomps == 6)
    {
        avtSymmetricTensorMetaData *tmd = new avtSymmetricTensorMetaData(vname, "Mesh", centering, ncomps);
        md->Add(tmd);
    }
    else if (ncomps == 9)
    {
        avtTensorMetaData *tmd = new avtTensorMetaData(vname, "Mesh", centering, ncomps);
        md->Add(tmd);
    }
}

// ****************************************************************************
//  Method: avtExodusFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data from the Exodus file.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Mar 11 08:52:59 PST 2002
//    Renamed to PopulateDatabaseMetaData.
//
//    Hank Childs, Tue May 28 14:07:25 PDT 2002
//    Renamed materials and domains to element block and files.  This is more
//    like how Exodus users think of these terms.
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Make sure the file is read in before proceeding.
//
//    Hank Childs, Sun Jun 27 13:20:51 PDT 2004
//    Indicate that we have global node ids.
//
//    Hank Childs, Thu Jul 22 14:41:51 PDT 2004
//    Use the real filenames when creating the SIL.
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Removed setting of avtMeshMetadata->containsGlobalNodeIds
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg since this is an MTXX database. Nonetheless,
//    timeState argument is ununsed
//
//    Eric Brugger, Fri Mar  9 14:43:07 PST 2007
//    Added support for element block names.
//
//    Hank Childs, Thu Dec  2 08:45:31 PST 2010
//    No longer sort the block names.  It creates indexing issues later.
//
// ****************************************************************************

void
avtExodusFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    int   i;

    GetFileHandle();

    // Acquire spatial dimension information
    int num_dim_dimId;
    VisItNCErr = nc_inq_dimid(ncExIIId, "num_dim", &num_dim_dimId);
    CheckNCError(nc_inq_dimid);
    size_t num_dim_len;
    VisItNCErr = nc_inq_dimlen(ncExIIId, num_dim_dimId, &num_dim_len);
    CheckNCError(nc_inq_dimlen);
    int spatialDimension = (int) num_dim_len;

    // Acquire topological dimension information (harder because we
    // need to inspect NC Attributes named "elem_type" on all "connectX"
    // datasets and take the max.
    int num_el_blk_dimId;
    VisItNCErr = nc_inq_dimid(ncExIIId, "num_el_blk", &num_el_blk_dimId);
    CheckNCError(nc_inq_dimid);

    size_t num_el_blk_len;
    VisItNCErr = nc_inq_dimlen(ncExIIId, num_el_blk_dimId, &num_el_blk_len);
    CheckNCError(nc_inq_dimlen);

#ifdef MAX
#undef MAX
#endif
#define MAX(A,B) ((A)>(B)?(A):(B))

    int topologicalDimension = -1;
    numBlocks = (int) num_el_blk_len;
    for (int i = 0; i < num_el_blk_len; i++)
    {
        int connect_varid;
        char connect_varname[NC_MAX_NAME+1];
        SNPRINTF(connect_varname, sizeof(connect_varname), "connect%d", i+1);
        VisItNCErr = nc_inq_varid(ncExIIId, connect_varname, &connect_varid);
        if (VisItNCErr == NC_ENOTVAR) continue;
        CheckNCError(nc_inq_varid);

        char *connect_elem_type_attval = new char[256];
        memset(connect_elem_type_attval, 0, 256);
        VisItNCErr = nc_get_att_text(ncExIIId, connect_varid, "elem_type", connect_elem_type_attval);
        if (VisItNCErr != NC_NOERR)
        {
            delete [] connect_elem_type_attval;
            continue;
        }

        topologicalDimension = MAX(topologicalDimension,
            ExodusElemTypeAtt2TopoDim(connect_elem_type_attval));
        delete [] connect_elem_type_attval;
    }

    if (topologicalDimension == -1)
        topologicalDimension = spatialDimension;

    string meshName = "Mesh";

    //
    // Set up the mesh.
    //
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = meshName;
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->numBlocks = 1;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = spatialDimension;
    mesh->topologicalDimension = topologicalDimension;
    mesh->blockTitle = "File";
    if (globalFileLists != NULL && fileList >= 0 && 
        (fileList < globalFileLists->size()))
    {
        mesh->blockNames = (*globalFileLists)[fileList];
    }
    mesh->blockPieceName = "File";
    md->Add(mesh);

    //
    // Query Exodus "title" information
    //
    char *global_title_attval = new char[256];
    memset(global_title_attval, 0, 256);
    VisItNCErr = nc_get_att_text(ncExIIId, NC_GLOBAL, "title", global_title_attval);
    CheckNCError(nc_get_att_text);
    if (VisItNCErr == NC_NOERR)
        md->SetDatabaseComment(global_title_attval);
    delete [] global_title_attval;

    //
    // Set up the material to 'spoof' Exodus element blocks. This is a critical
    // design consideration as Exodus element blocks can 'span' multiple VisIt
    // 'SD' block files. So, the logical way to handle that with VisIt is to 
    // treat Exodus' element blocks as materials and indeed, many Exodus
    // applications tend to think of them that way anyways. Finally, all Exodus
    // element blocks are homogeneous in material and mesh properties so no
    // need to worry about 'mixed' materials in this context.
    //
    GetElementBlockNamesAndIds(ncExIIId, numBlocks, blockName, blockId);

    string materialName = "ElementBlock";
    vector<string> matNames;
    if (numBlocks > 0 && blockName[0] == "")
    {
        for (i = 0 ; i < numBlocks ; i++)
        {
            char name[128];
            sprintf(name, "%d", blockId[i]);
            matNames.push_back(name);
        }
    }
    else
    {
        for (i = 0 ; i < numBlocks ; i++)
            matNames.push_back(blockName[i]);
    }
    AddMaterialToMetaData(md, materialName, meshName, numBlocks, matNames);

    //
    // Add nodal and zonal variables
    // We treat any NC variables whose dimensions are either num_nodes or
    // num_elem as 'variables'
    //
    int num_nodes_dimId, num_elem_dimId;
    VisItNCErr = nc_inq_dimid(ncExIIId, "num_nodes", &num_nodes_dimId);
    CheckNCError(nc_inq_dimid);
    VisItNCErr = nc_inq_dimid(ncExIIId, "num_elem", &num_elem_dimId);
    CheckNCError(nc_inq_dimid);

    char **node_var_names = GetStringListFromExodusIINCvar(ncExIIId, "name_nod_var");
    i = 0;
    while (node_var_names[i])
    {
        char composite_name[NC_MAX_NAME];
        int ncomps;
        if (AreSuccessiveStringsRelatedComponentNames(node_var_names, i,
                &ncomps, composite_name))
        {
            AddVar(md, composite_name, topologicalDimension, ncomps, AVT_NODECENT);
            i += ncomps;
        }
        else
        {
            AddVar(md, node_var_names[i], topologicalDimension, 1, AVT_NODECENT);
            i++;
        }
    }
    FreeStringListFromExodusIINCvar(node_var_names);

    char **elem_var_names = GetStringListFromExodusIINCvar(ncExIIId, "name_elem_var");
    i = 0;
    while (elem_var_names[i])
    {
        char composite_name[NC_MAX_NAME];
        int ncomps;
        if (AreSuccessiveStringsRelatedComponentNames(elem_var_names, i,
                &ncomps, composite_name))
        {
            AddVar(md, composite_name, topologicalDimension, ncomps, AVT_ZONECENT);
            i += ncomps;
        }
        else
        {
            AddVar(md, elem_var_names[i], topologicalDimension, 1, AVT_ZONECENT);
            i++;
        }
    }
    FreeStringListFromExodusIINCvar(elem_var_names);

    int nvars;
    VisItNCErr = nc_inq(ncExIIId, 0, &nvars, 0, 0);
    CheckNCError(nc_inq);

    int unlimited_dimid;
    nc_inq_unlimdim(ncExIIId, &unlimited_dimid);
    for (int i = 0; i < nvars; i++)
    {
        char vname[NC_MAX_NAME+1];
        nc_type vtype;
        int vndims, vdimids[NC_MAX_VAR_DIMS], nvatts;
        nc_inq_var(ncExIIId, i, vname, &vtype, &vndims, vdimids, &nvatts);
        //if (!strcmp(vname, "coord")) continue;
        if (!strncmp(vname, "vals_nod_var", 12)) continue;
        if (!strncmp(vname, "vals_elem_var", 13)) continue;
        avtCentering centering = AVT_UNKNOWN_CENT;
        int ncomps = 1;
        for (int j = 0; j < vndims; j++)
        {
            if (vdimids[j] == unlimited_dimid)
                continue;
            else if (vdimids[j] == num_nodes_dimId)
                centering = AVT_NODECENT;
            else if (vdimids[j] == num_elem_dimId)
                centering = AVT_ZONECENT;
            else
            {
                size_t dlen;
                VisItNCErr = nc_inq_dim(ncExIIId, vdimids[j], 0, &dlen);
                ncomps *= dlen;
            }
        }
        if (centering == AVT_UNKNOWN_CENT) continue;
        // We don't know if the variable may have already been added
        // by logic above, so we use this attempt to determine var
        // type as a way of avoiding duplicates. However, that function
        // throws an exception if the var is not already present
        // and so we need a try/catch block.
        TRY
        {
            const bool do_expr = false;
            avtVarType vt = md->DetermineVarType(vname, do_expr);
        }
        CATCH(InvalidVariableException)
        {
            AddVar(md, vname, topologicalDimension, ncomps, centering);
        }
        ENDTRY
    }

    //
    // Add exodus nodsets, if any exist, as enumerated scalars
    //
    vector<int> nsids;
    GetExodusSetIDs(ncExIIId, "node", nsids);
    if (nsids.size())
    {
        avtScalarMetaData *smd = new avtScalarMetaData("Nodesets", "Mesh", AVT_NODECENT);
        for (int i = 0; i < nsids.size(); i++)
        {
            char tmp[32];
            SNPRINTF(tmp, sizeof(tmp), "%d", nsids[i]);
            smd->AddEnumNameValue(tmp, i);
        }
        smd->SetEnumerationType(avtScalarMetaData::ByBitMask);
        smd->SetEnumPartialCellMode(avtScalarMetaData::Dissect);
        smd->hideFromGUI = true;
        md->Add(smd);
    }

    //
    // Add exodus sidesets, if any exist, as enumerated scalars
    //
    vector<int> ssids;
    GetExodusSetIDs(ncExIIId, "side", ssids);
    if (ssids.size())
    {
        avtScalarMetaData *smd = new avtScalarMetaData("Sidesets", "Mesh", AVT_NODECENT);
        for (int i = 0; i < ssids.size(); i++)
        {
            char tmp[32];
            SNPRINTF(tmp, sizeof(tmp), "%d", ssids[i]);
            smd->AddEnumNameValue(tmp, i);
        }
        smd->SetEnumerationType(avtScalarMetaData::ByBitMask);
        smd->SetEnumPartialCellMode(avtScalarMetaData::Dissect);
        smd->hideFromGUI = true;
        md->Add(smd);
    }
}

// ****************************************************************************
//  Method: avtExodusFileFormat::GetMesh
//
//  Purpose:
//      Returns the mesh for a specific timestep.
//
//  Arguments:
//      ts      The timestep of interest.
//      mesh    The name of the mesh of interest.
//
//  Returns:    The mesh.  Note that the "material" can be set beforehand and
//              that will alter how the mesh is read in.
//
//  Programmer: Hank Childs
//  Creation:   October 9, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002  
//    Use NewInstance instead of MakeObject in order to match vtk's new api.
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Make sure the file is read in before proceeding.
//
//    Mark C. Miller, Thu Aug  5 14:17:36 PDT 2004
//    Moved whole of implementation to ReadMesh
//
//    Mark C. Miller, Thu Jun 27 10:13:46 PDT 2013
//    Removed logic to automagically add displacements.
// ****************************************************************************

vtkDataSet *
avtExodusFileFormat::GetMesh(int ts, const char *mesh)
{
    GetFileHandle();

    // Get block count if we don't know it already
    if (numBlocks == -1)
    {
        int num_el_blk_dimId;
        VisItNCErr = nc_inq_dimid(ncExIIId, "num_el_blk", &num_el_blk_dimId);
        CheckNCError(nc_inq_dimid);

        size_t num_el_blk_len;
        VisItNCErr = nc_inq_dimlen(ncExIIId, num_el_blk_dimId, &num_el_blk_len);
        CheckNCError(nc_inq_dimlen);
        numBlocks = (int) num_el_blk_len;
    }

    vtkDataArray *coords = 0;
    TRY
    {
        coords = GetVectorVar(ts, "coord");
    }
    CATCH(InvalidVariableException)
    {
        vtkDataArray *coordx = GetVar(ts, "coordx");
        vtkDataArray *coordy = 0;
        vtkDataArray *coordz = 0;
        TRY { coordy = GetVar(ts, "coordy"); } CATCH(InvalidVariableException) { } ENDTRY
        TRY { coordz = GetVar(ts, "coordz"); } CATCH(InvalidVariableException) { } ENDTRY
        coords = ComposeCoords(coordx, coordy, coordz);
        coordx->Delete();
        if (coordy) coordy->Delete();
        if (coordz) coordz->Delete();
    }
    ENDTRY

    if (coords->GetNumberOfComponents() == 2)
    {
        vtkDataArray *newcoords = ExtendCoords(coords);
        coords->Delete();
        coords = newcoords;
    }

    //
    // I removed this becuase it appears Exodus users are more accustomed to having
    // to manually displace their mesh instead of having it happen automagically when
    // it is read from the file. What this block does is simply add displacements to
    // the mesh coordinates before handing it back up to VisIt. In fact, I don't think
    // we're being too smart about Exodus' mesh in the case where we are not adding
    // displacements here because it is the same (e.g. constant) from timestep to
    // timestep (unless there are topological changes (e.g. zones are added or removed)
    // and so we could do things better by just return the mesh from the variable
    // cache.
    //
#if 0
    // Not all Exodus' files have displacements
    TRY
    {
        vtkDataArray *displs = 0;
        TRY
        {
            displs = GetVectorVar(ts, "DISPL");
        }
        CATCH(InvalidVariableException)
        {
            vtkDataArray *displx = GetVar(ts, "DISPLX");
            vtkDataArray *disply = 0;
            vtkDataArray *displz = 0;
            TRY { disply = GetVar(ts, "DISPLY"); } CATCH(InvalidVariableException) { } ENDTRY
            TRY { displz = GetVar(ts, "DISPLZ"); } CATCH(InvalidVariableException) { } ENDTRY
            displs = ComposeCoords(displx, disply, displz);
            displx->Delete();
            if (disply) disply->Delete();
            if (displz) displz->Delete();
        }
        ENDTRY

        if (displs)
        {
            PlusEqual(coords, displs);
            displs->Delete();
        }
    }
    CATCH(InvalidVariableException)
    {
    }
    ENDTRY
#endif

    vtkPoints *points = vtkPoints::New();
    points->SetData(coords);
    numNodes = points->GetNumberOfPoints();

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();

    int num_elem_dimId;
    VisItNCErr = nc_inq_dimid(ncExIIId, "num_elem", &num_elem_dimId);
    CheckNCError(nc_inq_dimid);
    size_t num_elem_len;
    VisItNCErr = nc_inq_dimlen(ncExIIId, num_elem_dimId, &num_elem_len);
    CheckNCError(nc_inq_dimlen);
    int num_elem = (int) num_elem_len;
    numElems = num_elem;

    for (int i = 0; i < numBlocks; i++)
    {
        int connect_varid;
        char connect_varname[NC_MAX_NAME+1];
        SNPRINTF(connect_varname, sizeof(connect_varname), "connect%d", i+1);
        VisItNCErr = nc_inq_varid(ncExIIId, connect_varname, &connect_varid);
        if (VisItNCErr != NC_NOERR) continue;

        nc_type connect_vartype;
        VisItNCErr = nc_inq_vartype(ncExIIId, connect_varid, &connect_vartype);
        CheckNCError(nc_inq_varid);

        char *connect_elem_type_attval = new char[256];
        memset(connect_elem_type_attval, 0, 256);
        VisItNCErr = nc_get_att_text(ncExIIId, connect_varid, "elem_type", connect_elem_type_attval);
        if (VisItNCErr != NC_NOERR)
            strcpy(connect_elem_type_attval, "unknown");
        int vtk_celltype = ExodusElemTypeAtt2VTKCellType(connect_elem_type_attval);
        delete [] connect_elem_type_attval;

        char num_el_in_blk_dimname[NC_MAX_NAME+1];
        SNPRINTF(num_el_in_blk_dimname, sizeof(num_el_in_blk_dimname), "num_el_in_blk%d", i+1);
        int num_el_in_blk_dimId;
        VisItNCErr = nc_inq_dimid(ncExIIId, num_el_in_blk_dimname, &num_el_in_blk_dimId);
        CheckNCError(nc_inq_dimid);
        size_t num_el_in_blk_len;
        VisItNCErr = nc_inq_dimlen(ncExIIId, num_el_in_blk_dimId, &num_el_in_blk_len);
        CheckNCError(nc_inq_dimlen);
        int num_elems_in_blk = (int) num_el_in_blk_len;

        char num_nod_per_dimname[NC_MAX_NAME+1];
        SNPRINTF(num_nod_per_dimname, sizeof(num_nod_per_dimname), "num_nod_per_el%d", i+1);
        int num_nod_per_dimId;
        VisItNCErr = nc_inq_dimid(ncExIIId, num_nod_per_dimname, &num_nod_per_dimId);
        CheckNCError(nc_inq_dimid);
        size_t num_nod_per_len;
        VisItNCErr = nc_inq_dimlen(ncExIIId, num_nod_per_dimId, &num_nod_per_len);
        CheckNCError(nc_inq_dimlen);
        int num_nodes_per_elem = (int) num_nod_per_len;

        blockIdToMatMap[i+1] = num_elems_in_blk;

        vtkIdType verts[16];
        switch (connect_vartype)
        {
            case NC_INT:
            {
                int *conn_buf = new int[num_nodes_per_elem * num_elems_in_blk];
                VisItNCErr = nc_get_var_int(ncExIIId, connect_varid, conn_buf);
                CheckNCError(nc_get_var_int);
                if (VisItNCErr != NC_NOERR)
                {
                    char msg[256];
                    SNPRINTF(msg, sizeof(msg), "Unable to read connect%d: \"%s\"", i+1, nc_strerror(VisItNCErr));
                    EXCEPTION1(InvalidFilesException, msg);
                }
                int *p = conn_buf;
                for (int j = 0; j < num_elems_in_blk; j++)
                {
                    for (int k = 0; k < num_nodes_per_elem; k++, p++)
                    {
                        verts[k] = (vtkIdType) *p-1; // Exodus is 1-origin
                    }
                    ugrid->InsertNextCell(vtk_celltype, num_nodes_per_elem, verts);
                }
                delete [] conn_buf;
                break;
            }
            case NC_INT64:
            {
                long long *conn_buf = new long long[num_nodes_per_elem * num_elems_in_blk];
                VisItNCErr = nc_get_var_longlong(ncExIIId, connect_varid, conn_buf);
                CheckNCError(nc_get_var_longlong);
                if (VisItNCErr != NC_NOERR)
                {
                    char msg[256];
                    SNPRINTF(msg, sizeof(msg), "Unable to read connect%d: \"%s\"", i+1, nc_strerror(VisItNCErr));
                    EXCEPTION1(InvalidFilesException, msg);
                }
                long long *p = conn_buf;
                for (int j = 0; j < num_elems_in_blk; j++)
                {
                    for (int k = 0; k < num_nodes_per_elem; k++, p++)
                        verts[k] = (vtkIdType) *p-1; // Exodus is 1-origin
                    ugrid->InsertNextCell(vtk_celltype, num_nodes_per_elem, verts);
                }
                delete [] conn_buf;
                break;
            }
            default:
            {
                EXCEPTION2(UnexpectedValueException, "NC_INT || NC_INT64", connect_vartype);
            }
        }
    }

    return ugrid;
}

// ****************************************************************************
//  Method: avtExodusFileFormat::GetVar
//
//  Purpose:
//      Gets the scalar variable for the specified timestep and variable.
//
//  Arguments:
//      ts      The timestep of interest.
//      mesh    The name of the mesh of interest.
//
//  Returns:    The variable.  Note that the "material" can be set beforehand
//              and that will alter how the variable is read in.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Make sure the file is read in before proceeding.
//
// ****************************************************************************

vtkDataArray *
avtExodusFileFormat::GetVar(int ts, const char *var)
{
    nc_type type;
    int num_vals, num_comps;
    void *buf;

    if (string(var) == "Nodesets" || string(var) == "Sidesets")
        return GetExodusSetsVar(ncExIIId, ts, var, numNodes, numElems);

    GetData(ncExIIId, ts, var, numBlocks, &type, &num_comps, &num_vals, &buf);

    return MakeVTKDataArrayByTakingOwnershipOfNCVarData(type, num_comps, num_vals, buf);
}

// ****************************************************************************
//  Method: avtExodusFileFormat::GetVectorVar
//
//  Purpose:
//      Gets the vector variable for the specified timestep and variable.
//
//  Arguments:
//      ts      The timestep of interest.
//      mesh    The name of the mesh of interest.
//
//  Returns:    The variable.  Note that the "material" can be set beforehand
//              and that will alter how the variable is read in.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Make sure the file is read in before proceeding.
//
// ****************************************************************************

vtkDataArray *
avtExodusFileFormat::GetVectorVar(int ts, const char *var)
{
    nc_type type;
    int num_vals, num_comps;
    void *buf;

    GetData(ncExIIId, ts, var, numBlocks, &type, &num_comps, &num_vals, &buf);

    return MakeVTKDataArrayByTakingOwnershipOfNCVarData(type, num_comps, num_vals, buf);
}


// ****************************************************************************
//  Method: avtExodusFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      var        The variable of interest.
//      ts         The timestep of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that type -- not used.
//      df         Destructor function.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Added code to read global node/zone ids. Unfortunately, I saw no easy
//    way to do it *without* reading the whole mesh first. In typical usage,
//    that will have already been done and the global node/zone ids cached and
//    so we'll rarely, if ever, wind up in here making an explicit request
//    for them.
//
//    Hank Childs, Mon Aug 23 13:41:54 PDT 2004
//    Rename cache object to avoid namespace conflict with base class.
//    Also increment reference counts of arrays that are becoming void refs.
//
//    Hank Childs, Tue Feb  6 15:47:22 PST 2007
//    Do not assume that global node ids exist.
//
//    Eric Brugger, Fri Mar  9 14:43:07 PST 2007
//    Added support for element block names.
//
//    Hank Childs, Thu Dec  2 08:45:31 PST 2010
//    Fix problem with establishing material IDs.  There are two possible 
//    conventions and we were following the wrong one.  I confirmed with Greg
//    Sjaardema of Sandia that this new code follows the correct convention.
//
//    Mark C. Miller, Mon Jun 24 14:33:54 PDT 2013
//    Fixed off-by-one error in matlist entries. The should start from zero
//    for the avtMaterial constructor being used here.
// ****************************************************************************

void *
avtExodusFileFormat::GetAuxiliaryData(const char *var, int ts, 
                                      const char * type, void *,
                                      DestructorFunction &df)
{
    int i;

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        if (strstr(var, "ElementBlock") != var)
            EXCEPTION1(InvalidVariableException, var);

        int nzones = 0;
        for (map<int,int>::const_iterator
            it = blockIdToMatMap.begin(); it != blockIdToMatMap.end(); it++)
            nzones += it->second;

        int *matlist = new int[nzones];
        int zone = 0;
        for (map<int,int>::const_iterator
            it = blockIdToMatMap.begin(); it != blockIdToMatMap.end(); it++)
        {
            for (int j = 0; j < it->second; j++, zone++)
                matlist[zone] = it->first-1;
        }

        if (blockName.size() == 0 && blockId.size() == 0)
            GetElementBlockNamesAndIds(ncExIIId, numBlocks, blockName, blockId);

        vector<string> mats(numBlocks);
        if (numBlocks > 0 && blockName[0] == "")
        {
            for (i = 0 ; i < numBlocks ; i++)
            {
                char num[1024];
                sprintf(num, "%d", blockId[i]);
                mats[i] = num;
            }
        }
        else
        {
            for (i = 0 ; i < numBlocks ; i++)
                mats[i] = blockName[i];
        }

        avtMaterial *mat = new avtMaterial(numBlocks, mats, nzones, matlist,
                                           0, NULL, NULL, NULL, NULL);
        delete [] matlist;
        df = avtMaterial::Destruct;
        return (void*) mat;
    }
    else if (strcmp(type, AUXILIARY_DATA_GLOBAL_NODE_IDS) == 0)
    {
        // Unfortunately, without making broad changes to vtkVisItExodusIIReader
        // the only way to obtain global node ids is by first asking
        // the reader for the mesh and then taking them from that if
        // they are present. Fortunately, in most situations, the
        // mesh will have already been read and the global node ids
        // cached and we'll never wind up here having to read global
        // node ids for the first time without having already read
        // the mesh
#if 0

        //
        // If we happend to find that there exists also global zone Ids,
        // we'll remove them from from the vtkDataSet object and stick
        // them in the AVT cache now.
        //
        vtkDataArray *gzoneIds =ds->GetCellData()->GetArray(rdr->GetGlobalElementIdArrayName());
        if (gzoneIds != NULL)
        {
            gzoneIds->Register(NULL);
            void_ref_ptr vr = void_ref_ptr(gzoneIds,
                                          avtVariableCache::DestructVTKObject);
            cache->CacheVoidRef("Mesh", AUXILIARY_DATA_GLOBAL_ZONE_IDS, 
                                -1, myDomain, vr);
        }

        //
        // Remove the global node ids from the vtkDataSet object but DO NOT
        // cache them as avtGenericDatabase will do that for us upon return
        // from this call
        //
        vtkDataArray *gnodeIds=ds->GetPointData()->GetArray(rdr->GetGlobalNodeIdArrayName());

        //
        // Return what we came here for
        //
        if (gnodeIds != NULL)
            gnodeIds->Register(NULL);
        df = avtVariableCache::DestructVTKObject;
        return (void*) gnodeIds;
#endif
    }
    else if (strcmp(type, AUXILIARY_DATA_GLOBAL_ZONE_IDS) == 0)
    {
#if 0
        //
        // See long note, above, for AUXILIARY_DATA_GLOBAL_NODE_IDS query
        //
        bool okToRemoveAndCacheGlobalIds = false;
        vtkDataSet *ds = ReadMesh(ts, "Mesh", okToRemoveAndCacheGlobalIds);

        //
        // If we happend to find that there exists also global node Ids,
        // we'll remove them from from the vtkDataSet object and stick
        // them in the AVT cache now.
        //
        vtkDataArray *gnodeIds=ds->GetPointData()->GetArray(rdr->GetGlobalNodeIdArrayName());
        if (gnodeIds != NULL)
        {
            gnodeIds->Register(NULL);
            void_ref_ptr vr = void_ref_ptr(gnodeIds,
                                          avtVariableCache::DestructVTKObject);
            cache->CacheVoidRef("Mesh", AUXILIARY_DATA_GLOBAL_NODE_IDS,
                                -1, myDomain, vr);
        }

        //
        // Remove the global zone ids from the vtkDataSet object but DO NOT
        // cache them as avtGenericDatabase will do that for us upon return
        // from this call
        //
        vtkDataArray *gzoneIds =ds->GetCellData()->GetArray(rdr->GetGlobalElementIdArrayName());

        //
        // Return what we came here for
        //
        gzoneIds->Register(NULL);
        df = avtVariableCache::DestructVTKObject;
        return (void*) gzoneIds;
#endif
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtExodusFileFormat::SetTimestep
//
//  Purpose:
//      Sets the timestep of the Exodus file.
//
//  Programmer: Hank Childs
//  Creation:   October 9, 2001
//
// ****************************************************************************

void
avtExodusFileFormat::SetTimestep(int ts)
{
    int nTimesteps = GetNTimesteps();
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }
    debug1 << "avtExodusFileFormat::SetTimestep called but don't know what to do about about" << endl;
}
