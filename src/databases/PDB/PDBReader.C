#include <PDBReader.h>
#include <DebugStream.h>
#include <string.h>
#include <InvalidVariableException.h>

//
// Define some functions so if PDB was compiled with a weird non-g++ compiler,
// we still have symbols that we need in order to link.
//
extern "C" void ieee_handler(int)           { }
extern "C" void standard_arithmetic(int)    { }
extern "C" void nonstandard_arithmetic(int) { }

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
// Method: PDBReader::VariableData::VariableData
//
// Purpose: 
//   Constructor for the PDBReader::VariableData class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 14:55:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PDBReader::VariableData::VariableData(const std::string &name) : varName(name)
{
    data = 0;
    dataType = NO_TYPE;
    dims = 0;
    nDims = 0;
    nTotalElements = 0;
}

// ****************************************************************************
// Method: PDBReader::VariableData::~VariableData
//
// Purpose: 
//   Destructor for the PDBReader::VariableData class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 14:56:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PDBReader::VariableData::~VariableData()
{
    if(data)
        free_void_mem(data, dataType);

    delete [] dims;
}

// ****************************************************************************
// Method: PDBReader::VariableData::ReadValues
//
// Purpose: 
//   Reads data into the object.
//
// Arguments:
//   reader : The PDB reader object to use,
//
// Returns:    True if data was successfully read in; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 14:56:34 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
PDBReader::VariableData::ReadValues(PDBReader *reader)
{
    if(data == 0)
    {
        data = reader->ReadValues((char *)varName.c_str(), &dataType,
                                  &nTotalElements, &dims, &nDims);
    }

    return data != 0;
}

// ****************************************************************************
//
// Method: PDBReader::PDBReader
//
// Purpose: 
//   Constructor for the PDBReader class.
//
// Arguments:
//   p : A pointer to the PDBfile object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:58:35 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

PDBReader::PDBReader(PDBfile *p)
{
    pdb = p;
}

// ****************************************************************************
// Method: PDBReader::~PDBReader
//
// Purpose:
//   Destructor for the PDBReader class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:59:02 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

PDBReader::~PDBReader()
{
}

