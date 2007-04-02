// ************************************************************************* //
//                            avtFVCOMParticleFileFormat.C                   //
// ************************************************************************* //
    
#include <avtFVCOMParticleFileFormat.h>
#include <avtMTSDFileFormatInterface.h>
#include <avtTypes.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkCellType.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>

#include <NETCDFFileObject.h>
#include <netcdf.h>

using     std::string;
    
// ****************************************************************************
// Method: avtFVCOMParticleFileFormat::Identify
//
// Purpose:
//   This method checks to see if the file is an FVCOM file.
//
// Arguments:
//   fileObject : The file to check.
//
// Returns:    True if the file is a particle file; False otherwise.
//
// Note:
//
// Programmer: David Stuebe
// Creation:   Thu May 4 16:18:57 PST 2006
//
// Modifications:
//
// ****************************************************************************

bool
avtFVCOMParticleFileFormat::Identify(NETCDFFileObject *fileObject)
{
    bool isFVCOM = false;

    //
    // Use the fileObject to look for something in the file that will make us
    // believe that it is a FVCOM file. The check can be as complex as you want
    // but it should not return true unless this really is an FVCOM file.
    //
    std::string source;
    if(fileObject->ReadStringAttribute("source", source))
    {
        isFVCOM = strcmp("ParticleFVCOM",source.c_str() )==0;
    }

    return isFVCOM;
}

// ****************************************************************************
// Method: avtFVCOMParticleFileFormat::CreateInterface
//
// Purpose:
//   Creates the file format interface for this reader.
//
// Programmer: David Stuebe
// Creation:   Thu May 4 16:18:57 PST 2006
//
// Modifications:
//
// ****************************************************************************

avtFileFormatInterface *
avtFVCOMParticleFileFormat::CreateInterface(NETCDFFileObject *f, 
    const char *const *list, int nList, int nBlock)
{
    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
    {
        if(f != 0)
        {
            ffl[i] = new avtFVCOMParticleFileFormat(list[i], f);
            f = 0;
        }
        else
            ffl[i] = new avtFVCOMParticleFileFormat(list[i]);
    }

    return new avtMTSDFileFormatInterface(ffl, nList);
}

// ****************************************************************************
//  Method: avtFVCOM constructor
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************


avtFVCOMParticleFileFormat::avtFVCOMParticleFileFormat(const char *filename) :
    avtMTSDFileFormat(&filename,1)
{
    fileObject = new NETCDFFileObject(filename);
}

avtFVCOMParticleFileFormat::avtFVCOMParticleFileFormat(const char *filename,
    NETCDFFileObject *f) : avtMTSDFileFormat(&filename,1)
{
    fileObject = f;
}

