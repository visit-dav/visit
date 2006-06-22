// ************************************************************************* //
//                            avtFVCOMReader.C                               //
// ************************************************************************* //

#include <avtFVCOMReader.h>
    

#include <string>

#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>

#include <NETCDFFileObject.h>
#include <netcdf.h>


using     std::string;

// ****************************************************************************
// Method: avtFVCOMReader::Identify
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
avtFVCOMReader::Identify(NETCDFFileObject *fileObject)
{
    bool isFVCOM = false;

    // Simple statement to identify FVCOM files:
    // Do not change source statement in mod_ncdio !!!

    std::string source;
    if(fileObject->ReadStringAttribute("source", source))
    {
      isFVCOM = strncmp("FVCOM",source.c_str(),5)==0;
    }

    return isFVCOM;
}

// ****************************************************************************
//  Method: avtFVCOM constructor
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************


avtFVCOMReader::avtFVCOMReader(const char *filename)
{
    fileObject = new NETCDFFileObject(filename);
}

avtFVCOMReader::avtFVCOMReader(const char *filename, NETCDFFileObject *f)
{
    fileObject = f;
}

// ****************************************************************************
// Method: avtFVCOMReader::~avtFVCOMReader
//
// Purpose: 
//   Destructor for the avtFVCOMReader class.
//
// Programmer: David Stuebe
// Creation:   Thu May 4 16:18:57 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtFVCOMReader::~avtFVCOMReader()
{
    delete fileObject;
}

