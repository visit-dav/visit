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
//                           avtExodusFileFormat.C                           //
// ************************************************************************* //

#include <avtExodusFileFormat.h>
#include <avtExodusOptions.h>

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
#include <vtkLongLongArray.h>
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
#include <avtMixedVariable.h>
#include <avtVariableCache.h>

#include <BadIndexException.h>
#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <Namescheme.h>
#include <UnexpectedValueException.h>
#include <Utility.h>

#include <netcdf.h>

#include <string.h>
#ifdef HAVE_VTK_SIZEOF___INT64
#include <boost/cstdint.hpp>
#endif

#include <cstdlib> // for qsort
#include <cstdarg>

#include <map>
#include <string>
#include <vector>

#if defined(_WIN32)
#define STRNCASECMP _strnicmp
#else
#define STRNCASECMP strncasecmp
#endif

using     std::map;
using     std::string;
using     std::vector;
#ifdef HAVE_VTK_SIZEOF___INT64
using     boost::int64_t;
#endif
using namespace ExodusDBOptions;

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

// ****************************************************************************
// Compare function for qsort in GetStringListFromExodusIINCvar
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

static int CompareStrings(void const *_a, void const *_b)
{
    char const **a = (char const **) _a;
    char const **b = (char const **) _b;
    return strcmp(*a,*b);
}

// ****************************************************************************
// Given an Exodus variable known to contain a list of strings, read it and
// optionally, sort it.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

char **GetStringListFromExodusIINCvar(int exfid, char const *var_name, bool sort_results = false)
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
        retval[i] = C_strndup(p,len_string);
    retval[i] = 0;
    free(buf);

    if (sort_results)
        qsort(&retval[0], nstrings, sizeof(char *), CompareStrings);

    return retval;
}

// ****************************************************************************
// These functions and macros, fill_tmp_suffixes, BEGIN_CASES, CASE, END_CASES
// are used within AreSuccessive... to facilitate creation of expressions
// for composite variables when known patterns of groups of variable names
// are discovered. For example, if VEL_X, VEL_Y, VEL_Z are discovered, the
// vector expression for 'VEL' will be created.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

static char *tmp_suffixes[16];
static void fill_tmp_suffixes(int n, ...)
{
    va_list ap;

    memset(tmp_suffixes, 0, sizeof(tmp_suffixes));

    va_start(ap, n);
    for (int i = 0; i < n; i++)
        tmp_suffixes[i] = va_arg(ap, char*);
    va_end(ap);
}

#ifdef WIN32
#define strcasecmp stricmp
#endif

#define BEGIN_CASES                                                    \
{   bool found_match = false

#define CASE(STRLIST)                                                  \
{                                                                      \
    if (!found_match && unadornedEntry == 0)                           \
    {                                                                  \
        fill_tmp_suffixes STRLIST;                                     \
        int q = 0;                                                     \
        bool things_match = true;                                      \
        while (list[i+q] && tmp_suffixes[q] && things_match)           \
        {                                                              \
            char sepStr[2] = {sepChar, '\0'};                          \
            char ex_var_name[256];                                     \
            SNPRINTF(ex_var_name, sizeof(ex_var_name),                 \
                "%s%s%s", prefix.c_str(), &sepStr[0], tmp_suffixes[q]);\
            if (strcasecmp(ex_var_name, list[i+q]))                    \
                things_match = false;                                  \
            q++;                                                       \
        }                                                              \
        found_match = things_match && q == *ncomps;                    \
    }                                                                  \
}

#define END_CASES(DEFN, TYPE)                                          \
    if (found_match)                                                   \
    {                                                                  \
        bool do_it = true;                                             \
        if (Expression::TYPE == Expression::VectorMeshVar &&           \
            (sdim == matcnt || sdim != *ncomps))                       \
            do_it = false;                                             \
        if (do_it)                                                     \
        {                                                              \
            Expression expr;                                           \
            expr.SetName(prefix);                                      \
            expr.SetDefinition(DEFN);                                  \
            expr.SetType(Expression::TYPE);                            \
            md->AddExpression(&expr);                                  \
            return 1;                                                  \
        }                                                              \
    }                                                                  \
}

// ****************************************************************************
// At the ith member of the (sorted) list of Exodus variable names, try to 
// guess whether the successive members are really the components of a
// composite variable (like a vector or a tensor). If so, attempt to match
// the variable naming pattern to some known conventions and if a match is
// found, define the composite expression. However, be careful not to confuse
// material-specific variable naming conventions (e.g. FOO_1, FOO_2, FOO_3 with
// composite variable naming conventions. If there is potential for confusion,
// err on the side of caution and do not define the expression.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