// ****************************************************************************
// Method: PDBReader::ReadValues
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
//
// Returns:    A pointer to memory that contains the variable that was read in.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 13:45:43 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void *
PDBReader::ReadValues(const char *name, TypeEnum *t, int *nTotalElements,
    int **dimensions, int *nDims)
{
    void *retval = 0;
    syment *ep = 0;

    // Indicate that there is no type initially.
    *t = NO_TYPE;
    *nTotalElements = 0;
    *dimensions = 0;
    *nDims = 0;

    if((ep = PD_inquire_entry(pdb, (char *)name, 0, NULL)) != NULL)
    {
        dimdes *dimptr = NULL;
        int i = 0, nd = 0, length = 1;
        int *dims = 0;

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
        debug4 << "PDBReader::ReadValues: name=" << name << ", dimensions={";
        for(i = 0; i < nd; ++i)
            debug4 << dims[i] << ", ";
        debug4 << "}" << endl;

        // Set some of the return values.
        *dimensions = dims;
        *nDims = nd;
        *nTotalElements = length;

        //
        // Take the storage type along with the length to determine the real
        // type that we want to report. Also allocate memory for the
        // variable.
        //
        if(strcmp(PD_entry_type(ep), "char") == 0)
        {
            *t = (length > 1) ? CHARARRAY_TYPE : CHAR_TYPE;
            retval = (void *)new char[length];
        }
        else if(strcmp(PD_entry_type(ep), "int") == 0 ||
                strcmp(PD_entry_type(ep), "integer") == 0)
        {
            *t = (length > 1) ? INTEGERARRAY_TYPE : INTEGER_TYPE;
            retval = (void *)new int[length];
        }
        else if(strcmp(PD_entry_type(ep), "float") == 0)
        {
            *t = (length > 1) ? FLOATARRAY_TYPE : FLOAT_TYPE;
            retval = (void *)new float[length];
        }
        else if(strcmp(PD_entry_type(ep), "double") == 0)
        {
            *t = (length > 1) ? DOUBLEARRAY_TYPE : DOUBLE_TYPE;
            retval = (void *)new double[length];
        }
        else if(strcmp(PD_entry_type(ep), "long") == 0)
        {
            *t = (length > 1) ? LONGARRAY_TYPE : LONG_TYPE;
            retval = (void *)new long[length];
        }
        else
        {
            EXCEPTION1(InvalidVariableException, "unsupported type");
        }

        //
        // Try reading the variable from the file.
        //
        if(PD_read(pdb, (char *)name, retval) == FALSE)
        {
            debug4 << "PDBReader::ReadValues: PD_read failed for " << name
                   << ". " << PD_err << endl;

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
        debug4 << "PDBReader::ReadValues: Variable \"" << name
               << "\" could not be located. " << PD_err << endl;
    }

    return retval;
}

// ****************************************************************************
// Method: PDBReader::GetString
//
// Purpose: 
//   Reads the named string from the pdb file and returns its value.
//
// Arguments:
//   name  : The name of the string variable to read.
//   str   : A pointer to the returned string.
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
PDBReader::GetString(const char *name, char **str)
{
    int *dims = 0;
    int nDims = 0;
    int length = 0;
    TypeEnum t = NO_TYPE;

    // Initially set the return value to zero.
    *str = 0;

    // Read the variable.
    void *val = ReadValues(name, &t, &length, &dims, &nDims);

    // Set up the return array.
    if(val)
    {
        free_mem(dims);
        if(t == CHAR_TYPE || t == CHARARRAY_TYPE)
            *str = (char *)val;
        else
            free_void_mem(val, t);
    }

    return *str != 0;
}

// ****************************************************************************
// Method: PDBReader::GetDouble
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
PDBReader::GetDouble(const char *name, double *val)
{
    bool retval = false;
    syment *ep;

    if((ep = PD_inquire_entry(pdb, (char *)name, 0, NULL)) != NULL)
    {
        debug4 << "PDBReader::GetDouble: var="<< name
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

    return retval;
}

// ****************************************************************************
// Method: PDBReader::GetDoubleArray
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
//   
// ****************************************************************************

bool
PDBReader::GetDoubleArray(const char *name, double **d, int *nvals)
{
    int *dims = 0;
    int nDims = 0;
    int length = 0;
    TypeEnum t = NO_TYPE;

    // Initially set the return value to zero.
    *d = 0;
    *nvals = 0;

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
        else
            free_void_mem(val, t);
    }

    return *d != 0;
}

// ****************************************************************************
// Method: PDBReader::GetInteger
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
//   
// ****************************************************************************

bool
PDBReader::GetInteger(const char *name, int *val)
{
    bool retval = false;
    syment *ep;

    if((ep = PD_inquire_entry(pdb, (char *)name, 0, NULL)) != NULL)
    {
        debug4 << "PDBReader::GetInteger: var="<< name
               << ", type=" << PD_entry_type(ep) << endl;

        if(strcmp(PD_entry_type(ep), "integer") == 0 ||
           strcmp(PD_entry_type(ep), "int") == 0)
        {
            retval = (PD_read(pdb, (char *)name, (void *)val) ==  TRUE);
        }
        else if(strcmp(PD_entry_type(ep), "long") == 0)
        {
            long tmp;
            retval = (PD_read(pdb, (char *)name, (void *)&tmp) ==  TRUE);
            if(retval)
                *val = (int)tmp;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: PDBReader::GetIntegerArray
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
PDBReader::GetIntegerArray(const char *name, int **i, int *nvals)
{
    int *dims = 0;
    int nDims = 0;
    int length = 0;
    TypeEnum t = NO_TYPE;

    // Initially set the return value to zero.
    *i = 0;
    *nvals = 0;

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

    return *i != 0;
}

// ****************************************************************************
// Method: PDBReader::SymbolExists
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
PDBReader::SymbolExists(const char *name)
{
    return (PD_inquire_entry(pdb, (char *)name, 0, NULL) != NULL);
}

// ****************************************************************************
// Method: PDBReader::GetCycles
//
// Purpose: 
//   Returns the number of cycles.
//
// Arguments:
//   cycles : The return vector for the cycles.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 13:36:41 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PDBReader::GetCycles(std::vector<int> &cycles)
{
    cycles.clear();
    cycles.push_back(0);
}

// ****************************************************************************
// Method: PDBReader::GetNTimesteps
//
// Purpose: 
//   Returns the number of timesteps.
//
// Returns:    Returns the number of timesteps.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 13:39:16 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
PDBReader::GetNTimesteps()
{
    return 1;
}
