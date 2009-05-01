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

// ************************************************************************* //
//                            JMFileFormat.C                                 //
// ************************************************************************* //

#include <JMFileFormat.h>

#include <string>

#include <snprintf.h>

#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <vtkVisItUtility.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtMTSDFileFormatInterface.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>

#define JM_CYCLE  "cycle"
#define JM_TIME   "timeh"
#define JM_XCOORD "xx"
#define JM_YCOORD "yy"
#define JM_IREG   "ireg"

// The file's data is stored like this: [ny][nx][nts] which is somewhat backwards
// so we need to reorder it.
#define REVERSE_DATA

// ****************************************************************************
// Method: JMFileFormat::CreateInterface
//
// Purpose: 
//   Creates a single MTSD file format interface using the files in the list.
//
// Arguments:
//   pdb   : the open PDB file object.
//   list  : The number of files in the list.
//   nList : The number of files in the list.
//
// Returns:    A file format interface or 0 if one was not created.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 30 16:03:43 PDT 2009
//
// Modifications:
//
// ****************************************************************************

avtFileFormatInterface *
JMFileFormat::CreateInterface(PDBFileObject *pdb,
    const char *const *list, int nList)
{
    avtFileFormatInterface *inter = 0;

    // Create a JMFileFormat that uses the pdb file but does not own it.
    JMFileFormat *ff = new JMFileFormat(&list[0], pdb);

    // If the file format is JM then create a file format interface.
    if(ff->Identify())
    {
        ff->OwnsPDB();
        avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
        for (int i = 0 ; i < nList ; i++)
        {
            if(i == 0)
                ffl[0] = ff;
            else 
                ffl[i] = new JMFileFormat(&list[i]);
        }

        inter = new avtMTSDFileFormatInterface(ffl, nList);
    }
    else
        delete ff;

    return inter;
}

// ****************************************************************************
// Method: JMFileFormat::VarItem::VarItem
//
// Purpose: 
//   ctor
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 14:00:04 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

JMFileFormat::VarItem::VarItem()
{
    type = NO_TYPE;
    dims = 0;
    ndims = 0;
    nTotalElements = 0;
    data = 0;
}

// ****************************************************************************
// Method: JMFileFormat::VarItem::~VarItem
//
// Purpose: 
//   dtor
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 14:00:04 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

JMFileFormat::VarItem::~VarItem()
{
    if(dims != 0)
        delete [] dims;
    if(data != 0)
        free_void_mem(data, type);
}

// ****************************************************************************
// Method: JMFileFormat::VarItem::DataForTime
//
// Purpose: 
//   Returns the data for a specific time slice.
//
// Arguments:
//   ts : The time slice.
//
// Returns:    The data for the time slice.
//
// Note:       The file seems to require reversing the data since it's time
//             dimension changes most rapidly. Lame!
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 13:59:01 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void *
JMFileFormat::VarItem::DataForTime(int ts)
{
    char   *cptr = (char *)  data;
    int    *iptr = (int *)   data;
    float  *fptr = (float *) data;
    double *dptr = (double *)data;
    long   *lptr = (long *)  data;

#ifdef REVERSE_DATA
    // Time is the fastest varying dimension. Backwards!
    int nts = dims[0];
    int nx = dims[1];
    int ny = dims[2];
    switch(type)
    {
    case CHAR_TYPE:
    case CHARARRAY_TYPE:
        {
        char *dest = new char[nx * ny];
        for(int j = 0; j < ny; ++j)
            for(int i = 0; i < nx; ++i)
                dest[j * nx + i] = cptr[j*nx*nts + i*nts + ts];
        return (void *)dest;
        }
        break;
    case INTEGER_TYPE:
    case INTEGERARRAY_TYPE:
        {
        int *dest = new int[nx * ny];
        for(int j = 0; j < ny; ++j)
            for(int i = 0; i < nx; ++i)
                dest[j * nx + i] = iptr[j*nx*nts + i*nts + ts];
        return (void *)dest;
        }
        break;
    case FLOAT_TYPE:
    case FLOATARRAY_TYPE:
        {
        float *dest = new float[nx * ny];
        for(int j = 0; j < ny; ++j)
            for(int i = 0; i < nx; ++i)
                dest[j * nx + i] = fptr[j*nx*nts + i*nts + ts];
        return (void *)dest;
        }
        break;
    case DOUBLE_TYPE:
    case DOUBLEARRAY_TYPE:
        {
        double *dest = new double[nx * ny];
        for(int j = 0; j < ny; ++j)
            for(int i = 0; i < nx; ++i)
                dest[j * nx + i] = dptr[j*nx*nts + i*nts + ts];
        return (void *)dest;
        }
        break;
    case LONG_TYPE:
    case LONGARRAY_TYPE:
        {
        long *dest = new long[nx * ny];
        for(int j = 0; j < ny; ++j)
            for(int i = 0; i < nx; ++i)
                dest[j * nx + i] = lptr[j*nx*nts + i*nts + ts];
        return (void *)dest;
        }
        break;
    default:
        break;
    }
#else
    int sz = 1;
    for(int i = 1; i < ndims; ++i)
        sz *= dims[i];
    int offset = (data != 0) ? (sz * ts) : 0;

    switch(type)
    {
    case CHAR_TYPE:
    case CHARARRAY_TYPE:
        return cptr + offset;
        break;
    case INTEGER_TYPE:
    case INTEGERARRAY_TYPE:
        return iptr + offset;
        break;
    case FLOAT_TYPE:
    case FLOATARRAY_TYPE:
        return fptr + offset;
        break;
    case DOUBLE_TYPE:
    case DOUBLEARRAY_TYPE:
        return dptr + offset;
        break;
    case LONG_TYPE:
    case LONGARRAY_TYPE:
        return lptr + offset;
        break;
    default:
        break;
    }
#endif

    return 0;
}