static int AreSuccessiveStringsRelatedComponentNames(avtDatabaseMetaData *md,
    char const * const *list, int i, int sdim, int matcnt, int *ncomps)
{
    static char const *commonSepChars = "_-:.~@|>;+=";

    // If we're at the end of the list, return immediately.
    if (!list[i+1]) return 0;

    // Examine first two strings in list for longest common prefix
    int prefixlen = (int) strlen(list[i]);
    while (prefixlen)
    {
        if (!strncmp(list[i],list[i+1],prefixlen)) break;
        prefixlen--;
    }

    // Return immediately if no common prefix found in first two entries in list.
    if (prefixlen < 1) return 0;

    // Walk backwards looking for a seperator char to ensure we don't
    // accidentally gobble up a seperator char and part of the suffix
    // in the greatest common prefix matched above.
    int prefixlen_tmp = prefixlen;
    while (prefixlen_tmp > 0)
    {
        if (strchr(commonSepChars, list[i+1][prefixlen_tmp-1]))
        {
            prefixlen = prefixlen_tmp;
            break;
        }
        prefixlen_tmp--;
    }

    // Check if first entry is adorned with a component suffix string.
    int unadornedEntry = 0; // no unadorned entry
    if (strlen(list[i]) < strlen(list[i+1]))
        unadornedEntry = 1; // first item in list is unadorned

    // Look for a seperator character (use 2nd list entry for this in 
    // case first entry is unadorned).
    char sepChar = '\0';
    for (int j = 0; j < (int) sizeof(commonSepChars); j++)
    {
        if (list[i+1][prefixlen-1] == commonSepChars[j] ||
            list[i+1][prefixlen  ] == commonSepChars[j])
        {
            sepChar = commonSepChars[j];
            break;
        }
    }

    // We don't want to include the separator char in the prefix
    if (unadornedEntry == 0 && sepChar != '\0' && list[i+1][prefixlen-1] == sepChar)
        prefixlen--;
    int add1 = sepChar == '\0' ? 0 : 1;

    // Walk through successive entries with the common prefix.
    // Count them. Determine min/max number of chars in postix.
    // Depending on first two entries, any separator char present
    // in the strings in list may or may not be included in the
    // prefix.
    int suffixlen_min, suffixlen_max, k = i;
    suffixlen_min = strlen(&list[i+1][prefixlen+add1]);
    suffixlen_max = suffixlen_min;
    *ncomps = 1;
    while (list[k+1])
    {
        if (strncmp(list[k],list[k+1],prefixlen)) break;
        int suffixlen = strlen(&list[k+1][prefixlen+add1]);
        if (suffixlen < suffixlen_min) suffixlen_min = suffixlen;
        if (suffixlen > suffixlen_max) suffixlen_max = suffixlen;
        k++;
        *ncomps = *ncomps + 1;
    }

    // If first entry wasn't adorned, check if last entry is.
    if (unadornedEntry == 0)
    {
        if (strlen(list[i+*ncomps-1]) < strlen(list[i+*ncomps-2]))
            unadornedEntry = 2;
    }

    // The shorter the prefix string, the greater the chance we've bound together
    // strings that should really be treated as separate. We use heuristic of 2
    // characters as the cut-off of whether to check. If we decide to check, we
    // just examine all the strings and see if we wind up with larger prefixes
    // than what we started with.
    int pflen_max = 0;
    if (prefixlen < 3 && *ncomps > 1)
    {
        for (int q = 1; q < *ncomps-1; q++)
        {
            int pflen = strlen(list[i+q]);
            while (pflen)
            {
                if (!strncmp(list[i+q],list[i+q+1],pflen)) break;
                pflen--;
            }
            if (pflen > pflen_max) pflen_max = pflen;
        }
    }
    if (pflen_max > prefixlen)
        return 0;

    string prefix = string(list[i+1], prefixlen);

    if (unadornedEntry != 0 && (*ncomps == matcnt-1 || *ncomps == matcnt))
    {
        return 1;
    }

    char defn[256];
    int ok = 0;
    for (int k = 0; k < 9; k++)
    {
        if (list[i+k])
            ok++;
        else
            break;
    }
    char const * const list2[9] = {
        ok>0?list[i+0]:"",
        ok>1?list[i+1]:"",
        ok>2?list[i+2]:"",
        ok>3?list[i+3]:"",
        ok>4?list[i+4]:"",
        ok>5?list[i+5]:"",
        ok>6?list[i+6]:"",
        ok>7?list[i+7]:"",
        ok>8?list[i+8]:""
    };

    //
    // Examine variable names for common constructions and infer
    // expressions to be created.
    //
    BEGIN_CASES; // 2D cartesian vectors
        CASE((2, "x", "y"));
        CASE((2, "i", "j"));
        CASE((2, "u", "v"));
        CASE((2, "1", "2"));
        CASE((2, "0", "1"));
        CASE((2, "r", "z")); // Still cartesian in RZ plane
        CASE((2, "z", "r")); // Still cartesian in RZ plane
        SNPRINTF(defn, sizeof(defn), "{%s, %s}", list2[0], list2[1]);
    END_CASES(defn, VectorMeshVar);
    BEGIN_CASES; // 2D polar vectors
        CASE((2, "r", "t"));
        CASE((2, "r", "theta"));
        SNPRINTF(defn, sizeof(defn), "{%s*cos(%s), %s*sin(%s)}",
            list2[0], list2[1], list2[0], list2[1]);
    END_CASES(defn, VectorMeshVar);
    BEGIN_CASES; // 3D cartesian vectors
        CASE((3, "x", "y", "z"));
        CASE((3, "i", "j", "k"));
        CASE((3, "u", "v", "w"));
        CASE((3, "1", "2", "3"));
        CASE((3, "0", "1", "2"));
        SNPRINTF(defn, sizeof(defn), "{%s, %s, %s}", list2[0], list2[1], list2[2]);
    END_CASES(defn, VectorMeshVar);
    BEGIN_CASES; // 3D cylindrical vectors
        CASE((3, "r", "t", "z"));
        CASE((3, "r", "theta", "z"));
        SNPRINTF(defn, sizeof(defn), "{%s*cos(%s), %s*sin(%s), %s}",
            list2[0], list2[1], list2[0], list2[1], list2[2]);
    END_CASES(defn, VectorMeshVar);
    BEGIN_CASES; // 3D spherical vectors
        CASE((3, "r", "t", "p"));
        CASE((3, "r", "theta", "phi"));
        SNPRINTF(defn, sizeof(defn), "{%s*cos(%s)*sin(%s), %s*sin(%s)*sin(%s), %s*cos(%s)}",
            list2[0], list2[1], list2[2],
            list2[0], list2[1], list2[2],
            list2[0], list2[2]);
    END_CASES(defn, VectorMeshVar);
    BEGIN_CASES; // 2D, cartesian symmetric tensors (Voigt notation order)
        CASE((3, "xx", "yy", "xy"));
        CASE((3, "ii", "jj", "ij"));
        CASE((3, "uu", "vv", "uv"));
        CASE((3, "11", "22", "12"));
        CASE((3, "00", "11", "01"));
        SNPRINTF(defn, sizeof(defn), "{{%s, %s}, {%s, %s}}",
            list2[0], list2[2], list2[2], list2[1]);
    END_CASES(defn, TensorMeshVar);
    BEGIN_CASES; // 2D, cartesian symmetric tensors (upper triangular row-by-row)
        CASE((3, "xx", "xy", "yy"));
        CASE((3, "ii", "ij", "ji"));
        CASE((3, "uu", "uv", "vu"));
        CASE((3, "11", "12", "21"));
        CASE((3, "00", "01", "10"));
        SNPRINTF(defn, sizeof(defn), "{{%s, %s}, {%s, %s}}",
            list2[0], list2[1], list2[1], list2[2]);
    END_CASES(defn, TensorMeshVar);
    BEGIN_CASES; // 2D, cartesian full tensors
        CASE((4, "xx", "xy", "yx", "yy"));
        CASE((4, "ii", "ij", "ji", "jj"));
        CASE((4, "uu", "uv", "vu", "vv"));
        CASE((4, "11", "12", "21", "22"));
        CASE((4, "00", "01", "10", "11"));
        SNPRINTF(defn, sizeof(defn), "{{%s, %s}, {%s, %s}}",
            list2[0], list2[1], list2[2], list2[3]);
    END_CASES(defn, TensorMeshVar);
    BEGIN_CASES; // 3D, cartesian symmetric tensors (Voigt notation order) 
        CASE((6, "xx", "yy", "zz", "yz", "xz", "xy"));
        CASE((6, "ii", "jj", "kk", "jk", "ik", "ij"));
        CASE((6, "uu", "vv", "ww", "vw", "uw", "uv"));
        CASE((6, "11", "22", "33", "23", "13", "12"));
        CASE((6, "00", "11", "22", "12", "02", "01"));
        SNPRINTF(defn, sizeof(defn), "{{%s, %s, %s}, {%s, %s, %s}, {%s, %s, %s}}",
            list2[0], list2[5], list2[4],
            list2[5], list2[1], list2[3],
            list2[4], list2[3], list2[2]);
    END_CASES(defn, TensorMeshVar);
    BEGIN_CASES; // 3D, cartesian symmetric tensors (upper triangular row-by-row)
        CASE((6, "xx", "xy", "xz", "yy", "yz", "zz"));
        CASE((6, "ii", "ij", "ik", "jj", "jk", "kk"));
        CASE((6, "uu", "uv", "uw", "vv", "vw", "ww"));
        CASE((6, "11", "12", "13", "22", "23", "33"));
        CASE((6, "00", "01", "02", "11", "12", "22"));
        SNPRINTF(defn, sizeof(defn), "{{%s, %s, %s}, {%s, %s, %s}, {%s, %s, %s}}",
            list2[0], list2[1], list2[2],
            list2[1], list2[3], list2[4],
            list2[2], list2[4], list2[5]);
    END_CASES(defn, TensorMeshVar);
    BEGIN_CASES; // 3D, cartesian full tensors
        CASE((9, "xx", "xy", "xz", "yx", "yy", "yz", "zx", "zy", "zz"));
        CASE((9, "ii", "ij", "ik", "ji", "jj", "jk", "ki", "kj", "kk"));
        CASE((9, "uu", "uv", "uw", "vu", "vv", "vw", "wu", "wv", "ww"));
        CASE((9, "11", "12", "13", "21", "22", "23", "31", "32", "33"));
        CASE((9, "00", "01", "02", "10", "11", "12", "20", "21", "22"));
        SNPRINTF(defn, sizeof(defn), "{{%s, %s, %s}, {%s, %s, %s}, {%s, %s, %s}}",
            list2[0], list2[1], list2[2],
            list2[3], list2[4], list2[5],
            list2[6], list2[7], list2[8]);
    END_CASES(defn, TensorMeshVar);
    BEGIN_CASES; // 2D symmetric cartesian tensor with out of plane z-component
        CASE((4, "xx", "xy", "yy", "zz"));
        SNPRINTF(defn, sizeof(defn), "{{%s, %s, 0}, {%s, %s, 0}, {0, 0, %s}}",
            list2[0], list2[1], list2[1], list2[2], list2[3]);
    END_CASES(defn, TensorMeshVar);

    return 0;
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
#ifdef HAVE_VTK_SIZEOF___INT64
        case NC_INT64:
        case NC_UINT64:
            return sizeof(int64_t);
#endif
        case NC_FLOAT:
            return sizeof(float);
        case NC_DOUBLE:
            return sizeof(double);
    }
    return 0;
}

// ****************************************************************************
// Handle node order differences between Exodus and VTK for certain cell types
// known to be different.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

#define SWAP_NODES(A,B) {vtkIdType tmp=verts[A]; verts[A]=verts[B]; verts[B]=tmp;}
static bool
InsertExodusCellInVTKUnstructuredGrid(vtkUnstructuredGrid *ugrid, int vtk_celltype, int nnodes, vtkIdType *verts)
{
    bool contains_nonlinear_elems = false;
    switch (vtk_celltype)
    {
        case VTK_TETRA:
        {
            if (nnodes == 8)
                nnodes = 4;
            break;
        }
        case VTK_BIQUADRATIC_QUADRATIC_WEDGE:
        case VTK_QUADRATIC_WEDGE:
        {
            SWAP_NODES(9,12);
            SWAP_NODES(10,13);
            SWAP_NODES(11,14);
            contains_nonlinear_elems = true;
            break;
        }
        case VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON:
        case VTK_QUADRATIC_HEXAHEDRON:
        {
            SWAP_NODES(12,16);
            SWAP_NODES(13,17);
            SWAP_NODES(14,18);
            SWAP_NODES(15,19);
            contains_nonlinear_elems = true;
            break;
        }
        case VTK_TRIQUADRATIC_HEXAHEDRON:
        {
            // mid-edge nodes
            SWAP_NODES(12,16);
            SWAP_NODES(13,17);
            SWAP_NODES(14,18);
            SWAP_NODES(15,19);
            // face nodes
            SWAP_NODES(21,24);
            SWAP_NODES(22,25);
            SWAP_NODES(23,20);
            SWAP_NODES(24,21);
            SWAP_NODES(25,22);
            SWAP_NODES(26,23);
            // volume node
            SWAP_NODES(20,26);
            contains_nonlinear_elems = true;
            break;
        }
    }
    ugrid->InsertNextCell(vtk_celltype, nnodes, verts);
    return contains_nonlinear_elems;
}

// ****************************************************************************
// Main work-horse to read problem sized array data of various flavors.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