// ****************************************************************************
// Method: avtFVCOMReader::FreeUpResources
//
// Purpose: 
//   Frees up the resources that the file is using.
//
// Programmer: David Stuebe
// Creation:   Tue Jun 20 14:22:37 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtFVCOMReader::FreeUpResources()
{
    debug4 << "avtFVCOMReader::FreeUpResources: closing file." << endl;
    fileObject->Close();
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
avtFVCOMReader::GetNTimesteps(void)
{
    const char *mName = "avtFVCOMReader::GetNTimesteps: ";
    debug4 << mName << endl;

    size_t ntimesteps;
    int status, time_id, ncid;

    ncid=fileObject->GetFileHandle();
      
    status = nc_inq_dimid(ncid, "time", &time_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, time_id, &ntimesteps);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    debug4 << mName << "ntimesteps=" << ntimesteps << endl;
    debug4 << mName << "end" << endl;

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
avtFVCOMReader::GetTimes(doubleVector &t)
{
    const char *mName = "avtFVCOMReader::GetTimes: ";
    debug4 << mName << endl;

    int ncid;
    ncid=fileObject->GetFileHandle(); 

    size_t ntimesteps;
    int time_id;
    int status = nc_inq_dimid(ncid, "time", &time_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    
    status = nc_inq_dimlen(ncid, time_id, &ntimesteps);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    float *tf = new float[ntimesteps];
    fileObject->ReadVariableInto("time", FLOATARRAY_TYPE, tf);
    for(int n=0; n<ntimesteps; ++n)
        t.push_back(double(tf[n]));

    delete [] tf;

    debug4 << mName << "Got Times" << endl;
    debug4 << mName << "end" << endl;
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
avtFVCOMReader::GetCycles(intVector &cyc)
{
    const char *mName = "avtFVCOMReader::GetCycles: ";
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
    int iint_id;
    status = nc_inq_varid (ncid, "iint", &iint_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
     
    // Now get variable type!
    status = nc_inq_var(ncid, iint_id, varname, &vartype, &varndims, 
            vardims, &varnatts);
    if (status != NC_NOERR) fileObject-> HandleError(status);
  

    // Now get variable based on type
    if(vartype == NC_INT)
    {
        debug4 << mName << "IINT returned to cyc as NC_INT" << endl;
        int *ci = new int[ntimesteps];
        fileObject->ReadVariableInto("iint", INTEGERARRAY_TYPE, ci);
        for(int n=0; n<ntimesteps; ++n)
        {
            cyc.push_back(ci[n]);
        }
        delete [] ci;
    }
    else if(vartype == NC_FLOAT )
    { 
        debug4 << mName << "IINT returned to cyc as NC_FLOAT: Convert to INT" << endl;

        float *cf = new float[ntimesteps];
        fileObject->ReadVariableInto("iint", FLOATARRAY_TYPE, cf);

        // If float returned, change to double for VisIt   
        for(int n=0; n<ntimesteps; ++n)
        {
            cyc.push_back(int(cf[n]));
        }
        delete [] cf;
    }
    else    
    {
        debug4 << mName << "Could not return cycles: Wrong variable type" << endl;
    }

    debug4 << mName << "Got cycles" << endl;
    debug4 << mName << "end" << endl;
}


// ****************************************************************************
//  Method: avtFVCOMReader::PopulateDatabaseMetaData
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
avtFVCOMReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md, 
    const int ts, const char *dbtype)
{
    const char *mName = "avtFVCOM::PopulateDatabaseMetaData: ";

    debug4 << mName << endl;
    if(debug4_real)
        fileObject->PrintFileContents(debug4_real);

    // Assemble a database title.
    std::string comment(dbtype), titleString, source,
        history, references, conventions;
    if(fileObject->ReadStringAttribute("title", titleString))
    {
        comment += (std::string(" database: title=") + titleString);

        if(fileObject->ReadStringAttribute("source", source))
            comment += (std::string(", source=") + source);

        if(fileObject->ReadStringAttribute("history", history))
            comment += (std::string(", history=") + history);
        if(fileObject->ReadStringAttribute("references", references))
            comment += (std::string(", references=") + references);
        if(fileObject->ReadStringAttribute("Conventions", conventions))
            comment += (std::string(", Conventions=") + conventions);

        md->SetDatabaseComment(comment);
    }

    debug4 << mName <<"Finished SetDatabaseComments" << endl;

    //--------------------------------------------------------------------
    //---------------------------------------------------------------
    // Add the Bathymetry_Mesh.
    //
    std::string meshName_BM("Bathymetry_Mesh");
    avtMeshMetaData *md_BM = new avtMeshMetaData(meshName_BM, 
      1, 1, 1, 0, 3, 2, AVT_UNSTRUCTURED_MESH);

    //  Get the units for the mesh.
    std::string xUnits, xLabel, yUnits, yLabel, zUnits,zLabel; 
    fileObject->ReadStringAttribute("x", "units", xUnits);
    fileObject->ReadStringAttribute("x", "long_name", xLabel);
    fileObject->ReadStringAttribute("y", "units", yUnits);
    fileObject->ReadStringAttribute("y", "long_name", yLabel);
    fileObject->ReadStringAttribute("h", "units", zUnits);
    fileObject->ReadStringAttribute("h", "long_name", zLabel);
    md_BM->xUnits = xUnits;
    md_BM->xLabel = xLabel;
    md_BM->yUnits = yUnits;
    md_BM->yLabel = yLabel;
    md_BM->zUnits = zUnits;
    md_BM->zLabel = zLabel;
    md->Add(md_BM);
   
    debug4 << mName << "Added Bathymetry Mesh to MetaData" << endl;
    //-------------------------------------------------------------------
    //
    // Add the SSH_Mesh.
    //
    std::string meshName_SSH("SSH_Mesh");
    avtMeshMetaData *md_SSH = new avtMeshMetaData(meshName_SSH, 
      1, 1, 1, 0, 3, 2, AVT_UNSTRUCTURED_MESH);

    // Get the units for the mesh.
    fileObject->ReadStringAttribute("x", "units", xUnits);
    fileObject->ReadStringAttribute("x", "long_name", xLabel);
    fileObject->ReadStringAttribute("y", "units", yUnits);
    fileObject->ReadStringAttribute("y", "long_name", yLabel);
    fileObject->ReadStringAttribute("zeta", "units", zUnits);
    fileObject->ReadStringAttribute("zeta", "long_name", zLabel);
    md_SSH->xUnits = xUnits;
    md_SSH->xLabel = xLabel;
    md_SSH->yUnits = yUnits;
    md_SSH->yLabel = yLabel;
    md_SSH->zUnits = zUnits;
    md_SSH->zLabel = zLabel;
    md->Add(md_SSH);
    debug4 << mName << "Added SSH Mesh to MetaData" << endl;
    //-------------------------------------------------------------------
    //
    // Add the SigmaLayer_Mesh.
    //
    std::string meshName_LAY("SigmaLayer_Mesh");
    avtMeshMetaData *md_LAY = new avtMeshMetaData(meshName_LAY, 
      1, 1, 1, 0, 3, 3, AVT_UNSTRUCTURED_MESH);

    // Get the units for the mesh:
    fileObject->ReadStringAttribute("x", "units", xUnits);
    fileObject->ReadStringAttribute("x", "long_name", xLabel);
    fileObject->ReadStringAttribute("y", "units", yUnits);
    fileObject->ReadStringAttribute("y", "long_name", yLabel);
    fileObject->ReadStringAttribute("zeta", "units", zUnits);
    fileObject->ReadStringAttribute("zeta", "long_name", zLabel);
    md_LAY->xUnits = xUnits;
    md_LAY->xLabel = xLabel;
    md_LAY->yUnits = yUnits;
    md_LAY->yLabel = yLabel;
    md_LAY->zUnits = zUnits;
    md_LAY->zLabel = zLabel;
    md->Add(md_LAY);
    debug4 << mName << "Added Sigma Layer Mesh to MetaData" << endl;

    //------------------------------------------------------------------
    // Add the SigmaLevel_Mesh.
    //
    std::string meshName_LEV("SigmaLevel_Mesh");
    avtMeshMetaData *md_LEV = new avtMeshMetaData(meshName_LEV, 
      1, 1, 1, 0, 3, 3, AVT_UNSTRUCTURED_MESH);

    // Get the units for the mesh.
    fileObject->ReadStringAttribute("x", "units", xUnits);
    fileObject->ReadStringAttribute("x", "long_name", xLabel);
    fileObject->ReadStringAttribute("y", "units", yUnits);
    fileObject->ReadStringAttribute("y", "long_name", yLabel);
    fileObject->ReadStringAttribute("zeta", "units", zUnits);
    fileObject->ReadStringAttribute("zeta", "long_name", zLabel);
    md_LEV->xUnits = xUnits;
    md_LEV->xLabel = xLabel;
    md_LEV->yUnits = yUnits;
    md_LEV->yLabel = yLabel;
    md_LEV->zUnits = zUnits;
    md_LEV->zLabel = zLabel;
    md->Add(md_LEV);
    debug4 << mName << "Added SigmaLevel Mesh to MetaData" << endl;
    //-------------------------------------------------------
    //--------------------------------------------------------------------
    //--------------------------------------------------------------------
    // Add Variables!
    //
    debug4<< mName << "Add variables to meta data" << endl;

    // Inquire about the number of dims, attributes, vars.
    int ncid;
    ncid=fileObject->GetFileHandle(); 
    int i, status, nDims, nVars, nGlobalAtts, unlimitedDimension;
    status = nc_inq(ncid, &nDims, &nVars, &nGlobalAtts, &unlimitedDimension);
    if(status != NC_NOERR)
    {
        fileObject-> HandleError(status);
    }

    // Get the sizes of all dimensions. 
    //         (Referenced to get the real dimension of each variable)
    //          ( Example:  dimSizes[vardims[1]]  )
    size_t *dimSizes = new size_t[nDims];
    for(i = 0; i < nDims; ++i)
    {
        int status = nc_inq_dimlen(ncid, i, &dimSizes[i]);
        if(status != NC_NOERR)
            fileObject->HandleError(status);
    }

    // Get dimensions of time, siglay, siglev, nodes and cells to compare each variable:
    // That is how we identify what the variable is and how to read it.
    size_t ntimesteps;
    int time_id;
    status = nc_inq_dimid(ncid, "time", &time_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, time_id, &ntimesteps);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    size_t nSigLayers, nSigLevels, nNodesPerLayer, nCellsPerLayer;
    int siglay_id, siglev_id, node_id, nele_id;

    status = nc_inq_dimid(ncid, "siglay", &siglay_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, siglay_id, &nSigLayers);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    status = nc_inq_dimid(ncid, "siglev", &siglev_id);
    if (status != NC_NOERR) fileObject->HandleError(status);
    status = nc_inq_dimlen(ncid, siglev_id, &nSigLevels);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    
    status = nc_inq_dimid(ncid, "node", &node_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, node_id, &nNodesPerLayer);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    status = nc_inq_dimid(ncid, "nele", &nele_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, nele_id, &nCellsPerLayer);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    int got_velocity=0;
    int known_var =0; // is this a recognized variable type: scalar (1) or vector (2)
    int what_type = 0; // is the variable a float (1) or int (2)
    debug4 << "nVars = " << nVars << endl;
    debug4 << "Finding all variable to plot on the grid!" << endl;
    for(i = 0; i < nVars; ++i)
    {
        // First identify variable type!
        debug4 << "Examining variable#" << i ;

        known_var=0;
        what_type=0;
        char varname[NC_MAX_NAME+1];
        nc_type vartype;
        int  varndims;
        int  vardims[NC_MAX_VAR_DIMS];
        int  varnatts;
        if((status = nc_inq_var(ncid, i, varname, &vartype, &varndims, 
                                vardims, &varnatts)) == NC_NOERR)
        {
            debug4 << " ;" << varname << endl;
            debug4 << "variable type";
            if(vartype == NC_BYTE)
                debug4 << "NC_BYTE";
            else if(vartype == NC_CHAR)
                debug4 << "NC_CHAR";
            else if(vartype == NC_SHORT)
                debug4 << "NC_SHORT";
            else if(vartype == NC_INT)
            {
                debug4 << "NC_INT";
                what_type=2; // if int set what_type=2
            }
            else if(vartype == NC_FLOAT)
            { 
                debug4 << "NC_FLOAT";
                what_type = 1;  // if float set what_type=1
            }
            else if(vartype == NC_DOUBLE)
                debug4 << "NC_DOUBLE";
            else 
                debug4 << "unknown type";
        }
        else  // if(status = NC_NOERR
        {
            debug4 << "Could not nc_inq_var??? Try next variable";
            fileObject-> HandleError(status);
        }   // end if(status = NC_NOERR)

        debug4<< endl; // this ends either,
        // 'examining variable' or 'variable type'

        // Make variable for variable location
        avtCentering centering = AVT_NODECENT;
        // Make variable for mesh type (bathy, ssh, level, layer)
        string *var_mesh;

        // There are too many if statements nested here, 
        // but I can't think of a simpler way to to do this.

        if(what_type==1) 
        {
            // Match dimensions to a particular grid and add the variable 
            // to the meta data
            debug4 << "Var "<< varname << " is float: Has ndims="
                   << varndims << endl;
       
            if (varndims ==3) // Get 3D node center scalars
            {
                if(dimSizes[vardims[1]]== nSigLayers && 
                   dimSizes[vardims[2]]== nNodesPerLayer) 
                {   // Time must be the 0rd dim
                    var_mesh = &meshName_LAY;
                    centering = AVT_NODECENT;  // Most scalar variables are here
                    known_var=1;
                }
                // Now for velocity and turbulent variables! nCellsPerLayer
                else if(dimSizes[vardims[1]]== nSigLayers && 
                        dimSizes[vardims[2]]== nCellsPerLayer) 
                {   // Time must be the 0rd dim
                    std::string unknown_units;
                    fileObject->ReadStringAttribute(varname, "units",
                        unknown_units);  
                    // Check how many velocity variable we have:
                    // What is the are the units of the variable?
                    if(got_velocity == 0 && 
                       strcmp("meters s-1", unknown_units.c_str() )==0) 
                    {
                        // This is the first variable that looks like a velocity
                        // Wait for the second to add only one velocity variable.
                        got_velocity=got_velocity+1;
                    }
                    // Is it the diffusivity?
                    // Diffusivity has changed location: bug fix
                    // fvcom<2.5 has km on zonecent
                    // fvcom 2.5 has km on nodecent
                    else if(strcmp("meters2 s-1", unknown_units.c_str() )==0 )
                    {
                        known_var=1; // scalar!  KM for zonecent 
                        centering=AVT_ZONECENT; 
                        var_mesh = &meshName_LEV;
                    }
                    else if (got_velocity==1 && 
                             strcmp("meters s-1", unknown_units.c_str() )==0)
                    {
                        known_var=2; // Second velocity: add 2D vel (U V)
                        var_mesh = &meshName_LEV;
                        centering = AVT_ZONECENT;
                        // This puts velocity in natural location
                        got_velocity=got_velocity+1;
                    }
                    else if(got_velocity == 2 && 
                            strcmp("meters s-1", unknown_units.c_str() )==0)
                    {   // if there is a third velocity get 3d vel
                        known_var=3; // Vector 3D too!
                        var_mesh = &meshName_LEV;
                        centering = AVT_ZONECENT;              
                    }
                } // end if vardims ....
                else if(dimSizes[vardims[1]]== nSigLevels && 
                        dimSizes[vardims[2]]== nNodesPerLayer) 
                {   // Time must be the 0rd dim
                    var_mesh = &meshName_LEV;
                    centering = AVT_NODECENT;  // This is KM in FVCOM2.5+
                    known_var=1;
                }

            } // end if varndims=3        
            else if (varndims ==2) // Get Sea surface height
            {
                if(dimSizes[vardims[0]]== ntimesteps && 
                   dimSizes[vardims[1]]== nNodesPerLayer)
                {
                    var_mesh = &meshName_SSH;
                    centering = AVT_NODECENT;
                    known_var=1;
                } // end if vardims ....
            } // end if varndims=2...    
            else if(varndims==1) // Get Bathymetry
            {
                if(dimSizes[vardims[0]]== nNodesPerLayer &&
                   strncmp("h", varname,1)==0 )
                { // must compare varname, x, y, lat and lon have same dimension 
                    var_mesh= &meshName_BM;
                    centering= AVT_NODECENT;
                    known_var=1;
                }
            } // end if varndims==1

            if (known_var==1) // add the scalar variable
            {
                avtScalarMetaData *smd = new avtScalarMetaData(varname,
                    *var_mesh, centering);
                smd->hasUnits = fileObject->ReadStringAttribute(varname,
                    "units", smd->units);
                md->Add(smd);
            }
            else if (known_var==2) // add the variable
            {
                avtVectorMetaData *smd = new avtVectorMetaData("Velocity2D",
                    *var_mesh, centering,3);
                smd->hasUnits = fileObject->ReadStringAttribute(varname, 
                    "units", smd->units);
                md->Add(smd);
            }
            else if (known_var==3)
            {
                avtVectorMetaData *smd = new avtVectorMetaData("Velocity3D",
                    *var_mesh, centering,3);
                smd->hasUnits = fileObject->ReadStringAttribute(varname, 
                    "units", smd->units);
                md->Add(smd);
            }
            // if know_var==0  do nothing!
        }// end if what type ==1 : this is for Floats!
    
    // add nprocs and other INTs here using: what_type==2
    } // end for nvars !!!

    delete [] dimSizes;

    debug4 << "avtFVCOMReader: end PopulateDatabaseMetaData"<<endl;
}


// ****************************************************************************
//  Method: avtFVCOMReader::GetMesh
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
avtFVCOMReader::GetMesh(int timestate, const char *mesh)
{
    const char *mName = "avtFVCOMReader::GetMesh: ";
    debug4 << mName << "meshname=" << mesh  << " timestate="
           << timestate << endl;

    vtkDataSet *retval = 0;

    // Bail out if we can't get the file handle.
    int ncid = fileObject->GetFileHandle();
    if(ncid == -1)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    //Get Dimensions of the variables on grid
    size_t nSigLayers, nSigLevels, nNodesPerLayer, nCellsPerLayer;
    int status, siglay_id, siglev_id, node_id, nele_id;
     
    status = nc_inq_dimid(ncid, "siglay", &siglay_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, siglay_id, &nSigLayers);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    status = nc_inq_dimid(ncid, "siglev", &siglev_id);
    if (status != NC_NOERR) fileObject->HandleError(status);
    status = nc_inq_dimlen(ncid, siglev_id, &nSigLevels);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    
    status = nc_inq_dimid(ncid, "node", &node_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, node_id, &nNodesPerLayer);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    status = nc_inq_dimid(ncid, "nele", &nele_id);
    if (status != NC_NOERR) fileObject-> HandleError(status);
    status = nc_inq_dimlen(ncid, nele_id, &nCellsPerLayer);
    if (status != NC_NOERR) fileObject-> HandleError(status);

    debug4 << "avtFVCOMReader::GetMesh; Got mesh dimensions"<< endl;
    debug4 << "avtFVCOMReader::GetMesh; nNodesPerLayer=" << nNodesPerLayer << endl;
    debug4 << "avtFVCOMReader::GetMesh: nCellsPerLayer=" << nCellsPerLayer << endl;

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    // MAKE THE MESH FOR EACH TYPE
    if(strcmp(mesh, "Bathymetry_Mesh") == 0)
    {
        debug4 << mName << "Getting Bathymetry_Mesh" << endl;
        // for Bathymetry mesh we only need one layer, only nNodesPerLayer nodes!
        vtkPoints *pts = vtkPoints::New();
        pts->Allocate(nNodesPerLayer);

        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(pts);
        ugrid->Allocate(nCellsPerLayer);
        pts->Delete();
        debug4 << "Allocated ugrid and pts" << endl;

        // Get Node locations  
        float *xvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("x", FLOATARRAY_TYPE, xvals);
          
        float *yvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("y", FLOATARRAY_TYPE, yvals);
    
        float *zvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("h", FLOATARRAY_TYPE, zvals);


        // Get Node Connectivity
        int *nvvals= new int[3 * nCellsPerLayer];
        fileObject->ReadVariableInto("nv", INTEGERARRAY_TYPE, nvvals);

        debug4 << mName << "Read nodes locations and connectivity"<< endl;

        // insert nodes into mesh pts object
        for(int i= 0; i< nNodesPerLayer; ++i)
        {
            float pt[3]={xvals[i], yvals[i],-zvals[i]};
            pts->InsertNextPoint(pt);
        }
  

        // insert cells in to mesh cell object
        vtkIdType verts[3];
        for(int cell = 0; cell < nCellsPerLayer; ++cell)
        {
            for(int vrt =0; vrt <3; ++vrt)
            {
                verts[vrt] = nvvals[vrt * nCellsPerLayer + cell] -1;
            }
            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        }

        delete [] xvals;
        delete [] yvals;
        delete [] zvals;
        delete [] nvvals;

        debug4 << mName;
        debug4 << "Success Returning VTK_TRIANCLE for Bathymetry" << endl;

        retval = ugrid;
    } // End if Bathymetry_Mesh
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
    else if(strcmp(mesh, "SSH_Mesh") == 0) // is mesh: SSH_Mesh
    {    
        debug4 << mName << "Getting SSH_Mesh"<< endl;
        // for SSH mesh we only need one layer, only nNodesPerLayer nodes!
        vtkPoints *pts = vtkPoints::New();
        pts->Allocate(nNodesPerLayer);

        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(pts);
        ugrid->Allocate(nCellsPerLayer);
        pts->Delete();

        debug4 << "Allocated ugrid and pts" << endl;

        // Get Node locations
        float *xvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("x", FLOATARRAY_TYPE, xvals);
          
        float *yvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("y", FLOATARRAY_TYPE, yvals);
    
        // Need to specify time step to read SSH: timestate
        debug4 << "timestate=" << timestate << endl;

        size_t starts[]={timestate,0};
        size_t counts[]={1, nNodesPerLayer};
        ptrdiff_t stride[]={1,1};
        float *zvals = new float[nNodesPerLayer];
        int var_id;
        status = nc_inq_varid (fileObject->GetFileHandle(), "zeta", &var_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
            starts, counts, stride, zvals);
        if (status != NC_NOERR) fileObject-> HandleError(status);


        // Get Node Connectivity
        int *nvvals= new int[3 * nCellsPerLayer];
        fileObject->ReadVariableInto("nv", INTEGERARRAY_TYPE, nvvals);

        debug4 << mName;
        debug4 << "Read nodes locations and connectivity"<< endl;

        // insert nodes into mesh pts object
        for(int i= 0; i< nNodesPerLayer; ++i)
        {
            float pt[3]={xvals[i], yvals[i], zvals[i]};
            pts->InsertNextPoint(pt);
        } // end for nNodesPerLayer

        // insert cells in to mesh cell object
        vtkIdType verts[3];
        for(int cell = 0; cell < nCellsPerLayer; ++cell)
        {
            for(int vrt =0; vrt <3; ++vrt)
            {
                verts[vrt] = nvvals[vrt * nCellsPerLayer + cell] -1;
            }
            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        } // End for nCellsPerLayer

        delete [] xvals;
        delete [] yvals;
        delete [] zvals;
        delete [] nvvals;

        debug4 << mName;
        debug4 << "Success Returning VTK_TRIANCLE for ssh mesh" << endl;

        retval = ugrid;
    } // End if SSH_Mesh

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
    else if(strcmp(mesh, "SigmaLayer_Mesh") == 0)
    { // SigmaLayer_Mesh
        // Plot scalars which are calculated at cell edge centers
        // On this grid these scalars will be on cell nodes
        
        debug4<< mName << "Getting SigmaLayer_Mesh" << endl;
        int nNodes = nNodesPerLayer * (nSigLevels-1);
        int nCells = nCellsPerLayer * (nSigLayers-1);

        vtkPoints *pts = vtkPoints::New();
        pts->Allocate(nNodes);

        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(pts);
        ugrid->Allocate(nCells);
        pts->Delete();

        debug4 << "Allocated ugrid and pts" << endl;

        // Get Node locations
        // X
        float *xvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("x", FLOATARRAY_TYPE, xvals);

        // Y  
        float *yvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("y", FLOATARRAY_TYPE, yvals);

        // Bathymetry
        float *zvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("h", FLOATARRAY_TYPE, zvals);

        // sigmalevels
        float *SigLayers = new float[nSigLayers];
        fileObject->ReadVariableInto("siglay", FLOATARRAY_TYPE, SigLayers); 

        // Need to specify time step to read SSH: timestate
        debug4 << "timestate=" << timestate << endl;

        size_t starts[]={timestate,0};
        size_t counts[]={1, nNodesPerLayer};
        ptrdiff_t stride[]={1,1};
        float *sshvals = new float[nNodesPerLayer];
        int var_id;
        status = nc_inq_varid (fileObject->GetFileHandle(), "zeta", &var_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);
       
        // SSH
        status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
            starts, counts, stride, sshvals);
        if (status != NC_NOERR) fileObject-> HandleError(status);
       

        // Get Node Connectivity
        int *nvvals= new int[3 * nCellsPerLayer];
        fileObject->ReadVariableInto("nv", INTEGERARRAY_TYPE, nvvals);
        debug4 << mName; 
        debug4 << "Read nodes locations and connectivity"<< endl;

        // insert nodes into mesh pts object
        float depth;
        for(int lay = 0; lay< nSigLayers; ++lay) // order! surface -> bottom
        {
            for(int node= 0; node< nNodesPerLayer; ++node)
            {
                depth= SigLayers[lay]*(zvals[node]+sshvals[node])+sshvals[node];
                float pt[3]={xvals[node], yvals[node], depth};
                pts->InsertNextPoint(pt);
            }// end for nNodesPerLayer
        }// end for nSigLevels
        debug4 << "Inserted points" << endl;

        // insert cells into mesh cell object
        vtkIdType verts[6];

        for(int lay = 0; lay < (nSigLayers-1); ++lay)
        {
            for(int cell = 0; cell < nCellsPerLayer; ++cell)
            {
                for(int TB=0; TB<2; ++TB) // TopOrBottom of cell!
                    for(int vrt =0; vrt <3; ++vrt)
                    {
                        verts[vrt + TB*3] = lay*nNodesPerLayer + 
                        // offset each layer by the number of nodes

                        (nvvals[vrt * nCellsPerLayer + cell]-1) + 
                        // offset each nvval by the number of cells per layer

                        TB*nNodesPerLayer;
                        // offset the bottom nodes from the top by nNodesPerLayer

                    } // End for verts per cell and TB

                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            } // End for nCellsPerLayer
        } // End For nSigLayers

        delete [] xvals;
        delete [] yvals;
        delete [] zvals;
        delete [] nvvals;
        delete [] sshvals;
        delete [] SigLayers;

        debug4 << mName;
        debug4 << "Success Returning VTK_WEDGE: SigmaLayerMesh" << endl;

        retval = ugrid;
    } // end SigmaLayers Mesh!
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
    else if(strcmp(mesh, "SigmaLevel_Mesh") == 0)
    {   // is mesh: SigmaLevel_Mesh
        // plot turbulent quantities which occur at cell nodes (fvcom 2.5+)
        // plot turbulent quantities which occur at cell center (fvcom <2.5)
        // [Note cell center km is not at the right level!]

        // plot velocity at cell centers
        debug4 << mName << "Getting SigmaLevel Mesh" << endl;

        int nNodes = nNodesPerLayer * nSigLevels;
        int nCells = nCellsPerLayer * nSigLayers;

        vtkPoints *pts = vtkPoints::New();
        pts->Allocate(nNodes);

        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(pts);
        ugrid->Allocate(nCells);
        pts->Delete();
        debug4 << "Allocated ugrid and pts" << endl;

        // Get Node locations
        // X
        float *xvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("x", FLOATARRAY_TYPE, xvals);

        // Y  
        float *yvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("y", FLOATARRAY_TYPE, yvals);

        // Bathymetry
        float *zvals = new float[nNodesPerLayer];
        fileObject->ReadVariableInto("h", FLOATARRAY_TYPE, zvals);

        // sigmalevels
        float *SigLevels = new float[nSigLevels];
        fileObject->ReadVariableInto("siglev", FLOATARRAY_TYPE, SigLevels); 

        // Need to specify time step to read SSH: timestate
        debug4 << "timestate=" << timestate << endl;

        size_t starts[]={timestate,0};
        size_t counts[]={1, nNodesPerLayer};
        ptrdiff_t stride[]={1,1};
        float *sshvals = new float[nNodesPerLayer];
        int var_id;
        status = nc_inq_varid (fileObject->GetFileHandle(), "zeta", &var_id);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        // SSH
        status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
            starts, counts, stride, sshvals);
        if (status != NC_NOERR) fileObject-> HandleError(status);

        // Get Node Connectivity
        int *nvvals= new int[3 * nCellsPerLayer];
        fileObject->ReadVariableInto("nv", INTEGERARRAY_TYPE, nvvals);

        debug4 << "Read nodes locations and connectivity"<< endl;

        // insert nodes into mesh pts object
        float depth;
        for(int lev = 0; lev< nSigLevels; ++lev) // order! surface -> bottom
        {
            for(int node= 0; node< nNodesPerLayer; ++node)
            {
                depth= SigLevels[lev]*(zvals[node]+sshvals[node])+sshvals[node];
                float pt[3]={xvals[node], yvals[node], depth};
                pts->InsertNextPoint(pt);
            }// end for nNodesPerLayer
        }// end for nSigLevels

        // insert cells into mesh cell object
        vtkIdType verts[6];
        for(int lay = 0; lay < nSigLayers; ++lay)
        {
            for(int cell = 0; cell < nCellsPerLayer; ++cell)
            {
                for(int TB=0; TB<2; ++TB) // TopOrBottom of cell!
                    for(int vrt =0; vrt <3; ++vrt)
                    {
                        verts[vrt + TB*3] = lay*nNodesPerLayer + 
                        // offset each layer by the number of nodes

                        (nvvals[vrt * nCellsPerLayer + cell]-1) + 
                        // offset each nvval by the number of cells per layer

                        TB*nNodesPerLayer;
                        // offset the bottom nodes from the top by nNodesPerLayer

                    } // End for verts per cell and TB
    
                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            } // End for nCellsPerLayer
        } // End For nSigLayers

        delete [] xvals;
        delete [] yvals;
        delete [] zvals;
        delete [] nvvals;
        delete [] sshvals;
        delete [] SigLevels;

        debug4 << mName;
    debug4 << "Success Returning VTK_WEDGE: SigmaLevels" << endl;

        retval = ugrid;
    } // end SigmaLevels Mesh!
    else // Can't match mesh!!!
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    return retval;
}


// ****************************************************************************
//  Method: avtFVCOMReader::GetVar
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
avtFVCOMReader::GetVar(int timestate, const char *varname)
{
    const char *mName = "avtFVCOMReader::GetVar: ";
    debug4 << mName << "timestate=" << timestate
           << ", varname=" << varname << endl;

    // Bail out if we can't get the file handle.
    int ncid = fileObject->GetFileHandle();
    if(ncid == -1)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims;
    fileObject->InqVariable(varname, &t, &ndims, &dims);

    debug4 << mName << ", varname=" << varname
           << "; vartype=" << t 
           << "; varndims="<< ndims<< endl;
    debug4 << "; Vardims:" << endl;
    for(int i=0; i<ndims; ++i)
    {
        debug4 << dims[i] << endl;
    }

    // declare variables for vtk      
    int ntuples=0;
    float *vals;
    if(ndims == 1)
    {
        debug4 << "Variable: " << varname << "is 1d" << endl;
    
        if (t == FLOATARRAY_TYPE)
        {
            ntuples = dims[0];  
            vals = new float[ntuples];
            fileObject->ReadVariableInto(varname, FLOATARRAY_TYPE, vals);       
        }
    }
    else if(ndims == 2)    
    {
        debug4 << "Variable: " << varname << "is 2d" << endl;

        if (t == FLOATARRAY_TYPE)
        {
            ntuples = dims[1];    
            size_t starts[]={timestate,0};
            size_t counts[]={1, dims[1]};
            ptrdiff_t stride[]={1,1};
            vals = new float[ntuples];
            int var_id;

            int status = nc_inq_varid (fileObject->GetFileHandle(),
                varname, &var_id);
            if (status != NC_NOERR) fileObject-> HandleError(status);
            status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
                starts, counts, stride, vals);
            if (status != NC_NOERR) fileObject-> HandleError(status);   
        }
    }
    else if (ndims == 3)
    {
        debug4 << "Variable: " << varname << "is 3d" << endl;

        if (t == FLOATARRAY_TYPE)
        {
            ntuples = dims[1]*dims[2];    
            size_t starts[]={timestate,0,0};
            size_t counts[]={1, dims[1], dims[2]};
            ptrdiff_t stride[]={1,1,1};
            vals = new float[ntuples];
            int var_id;
    
            int status = nc_inq_varid (fileObject->GetFileHandle(), 
                varname, &var_id);
            if (status != NC_NOERR) fileObject-> HandleError(status);

            status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
                starts, counts, stride, vals);
            if (status != NC_NOERR) fileObject-> HandleError(status);
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    debug4 << "ntuples=" << ntuples << endl;

    // Put the data into the vtkFloatArray   
    vtkFloatArray *rv = vtkFloatArray::New();

    rv->SetNumberOfTuples(ntuples);
    for (int i = 0 ; i < ntuples ; i++)
    {
        rv->SetTuple1(i, vals[i]);
    }

    delete [] dims;
    delete [] vals;
    
    debug4 << mName << "end" << endl;

    return rv;
}


// ****************************************************************************
//  Method: avtFVCOMReader::GetVectorVar
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
avtFVCOMReader::GetVectorVar(int timestate, const char *var)
{
    const char *mName = "avtFVCOMReader::GetVectorVar: ";
    debug4 << mName << "timestate=" << timestate
           << ", varname=" << var << endl;

    // Bail out if we can't get the file handle.
    int ncid = fileObject->GetFileHandle();
    if(ncid == -1)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    // Determine which type of Velocity we are loading
    int type;    
    if (strcmp("Velocity2D", var)==0) 
    {
        type=2;
    }
    else if (strcmp("Velocity3D",var)==0)
    {
        type=3;
    }
    else 
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    // Get V info:
    int v_id, status;
    status = nc_inq_varid (ncid, "v", &v_id);
    if(status != NC_NOERR)
    {
        fileObject-> HandleError(status);
        debug4<< mName << ": Returned early, could not nc_inq this file!" << endl;
        return 0;
    }

    TypeEnum v_t = NO_TYPE;
    int v_ndims = 0, *v_dims;
    fileObject->InqVariable("v", &v_t, &v_ndims, &v_dims);
    debug4 << mName << ", v_ndims="<< v_ndims<< endl;
    debug4 << "v_dims:" << endl;
    for(int i=0; i<v_ndims; ++i)
    {
        debug4 << v_dims[i] << endl;
    }

    // Get U info:
    int u_id;
    status = nc_inq_varid (ncid, "u", &u_id);
    if(status != NC_NOERR)
    {
        fileObject-> HandleError(status);
        debug4<< mName << ": Returned early, could not nc_inq this file!" << endl;
        return 0;
    }

    TypeEnum u_t = NO_TYPE;
    int u_ndims = 0, *u_dims;
    fileObject->InqVariable("u", &u_t, &u_ndims, &u_dims);
    debug4 << mName << ", u_ndims="<< u_ndims<< endl;
    debug4 << "u_dims:" << endl;
    for(int i=0; i<u_ndims; ++i)
    {
        debug4 << u_dims[i] << endl;
    }

    // Get variables for vtk      
    float *v_vals = 0, *u_vals = 0;
    int ntuples = v_dims[1]*v_dims[2];    
    size_t starts[]={timestate,0,0};
    size_t counts[]={1, v_dims[1], v_dims[2]};
    ptrdiff_t stride[]={1,1,1};

    v_vals = new float[ntuples];    
    status = nc_get_vars_float(fileObject->GetFileHandle(),v_id,
                   starts, counts, stride, v_vals);
    if (status != NC_NOERR)
    {
        fileObject-> HandleError(status);
        delete [] v_vals;
        EXCEPTION1(InvalidVariableException, var);
    }

    u_vals = new float[ntuples];    
    status = nc_get_vars_float(fileObject->GetFileHandle(),u_id,
                   starts, counts, stride, u_vals);
    if (status != NC_NOERR)
    {
        fileObject-> HandleError(status);
        delete [] v_vals;
        delete [] u_vals;
        EXCEPTION1(InvalidVariableException, var);
    }

    debug4 << "ntuples=" << ntuples << endl;
    vtkFloatArray *rv = vtkFloatArray::New();
    if (type==2)
    {
        int ncomps = 3;  // This is the rank of the vector
        rv->SetNumberOfComponents(ncomps);
        rv->SetNumberOfTuples(ntuples);
        float one_entry[3];
        for (int i = 0 ; i < ntuples ; i++)
        {
            one_entry[0]=u_vals[i];
            one_entry[1]=v_vals[i];
            one_entry[2]=0;
            rv->SetTuple(i, one_entry); 
        }
        delete [] u_vals; 
        delete [] v_vals;
        delete [] u_dims;
        delete [] v_dims;
    } //end if type==2
    else if (type==3) // Get W info:
    {
        int ncid, ww_id, status;

        ncid=fileObject->GetFileHandle();
        status = nc_inq_varid (ncid, "ww", &ww_id); 
        if(status != NC_NOERR)
        {
            fileObject-> HandleError(status);
            debug4<< mName << ": Returned early, could not nc_inq this file!" << endl;
            return 0;
        }

        TypeEnum ww_t = NO_TYPE;
        int ww_ndims = 0, *ww_dims = 0;
        fileObject->InqVariable("ww", &ww_t, &ww_ndims, &ww_dims);
        debug4 << mName << ", ww_ndims="<< ww_ndims<< endl;
        debug4 << "ww_dims:" << endl;
        for(int i=0; i<ww_ndims; ++i)
        {
            debug4 << ww_dims[i] << endl;
        }

        float *ww_vals = new float[ntuples];    
        status = nc_get_vars_float(fileObject->GetFileHandle(),ww_id,
            starts, counts, stride, ww_vals);
        if (status != NC_NOERR)
        {
            fileObject-> HandleError(status);
            delete [] u_vals; 
            delete [] u_dims;
            delete [] v_vals;
            delete [] v_dims;
            delete [] ww_vals;
            delete [] ww_dims;
            rv->Delete();
            EXCEPTION1(InvalidVariableException, var);
        }

        debug4 << "ntuples=" << ntuples << endl;
      
        int ncomps = 3;  // This is the rank of the vector
        rv->SetNumberOfComponents(ncomps);
        rv->SetNumberOfTuples(ntuples);
        float one_entry[3];
        for (int i = 0 ; i < ntuples ; i++)
        {
            one_entry[0]=u_vals[i];
            one_entry[1]=v_vals[i];
            one_entry[2]=ww_vals[i];
            rv->SetTuple(i, one_entry); 
        }

        delete [] u_vals; 
        delete [] v_vals;
        delete [] u_dims;
        delete [] v_dims;

        delete [] ww_vals;
        delete [] ww_dims;
    } // end if type==3

    debug4<< "Got RV in GetVectorVar" << endl;
    debug4<< mName << "Returned Velocity data" << endl;

    return rv;    
}