// ****************************************************************************
//  Method: JMFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 30 15:50:23 PST 2009
//
// ****************************************************************************

JMFileFormat::JMFileFormat(const char * const*filename, PDBFileObject *p)
    : avtMTSDFileFormat(filename, 1), variableCache(), curveNames()
{
    pdb = p;
    ownsPDB = false;
}

JMFileFormat::JMFileFormat(const char * const*filename)
    : avtMTSDFileFormat(filename, 1), variableCache(), curveNames()
{
    pdb = new PDBFileObject(*filename);
    ownsPDB = true;
}

// ****************************************************************************
// Method: JMFileFormat::~JMFileFormat
//
// Purpose: 
//   dtor
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 14:01:37 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

JMFileFormat::~JMFileFormat()
{
    if(ownsPDB)
        delete pdb;

    for(std::map<std::string, VarItem *>::iterator it = variableCache.begin();
        it != variableCache.end(); ++it)
    {
        delete it->second;
    }
}

// ****************************************************************************
// Method: JMFileFormat::OwnsPDB
//
// Purpose: 
//   Tell the file format it owns its pdb object.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 15:09:16 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
JMFileFormat::OwnsPDB()
{
    ownsPDB = true;
}

// ****************************************************************************
// Method: JMFileFormat::ActivateTimestep
//
// Purpose: 
//   Activates the current time step.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 14:01:49 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
JMFileFormat::ActivateTimestep(int ts)
{
    debug4 << "ActivateTimestep: ts=" << ts << endl;
}

// ****************************************************************************
// Method: JMFileFormat::Identify
//
// Purpose: 
//   Returns whether the file contains names that identify it as a "JM" file.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 30 16:10:17 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
JMFileFormat::Identify()
{
    const char *required[] = {"xx", "yy", "ireg", "tr", "ti", "cycle", "timeh"};
    bool identified = true;
    for(int i = 0; i < 7 && identified; ++i)
        identified &= pdb->SymbolExists(required[i]);
    debug1 << "JMFileFormat::Identify: " << (identified?"true":"false") << endl;
    return identified;
}

// ****************************************************************************
// Method: JMFileFormat::ReadVariable
//
// Purpose: 
//   Reads the named variable and puts it into a cache.
//
// Arguments:
//   var : The name of the variable to read.
//
// Returns:    The item that contains the data that was read.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 13:58:12 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

JMFileFormat::VarItem *
JMFileFormat::ReadVariable(const std::string &var)
{
    VarItem *item = 0;
    std::map<std::string, VarItem *>::iterator it = variableCache.find(var);
    if(it != variableCache.end())
        item = it->second;
    else
    {
        item = new VarItem;
        item->data = pdb->ReadValues(var.c_str(), &item->type, 
            &item->nTotalElements, &item->dims, &item->ndims);
        if(item->data != 0)
        {
            variableCache[var] = item;
        }
        else
        {
            delete item;
            item = 0;
        }
        return item;
    }

    return item;
}