static void
GetData(int exncfid, int ts, const char *visit_varname, int numBlocks, avtVarType vt, 
    avtCentering vc, nc_type *rettype, int *retnum_comps, int *retnum_vals, void **retbuf)
{
    int ncerr;
    int i;
    void *buf;
    int num_vals = -1; // TODO: check on fix for uninitialized warning 
    int num_comps = 0;
    nc_type type;
    size_t len = strlen(visit_varname);
    bool isCoord = !strncmp(visit_varname, "coord", 5);
    int exvaridx = -1;
    avtCentering cent = vc==AVT_UNKNOWN_CENT?AVT_NODECENT:vc;

    if (isCoord)
    {
        cent = AVT_NODECENT;
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
            if ((vt == AVT_VECTOR_VAR && !strncmp(visit_varname, node_var_names[i], len)) ||
                (vt == AVT_SCALAR_VAR && !strcmp(visit_varname, node_var_names[i])))
            {
                if (exvaridx == -1)
                {
                    cent = AVT_NODECENT;
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
            char **elem_var_names = GetStringListFromExodusIINCvar(exncfid, "name_elem_var");
            i = 0;
            while (elem_var_names[i])
            {
                if ((vt == AVT_VECTOR_VAR && !strncmp(visit_varname, elem_var_names[i], len)) ||
                    (vt == AVT_SCALAR_VAR && !strcmp(visit_varname, elem_var_names[i])))
                {
                    if (exvaridx < 0)
                    {
                        cent = AVT_ZONECENT;
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
    }

    if (exvaridx < 0)
    {
        // Lets see if this is a raw netcdf variable
        int varid;
        ncerr = nc_inq_varid(exncfid, visit_varname, &varid);
        if (ncerr != NC_NOERR)
            EXCEPTION1(InvalidVariableException, visit_varname);

        int ndims, dimids[NC_MAX_VAR_DIMS];
        ncerr = nc_inq_var(exncfid, varid, 0, &type, &ndims, dimids, 0);
        if (ndims > 3)
        {
            char msg[256];
            SNPRINTF(msg, sizeof(msg), "%s has %d>3 dimensions", visit_varname, ndims);
            EXCEPTION1(InvalidVariableException, msg);
        }

        // Get some dimension information
        int unlimited_dimId, num_nodes_dimId, num_elem_dimId;
        nc_inq_unlimdim(exncfid, &unlimited_dimId);
        nc_inq_dimid(exncfid, "num_nodes", &num_nodes_dimId);
        nc_inq_dimid(exncfid, "num_elem", &num_elem_dimId);
        size_t num_nodes, num_elem;
        nc_inq_dimlen(exncfid, num_nodes_dimId, &num_nodes);
        nc_inq_dimlen(exncfid, num_elem_dimId, &num_elem);

        int ul_dim_idx=-1, num_ents_dim_idx = -1, other_dim_idx=-1; ///TODO: check for fix of uninitialized variable
        num_comps = 1;
        for (i = 0; i < ndims; i++)
        {
            if (dimids[i] == unlimited_dimId)
                ul_dim_idx = i;
            else if (dimids[i] == num_nodes_dimId)
            {
                cent = AVT_NODECENT;
                num_ents_dim_idx = i;
                num_vals = num_nodes;
            }
            else if (dimids[i] == num_elem_dimId)
            {
                cent = AVT_ZONECENT;
                num_ents_dim_idx = i;
                num_vals = num_elem;
            }
            else
            {
                size_t dlen;
                nc_inq_dimlen(exncfid, dimids[i], &dlen);
                num_comps = (int) dlen;
                other_dim_idx = i;
            }
        }

        size_t starts[3], counts[3];
        if (ul_dim_idx != -1)
        {
            starts[ul_dim_idx] = ts;
            counts[ul_dim_idx] = 1;
        }
        if (other_dim_idx != -1)
        {
            starts[other_dim_idx] = 0;
            counts[other_dim_idx] = num_comps;
        }
        starts[num_ents_dim_idx] = 0;
        counts[num_ents_dim_idx] = num_vals;

        buf = (void*) malloc(num_comps * num_vals * SizeOfNCType(type));
        ncerr = nc_get_vara(exncfid, varid, starts, counts, buf);
        CheckNCError2(ncerr, nc_get_vara, __LINE__, __FILE__)      
        if (ncerr != NC_NOERR)
        {
            free(buf);
            EXCEPTION1(InvalidVariableException, visit_varname);
        }

    }
    else if (cent == AVT_NODECENT)
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
        buf = 0;
        type = 0;
        for (int pass = 0; pass < 2; pass++)
        {
            int ndims, dimids[NC_MAX_VAR_DIMS];
            int num_elem = 0;
            size_t dlen;
            char *p = (char *) buf; // p not relevant on first pass
            for (int i = 0; i < numBlocks; i++)
            {
                int elem_varid;
                char elem_varname[NC_MAX_NAME+1];
//warning THIS LOGIC DOESNT WORK FOR ZONAL VARS WITH MULTIPLE COMPONENTS
                SNPRINTF(elem_varname, sizeof(elem_varname), "vals_elem_var%deb%d", exvaridx+1, i+1);
                ncerr = nc_inq_varid(exncfid, elem_varname, &elem_varid);
                if (ncerr != NC_NOERR) // handle no-var on this eb case
                {
                    char num_el_in_blk_dimname[NC_MAX_NAME+1];
                    SNPRINTF(num_el_in_blk_dimname, sizeof(num_el_in_blk_dimname), "num_el_in_blk%d", i+1);
                    int num_el_in_blk_dimId;
                    ncerr = nc_inq_dimid(exncfid, num_el_in_blk_dimname, &num_el_in_blk_dimId);
                    if (ncerr == NC_NOERR) // handle no eb on this proc case.
                    {
                        size_t num_el_in_blk_len;
                        nc_inq_dimlen(exncfid, num_el_in_blk_dimId, &num_el_in_blk_len);
                        int num_elems_in_blk = (int) num_el_in_blk_len;
                        dlen = num_elems_in_blk;
                        num_elem += (int) dlen;
                        if (pass == 1)
                            p += (((int) dlen) * SizeOfNCType(type));
                    }
                }
                else
                {
                    nc_inq_var(exncfid, elem_varid, 0, &type, &ndims, dimids, 0);
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
            }

            if (pass == 0)
            {
                if (num_elem == 0 || type == 0)
                    EXCEPTION1(InvalidVariableException, visit_varname);
                num_vals = num_elem;
                buf = (void*) calloc(num_comps * num_elem, SizeOfNCType(type));
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


// ****************************************************************************
// Read the identifiers (e.g. numerical names) for various sets identified
// in the Exodus file.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

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

// ****************************************************************************
// Read an Exodus variable known to contain subset ids (nodesets, sidesets,
// etc.) and return a vtkBitArray for use in an enum scalar variable.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

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

        char tmp[32], tmp1[32];
        if (stype == "node")
            SNPRINTF(tmp, sizeof(tmp), "node_ns%d", i+1);
        else
        {
            SNPRINTF(tmp,  sizeof(tmp), "elem_ss%d", i+1);
            SNPRINTF(tmp1, sizeof(tmp1), "side_ss%d", i+1);
        }
 
        int setlist_varid, sidelist_varid;
        ncerr = nc_inq_varid(exncfid, tmp, &setlist_varid);
        if (ncerr != NC_NOERR) continue;
        if (stype == "side")
        {
            ncerr = nc_inq_varid(exncfid, tmp1, &sidelist_varid);
            if (ncerr != NC_NOERR) continue;
        }

        int dimids[NC_MAX_VAR_DIMS];
        ncerr = nc_inq_var(exncfid, setlist_varid, 0, &type, &ndims, dimids, 0);
        if (ncerr != NC_NOERR) continue;
//warning WHAT ABOUT 64-BIT INTEGER TYPE
        if (type != NC_INT || ndims != 1) continue;

        size_t dlen;
        ncerr = nc_inq_dimlen(exncfid, dimids[0], &dlen);
        if (ncerr != NC_NOERR) continue;
        if (!dlen) continue;

        int *setids = new int[dlen], *sideids = 0;
        ncerr = nc_get_var_int(exncfid, setlist_varid, setids);
        if (ncerr != NC_NOERR)
        {
            delete [] setids;
            continue;
        }
        if (stype == "side")
        {
            sideids = new int[dlen];
            ncerr = nc_get_var_int(exncfid, sidelist_varid, sideids);
            if (ncerr != NC_NOERR)
            {
                delete [] setids;
                delete [] sideids;
                continue;
            }

            // Handling of side sets is in-complete. Need to get connectivity
            // and iterate over elems in connects using sideids for a 'side' of
            // an elem.
        }

        for (int j = 0; j < (int) dlen; j++)
            retval->SetComponent(setids[j]-1, i, 1);

        delete [] setids;
        if (sideids) delete [] sideids;
    }

    delete [] status;

    return retval;
}

// ****************************************************************************
// Template to read integer block ids as int or int64
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

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

// ****************************************************************************
// Convenient shorter name for instancing a block id read
// ****************************************************************************

#define READ_BLOCK_IDS(F,V,N,BIDS,TYPE) ReadBlockIds<TYPE>(F,V,N,BIDS,nc_get_var_ ## TYPE);

// ****************************************************************************
// Reads a string-valued variables and attributes from the netcdf file known
// to contain Exodus element block names and ids.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

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
#ifdef HAVE_VTK_SIZEOF___INT64
        case NC_INT64: READ_BLOCK_IDS(ncExIIId, eb_blockid_varid, numBlocks, blockId, int64_t); break;
#endif
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

    return (int)globalFileLists->size()-1;
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
//    Mark C. Miller, Fri Dec 19 11:14:09 PST 2014
//    Added options for material conventions and automagic composite variable
//    detection.
// ****************************************************************************

avtExodusFileFormat::avtExodusFileFormat(const char *name, DBOptionsAttributes *rdatts)
   : avtMTSDFileFormat(&name, 1)
{
    fileList = -1;
    ncExIIId = -1;
    numBlocks = -1;
    numNodes = -1;
    numElems = -1;
    autoDetectCompoundVars = false;
    matConvention = None;
    matCount = -1;

    // Make a pass through read options but skip material nameschemes for now.
    for (int i = 0; rdatts != 0 && i < rdatts->GetNumberOfOptions(); ++i)
    {
        if      (rdatts->GetName(i) == EXODUS_DETECT_COMPOUND_VARS)
            autoDetectCompoundVars = rdatts->GetBool(EXODUS_DETECT_COMPOUND_VARS) ? 1 : 0;
        else if (rdatts->GetName(i) == EXODUS_MATERIAL_COUNT)
            matCount = rdatts->GetInt(EXODUS_MATERIAL_COUNT);
        else if (rdatts->GetName(i) == EXODUS_MATERIAL_CONVENTION)
            matConvention = rdatts->GetEnum(EXODUS_MATERIAL_CONVENTION);
        else if (rdatts->GetName(i) == EXODUS_VOLFRAC_NAMESCHEME)
            continue;
        else if (rdatts->GetName(i) == EXODUS_MATSPEC_NAMESCHEME)
            continue;
        else
            debug1 << "Ignoring unknown option \"" << rdatts->GetName(i) << "\"" << endl;
    }

    // Based on specified convention, set nameschemes
    switch (matConvention)
    {
        case Custom: // Get var nameschemes from read options
        {
            for (int i = 0; rdatts != 0 && i < rdatts->GetNumberOfOptions(); ++i)
            {
                if      (rdatts->GetName(i) == EXODUS_VOLFRAC_NAMESCHEME)
                    matVolFracNamescheme = rdatts->GetString(EXODUS_VOLFRAC_NAMESCHEME);
                else if (rdatts->GetName(i) == EXODUS_MATSPEC_NAMESCHEME)
                    matVarSpecNamescheme = rdatts->GetString(EXODUS_MATSPEC_NAMESCHEME);
            }
            break;
        }
        case Alegra: // Use Alegra's conventions
        {
            matVolFracNamescheme = EXODUS_VOLFRAC_NAMESCHEME_ALEGRA;
            matVarSpecNamescheme = EXODUS_MATSPEC_NAMESCHEME_ALEGRA;
            break;
        }
        case CTH: // Use CTH's conventions
        {
            matVolFracNamescheme = EXODUS_VOLFRAC_NAMESCHEME_CTH;
            matVarSpecNamescheme = EXODUS_MATSPEC_NAMESCHEME_CTH;
            break;
        }
        default: break;
    }
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
//  Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
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
            for (size_t i = 0; i < len; i++)
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

// ****************************************************************************
// Template for extending a vtkDataArray with additional components of zeros
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

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

// ****************************************************************************
// Convenient name for ExtendCoordsTemplate
// ****************************************************************************
#define ECT(T,t) ExtendCoordsTemplate<vtk ## T ## Array, t>((vtk ## T ## Array *)rhs)

// ****************************************************************************
// Takes a single vtkDataArray of any type that may have only 2-tuples
// (e.g. xy) and turns it into a 3-component vtkDataArray with 3rd component
// zero.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

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

// ****************************************************************************
// Template to compose vtkDataArrays
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

template <class T, typename N>
static T* ComposeUpTo3ArraysTemplate(int narrs, T *rhsx, T *rhsy, T* rhsz)
{
    T* retval = T::New();
    retval->SetNumberOfComponents(narrs);
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

// ****************************************************************************
// A convenient macro-name to call ComposeUpTo3ArraysTemplate
// ****************************************************************************
#define CCT(N,T,t) ComposeUpTo3ArraysTemplate<vtk ## T ## Array, t>(N,(vtk ## T ## Array *)rhsx,(vtk ## T ## Array *)rhsy,(vtk ## T ## Array *)rhsz)

// ****************************************************************************
// Takes as many as 3 individual vtkDataArrays of any type and combines them
// into a single vtkDataArray (typically used to compose vectors -- coords).
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
// ****************************************************************************

static vtkDataArray * ComposeCoords(vtkDataArray *rhsx, vtkDataArray *rhsy, vtkDataArray *rhsz)
{
    switch (rhsx->GetDataType())
    {
        case VTK_CHAR:           return CCT(3,Char,char);
        case VTK_UNSIGNED_CHAR:  return CCT(3,UnsignedChar,unsigned char);
        case VTK_SHORT:          return CCT(3,Short,short);
        case VTK_UNSIGNED_SHORT: return CCT(3,UnsignedShort,unsigned short);
        case VTK_INT:            return CCT(3,Int,int);
        case VTK_UNSIGNED_INT:   return CCT(3,UnsignedInt,unsigned int);
        case VTK_LONG_LONG:      return CCT(3,LongLong,long long);
        case VTK_FLOAT:          return CCT(3,Float,float);
        case VTK_DOUBLE:         return CCT(3,Double,double);
    }
    return NULL;
}

// ****************************************************************************
// Examine Exodus strings encoding element type and together with that and
// knowledge of node count, and number of spatial dimensions, make a good
// guess regarding the topological dimension of the type and its assoc.
// VTK cell type.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:05:11 PST 2014
//
//  Modifications:
//    Kathleen Biagas, Tue Sep 10 16:06:30 PDT 2013
//    Add 'num_nodes' argument and use it to determine if cell types are
//    quadratic.  Borrowing from vtk's reader, reduced complexity by only 
//    comparing first 3 chars of ex_elem_type_att in most instances, and 
//    supporting more exodus types.  Set vtk_celltype to -1 for types that
//    VisIt may not yet support. 
//
// ****************************************************************************

static void 
DecodeExodusElemTypeAttText(const char *ex_elem_type_att, int *tdim, int *vtk_celltype, int num_nodes, int num_spatial_dims)
{
    std::string elemType(ex_elem_type_att);
    std::transform(elemType.begin(), elemType.end(), elemType.begin(), ::tolower);    
    if (elemType.substr(0, 3) == "cir")
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype) *vtk_celltype = VTK_VERTEX;
        return;
    }
    else if (elemType.substr(0, 3) == "sph")
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype) *vtk_celltype = VTK_VERTEX;
        return;
    }
    else if (elemType.substr(0, 3) == "qua")
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype) 
        {
            if (4 == num_nodes)
                *vtk_celltype = VTK_QUAD;
            else if (8 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_QUAD;
            else if (9 == num_nodes)
                *vtk_celltype = VTK_BIQUADRATIC_QUAD;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "tri")
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype)
        {
            if (3 == num_nodes)
                *vtk_celltype = VTK_TRIANGLE;
            else if (6 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_TRIANGLE;
        }
        return;
    }
    else if (elemType.substr(0, 4) == "nsid")
    {
        if (num_spatial_dims == 2)
        {
            if (tdim) *tdim = 2;
            if (vtk_celltype) *vtk_celltype = VTK_POLYGON;
        }
        else if (num_spatial_dims == 3)
        {
            if (tdim) *tdim = 3;
            if (vtk_celltype) *vtk_celltype = VTK_POLYHEDRON;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "she")
    {
        if (tdim) *tdim = 2;
        if (vtk_celltype)
        {
            if (3 == num_nodes)
                *vtk_celltype = VTK_TRIANGLE;
            else if (4 == num_nodes)
                *vtk_celltype = VTK_QUAD;
            else if (8 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_QUAD;
            else if (8 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_QUAD;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "hex")
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype)
        {
            if (8 == num_nodes)
                *vtk_celltype = VTK_HEXAHEDRON;
            else if (20 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_HEXAHEDRON;
            else if (21 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_HEXAHEDRON;
            else if (27 == num_nodes)
                *vtk_celltype = VTK_TRIQUADRATIC_HEXAHEDRON;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "tet")
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype)
        {
            if (4 == num_nodes)
                *vtk_celltype = VTK_TETRA;
            else if (8 == num_nodes) // no VTK support for tet8
                *vtk_celltype = VTK_TETRA;
            else if (10 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_TETRA;
            else if (11 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_TETRA;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "wed")
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype)
        {
            if (6 == num_nodes)
                *vtk_celltype = VTK_WEDGE;
            else if (15 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_WEDGE;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "pyr")
    {
        if (tdim) *tdim = 3;
        if (vtk_celltype)
        {
            if (5 == num_nodes)
                *vtk_celltype = VTK_PYRAMID;
            else if (13 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_PYRAMID;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "tru")
    {
        if (tdim) *tdim = 1;
        if (vtk_celltype)
        {
            if (2 == num_nodes)
                *vtk_celltype = VTK_LINE;
            else if (3 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_EDGE;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "bea")
    {
        if (tdim) *tdim = 1;
        if (vtk_celltype)
        {
            if (2 == num_nodes)
                *vtk_celltype = VTK_LINE;
            else if (3 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_EDGE;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "bar")
    {
        if (tdim) *tdim = 1;
        if (vtk_celltype)
        {
            if (2 == num_nodes)
                *vtk_celltype = VTK_LINE;
            else if (3 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_EDGE;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "edg")
    {
        if (tdim) *tdim = 1;
        if (vtk_celltype)
        {
            if (2 == num_nodes)
                *vtk_celltype = VTK_LINE;
            else if (3 == num_nodes)
                *vtk_celltype = VTK_QUADRATIC_EDGE;
        }
        return;
    }
    else if (elemType.substr(0, 8) == "straight")
    {
        if (tdim) *tdim = 1;
        if (vtk_celltype)
        {
            if (2 == num_nodes)
                *vtk_celltype = VTK_LINE;
        }
        return;
    }
    else if (elemType.substr(0, 3) == "sup")
    {
        if (tdim) *tdim = 1;
        if (vtk_celltype)
            *vtk_celltype = VTK_POLY_VERTEX;
        return;
    }
    else if (elemType.substr(0, 4) == "null")
    {
        if (tdim) *tdim = 0;
        if (vtk_celltype) *vtk_celltype = VTK_EMPTY_CELL;
        return;
    }
    if (tdim) *tdim = -1;
    if (vtk_celltype) *vtk_celltype = VTK_EMPTY_CELL;
}

static int
ExodusElemTypeAtt2TopoDim(const char *ex_elem_type_att, int spatial_dim)
{
    int tdim;
    DecodeExodusElemTypeAttText(ex_elem_type_att, &tdim, 0, 0, spatial_dim);
    debug5 << "For element type attribute text \"" << ex_elem_type_att << "\""
           << ", got topo dim = " << tdim << endl;
    return tdim;
}

// ****************************************************************************
//  Modifications:
//    Kathleen Biagas, Tue Sep 10 16:06:30 PDT 2013
//    Add 'num_nodes' argument and pass it to DecodeExodusElemTypeAttText.
//
// ****************************************************************************

static int
ExodusElemTypeAtt2VTKCellType(const char *ex_elem_type_att, int num_nodes, int num_spatial_dims)
{
    int ctype = -1;
    DecodeExodusElemTypeAttText(ex_elem_type_att, 0, &ctype, num_nodes, num_spatial_dims);
    debug5 << "For element type attribute text \"" << ex_elem_type_att << "\" "
           << "and num_nodes = " << num_nodes << ", got vtk_celltype = " << ctype << endl;
    return ctype;
}

// ****************************************************************************
// Add a variable making smart guesses about the variable's type.
//
// Programmer: Mark C. Miller, Fri Dec 19 11:04:16 PST 2014
// ****************************************************************************

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
//  Programmer: Mark C. Miller, Fri Dec 19 11:02:07 PST 2014
//
//  Modifications:
//
//    Mark C. Miller, Tue Sep 17 16:04:32 PDT 2013
//    Add logic to populate *both* composite variables like vectors and
//    tensors as well as scalar components.
//
//    Mark C. Miller, Fri Dec 19 11:02:32 PST 2014
//    Adjust logic for composite variables to be smarter about combining
//    related strings into composite variables and also to define expressions
//    instead of just variables the plugin is responsible for serving up.
// ****************************************************************************

#ifdef MAX
#undef MAX
#endif
#define MAX(A,B) ((A)>(B)?(A):(B))

void
avtExodusFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    int   i;

    GetFileHandle();

    // Acquire spatial dimension information
    int num_dim_dimId;
    VisItNCErr = nc_inq_dimid(ncExIIId, "num_dim", &num_dim_dimId);
    if (VisItNCErr != NC_NOERR) return;

    size_t num_dim_len;
    VisItNCErr = nc_inq_dimlen(ncExIIId, num_dim_dimId, &num_dim_len);
    CheckNCError(nc_inq_dimlen);
    int spatialDimension = (int) num_dim_len;

    // Acquire topological dimension information (harder because we
    // need to inspect NC Attributes named "elem_type" on all "connectX"
    // datasets and take the max.
    int num_el_blk_dimId;
    VisItNCErr = nc_inq_dimid(ncExIIId, "num_el_blk", &num_el_blk_dimId);
    if (VisItNCErr != NC_NOERR) return;

    size_t num_el_blk_len;
    VisItNCErr = nc_inq_dimlen(ncExIIId, num_el_blk_dimId, &num_el_blk_len);
    CheckNCError(nc_inq_dimlen);

    int topologicalDimension = -1;
    numBlocks = (int) num_el_blk_len;
    for (size_t i = 0; i < num_el_blk_len; i++)
    {
        int connect_varid;
        char connect_varname[NC_MAX_NAME+1];
        SNPRINTF(connect_varname, sizeof(connect_varname), "connect%d", (int)i+1);
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
            ExodusElemTypeAtt2TopoDim(connect_elem_type_attval, spatialDimension));
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
        (fileList < (int)globalFileLists->size()))
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
    // Previous version of the Exodus plugin used a material object
    // to 'spoof' Exodus element blocks. However, that prevents the plugin
    // from then properly handling material data in Exodus files such as
    // codes like Alegra produce. So, here, we handle the element block 
    // decomposition as an enumerated scalar variable.
    //
    GetElementBlockNamesAndIds(ncExIIId, numBlocks, blockName, blockId);
    avtScalarMetaData *ebsmd = new avtScalarMetaData("ElementBlock", meshName, AVT_ZONECENT);
    ebsmd->SetEnumerationType(avtScalarMetaData::ByValue);
    for (i = 0 ; i < numBlocks ; i++)
    {
        char name[256];
        if (blockName[0] == "")
            SNPRINTF(name, sizeof(name), "%d", blockId[i]);
        else
            SNPRINTF(name, sizeof(name), "%s_%d", blockName[i].c_str(), blockId[i]);
        ebsmd->AddEnumNameValue(name, i+1);
    }
    md->Add(ebsmd);

    // 
    // Examine element variables names for those matching material namescheme conventions
    //
    if (matConvention != None)
    {
        bool const do_sort = false;
        char **elem_var_names = GetStringListFromExodusIINCvar(ncExIIId, "name_elem_var", do_sort);

        // First, look for volume fraction variable names until we cannot find any more
        Namescheme vfns(matVolFracNamescheme.c_str());
        int cur_mat_num = 0;
        while (true)
        {
            char const *cur_vfrac_varname = vfns.GetName(cur_mat_num);
            i = 0;
            bool found_it = false;
            while (elem_var_names[i])
            {
                if (!strcmp(cur_vfrac_varname, elem_var_names[i]))
                {
                    cur_mat_num++;
                    found_it = true;
                    break;
                }
                i++;
            }
            if (!found_it)
                break;
        }
        if (matCount == -1)
        {
            matCount = cur_mat_num;
            debug1 << "Guessed " << matCount << " materials in this database." << endl;
            debug4 << "Volume fraction variables found..." << endl;
            for (int m = 0; m < matCount; m++)
                debug4 << "    \"" << vfns.GetName(m) << "\"" << endl;
        }
        else if (matCount != cur_mat_num)
        {
            debug1 << "User specified material count of " << matCount
                   << " doesn't match number of volume fraction variables found " << cur_mat_num << "." << endl;
            debug1 << "Further attempts to configure materials for this database will be skipped." << endl;
            matConvention = None;
        }

        if (matConvention != None && matCount > 0)
        {
            vector<string> matnames;
            for (int m = 0; m < matCount; m++)
            {
                char tmpstr[8];
                SNPRINTF(tmpstr, sizeof(tmpstr), "%d", m);
                matnames.push_back(tmpstr);
            }
            avtMaterialMetaData *mmd = new avtMaterialMetaData("Materials", "Mesh", matCount, matnames);
            md->Add(mmd);
        }

        FreeStringListFromExodusIINCvar(elem_var_names);
        vfns.GetName(-1); // clear static circ-buff (non-essential)
    }

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

    bool const do_sort = true;
    char **node_var_names = GetStringListFromExodusIINCvar(ncExIIId, "name_nod_var", do_sort);
    i = 0;
    int skip_composite = -1;
    while (node_var_names[i])
    {
        if (i > skip_composite)
        {
            int ncomps;
            if (autoDetectCompoundVars &&
                AreSuccessiveStringsRelatedComponentNames(md, node_var_names, i,
                    spatialDimension, matCount, &ncomps))
            {
                skip_composite = i + ncomps - 1;
            }
        }

        // Add the scalar (component) variable.
        AddVar(md, node_var_names[i], topologicalDimension, 1, AVT_NODECENT);
        i++;
    }
    FreeStringListFromExodusIINCvar(node_var_names);

    char **elem_var_names = GetStringListFromExodusIINCvar(ncExIIId, "name_elem_var", do_sort);
    i = 0;
    skip_composite = -1;
    while (elem_var_names[i])
    {
        if (i > skip_composite)
        {
            int ncomps;
            if (autoDetectCompoundVars && 
                AreSuccessiveStringsRelatedComponentNames(md, elem_var_names, i,
                    spatialDimension, matCount, &ncomps))
            {
                skip_composite = i + ncomps - 1;
            }
        }

        // Add the scalar (component) variable.
        AddVar(md, elem_var_names[i], topologicalDimension, 1, AVT_ZONECENT);
        i++;
    }
    FreeStringListFromExodusIINCvar(elem_var_names);

    int nvars;
    VisItNCErr = nc_inq(ncExIIId, 0, &nvars, 0, 0);
    CheckNCError(nc_inq);

    int unlimited_dimId;
    nc_inq_unlimdim(ncExIIId, &unlimited_dimId);
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
            if (vdimids[j] == unlimited_dimId)
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

        const bool do_expr = false;
        if (md->DetermineVarType(vname, do_expr) == AVT_UNKNOWN_TYPE)
            AddVar(md, vname, topologicalDimension, ncomps, centering);
    }

    //
    // Add exodus nodsets, if any exist, as enumerated scalars
    //
    vector<int> nsids;
    GetExodusSetIDs(ncExIIId, "node", nsids);
    if (nsids.size())
    {
        avtScalarMetaData *smd = new avtScalarMetaData("Nodesets", "Mesh", AVT_NODECENT);
        for (size_t i = 0; i < nsids.size(); i++)
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

    // This is presently disabled because we don't have logic to paint a nodal
    // var from side ids yet.
#if 0
    //
    // Add exodus sidesets, if any exist, as enumerated scalars
    //
    vector<int> ssids;
    GetExodusSetIDs(ncExIIId, "side", ssids);
    if (ssids.size())
    {
        avtScalarMetaData *smd = new avtScalarMetaData("Sidesets", "Mesh", AVT_NODECENT);
        for (size_t i = 0; i < ssids.size(); i++)
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
#endif
}

// ****************************************************************************
//  Method: avtExodusFileFormat::GetMesh
//
//  Programmer: Mark C. Miller, Fri Dec 19 11:00:59 PST 2014
//
//  Modifications:
//
//    Mark C. Miller, Thu Jun 27 10:13:46 PDT 2013
//    Removed logic to automagically add displacements.
//
//    Kathleen Biagas, Mon Sep  9 15:51:17 PDT 2013
//    Use num_nodes_per_elem when determining vtk_celltype so quadratic cells
//    are handled correctly.  Change 'verts' size to 20 to handle quadratic
//    hexes. 
//
//    Mark C. Miller, Thu Sep 26 12:01:01 PDT 2013
//    Fixed assumption that length of connect variable was always dimension 0.
// ****************************************************************************

vtkDataSet *
avtExodusFileFormat::GetMesh(int ts, const char *mesh)
{
    GetFileHandle();

    CopyFromZeroIndexInstance();

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
    int num_spatial_dims = -1;
    TRY
    {
        coords = GetVectorVar(ts, "coord");
        num_spatial_dims = coords->GetNumberOfComponents();
    }
    CATCH(InvalidVariableException)
    {
        num_spatial_dims = 3;
        vtkDataArray *coordx = GetVar(ts, "coordx");
        vtkDataArray *coordy = 0;
        vtkDataArray *coordz = 0;
        TRY { coordy = GetVar(ts, "coordy"); } CATCH(InvalidVariableException) { num_spatial_dims--;} ENDTRY
        TRY { coordz = GetVar(ts, "coordz"); } CATCH(InvalidVariableException) { num_spatial_dims--;} ENDTRY
        coords = ComposeCoords(coordx, coordy, coordz);
        coordx->Delete();
        if (coordy) coordy->Delete();
        if (coordz) coordz->Delete();
    }
    ENDTRY

    if (num_spatial_dims == 2)
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

        bool arb3D = false;
        if (VisItNCErr != NC_NOERR)
        {
            // Check for possible arbitrary polyehdra connectivities

            int nodelist_varid;
            char nodelist_varname[NC_MAX_NAME+1];
            SNPRINTF(nodelist_varname, sizeof(nodelist_varname), "fbconn%d", i+1);
            VisItNCErr = nc_inq_varid(ncExIIId, nodelist_varname, &nodelist_varid);
            if (VisItNCErr != NC_NOERR) continue;

            char nodecnts_varname[NC_MAX_NAME+1];
            int nodecnts_varid;
            SNPRINTF(nodecnts_varname, sizeof(nodecnts_varname), "fbepecnt%d", i+1);
            VisItNCErr = nc_inq_varid(ncExIIId, nodecnts_varname, &nodecnts_varid);
            if (VisItNCErr != NC_NOERR) continue;

            char facelist_varname[NC_MAX_NAME+1];
            int facelist_varid;
            SNPRINTF(facelist_varname, sizeof(facelist_varname), "facconn%d", i+1);
            VisItNCErr = nc_inq_varid(ncExIIId, facelist_varname, &facelist_varid);
            if (VisItNCErr != NC_NOERR) continue;

            char facecnts_varname[NC_MAX_NAME+1];
            int facecnts_varid;
            SNPRINTF(facecnts_varname, sizeof(facecnts_varname), "ebepecnt%d", i+1);
            VisItNCErr = nc_inq_varid(ncExIIId, facecnts_varname, &facecnts_varid);
            if (VisItNCErr != NC_NOERR) continue;

            char num_el_in_blk_dimname[NC_MAX_NAME+1];
            SNPRINTF(num_el_in_blk_dimname, sizeof(num_el_in_blk_dimname), "num_el_in_blk%d", i+1);
            int num_el_in_blk_dimId;
            VisItNCErr = nc_inq_dimid(ncExIIId, num_el_in_blk_dimname, &num_el_in_blk_dimId);
            CheckNCError(nc_inq_dimid);
            size_t num_el_in_blk_len;
            VisItNCErr = nc_inq_dimlen(ncExIIId, num_el_in_blk_dimId, &num_el_in_blk_len);
            CheckNCError(nc_inq_dimlen);
            int num_elems_in_blk = (int) num_el_in_blk_len;

            char num_fa_in_blk_dimname[NC_MAX_NAME+1];
            SNPRINTF(num_fa_in_blk_dimname, sizeof(num_fa_in_blk_dimname), "num_fa_in_blk%d", i+1);
            int num_fa_in_blk_dimId;
            VisItNCErr = nc_inq_dimid(ncExIIId, num_fa_in_blk_dimname, &num_fa_in_blk_dimId);
            CheckNCError(nc_inq_dimid);
            size_t num_fa_in_blk_len;
            VisItNCErr = nc_inq_dimlen(ncExIIId, num_fa_in_blk_dimId, &num_fa_in_blk_len);
            CheckNCError(nc_inq_dimlen);
            int num_faces_in_blk = (int) num_fa_in_blk_len;

            // Read the face counts array
            int *facecnts_buf = new int[num_elems_in_blk];
            VisItNCErr = nc_get_var_int(ncExIIId, facecnts_varid, facecnts_buf);
            CheckNCError(nc_get_var_int);
            if (VisItNCErr != NC_NOERR)
            {
                char msg[256];
                SNPRINTF(msg, sizeof(msg), "Unable to read ebepecnt%d: \"%s\"", i+1, nc_strerror(VisItNCErr));
                delete [] facecnts_buf;
                EXCEPTION1(InvalidFilesException, msg);
            }

            // Read the facelist array which enumerates the face ids comprising each elem
            int facelist_len = 0;
            for (int j = 0; j < num_elems_in_blk; j++)
                facelist_len += facecnts_buf[j];
            int *facelist_buf = new int[facelist_len];
            VisItNCErr = nc_get_var_int(ncExIIId, facelist_varid, facelist_buf);
            CheckNCError(nc_get_var_int);
            if (VisItNCErr != NC_NOERR)
            {
                char msg[256];
                SNPRINTF(msg, sizeof(msg), "Unable to read faconn%d: \"%s\"", i+1, nc_strerror(VisItNCErr));
                delete [] facecnts_buf;
                delete [] facelist_buf;
                EXCEPTION1(InvalidFilesException, msg);
            }

            // Read the node counts array
            int *nodecnts_buf = new int[num_faces_in_blk];
            VisItNCErr = nc_get_var_int(ncExIIId, nodecnts_varid, nodecnts_buf);
            CheckNCError(nc_get_var_int);
            if (VisItNCErr != NC_NOERR)
            {
                char msg[256];
                SNPRINTF(msg, sizeof(msg), "Unable to read fbepecnt%d: \"%s\"", i+1, nc_strerror(VisItNCErr));
                delete [] facecnts_buf;
                delete [] facelist_buf;
                delete [] nodecnts_buf;
                EXCEPTION1(InvalidFilesException, msg);
            }

            int nodelist_len = 0;
            int *nodelist_offsets = new int[num_faces_in_blk];
            for (int j = 0; j < num_faces_in_blk; j++)
            {
                nodelist_offsets[j] = nodelist_len;
                nodelist_len += nodecnts_buf[j];
            }
            int *nodelist_buf = new int[nodelist_len];
            VisItNCErr = nc_get_var_int(ncExIIId, nodelist_varid, nodelist_buf);
            CheckNCError(nc_get_var_int);
            if (VisItNCErr != NC_NOERR)
            {
                char msg[256];
                SNPRINTF(msg, sizeof(msg), "Unable to read fbconn%d: \"%s\"", i+1, nc_strerror(VisItNCErr));
                delete [] facecnts_buf;
                delete [] facelist_buf;
                delete [] nodecnts_buf;
                delete [] nodelist_offsets;
                delete [] nodelist_buf;
                EXCEPTION1(InvalidFilesException, msg);
            }

            // Ok, now loop to add all the arbitrary polyedra
            int facelist_offset = 0;
            for (int ii = 0; ii < num_elems_in_blk; ii++)
            {
                vtkIdType cellarr_buf[1024];
                int const cellarr_buflen = (int) sizeof(cellarr_buf)/sizeof(cellarr_buf[0]);
                int qq = 0;
                int nfaces = facecnts_buf[ii];
                for (int jj = 0; jj < nfaces; jj++)
                {
                    int exface_id = facelist_buf[facelist_offset+jj];
                    int nodecnt = nodecnts_buf[exface_id-1];
                    int nodelist_offset = nodelist_offsets[exface_id-1];
                    cellarr_buf[qq++] = nodecnt;
                    for (int kk = 0; kk < nodecnt && qq < cellarr_buflen; kk++)
                        cellarr_buf[qq++] = nodelist_buf[nodelist_offset+kk]-1; 
                }
                ugrid->InsertNextCell(VTK_POLYHEDRON, nfaces, cellarr_buf);
                facelist_offset += nfaces;
            }

            // cleanup
            delete [] nodelist_buf;
            delete [] nodecnts_buf;
            delete [] nodelist_offsets;
            delete [] facelist_buf;
            delete [] facecnts_buf;
 
            arb3D = true;
            continue;
        }

        int connect_vardimids[NC_MAX_VAR_DIMS];
        nc_inq_var(ncExIIId, connect_varid, 0, 0, 0, connect_vardimids, 0);
        size_t connect_dim0varlen;
        VisItNCErr = nc_inq_dimlen(ncExIIId, connect_vardimids[0], &connect_dim0varlen);
        CheckNCError(nc_inq_dimlen);
        nc_type connect_vartype;
        VisItNCErr = nc_inq_vartype(ncExIIId, connect_varid, &connect_vartype);
        CheckNCError(nc_inq_varid);

        char *connect_elem_type_attval = new char[256];
        memset(connect_elem_type_attval, 0, 256);
        VisItNCErr = nc_get_att_text(ncExIIId, connect_varid, "elem_type", connect_elem_type_attval);
        if (VisItNCErr != NC_NOERR)
            strcpy(connect_elem_type_attval, "unknown");

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
        int vtk_celltype = ExodusElemTypeAtt2VTKCellType(connect_elem_type_attval, num_nodes_per_elem, num_spatial_dims);
        delete [] connect_elem_type_attval;

        if (vtk_celltype == VTK_POLYHEDRON && !arb3D)
            vtk_celltype = VTK_POLYGON;

        int *ebepecnt_buf = 0;
        int connect_varlen = num_elems_in_blk * num_nodes_per_elem;
        if (vtk_celltype == VTK_POLYGON)
        {
            int ebepecnt_varid;
            char ebepecnt_varname[NC_MAX_NAME+1];
            SNPRINTF(ebepecnt_varname, sizeof(ebepecnt_varname), "ebepecnt%d", i+1);
            VisItNCErr = nc_inq_varid(ncExIIId, ebepecnt_varname, &ebepecnt_varid);
            if (VisItNCErr != NC_NOERR)
            {
                char msg[256];
                SNPRINTF(msg, sizeof(msg), "Unable to get var id for ebepecnt%d: \"%s\"", i+1, nc_strerror(VisItNCErr));
                EXCEPTION1(InvalidFilesException, msg);
            }

            ebepecnt_buf = new int[num_elems_in_blk];
            VisItNCErr = nc_get_var_int(ncExIIId, ebepecnt_varid, ebepecnt_buf);
            CheckNCError(nc_get_var_int);
            if (VisItNCErr != NC_NOERR)
            {
                char msg[256];
                SNPRINTF(msg, sizeof(msg), "Unable to read ebepecnt%d: \"%s\"", i+1, nc_strerror(VisItNCErr));
                EXCEPTION1(InvalidFilesException, msg);
            }

            connect_varlen = connect_dim0varlen;
        }

        blockIdToMatMap[i+1] = num_elems_in_blk;

        // Note: for poly-case, num_nodes_per_elem should be size of entire connect array
        bool contains_nonlinear_elems = false;
        vtkIdType *verts = new vtkIdType[num_nodes_per_elem];
        switch (connect_vartype)
        {
            case NC_INT:
            {
                int *conn_buf = new int[connect_varlen];
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
                    int nnodes = ebepecnt_buf?ebepecnt_buf[j]:num_nodes_per_elem;
                    for (int k = 0; k < nnodes; k++, p++)
                        verts[k] = (vtkIdType) *p-1; // Exodus is 1-origin
                    if (InsertExodusCellInVTKUnstructuredGrid(ugrid, vtk_celltype, nnodes, verts))
                        contains_nonlinear_elems = true;
                }
                delete [] conn_buf;
                break;
            }
            case NC_INT64:
            {
                long long *conn_buf = new long long[connect_varlen];
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
                    int nnodes = ebepecnt_buf?ebepecnt_buf[j]:num_nodes_per_elem;
                    for (int k = 0; k < nnodes; k++, p++)
                        verts[k] = (vtkIdType) *p-1; // Exodus is 1-origin
                    if (InsertExodusCellInVTKUnstructuredGrid(ugrid, vtk_celltype, nnodes, verts))
                        contains_nonlinear_elems = true;
                }
                delete [] conn_buf;
                break;
            }
            default:
            {
                EXCEPTION2(UnexpectedValueException, "NC_INT || NC_INT64", connect_vartype);
            }
        }
        if (ebepecnt_buf) delete [] ebepecnt_buf;
        delete [] verts;

        // Add zone numbers if we have non-linear elements so when split, VisIt will
        // track element edges correctly
        if (contains_nonlinear_elems)
        {
            vtkDataArray *gzoneIds = GetVar(ts, "elem_num_map");
            if (gzoneIds)
            {
                vtkDataArray *domNums;
                int sz = 0;
                if (gzoneIds->GetDataType() == VTK_INT)
                {
                    domNums = vtkIntArray::New();
                    sz = sizeof(int);
                }
                else if (gzoneIds->GetDataType() == VTK_LONG_LONG)
                {
                    domNums = vtkLongLongArray::New();
                    sz = sizeof(long long);
                }

                domNums->SetNumberOfComponents(1);
                domNums->SetNumberOfTuples(gzoneIds->GetNumberOfTuples());
                memset(domNums->GetVoidPointer(0), 0, sz*gzoneIds->GetNumberOfTuples());

                vtkDataArray *oca;
                if (gzoneIds->GetDataType() == VTK_INT)
                    oca = ComposeUpTo3ArraysTemplate<vtkIntArray,int>(2, (vtkIntArray*)gzoneIds,
                              (vtkIntArray*)domNums, (vtkIntArray*)0);
                else if (gzoneIds->GetDataType() == VTK_LONG_LONG)
                    oca = ComposeUpTo3ArraysTemplate<vtkLongLongArray,long long>(2, (vtkLongLongArray*)gzoneIds,
                              (vtkLongLongArray*)domNums, (vtkLongLongArray*)0);
                gzoneIds->Delete();
                domNums->Delete();
                oca->SetName("avtOriginalCellNumbers");
                ugrid->GetCellData()->AddArray(oca);
                oca->Delete();
            }
        }
    }

    return ugrid;
}

// ****************************************************************************
//  Returns element block decomposition as enumerated scalar variable.
//
//  Programmer: Mark C. Miller, Fri Dec 19 10:58:59 PST 2014
// ****************************************************************************

vtkDataArray *
avtExodusFileFormat::GetEBDecompAsEnumScalar()
{
    int nzones = 0;
    for (map<int,int>::const_iterator
        it = blockIdToMatMap.begin(); it != blockIdToMatMap.end(); it++)
        nzones += it->second;

    vtkIntArray *ebarr = vtkIntArray::New();
    ebarr->SetNumberOfComponents(1);
    ebarr->SetNumberOfTuples(nzones);

    int zone = 0;
    for (map<int,int>::const_iterator
        it = blockIdToMatMap.begin(); it != blockIdToMatMap.end(); it++)
    {
        for (int j = 0; j < it->second; j++, zone++)
            ebarr->SetComponent(zone, 0, it->first);
    }
    ebarr->SetName("avtSubsets");

    return ebarr;
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
//  Programmer: Mark C. Miller, Fri Dec 19 10:58:59 PST 2014
//
// ****************************************************************************

#define GET_CENTERING1(CVAL, VAR, TYP)                                  \
if (CVAL == AVT_UNKNOWN_CENT && metadata)                               \
{                                                                       \
    avt ## TYP ## MetaData const *p = metadata->Get ## TYP(VAR);        \
    if (p) CVAL = p->centering;                                         \
}

#define GET_CENTERING(CVAL, VAR)             \
    GET_CENTERING1(CVAL, VAR, Scalar)        \
    GET_CENTERING1(CVAL, VAR, Vector)        \
    GET_CENTERING1(CVAL, VAR, Tensor)

vtkDataArray *
avtExodusFileFormat::GetVar(int ts, const char *var)
{
    nc_type type;
    int num_vals, num_comps;
    void *buf;
    avtCentering vc = AVT_UNKNOWN_CENT;

    CopyFromZeroIndexInstance();

    if (string(var) == "ElementBlock")
        return GetEBDecompAsEnumScalar();

    GET_CENTERING(vc, var);

    if (string(var) == "Nodesets" || string(var) == "Sidesets")
        return GetExodusSetsVar(ncExIIId, ts, var, numNodes, numElems);

    GetData(ncExIIId, ts, var, numBlocks, AVT_SCALAR_VAR, vc,
        &type, &num_comps, &num_vals, &buf);

    if (!buf) return 0;

    // Handle possible material specific values by attempting to read them
    // and, if succesful, contrusting the avtMixedVariable thingy and poking
    // it into the variable cache.
    if (matConvention != None && matCount != -1 && matVarSpecNamescheme != "")
    {
        // Replace "%V" everywhere it occurs in namescheme with the variable name
        string mvstr = string(matVarSpecNamescheme);
        while (true)
        {
            bool found_one = false;
            for (int i = 0; i < ((int) mvstr.size())-1; i++)
            {
                if (mvstr[i] == '%' && mvstr[i+1] == 'V')
                {
                    mvstr.replace(i,2,var);
                    found_one = true;
                    break;
                }
            }
            if (!found_one) break;
        }

        Namescheme mvns(mvstr.c_str());

        // Read all material specific variables and create temporary vtkDataArray
        // objects from them.
        void **mv_buf = new void*[matCount](); // initializes to all zeros
        vtkDataArray **mvarr = new vtkDataArray*[matCount]();
        int foundCount = 0;
        for (int m = 0; m < matCount; m++)
        {
            nc_type mv_type;
            int mv_num_vals, mv_num_comps;

            TRY
            {
                GetData(ncExIIId, ts, mvns.GetName(m), numBlocks, AVT_SCALAR_VAR, vc,
                    &mv_type, &mv_num_comps, &mv_num_vals, &mv_buf[m]);
            }
            CATCH(InvalidVariableException)
            {
                mv_buf[m] = 0;
            }
            ENDTRY

            if (!mv_buf[m]) continue;
            
            foundCount++;
            mvarr[m] = MakeVTKDataArrayByTakingOwnershipOfNCVarData(
                mv_type, mv_num_comps, mv_num_vals, mv_buf[m]);
        }

        if (foundCount == matCount - 1 || foundCount == matCount)
        {
            // First, see if material object is already cached
            void_ref_ptr vr = cache->GetVoidRef("Materials", AUXILIARY_DATA_MATERIAL, ts, myDomain);
            avtMaterial *mat = (avtMaterial*) *vr;

            // If we don't have it, read it and cache it
            if (!mat)
            {
                DestructorFunction df;
                void *p = GetAuxiliaryData("Materials", ts, AUXILIARY_DATA_MATERIAL, (void*)0, df);
                mat = (avtMaterial*) p;
                void_ref_ptr vrtmp = void_ref_ptr(p, df);
                cache->CacheVoidRef("Materials", AUXILIARY_DATA_MATERIAL, ts, myDomain, vrtmp);
            }

            if (!mat) goto skipMatSpecific;

            // Build the mixvals array we need to construct the avtMixedMaterial
            // object we need to return here. Along the way, convert everything to float.
            vector<float> mixvals;
            for (int z = 0; z < mat->GetNZones(); z++)
            {
                vector<float> vfracs;
                mat->GetVolFracsForZone(z, vfracs);
                for (int m = 0; m < vfracs.size(); m++)
                {
                    if (vfracs[m] > 0 && vfracs[m] < 1 && mvarr[m])
                        mixvals.push_back(mvarr[m]->GetTuple1(z));
                }
            }

            if ((int) mixvals.size() != mat->GetMixlen())
            {
                char errmsg[256];
                SNPRINTF(errmsg, sizeof(errmsg), "Mixed variable size, %d, doesn't agree with material "
                    "mixlen, %d, for variable \"%s\"", (int) mixvals.size(), mat->GetMixlen(), var);
                avtCallback::IssueWarning(errmsg);
                goto skipMatSpecific;
            }

            avtMixedVariable *mv = new avtMixedVariable(&mixvals[0], (int) mixvals.size(), var);
            vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
            cache->CacheVoidRef(var, AUXILIARY_DATA_MIXED_VARIABLE, ts, myDomain, vr);
        }

skipMatSpecific:
            ;

        for (int m = 0; m < matCount; m++)
        {
            if (mvarr[m])
                mvarr[m]->Delete(); // deletes the individual mv_bufs too because mvarr owns them
        }
        delete [] mvarr;
        delete [] mv_buf;
    }

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
//  Programmer: Mark C. Miller, Fri Dec 19 10:57:20 PST 2014
// ****************************************************************************

vtkDataArray *
avtExodusFileFormat::GetVectorVar(int ts, const char *var)
{
    nc_type type;
    int num_vals, num_comps;
    void *buf;
    avtCentering vc = AVT_UNKNOWN_CENT;

    GET_CENTERING(vc, var);

    GetData(ncExIIId, ts, var, numBlocks, AVT_VECTOR_VAR, vc,
        &type, &num_comps, &num_vals, &buf);

    vtkDataArray *arr = MakeVTKDataArrayByTakingOwnershipOfNCVarData(type, 
                            num_comps, num_vals, buf);

    // See if the vector needs converting.
    if (num_comps == 2 && metadata != NULL && 
        metadata->GetMesh("Mesh")->spatialDimension == 3)
    {
        // convert the 2-component vector to 3-component
        vtkDataArray *vec = arr->NewInstance();
        vec->SetNumberOfComponents(3);
        vec->SetNumberOfTuples(num_vals);
        for (int i = 0; i < num_vals; ++i)
        {
            vec->SetComponent(i, 0, arr->GetComponent(i, 0));
            vec->SetComponent(i, 1, arr->GetComponent(i, 1));
            vec->SetComponent(i, 2, 0);
        }
        arr->Delete();
        return vec;
    }
    return arr;
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
//  Programmer: Mark C. Miller, Fri Dec 19 10:58:11 PST 2014
//
//  Modifications
//    Mark C. Miller, Fri Dec 19 11:17:16 PST 2014
//    Added support for (true) materials from Exodus files.
// ****************************************************************************

void *
avtExodusFileFormat::GetAuxiliaryData(const char *var, int ts, 
                                      const char * type, void *,
                                      DestructorFunction &df)
{
    CopyFromZeroIndexInstance();

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        if (matConvention == None || matCount <= 0)
            return 0;

        float **volFracRawData = new float*[matCount]();
        int nzones = -1;
        TRY
        {
            // Read all volume fraction variables
            Namescheme vfns(matVolFracNamescheme.c_str());
            for (int m = 0; m < matCount; m++)
            {
                vtkDataArray *volFracVarArray = GetVar(ts, vfns.GetName(m));
                if (!volFracVarArray) 
                {
                    char errmsg[256];
                    SNPRINTF(errmsg, sizeof(errmsg), "Unable to get material volume "
                        "fraction array \"%s\"", vfns.GetName(m));
                    EXCEPTION1(InvalidVariableException, errmsg);
                }
                if (nzones == -1)
                    nzones = volFracVarArray->GetNumberOfTuples();
                else if (nzones != volFracVarArray->GetNumberOfTuples())
                {
                    char errmsg[256];
                    SNPRINTF(errmsg, sizeof(errmsg), "Material volume fraction array \"%s\" "
                        "not the same size (%d) as previously read arrays (%d)",
                        vfns.GetName(m),(int)volFracVarArray->GetNumberOfTuples(),nzones);
                    EXCEPTION1(InvalidVariableException, errmsg);
                }
                volFracRawData[m] = new float[nzones];
                for (int z = 0; z < nzones; z++)
                    volFracRawData[m][z] = volFracVarArray->GetTuple1(z);
                volFracVarArray->Delete();
            }
        }
        CATCH(InvalidVariableException)
        {
            for (int q = 0; q < matCount; q++)
            {
                if (volFracRawData[q])
                    delete [] volFracRawData[q];
            }
            delete [] volFracRawData;
            RETHROW;
        }
        ENDTRY

        char thisDomain[16];
        SNPRINTF(thisDomain, sizeof(thisDomain), "File%d", myDomain);
        int *matids = new int[matCount];
        char **matnames = new char*[matCount];
        for (int m = 0; m < matCount; m++)
        {
            matids[m] = m;
            matnames[m] = new char[16];
            SNPRINTF(matnames[m],sizeof(matnames[m]),"%d",m);
        }

        avtMaterial *mat = new avtMaterial(matCount, matids, matnames,
                         1, &nzones, 0, volFracRawData, thisDomain);

        for (int m = 0; m < matCount; m++)
        {
            delete [] matnames[m];
            delete [] volFracRawData[m];
        }
        delete [] matids;
        delete [] matnames;
        delete [] volFracRawData;

        df = avtMaterial::Destruct;
        return (void*) mat;
    }
    else if (strcmp(type, AUXILIARY_DATA_GLOBAL_NODE_IDS) == 0)
    {
        vtkDataArray *gnodeIds = 0;
        TRY
        {
            gnodeIds = GetVar(ts, "node_num_map");
        }
        CATCH(InvalidVariableException)
        {
            ; // no-op
        }
        ENDTRY
        if (!gnodeIds) return 0;
        gnodeIds->Register(NULL);
        df = avtVariableCache::DestructVTKObject;
        return (void*) gnodeIds;
    }
    else if (strcmp(type, AUXILIARY_DATA_GLOBAL_ZONE_IDS) == 0)
    {
        vtkDataArray *gzoneIds = 0;
        TRY
        {
            gzoneIds = GetVar(ts, "elem_num_map");
        }
        CATCH(InvalidVariableException)
        {
            ; // no-op
        }
        ENDTRY
        if (!gzoneIds) return 0;
        gzoneIds->Register(NULL);
        df = avtVariableCache::DestructVTKObject;
        return (void*) gzoneIds;
    }

    return NULL;
}