// ****************************************************************************
// Method: avtFVCOMParticleFileFormat::~avtFVCOMParticleFileFormat
//
// Purpose: 
//   Destructor for the avtFVCOMParticleFileFormat class.
//
// Programmer: David Stuebe
// Creation:   Thu May 4 16:18:57 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtFVCOMParticleFileFormat::~avtFVCOMParticleFileFormat()
{
    delete fileObject;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

int
avtFVCOMParticleFileFormat::GetNTimesteps(void)
{
    const char *mName = "avtFVCOMParticleFileObject::GetNTimesteps: ";

    debug4 << mName << endl;

    size_t ntimesteps;
    int status, time_id, ncid;

    ncid=fileObject->GetFileHandle();
      
    status = nc_inq_dimid(ncid, "time", &time_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, time_id, &ntimesteps);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    return ntimesteps;
}


// ****************************************************************************
// Method: avtFVCOMReader::GetTimes
//
// Purpose: 
//   Returns the times in the file.
//
// Arguments:
//   t : The times to be returned.
//
// Programmer: David Stuebe
// Creation:   Thu May 18 08:39:01 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtFVCOMParticleFileFormat::GetTimes(doubleVector &t)
{

    const char *mName = "avtFVCOMParticleFileObject::GetTimes: ";

    debug4 << mName << endl;
    
    int ncid;
    ncid=fileObject->GetFileHandle(); 


    size_t ntimesteps;
    int time_id;
    int status = nc_inq_dimid(ncid, "time", &time_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    status = nc_inq_dimlen(ncid, time_id, &ntimesteps);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    TypeEnum type = NO_TYPE;
    int ndims = 0, *dims;
    fileObject->InqVariable("time", &type, &ndims, &dims);

    if (ndims==1)
    {
        float *tf = new float[ntimesteps+1];
        fileObject->ReadVariableInto("time", type, tf);

        for(int n=0; n<ntimesteps; ++n)
        {
            t.push_back(double(tf[n]));
        }
        delete [] tf;
    }
    else
    {
        debug4 << mName << "Wrong dimension for variable Time" << endl;
    }

    delete [] dims;
}


// ****************************************************************************
// Method: avtFVCOMReader::GetCycles
//
// Purpose: 
//   Returns the time cycle in the file.
//
// Arguments:
//   cyc : The times cycle to be returned.
//
// Programmer: David Stuebe
// Creation:   Thu May 18 08:39:01 PDT 2006
//
// Modifications:
//   
// ****************************************************************************


void
avtFVCOMParticleFileFormat::GetCycles(intVector &cyc)
{
    const char *mName = "avtFVCOMParticleFileObject::GetCycles: ";

    debug4 << mName << endl;

    int ncid;
    ncid=fileObject->GetFileHandle(); 

    size_t ntimesteps;
    int time_id;
    int status = nc_inq_dimid(ncid, "time", &time_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);


    status = nc_inq_dimlen(ncid, time_id, &ntimesteps);
    if (status != NC_NOERR) fileObject-> HandleError(status);


    char varname[NC_MAX_NAME+1];
    nc_type vartype;
    int  varndims;
    int  vardims[NC_MAX_VAR_DIMS];
    int  varnatts;
    int cycle_id;
    status = nc_inq_varid (ncid, "cycle", &cycle_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    
    // Now get variable type!
    status = nc_inq_var(ncid, cycle_id, varname, &vartype, &varndims, 
            vardims, &varnatts);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    if (varndims != 1 )
    {
        debug4 << mName << "Cycles has the wrong dimensions" << endl; 
    
    }

    else if (varndims == 1)
    {
        if(vartype == NC_INT)
        {
            debug4 << "IINT returned to cyc as NC_INT" << endl;
        int *ci = new int[ntimesteps+1];
            fileObject->ReadVariableInto("iint", INTEGERARRAY_TYPE, ci);
        for(int n=0; n<ntimesteps; ++n)
            {
                cyc.push_back(ci[n]);
            }
            delete [] ci;
        }
        else if(vartype == NC_FLOAT )
        { 
            debug4 << "iint is float: Returned to cyc as INT" << endl;

            float *cf = new float[ntimesteps+1];
            fileObject->ReadVariableInto("cycle", FLOATARRAY_TYPE, cf);
   
            for(int n=0; n<ntimesteps; ++n)
            {
                cyc.push_back(int(cf[n]));
            }

            delete [] cf;
        }
    }
    else    
    {
        debug4 << "Could not return cycles: Wrong variable type" << endl;
    }
}

// ****************************************************************************
//  Method: avtFVCOMParticleFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

void
avtFVCOMParticleFileFormat::FreeUpResources(void)
{
    debug4 << "avtFVCOMParticleFileFormat::FreeUpResources: closing file." << endl;
    fileObject->Close();

    // Free any resources that you allocate per time step.

    // Since multiple time steps are in the file, it no longer really makes
    // sense to close the file here since, if you did, VisIt would open and close
    // the file when reading each time step.
}


// ****************************************************************************
//  Method: avtFVCOMParticleFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

void
avtFVCOMParticleFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int)
{
    const char *mName = "avtFVCOMParticleFileObject::PopulateDatabaseMetaData: ";

    debug4 << mName << endl;
    if(debug4_real)
        fileObject->PrintFileContents(debug4_real);

    // Assemble a database title.
    std::string comment(GetType()), titleString, source,
        modeler,institution;
    if(fileObject->ReadStringAttribute("title", titleString))
    {
        comment += (std::string(" database: title=") + titleString);

        if(fileObject->ReadStringAttribute("source", source))
            comment += (std::string(", source=") + source);
        if(fileObject->ReadStringAttribute("institution", institution))
            comment += (std::string(", institution=") + institution);
        if(fileObject->ReadStringAttribute("modeler", modeler))
            comment += (std::string(", modeler=") + modeler);

        md->SetDatabaseComment(comment);
    }

    debug4 << mName << "Finished SetDatabaseComment in MetaData" << endl;

    std::string mesh_name("Drifter");
    avtMeshMetaData *mmd = new avtMeshMetaData(mesh_name, 
      1, 1, 1, 0, 3, 0, AVT_UNSTRUCTURED_MESH);

    std::string xUnits, xLabel, yUnits, yLabel, zUnits,zLabel; 
    fileObject->ReadStringAttribute("x", "units", xUnits);
    fileObject->ReadStringAttribute("x", "long_name", xLabel);
    fileObject->ReadStringAttribute("y", "units", yUnits);
    fileObject->ReadStringAttribute("y", "long_name", yLabel);
    fileObject->ReadStringAttribute("z", "units", zUnits);
    fileObject->ReadStringAttribute("z", "long_name", zLabel);
    mmd->xUnits = xUnits;
    mmd->xLabel = xLabel;
    mmd->yUnits = yUnits;
    mmd->yLabel = yLabel;
    mmd->zUnits = zUnits;
    mmd->zLabel = zLabel;
    md->Add(mmd);
    debug4 << mName << "Added Drifter Mesh to MetaData" << endl;


    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims1;
    fileObject->InqVariable("s1", &t, &ndims, &dims1);
    if (ndims > 0)
    {
        avtScalarMetaData *smd = new avtScalarMetaData("s1", "Drifter", AVT_NODECENT);
        smd->hasUnits = fileObject->ReadStringAttribute("s1", "units", smd->units);
        md->Add(smd);
    }

    int *dims2;
    fileObject->InqVariable("s2", &t, &ndims, &dims2);
    if (ndims > 0)
    {
        avtScalarMetaData *smd2 = new avtScalarMetaData("s2", "Drifter", AVT_NODECENT);
        smd2->hasUnits = fileObject->ReadStringAttribute("s2", "units", smd2->units);
        md->Add(smd2);
    }
    delete [] dims1;
    delete [] dims2;

    avtScalarMetaData *zmd = new avtScalarMetaData("z", "Drifter", AVT_NODECENT);
    zmd->hasUnits = fileObject->ReadStringAttribute("z", "units", zmd->units);
    md->Add(zmd);

    debug4 << mName << "end" << endl;
}


// ****************************************************************************
//  Method: avtFVCOMParticleFileFormat::GetMesh
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
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

vtkDataSet *
avtFVCOMParticleFileFormat::GetMesh(int timestate, const char *mesh)
{
    const char *mName = "avtFVCOMParticleFileFormat::GetMesh: ";
    debug4 << mName << "meshname=" << mesh  << " timestate="
           << timestate << endl;

    vtkDataSet *retval = 0;
 
        
    // Bail out if we can't get the file handle.
    int ncid = fileObject->GetFileHandle();
    if(ncid == -1)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    if(strcmp(mesh, "Drifter") == 0)
    {
        int status;
        size_t nlag;
        int nlag_id;
        status = nc_inq_dimid(ncid, "nlag", &nlag_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
        status = nc_inq_dimlen(ncid, nlag_id, &nlag);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        debug4 << "nlag = " << nlag << endl;

        float *x = new float[nlag];
        float *y = new float[nlag];
        float *z = new float[nlag];

        // Fill x,y,z from NETCDF file

        // Need to specify time step to read SSH: timestate
        debug4 << "timestate=" << timestate << endl;

        size_t starts[]={timestate,0};
        size_t counts[]={1, nlag};
        ptrdiff_t stride[]={1,1};
        int x_id, y_id,z_id;

        //READ X
        status = nc_inq_varid (fileObject->GetFileHandle(), "x", &x_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
        //    debug4 << "Status="<< status <<", var_id="<< var_id << endl;
        status = nc_get_vars_float(fileObject->GetFileHandle(),x_id,
                                   starts, counts, stride, x);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        //READ Y
        status = nc_inq_varid (fileObject->GetFileHandle(), "y", &y_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
        //    debug4 << "Status="<< status <<", var_id="<< var_id << endl;
        status = nc_get_vars_float(fileObject->GetFileHandle(),y_id,
                                   starts, counts, stride, y);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        // READ Z
        status = nc_inq_varid (fileObject->GetFileHandle(), "z", &z_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
        //    debug4 << "Status="<< status <<", var_id="<< var_id << endl;

        status = nc_get_vars_float(fileObject->GetFileHandle(),z_id,
                               starts, counts, stride, z);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        debug4 << "Adding points to vtkPoints: HERE I AM"  << endl;
        // Store the points in the vtkPoints object.
        vtkPoints *p = vtkPoints::New();
        p->SetNumberOfPoints(nlag);
        for (int i = 0 ; i < nlag ; i++)
        {
            p->SetPoint(i, x[i], y[i], z[i]);
            debug4 << "[" << x[i] << ", "  << y[i] << ", "  << z[i] << "]" << endl;
        }
      
        delete [] x;
        delete [] y;
        delete [] z;

        // Create an unstructured grid of "vertex cells" that use the 
        // vtkPoints that we created.
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(p);
        p->Delete();
        ugrid->Allocate(nlag);
        vtkIdType onevertex[1];
        for (int i = 0 ; i < nlag ; i++)
        {
            onevertex[0] = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
        }

        return ugrid;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    return retval;
}


// ****************************************************************************
//  Method: avtFVCOMParticleFileFormat::GetVar
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
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

vtkDataArray *
avtFVCOMParticleFileFormat::GetVar(int timestate, const char *varname)
{
    const char *mName = "avtFVCOMParticleFileFormat::GetMesh: ";
    debug4 << mName << "Varname=" << varname  << " timestate="
           << timestate << endl;


    // Bail out if we can't get the file handle.
    int ncid = fileObject->GetFileHandle(); 
    if(ncid == -1)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
    int status;
    size_t nlag;
    int nlag_id;
    status = nc_inq_dimid(ncid, "nlag", &nlag_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, nlag_id, &nlag);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    size_t starts[]={timestate,0};
    size_t counts[]={1, nlag};
    ptrdiff_t stride[]={1,1};
    int z_id, s1_id, s2_id;
    
    if (strcmp("s1", varname)==0)
    { // READ s1
        float *s1 = new float[nlag];
        status = nc_inq_varid (ncid, "s1", &s1_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
        status = nc_get_vars_float(fileObject->GetFileHandle(),s1_id,
                               starts, counts, stride, s1);
        if (status != NC_NOERR) fileObject-> HandleError(status);
     
        // Put the data into the vtkFloatArray   
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(nlag);
        for (int i = 0 ; i < nlag ; i++)
        {
            rv->SetTuple1(i, s1[i]);  // you must determine value for ith entry.
        }

        delete [] s1;
    
        return rv;
    }
    else if (strcmp("s2", varname)==0)
    { // READ s2
        float *s2 = new float[nlag];
        status = nc_inq_varid (ncid, "s2", &s2_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
        status = nc_get_vars_float(fileObject->GetFileHandle(),s2_id,
                               starts, counts, stride, s2);
        if (status != NC_NOERR) fileObject-> HandleError(status);
     
        // Put the data into the vtkFloatArray   
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(nlag);
        for (int i = 0 ; i < nlag ; i++)
        {
            rv->SetTuple1(i, s2[i]);  // you must determine value for ith entry.
        }

        delete [] s2;
    
        return rv;
    }
    else if (strcmp("z", varname)==0)
    {
        float *z = new float[nlag];

        // READ Z
        status = nc_inq_varid (fileObject->GetFileHandle(), "z", &z_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
        //    debug4 << "Status="<< status <<", var_id="<< var_id << endl;

        status = nc_get_vars_float(fileObject->GetFileHandle(),z_id,
                               starts, counts, stride, z);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        // Put the data into the vtkFloatArray   
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(nlag);
        for (int i = 0 ; i < nlag ; i++)
        {
            rv->SetTuple1(i, z[i]);  // you must determine value for ith entry.
        }

        delete [] z;
    
        return rv;
    }

    return 0;
}


// ****************************************************************************
//  Method: avtFVCOMParticleFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      var        The name of the variable requested.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

vtkDataArray *
avtFVCOMParticleFileFormat::GetVectorVar(int timestate, const char *var)
{
    EXCEPTION1(InvalidVariableException, var);

    // YOU MUST IMPLEMENT THIS IF YOU HAVE VECTOR VARIABLES.

    //
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION1(InvalidVariableException, var);
    //

    //
    // If you do have a vector variable, here is some code that may be helpful.
    //
    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3.
    // int ntuples = XXX; // this is the number of entries in the variable.
    // vtkFloatArray *rv = vtkFloatArray::New();
    // int ucomps = (ncomps == 2 ? 3 : ncomps);
    // rv->SetNumberOfComponents(ucomps);
    // rv->SetNumberOfTuples(ntuples);
    // float *one_entry = new float[ucomps];
    // for (int i = 0 ; i < ntuples ; i++)
    // {
    //      int j;
    //      for (j = 0 ; j < ncomps ; j++)
    //           one_entry[j] = ...
    //      for (j = ncomps ; j < ucomps ; j++)
    //           one_entry[j] = 0.;
    //      rv->SetTuple(i, one_entry); 
    // }
    //
    // delete [] one_entry;
    // return rv;
    //
}
