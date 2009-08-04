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

#include <PDBFileObject.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>

#if !defined(__APPLE__)
//
// Define some functions so if PDB was compiled with a weird non-g++ compiler,
// we still have symbols that we need in order to link.
//
extern "C" void ieee_handler(int)           { }
extern "C" void standard_arithmetic(int)    { }
extern "C" void nonstandard_arithmetic(int) { }
extern "C" void __mth_i_dpowd(void)         { }
extern "C" void __arg_reduce_(void)         { }
extern "C" void __pgdbg_stub(void)          { }
#endif

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
    int    *iptr = (int *)   ptr;
    float  *fptr = (float *) ptr;
    double *dptr = (double *)ptr;
    long   *lptr = (long *)  ptr;

    switch(t)
    {
    case CHAR_TYPE:
    case CHARARRAY_TYPE:
        delete [] cptr;
        break;
    case INTEGER_TYPE:
    case INTEGERARRAY_TYPE:
        delete [] iptr;
        break;
    case FLOAT_TYPE:
    case FLOATARRAY_TYPE:
        delete [] fptr;
        break;
    case DOUBLE_TYPE:
    case DOUBLEARRAY_TYPE:
        delete [] dptr;
        break;
    case LONG_TYPE:
    case LONGARRAY_TYPE:
        delete [] lptr;
        break;
    default:
        break;
    }
}

// ****************************************************************************
// Method: PDBFileObject::PDBFileObject
//
// Purpose: 
//   Constructor
//
// Arguments:
//   f : The name of the PDB file to open.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 09:13:57 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

PDBFileObject::PDBFileObject(const char *f) : filename(f)
{
    pdb = 0;
}

// ****************************************************************************
// Method: PDBFileObject::~PDBFileObject
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 09:14:18 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

PDBFileObject::~PDBFileObject()
{
    Close();
}

// ****************************************************************************
// Method: PDBFileObject::Destruct
//
// Purpose: 
//   Static destruct function that is used by the variable cache to delete
//   an instance of PDBFileObject.
//
// Arguments:
//   ptr : A pointer to the object to delete.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 12 10:26:00 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
PDBFileObject::Destruct(void *ptr)
{
    if(ptr != 0)
    {
        PDBFileObject *obj = (PDBFileObject *)ptr;
        debug4 << "PDBFileObject::Destruct: file=" << obj->GetName().c_str() << endl;
        delete obj;
    }
}

// ****************************************************************************
// Method: PDBFileObject::IsOpen
//
// Purpose: 
//   Returns whether the PDB file is open.
//
// Returns:    True if the file is open; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 09:14:32 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
PDBFileObject::IsOpen() const
{
    return pdb != 0;
}

// ****************************************************************************
// Method: PDBFileObject::Open
//
// Purpose: 
//   Opens the PDB file.
//
// Returns:    True if the file opened; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 09:15:05 PDT 2003
//
// Modifications:
//    Jeremy Meredith, Thu Aug  7 15:59:40 EDT 2008
//    Assume PDB won't modify our string literals, so cast to char* as needed.
//   
// ****************************************************************************

bool
PDBFileObject::Open()
{
    Close();

    pdb = PD_open((char *)filename.c_str(), (char*)"r");

    // Should probably throw an exception here if pdb == 0.

    return pdb != 0;
}

// ****************************************************************************
// Method: PDBFileObject::Close
//
// Purpose: 
//   Closes the PDB file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 09:16:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
PDBFileObject::Close()
{
    if(pdb)
    {
        PD_close(pdb);
        pdb = 0;
    }
}

// ****************************************************************************
// Method: PDBFileObject::AutoOpen
//
// Purpose: 
//   Opens the file if it has not been opened.
//
// Returns:    True if the file is open; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 11:24:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
PDBFileObject::AutoOpen()
{
    if(pdb == 0)
        Open();

    return pdb != 0;
}

// ****************************************************************************
// Method: PDBFileObject::GetName
//
// Purpose: 
//   Returns the filename.
//
// Returns:    The filename.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 11:02:45 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

const std::string &
PDBFileObject::GetName() const
{
    return filename;
}

// ****************************************************************************
// Method: PDBFileObject::filePointer
//
// Purpose: 
//   Returns the raw PDB file pointer so users can do fancier things than
//   this class provides.
//
// Returns:    The PDB file pointer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 16:04:58 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PDBfile *
PDBFileObject::filePointer()
{
    AutoOpen();
    return pdb;
}