// ****************************************************************************
//  Method: JMFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 30 15:50:23 PST 2009
//
// ****************************************************************************

int
JMFileFormat::GetNTimesteps(void)
{
    VarItem *c = ReadVariable(JM_CYCLE);
    int nts = (c != 0) ? c->dims[0] : 0;
    return nts;
}

// ****************************************************************************
// Method: JMFileFormat::GetCycles
//
// Purpose: 
//   Returns the cycles from the file.
//
// Arguments:
//   cycles : The cycles.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 13:57:47 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
JMFileFormat::GetCycles(std::vector<int> &cycles)
{
    VarItem *c = ReadVariable(JM_CYCLE);
    if(c != 0)
    {
        long *cvalues = (long *)c->data;
        for(int i = 0; i < c->nTotalElements; ++i)
            cycles.push_back((int)cvalues[i]);
    }
}

// ****************************************************************************
// Method: JMFileFormat::GetTimes
//
// Purpose: 
//   Returns the times from the file.
//
// Arguments:
//   times : The times.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 13:57:26 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
JMFileFormat::GetTimes(std::vector<double> &times)
{
    VarItem *t = ReadVariable(JM_TIME);
    if(t != 0)
    {
        double *tvalues = (double *)t->data;
        for(int i = 0; i < t->nTotalElements; ++i)
            times.push_back(tvalues[i]);
    }
}
    
// ****************************************************************************
//  Method: JMFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 30 15:50:23 PST 2009
//
// ****************************************************************************

void
JMFileFormat::FreeUpResources(void)
{
    pdb->Close();
}

// ****************************************************************************
//  Method: JMFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 30 15:50:23 PST 2009
//
// ****************************************************************************

intVector
JMFileFormat::GetSize(const char *varName)
{
    TypeEnum t;
    int nTotalElements = 0;
    int *dims = 0;
    int ndims = 0;
    intVector retval;
    if(pdb->SymbolExists(varName, &t, &nTotalElements, &dims, &ndims))
    {
        for(int i = 0; i < ndims; ++i)
            retval.push_back(dims[i]);
        delete [] dims;
    }
    return retval;
}

// ****************************************************************************
// Method: JMFileFormat::GetMaterialNames
//
// Purpose: 
//   Gets the material names from the file.
//
// Arguments:
//   materialNames : The material names to populate.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 14:14:16 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
JMFileFormat::GetMaterialNames(stringVector &materialNames)
{
    VarItem *ireg = ReadIREG();
    if(ireg != 0)
    {
        std::set<int> matno_set;
        int *ireg_data = (int *)ireg->data;
        for(int i = 0; i < ireg->nTotalElements; ++i)
             matno_set.insert(ireg_data[i]);
        for(std::set<int>::const_iterator it = matno_set.begin();
            it != matno_set.end(); ++it)
        {
            char tmp[10];
            SNPRINTF(tmp, 10, "%d", *it);
            materialNames.push_back(tmp);
        }
    }
}

// ****************************************************************************
// Method: JMFileFormat::PopulateDatabaseMetaData
//
// Purpose: 
//   Populates the metadata with the values from the file.
//
// Arguments:
//   md :        The metadata to populate.
//   timeState : The time state.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 13:56:45 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
JMFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    intVector nodalSize, zonalSize, curveSize;
    nodalSize = GetSize(JM_XCOORD);
    zonalSize = GetSize(JM_IREG);
    curveSize = GetSize(JM_CYCLE);

    avtMeshMetaData *mmd = new avtMeshMetaData();
    mmd->name = "mesh";
    mmd->meshType = AVT_CURVILINEAR_MESH;
    mmd->spatialDimension = 2;
    mmd->topologicalDimension = 2;
    md->Add(mmd);

    //
    // Iterate over all variables and add the problem-sized ones.
    //
    PDBfile *pdbPtr = pdb->filePointer();
    int numVars = 0;
    char **varList = PD_ls(pdbPtr, NULL /*path*/, NULL /*pattern*/, &numVars);
    if(varList != NULL)
    {
        for(int j = 0; j < numVars; ++j)
        {
            intVector sz = GetSize(varList[j]);
            int cent = 0; 
            if(sz == nodalSize)
                cent = 1;
            else if(sz == zonalSize)
                cent = 2;
            else if(sz == curveSize)
                cent = 3;

            if(cent == 1 || cent == 2)
            {
                avtScalarMetaData *smd = new avtScalarMetaData(
                    varList[j], "mesh", 
                    (cent == 1) ? AVT_NODECENT : AVT_ZONECENT);
                md->Add(smd);
            }
            else if(cent == 3)
            {
                avtCurveMetaData *cmd = new avtCurveMetaData(varList[j]);
                cmd->xLabel = "Time";
                cmd->yLabel = varList[j];
                cmd->validVariable = (strcmp(varList[j], "file_names") != 0);
                md->Add(cmd);
                curveNames.insert(varList[j]);
            }
        }

        SFREE(varList);
    }

    // Add a material
    stringVector materialNames;
    GetMaterialNames(materialNames);
    if(materialNames.size() > 0)
    {
        avtMaterialMetaData *mmd = new avtMaterialMetaData("material",
            "mesh", materialNames.size(), materialNames);
        md->Add(mmd);
    }
}

// ****************************************************************************
// Method: JMFileFormat::ReadIREG
//
// Purpose: 
//   Read IREG, converting the type if needed.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 14:38:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

JMFileFormat::VarItem *
JMFileFormat::ReadIREG()
{
    VarItem *ireg = ReadVariable(JM_IREG);
    if(ireg != 0)
    {
        // Convert from long to int
        if(ireg->type == LONGARRAY_TYPE)
        {
            int *imats = new int[ireg->nTotalElements];
            long *src = (long *)ireg->data;
            for(int i = 0; i < ireg->nTotalElements; ++i)
                imats[i] = (int)src[i];
            free_void_mem(ireg->data, ireg->type);
            ireg->data = (void*)imats;
            ireg->type = INTEGERARRAY_TYPE;
        }
    }
    return ireg;
}

// ****************************************************************************
// Method: StoreValues
//
// Purpose: 
//   Stores values from the input array into the destination array, casting
//   the values from the input array to the appropriate type.
//
// Arguments:
//   dest : The destination array.
//   stride : The stride in the destination array.
//   offset : The offset in the destination array.
//   ptr    : The source data.
//   t      : The type of the source data.
//   nvals  : The number of values in the source data.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 13:54:36 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

template <class T>
static void
StoreValues(T *dest, int stride, int offset, void *ptr, TypeEnum t, int nvals)
{
    int i;
    char   *cptr = (char *)  ptr;
    int    *iptr = (int *)   ptr;
    float  *fptr = (float *) ptr;
    double *dptr = (double *)ptr;
    long   *lptr = (long *)  ptr;

    switch(t)
    {
    case CHAR_TYPE:
    case CHARARRAY_TYPE:
        for(i = 0; i < nvals; ++i)
            dest[i * stride + offset] = (T)cptr[i];
        break;
    case INTEGER_TYPE:
    case INTEGERARRAY_TYPE:
        for(i = 0; i < nvals; ++i)
            dest[i * stride + offset] = (T)iptr[i];
        break;
    case FLOAT_TYPE:
    case FLOATARRAY_TYPE:
        for(i = 0; i < nvals; ++i)
            dest[i * stride + offset] = fptr[i];
        break;
    case DOUBLE_TYPE:
    case DOUBLEARRAY_TYPE:
        for(i = 0; i < nvals; ++i)
            dest[i * stride + offset] = (T)dptr[i];
        break;
    case LONG_TYPE:
    case LONGARRAY_TYPE:
        for(i = 0; i < nvals; ++i)
            dest[i * stride + offset] = (T)lptr[i];
        break;
    default:
        break;
    }
}