// ****************************************************************************
// Method: PDBFileObject::ReadValues
//
// Purpose: 
//   This method reads a PDB variable from the pdb file. Any array type or
//   single element value can be read.
//
// Arguments:
//   name           : The name of the variable to read.
//   t              : The returned type of the variable.
//   nTotalElements : The returned number of elements in the variable.
//   dimensions     : The returned array of dimensions.
//   nDims          : The returned number of dimensions.
//   extraElements  : The number of extra elements to allocate (optional)
//
// Returns:    A pointer to memory that contains the variable that was read in.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 13:45:43 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug 8 11:17:30 PDT 2003
//   Moved all of the code to get information about the variable to the
//   SymbolExists method.
//
//   Brad Whitlock, Fri Mar 5 11:01:43 PDT 2004
//   Fixed for Windows compiler.
//
// ****************************************************************************

void *
PDBFileObject::ReadValues(const char *name, TypeEnum *t, int *nTotalElements,
    int **dimensions, int *nDims, int extraElements)
{
    void *retval = 0;

    if(AutoOpen())
    {
        //
        // Get information about the variable if it is in the file.
        //
        if(SymbolExists(name, t, nTotalElements, dimensions, nDims))
        {
            int nels = *nTotalElements + extraElements;

            //
            // Allocate memory for the variable.
            //
            switch(*t)
            {
            case CHAR_TYPE:
            case CHARARRAY_TYPE:
                retval = (void *)new char[nels];
                break;
            case INTEGER_TYPE:
            case INTEGERARRAY_TYPE:
                retval = (void *)new int[nels];
                break;
            case FLOAT_TYPE:
            case FLOATARRAY_TYPE:
                retval = (void *)new float[nels];
                break;
            case DOUBLE_TYPE:
            case DOUBLEARRAY_TYPE:
                retval = (void *)new double[nels];
                break;
            case LONG_TYPE:
            case LONGARRAY_TYPE:
                retval = (void *)new long[nels];
                break;
            default:
                EXCEPTION1(InvalidVariableException, "unsupported type");
            }

            //
            // Try reading the variable from the file.
            //
            if(PD_read(pdb, (char *)name, retval) == FALSE)
            {
                debug4 << "PDBFileObject::ReadValues: PD_read failed for " << name
                       << ". " << PDBLIB_ERRORSTRING << endl;

                free_void_mem(retval, *t);
                retval = 0;
                *t = NO_TYPE;
                *nTotalElements = 0;
                free_mem(*dimensions);
                *dimensions = 0;
                *nDims = 0;
            }
        }
        else
        {
            debug4 << "PDBFileObject::ReadValues: Variable \"" << name
                   << "\" could not be located. " << PDBLIB_ERRORSTRING << endl;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: PDBFileObject::GetString
//
// Purpose: 
//   Reads the named string from the pdb file and returns its value.
//
// Arguments:
//   name  : The name of the string variable to read.
//   str   : A pointer to the returned string.
//   len   : A pointer to an int where we can store the length.
//
// Returns:    true if successful; otherwise false.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 17:45:28 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Aug 11 09:35:22 PDT 2003
//   I added an optional length pointer to the argument list.
//
//   Brad Whitlock, Wed Sep 17 14:32:46 PST 2003
//   I made sure that the string is NULL terminated.
//
//   Brad Whitlock, Fri Jun 4 17:36:53 PST 2004
//   I fixed an error that prevented the string from getting NULL terminated
//   sometimes.
//
// ****************************************************************************

bool
PDBFileObject::GetString(const char *name, char **str, int *len)
{
    int *dims = 0;
    int nDims = 0;
    int length = 0;
    TypeEnum t = NO_TYPE;

    // Initially set the return value to zero.
    *str = 0;

    if(AutoOpen())
    {
        // Read the variable.
        void *val = ReadValues(name, &t, &length, &dims, &nDims, 1);

        // Set up the return array.
        if(val)
        {
            free_mem(dims);
            if(t == CHAR_TYPE || t == CHARARRAY_TYPE)
            {
                *str = (char *)val;
                // We allocated the array so it can hold the extra
                // character required for a NULL terminator.
                (*str)[length] = '\0';

                if(len != 0)
                    *len = length;
            }
            else
                free_void_mem(val, t);
        }
    }

    return *str != 0;
}

// ****************************************************************************
// Method: PDBFileObject::GetDouble
//
// Purpose: 
//   Reads the named double from the pdb file and returns its value.
//
// Arguments:
//   name : The name of the variable to read.
//   val  : The destination variable.
//
// Returns:    true if successful; otherwise false.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 9 09:26:54 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
PDBFileObject::GetDouble(const char *name, double *val)
{
    bool retval = false;

    if(AutoOpen())
    {
        syment *ep;

        if((ep = PD_inquire_entry(pdb, (char *)name, 0, NULL)) != NULL)
        {
            debug4 << "PDBFileObject::GetDouble: var="<< name
                   << ", type=" << PD_entry_type(ep) << endl;

            if(strcmp(PD_entry_type(ep), "double") == 0)
            {
                retval = (PD_read(pdb, (char *)name, (void *)val) ==  TRUE);
            }
            else if(strcmp(PD_entry_type(ep), "float") == 0)
            {
                float tmp;
                retval = (PD_read(pdb, (char *)name, (void *)&tmp) ==  TRUE);
                if(retval)
                    *val = (double)tmp;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: PDBFileObject::GetDoubleArray
//
// Purpose: 
//   Reads the named double array from the pdb file and returns its value.
//
// Arguments:
//   name  : The name of the variable to read.
//   val   : The destination variable array.
//   nvals : The number of values in the array.
//
// Returns:    true if successful; otherwise false.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 17:45:28 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Jun 7 08:57:12 PDT 2004
//   Added code to convert float arrays into double arrays.
//
// ****************************************************************************

bool
PDBFileObject::GetDoubleArray(const char *name, double **d, int *nvals)
{
    int *dims = 0;
    int nDims = 0;
    int length = 0;
    TypeEnum t = NO_TYPE;

    // Initially set the return value to zero.
    *d = 0;
    *nvals = 0;

    if(AutoOpen())
    {
        // Read the variable.
        void *val = ReadValues(name, &t, &length, &dims, &nDims);

        // Set up the return array.
        if(val)
        {
            free_mem(dims);
            if(t == DOUBLE_TYPE || t == DOUBLEARRAY_TYPE)
            {
                *d = (double *)val;
                *nvals = length;
            }
            else if(t == FLOAT_TYPE || t == FLOATARRAY_TYPE)
            {
                float *fptr = (float *)val;
                double *dstorage = new double[length];
                double *dptr = dstorage;
                for(int i = 0; i < length; ++i)
                    *dptr++ = double(*fptr++);
                free_void_mem(val, t);

                *d = dstorage;
                *nvals = length;
            }
            else
                free_void_mem(val, t);
        }
    }

    return *d != 0;
}

// ****************************************************************************
// Method: PDBFileObject::GetInteger
//
// Purpose: 
//   Reads the named integer from the pdb file and returns its value.
//
// Arguments:
//   pdb  : A pointer to a pdb file object.
//   name : The name of the variable to read.
//   val  : The destination variable.
//
// Returns:    true if successful; otherwise false.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:56:46 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Dec 7 16:25:25 PST 2004
//   I made it use GetIntegerArray when there is an array of integers
//   instead of just one integer to prevent us from clobbering memory.
//
// ****************************************************************************

bool
PDBFileObject::GetInteger(const char *name, int *val)
{
    bool retval = false;

    if(AutoOpen())
    {
        syment *ep;

        if((ep = PD_inquire_entry(pdb, (char *)name, 0, NULL)) != NULL)
        {
            debug4 << "PDBFileObject::GetInteger: var="<< name
                   << ", type=" << PD_entry_type(ep) << endl;

            if(strcmp(PD_entry_type(ep), "integer") == 0 ||
               strcmp(PD_entry_type(ep), "int") == 0)
            {
                //
                // Get the integer as an array in case there is more than 1
                // integer. This way, we don't clobber memory when we read
                // the integer if it happens to be an array of integers.
                //
                int *vals = 0, nvals = 0;
                if(GetIntegerArray(name, &vals, &nvals) && nvals > 0)
                {
                    *val = vals[0];
                    delete [] vals;
                    retval = true;
                }
            }
            else if(strcmp(PD_entry_type(ep), "long") == 0)
            {
                long tmp;
                retval = (PD_read(pdb, (char *)name, (void *)&tmp) ==  TRUE);
                if(retval)
                    *val = (int)tmp;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: PDBFileObject::GetIntegerArray
//
// Purpose: 
//   Reads the named int array from the pdb file and returns its value.
//
// Arguments:
//   name  : The name of the variable to read.
//   val   : The destination variable array.
//   nvals : The number of values in the array.
//
// Returns:    true if successful; otherwise false.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 17:45:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
PDBFileObject::GetIntegerArray(const char *name, int **i, int *nvals)
{
    int *dims = 0;
    int nDims = 0;
    int length = 0;
    TypeEnum t = NO_TYPE;

    // Initially set the return value to zero.
    *i = 0;
    *nvals = 0;

    if(AutoOpen())
    {
        // Read the variable.
        void *val = ReadValues(name, &t, &length, &dims, &nDims);

        // Set up the return array.
        if(val)
        {
            free_mem(dims);
            if(t == INTEGER_TYPE || t == INTEGERARRAY_TYPE)
            {
                *i = (int *)val;
                *nvals = length;
            }
            else
                free_void_mem(val, t);
        }
    }

    return *i != 0;
}

// ****************************************************************************
// Method: PDBFileObject::SymbolExists
//
// Purpose: 
//   Tests the PDB file to see if a symbol exists.
//
// Arguments:
//   name : The name of the symbol we're looking for.
//
// Returns:    True if the symbol is in the file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:55:59 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
PDBFileObject::SymbolExists(const char *name)
{
    bool retval = false;

    if(AutoOpen())
        retval = (PD_inquire_entry(pdb, (char *)name, 0, NULL) != NULL);

    return retval;
}

// ****************************************************************************
// Method: PDBFileObject::SymbolExists.
//
// Purpose: 
//   This method returns information about a PDB variable if it is in the file.
//
// Arguments:
//   name           : The name of the variable to read.
//   t              : The returned type of the variable.
//   nTotalElements : The returned number of elements in the variable.
//   dimensions     : The returned array of dimensions.
//   nDims          : The returned number of dimensions.
//
// Returns:    True if the symbol exists; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 8 11:12:52 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 22 11:32:48 PDT 2004
//   I made "string" be a synonym for "char".
//
// ****************************************************************************

bool
PDBFileObject::SymbolExists(const char *name, TypeEnum *t, int *nTotalElements,
    int **dimensions, int *nDims)
{
    std::string tmp;
    return SymbolExists(name, t, tmp, nTotalElements, dimensions, nDims);
}

bool
PDBFileObject::SymbolExists(const char *name, TypeEnum *t,
    std::string &typeString, int *nTotalElements, int **dimensions,
    int *nDims)
{
    bool retval = false;
    syment *ep = 0;

    // Indicate that there is no type initially.
    if (t)
        *t = NO_TYPE;
    if (nTotalElements)
        *nTotalElements = 0;
    if (dimensions)
        *dimensions = 0;
    if (nDims)
        *nDims = 0;

    if(AutoOpen())
    {
        if((ep = PD_inquire_entry(pdb, (char *)name, 0, NULL)) != NULL)
        {
            dimdes *dimptr = NULL;
            int i = 0, nd = 0, length = 1;
            int *dims = 0;

            // Return the actual name of type
            typeString = PD_entry_type(ep);

            // Figure out the number of dimensions and the number of elements
            // that are in the entire array.
            dimptr = PD_entry_dimensions(ep);
            if(dimptr != NULL)
            {
                // Figure out the number of dimensions.
                while(dimptr != NULL)
                {
                    length *= dimptr->number;
                    dimptr = dimptr->next;
                    ++nd;
                }

                // Store the dimensions of the array.
                dims = new int[nd];
                dimptr = PD_entry_dimensions(ep);
                while(dimptr != NULL)
                {
                    dims[i++] = dimptr->number;
                    dimptr = dimptr->next;
                }
            }
            else
            {
                dims = new int[1];
                dims[0] = 1;
            }

            // Print the dimensions to the debug log.
            debug4 << "PDBFileObject::SymbolExists: name=" << name << ", dimensions={";
            for(i = 0; i < nd; ++i)
                debug4 << dims[i] << ", ";
            debug4 << "}" << endl;

            // Set some of the return values.
            if (dimensions)
                *dimensions = dims;
            if (nDims)
                *nDims = nd;
            if (nTotalElements)
                *nTotalElements = length;

            //
            // Take the storage type along with the length to determine the real
            // type that we want to report. Also allocate memory for the
            // variable.
            //
            if (t)
            {
                if(strcmp(PD_entry_type(ep), "char") == 0 ||
                   strcmp(PD_entry_type(ep), "string") == 0)
                    *t = (length > 1) ? CHARARRAY_TYPE : CHAR_TYPE;
                else if(strcmp(PD_entry_type(ep), "int") == 0 ||
                        strcmp(PD_entry_type(ep), "integer") == 0)
                    *t = (length > 1) ? INTEGERARRAY_TYPE : INTEGER_TYPE;
                else if(strcmp(PD_entry_type(ep), "float") == 0)
                    *t = (length > 1) ? FLOATARRAY_TYPE : FLOAT_TYPE;
                else if(strcmp(PD_entry_type(ep), "double") == 0)
                    *t = (length > 1) ? DOUBLEARRAY_TYPE : DOUBLE_TYPE;
                else if(strcmp(PD_entry_type(ep), "long") == 0)
                    *t = (length > 1) ? LONGARRAY_TYPE : LONG_TYPE;
                else
                {
                    *t = OBJECT_TYPE;
                }
            }

            retval = true;
        }
    }

    return retval;
}