// ****************************************************************************
//  Method: JMFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 30 15:50:23 PST 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
JMFileFormat::GetMesh(int timestate, const char *meshname)
{
    vtkDataSet *ds = 0;

    if(curveNames.find(meshname) != curveNames.end())
    {
        // It's a curve
        VarItem *time = ReadVariable(JM_TIME);
        VarItem *var = ReadVariable(meshname);
        if(time == 0 || var == 0)
        {
            EXCEPTION1(InvalidVariableException, meshname);
        }

        vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(
            time->nTotalElements, VTK_FLOAT);
 
        vtkFloatArray *yc = vtkFloatArray::New();
        yc->SetNumberOfComponents(1);
        yc->SetNumberOfTuples(time->nTotalElements);
        yc->SetName(meshname);

        rg->GetPointData()->SetScalars(yc);
        vtkFloatArray *xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());

        StoreValues<float>((float *)xc->GetVoidPointer(0), 1, 0,
            time->data, time->type, time->nTotalElements);
        StoreValues<float>((float *)yc->GetVoidPointer(0), 1, 0,
            var->data, var->type, var->nTotalElements);
 
        yc->Delete();

        ds = rg;
    }
    else
    {
        // It's a mesh
        VarItem *yy = ReadVariable(JM_YCOORD);
        VarItem *xx = ReadVariable(JM_XCOORD);
        if(xx == 0)
        {
            EXCEPTION1(InvalidVariableException, "Can't read X coordinates");
        }
        if(yy == 0)
        {
            EXCEPTION1(InvalidVariableException, "Can't read Y coordinates");
        }

        int ndims = 2;
        int dims[3];
        dims[0] = xx->dims[1];
        dims[1] = xx->dims[2];
        dims[2] = 1;
        int nnodes = dims[0]*dims[1];

        //
        // Create the vtkStructuredGrid and vtkPoints objects.
        //
        vtkStructuredGrid *sgrid  = vtkStructuredGrid::New(); 
        vtkPoints         *points = vtkPoints::New();
        sgrid->SetPoints(points);
        sgrid->SetDimensions(dims);
        points->Delete();
        points->SetNumberOfPoints(nnodes);

        //
        // Copy the coordinate values into the vtkPoints object.
        //
        float *pts = (float *) points->GetVoidPointer(0);
        memset(pts, 0, sizeof(float) * nnodes * 3);
        void *xx_data = xx->DataForTime(timestate);
        void *yy_data = yy->DataForTime(timestate);
        StoreValues<float>(pts, 3, 0, xx_data, xx->type, nnodes);
        StoreValues<float>(pts, 3, 1, yy_data, yy->type, nnodes);
#ifdef REVERSE_DATA
        free_void_mem(xx_data, xx->type);
        free_void_mem(yy_data, yy->type);
#endif
        ds = sgrid;
    }

    return ds;
}


// ****************************************************************************
//  Method: JMFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 30 15:50:23 PST 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
JMFileFormat::GetVar(int timestate, const char *varname)
{
    vtkDataArray *arr = 0;
    VarItem *var = ReadVariable(varname);
    if(var == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    int nvals = var->dims[1] * var->dims[2];
    void *var_data = var->DataForTime(timestate);
    if(var->type == DOUBLE_TYPE || var->type == DOUBLEARRAY_TYPE)
    {
        arr = vtkDoubleArray::New();
        arr->SetNumberOfTuples(nvals);
        StoreValues<double>((double *)arr->GetVoidPointer(0), 1, 0,
            var_data, var->type, nvals);
    }
    else
    {
        arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(nvals);
        StoreValues<float>((float *)arr->GetVoidPointer(0), 1, 0,
            var_data, var->type, nvals);
    }
#ifdef REVERSE_DATA
    free_void_mem(var_data, var->type);
#endif

    return arr;
}


// ****************************************************************************
//  Method: JMFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 30 15:50:23 PST 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
JMFileFormat::GetVectorVar(int timestate, const char *varname)
{
    return NULL;
}

// ****************************************************************************
// Method: 
//
// Purpose: 
//   
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May  1 14:03:52 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void *
JMFileFormat::GetAuxiliaryData(const char *var, int timestep, 
    const char *type, void *args, DestructorFunction &df)
{
    void *retval = 0;
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        VarItem *ireg = ReadIREG();
        if(ireg != 0)
        {
            stringVector materialNames;
            GetMaterialNames(materialNames);
            int *matnos = new int[materialNames.size()];
            char **names = new char*[materialNames.size()];
            for(int i = 0; i < materialNames.size(); ++i)
            {
                names[i] = (char*)materialNames[i].c_str();
                sscanf(names[i], "%d", &matnos[i]);
            }

            int *matlist = (int *)ireg->DataForTime(timestep);
            int dims[3] = {1,1,1};
            int ndims = 2;
            dims[0] = ireg->dims[1];
            dims[1] = ireg->dims[2];

            df = avtMaterial::Destruct;
            retval = new avtMaterial(
                materialNames.size(),
                matnos,
                names,
                ndims,
                dims,
                0,
                matlist,
                0,
                0,
                0,
                0,
                0
                );

            delete [] names;
            delete [] matnos;
#ifdef REVERSE_DATA
            delete [] matlist;
#endif
        }
    }

    return retval;
}
