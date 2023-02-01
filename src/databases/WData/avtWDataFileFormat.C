/*****************************************************************************
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
*****************************************************************************/

// ************************************************************************* //
//                            avtWDataFileFormat.C                           //
// ************************************************************************* //

#include <avtWDataFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>
#include <DebugStream.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using std::string;

#include "wdata.c"

#define wdata_operation(cmd)                                                       \
    {                                                                              \
        ierr = cmd;                                                                \
        debug4 << "[WDATA] Executed " << #cmd << ", Return code=" << ierr << endl; \
        if (ierr != 0)                                                             \
        {                                                                          \
            char errmsg[256];                                                      \
            sprintf(errmsg, "[WDATA] Cannot execute `%s`. Error=%s", #cmd, ierr);  \
            EXCEPTION1(InvalidDBTypeException, errmsg);                            \
        }                                                                          \
    }

WDataVariable::WDataVariable(wdata_metadata *wdmd, int varid, int precdowngrade)
{
    md = wdmd;
    vid = varid;
    loadedcycle = -1;
    data = NULL;
    d2f = precdowngrade;
}

int WDataVariable::loadCycle(int cycleid)
{
    int ierr = 0;

    if (loadedcycle != cycleid)
    {
        wdata_operation(wdata_read_cycle(md, md->var[vid].name, cycleid, data));

        if (ierr == 0 && d2f == 1) // downgrade precision
        {
            int bs = (int)(wdata_get_blocksize(md, &md->var[vid]) / sizeof(double));
            double *dd = (double *)data;
            float *df = (float *)data;
            for (int i = 0; i < bs; i++)
                df[i] = (float)dd[i];
        }
    }

    // update loadedcycle
    if (ierr == 0)
        loadedcycle = cycleid;
    else
        loadedcycle = -1;

    return ierr;
}

// =======================================================================================
// ============================= WDataRealVariable =======================================
// =======================================================================================
WDataRealVariable::WDataRealVariable(wdata_metadata *wdmd, int varid, int precdowngrade) : WDataVariable(wdmd, varid, precdowngrade)
{
    // allocate memory for data
    int bs = wdata_get_blocklength(md);
    data = new double[bs];

    dataR = (float *)data; // for easier algebra

    // create list of varaibles
    varname.push_back(md->var[vid].name);
    varunit.push_back(md->var[vid].unit);

    // check links
    for (int i = 0; i < md->nlink; i++)
        if (strcmp(md->link[i].linkto, md->var[vid].name) == 0)
        {
            varname.push_back(md->link[i].name);
            varunit.push_back(md->var[vid].unit);
        }
}

bool WDataRealVariable::getVariable(const char *_varname, int cycleid, float *data_for_visit)
{
    // check it this varaible can generate data for varname
    int varid = -1;

    for (int i = 0; i < varname.size(); i++)
        if (strcmp(_varname, varname[i].c_str()) == 0)
            varid = i;

    if (varid == -1)
        return false;

    // load data
    loadCycle(cycleid);

    int ix, iy, iz;
    int ixyz1 = 0, ixyz2 = 0;
    for (ix = 0; ix < md->nx; ix++)
        for (iy = 0; iy < md->ny; iy++)
            for (iz = 0; iz < md->nz; iz++)
            {
                ixyz2 = ix + md->nx * iy + md->nx * md->ny * iz;
                data_for_visit[ixyz2] = (float)dataR[ixyz1];
                //             debug4<<"[XXXX]"<<ix<<" "<<iy<<" "<<iz<<" "<<ixyz1<<" "<<ixyz2<<" "<<data[ixyz1]<<endl;
                ixyz1++;
            }

    return true;
}

// =======================================================================================
// ============================= WDataComplexVariable ====================================
// =======================================================================================
// Modifications:
//   @Belyor on GitHub: Bugfix. Array iterator had a wrong name
WDataComplexVariable::WDataComplexVariable(wdata_metadata *wdmd, int varid, int precdowngrade) : WDataVariable(wdmd, varid, precdowngrade)
{
    // allocate memory for data
    int bs = wdata_get_blocklength(md);
    data = new double[bs * 2];

    dataC = (Complex *)data; // for easier algebra

    // create list of varaibles
    string varlabel;

    varlabel = md->var[vid].name;
    varlabel += "_abs";
    varname.push_back(varlabel);
    varunit.push_back(md->var[vid].unit);
    trans.push_back(cabs);

    varlabel = md->var[vid].name;
    varlabel += "_arg";
    varname.push_back(varlabel);
    varunit.push_back("PI");
    trans.push_back(carg);

    varlabel = md->var[vid].name;
    varlabel += "_re";
    varname.push_back(varlabel);
    varunit.push_back(md->var[vid].unit);
    trans.push_back(cre);

    varlabel = md->var[vid].name;
    varlabel += "_im";
    varname.push_back(varlabel);
    varunit.push_back(md->var[vid].unit);
    trans.push_back(cim);

    // check links
    for (int i = 0; i < md->nlink; i++)
        if (strcmp(md->link[i].linkto, md->var[vid].name) == 0)
        {
            varlabel = md->link[i].name;
            varlabel += "_abs";
            varname.push_back(varlabel);
            varunit.push_back(md->var[vid].unit);
            trans.push_back(cabs);

            varlabel = md->link[i].name;
            varlabel += "_arg";
            varname.push_back(varlabel);
            varunit.push_back("PI");
            trans.push_back(carg);

            varlabel = md->link[i].name;
            varlabel += "_re";
            varname.push_back(varlabel);
            varunit.push_back(md->var[vid].unit);
            trans.push_back(cre);

            varlabel = md->link[i].name;
            varlabel += "_im";
            varname.push_back(varlabel);
            varunit.push_back(md->var[vid].unit);
            trans.push_back(cim);
        }
}

bool WDataComplexVariable::getVariable(const char *_varname, int cycleid, float *data_for_visit)
{
    // check it this varaible can generate data for varname
    int varid = -1;

    for (int i = 0; i < varname.size(); i++)
        if (strcmp(_varname, varname[i].c_str()) == 0)
            varid = i;

    if (varid == -1)
        return false;

    // load data
    loadCycle(cycleid);

    int ix, iy, iz;
    int ixyz1 = 0, ixyz2 = 0;
    if (trans[varid] == cabs)
    {
        for (ix = 0; ix < md->nx; ix++)
            for (iy = 0; iy < md->ny; iy++)
                for (iz = 0; iz < md->nz; iz++)
                {
                    ixyz2 = ix + md->nx * iy + md->nx * md->ny * iz;
                    data_for_visit[ixyz2] = (float)abs(dataC[ixyz1]);

                    ixyz1++;
                }
    }
    else if (trans[varid] == carg)
    {
        for (ix = 0; ix < md->nx; ix++)
            for (iy = 0; iy < md->ny; iy++)
                for (iz = 0; iz < md->nz; iz++)
                {
                    ixyz2 = ix + md->nx * iy + md->nx * md->ny * iz;
                    data_for_visit[ixyz2] = (float)(arg(dataC[ixyz1]) / M_PI);

                    ixyz1++;
                }
    }
    else if (trans[varid] == cre)
    {
        for (ix = 0; ix < md->nx; ix++)
            for (iy = 0; iy < md->ny; iy++)
                for (iz = 0; iz < md->nz; iz++)
                {
                    ixyz2 = ix + md->nx * iy + md->nx * md->ny * iz;
                    data_for_visit[ixyz2] = (float)(dataC[ixyz1].real());

                    ixyz1++;
                }
    }
    else if (trans[varid] == cim)
    {
        for (ix = 0; ix < md->nx; ix++)
            for (iy = 0; iy < md->ny; iy++)
                for (iz = 0; iz < md->nz; iz++)
                {
                    ixyz2 = ix + md->nx * iy + md->nx * md->ny * iz;
                    data_for_visit[ixyz2] = (float)(dataC[ixyz1].imag());

                    ixyz1++;
                }
    }

    return true;
}

// =======================================================================================
// ============================= WDataVectorVariable =====================================
// =======================================================================================
WDataVectorVariable::WDataVectorVariable(wdata_metadata *wdmd, int varid, int precdowngrade) : WDataVariable(wdmd, varid, precdowngrade)
{
    // allocate memory for data
    int bs = wdata_get_blocklength(md);
    data = new double[bs * 3];

    dataVx = (float *)data; // for easier algebra
    dataVy = dataVx + bs;
    dataVz = dataVy + bs;

    // create list of varaibles
    varname.push_back(md->var[vid].name);
    varunit.push_back(md->var[vid].unit);

    // check links
    for (int i = 0; i < md->nlink; i++)
        if (strcmp(md->link[i].linkto, md->var[vid].name) == 0)
        {
            varname.push_back(md->link[i].name);
            varunit.push_back(md->var[vid].unit);
        }
}

bool WDataVectorVariable::getVariable(const char *_varname, int cycleid, float *data_for_visit)
{
    // check it this varaible can generate data for varname
    int varid = -1;

    for (int i = 0; i < varname.size(); i++)
        if (strcmp(_varname, varname[i].c_str()) == 0)
            varid = i;

    if (varid == -1)
        return false;

    // load data
    loadCycle(cycleid);

    int ix, iy, iz;
    int ixyz1 = 0, ixyz2 = 0;
    for (ix = 0; ix < md->nx; ix++)
        for (iy = 0; iy < md->ny; iy++)
            for (iz = 0; iz < md->nz; iz++)
            {
                ixyz2 = ix + md->nx * iy + md->nx * md->ny * iz;
                data_for_visit[3 * ixyz2 + 0] = (float)dataVx[ixyz1];
                data_for_visit[3 * ixyz2 + 1] = (float)dataVy[ixyz1];
                data_for_visit[3 * ixyz2 + 2] = (float)dataVz[ixyz1];
                //             debug4<<"[XXXX]"<<ix<<" "<<iy<<" "<<iz<<" "<<ixyz1<<" "<<ixyz2<<" "<<data[ixyz1]<<endl;
                ixyz1++;
            }

    return true;
}

// ****************************************************************************
//  Method: avtWDataFileFormat constructor
//
//  Programmer: gabrielw -- generated by xml2avt
//  Creation:   Wed Jul 29 19:18:00 PST 2020
//
// ****************************************************************************

avtWDataFileFormat::avtWDataFileFormat(const char *filename)
    : avtMTSDFileFormat(&filename, 1)
{
    // INITIALIZE DATA MEMBERS

    // rest values
    wdmd.nx = 0;
    wdmd.ny = 0;
    wdmd.nz = 0;
    wdmd.dx = 0.0;
    wdmd.dy = 0.0;
    wdmd.dz = 0.0;
    int ierr;

    ierr = wdata_parse_metadata_file(filename, &wdmd);

    if (ierr != 0 || wdmd.nx == 0 || wdmd.ny == 0 || wdmd.nz == 0 || wdmd.dx == 0.0 || wdmd.dy == 0.0 || wdmd.dz == 0.0)
    {
        EXCEPTION1(InvalidDBTypeException,
                   "[WDATA] Incorrect metadata file!");
    }

    // reduced data formats
    if (wdmd.datadim == 2)
    {
        wdmd.nz = 1;
    }
    if (wdmd.datadim == 1)
    {
        wdmd.nz = 1;
        wdmd.ny = 1;
    }

    //     std::string str = filename;
    //     unsigned found = str.find_last_of("/\\");
    //     std::string path = str.substr(0,found);
    //     debug4<<"[WDATA] avtWDataFileFormat::avtWDataFileFormat: path="<<path<<endl;
    //     if(path!="")
    //     {
    //         str = path + "/" + wdmd.prefix;
    //         // debug4<<"[WDATA] avtWDataFileFormat::avtWDataFileFormat: str="<<str<<endl;
    //         strcpy(wdmd.prefix, str.c_str());
    //     }
    //     debug4<<"[WDATA] avtWDataFileFormat::avtWDataFileFormat: wdmd.prefix="<<wdmd.prefix<<endl;

    // create list of variables
    WDataVariable *_var;
    int _correct_type;
    for (int i = 0; i < wdmd.nvar; i++)
    {
        _correct_type = 1;

        if (strcmp(wdmd.var[i].type, "real") == 0)
            _var = new WDataRealVariable(&wdmd, i, 1);
        else if (strcmp(wdmd.var[i].type, "real8") == 0)
            _var = new WDataRealVariable(&wdmd, i, 1);
        else if (strcmp(wdmd.var[i].type, "real4") == 0)
            _var = new WDataRealVariable(&wdmd, i, 0);
        else if (strcmp(wdmd.var[i].type, "complex") == 0)
            _var = new WDataComplexVariable(&wdmd, i, 1);
        else if (strcmp(wdmd.var[i].type, "complex16") == 0)
            _var = new WDataComplexVariable(&wdmd, i, 1);
        else if (strcmp(wdmd.var[i].type, "complex8") == 0)
            _var = new WDataComplexVariable(&wdmd, i, 0);
        else if (strcmp(wdmd.var[i].type, "vector") == 0)
            _var = new WDataVectorVariable(&wdmd, i, 1);
        else if (strcmp(wdmd.var[i].type, "vector8") == 0)
            _var = new WDataVectorVariable(&wdmd, i, 1);
        else if (strcmp(wdmd.var[i].type, "vector4") == 0)
            _var = new WDataVectorVariable(&wdmd, i, 0);
        else
            _correct_type = 0;

        if (_correct_type == 1)
            variable.push_back(_var);
    }

// fill comment section
#define MAX_REC_LEN 1024
    FILE *inp;

    // open file
    inp = fopen(filename, "r");
    if (inp == NULL) // error - cannot create the file
    {
        EXCEPTION1(InvalidDBTypeException,
                   "[WDATA] Incorrect metadata file!");
    }

    char s[MAX_REC_LEN];
    dbcomment = "\n";
    dbcomment += "========================================================================================================================\n";
    dbcomment += "======================================================== WTXT ==========================================================\n";
    dbcomment += "========================================================================================================================\n";
    while (fgets(s, MAX_REC_LEN, inp) != NULL)
        dbcomment += s;

    fclose(inp);

    std::string str;

    str = wdmd.prefix;
    str += "_input.txt";
    inp = fopen(str.c_str(), "r");
    if (inp != NULL) // if file exists
    {
        dbcomment += "\n";
        dbcomment += "========================================================================================================================\n";
        dbcomment += "======================================================= INPUT ==========================================================\n";
        dbcomment += "========================================================================================================================\n";
        while (fgets(s, MAX_REC_LEN, inp) != NULL)
            dbcomment += s;

        fclose(inp);
    }
    str = wdmd.prefix;
    str += ".wlog";
    inp = fopen(str.c_str(), "r");
    if (inp != NULL) // if file exists
    {
        dbcomment += "\n";
        dbcomment += "========================================================================================================================\n";
        dbcomment += "======================================================== WLOG ==========================================================\n";
        dbcomment += "========================================================================================================================\n";
        while (fgets(s, MAX_REC_LEN, inp) != NULL)
            dbcomment += s;

        fclose(inp);
    }
    str = wdmd.prefix;
    str += "_predefines.h";
    inp = fopen(str.c_str(), "r");
    if (inp != NULL) // if file exists
    {
        dbcomment += "\n";
        dbcomment += "========================================================================================================================\n";
        dbcomment += "==================================================== predefines.h ======================================================\n";
        dbcomment += "========================================================================================================================\n";
        while (fgets(s, MAX_REC_LEN, inp) != NULL)
            dbcomment += s;

        fclose(inp);
    }
    str = wdmd.prefix;
    str += "_problem-definition.h";
    inp = fopen(str.c_str(), "r");
    if (inp != NULL) // if file exists
    {
        dbcomment += "\n";
        dbcomment += "========================================================================================================================\n";
        dbcomment += "================================================ problem-definition.h ==================================================\n";
        dbcomment += "========================================================================================================================\n";
        while (fgets(s, MAX_REC_LEN, inp) != NULL)
            dbcomment += s;

        fclose(inp);
    }
    str = wdmd.prefix;
    str += "_logger.h";
    inp = fopen(str.c_str(), "r");
    if (inp != NULL) // if file exists
    {
        dbcomment += "\n";
        dbcomment += "========================================================================================================================\n";
        dbcomment += "===================================================== logger.h =========================================================\n";
        dbcomment += "========================================================================================================================\n";
        while (fgets(s, MAX_REC_LEN, inp) != NULL)
            dbcomment += s;

        fclose(inp);
    }
    str = wdmd.prefix;
    str += ".stdout";
    inp = fopen(str.c_str(), "r");
    if (inp != NULL) // if file exists
    {
        dbcomment += "\n";
        dbcomment += "========================================================================================================================\n";
        dbcomment += "====================================================== stdout ==========================================================\n";
        dbcomment += "========================================================================================================================\n";
        while (fgets(s, MAX_REC_LEN, inp) != NULL)
            dbcomment += s;

        fclose(inp);
    }

    dbcomment += "========================================================================================================================\n";
#undef MAX_REC_LEN
}

avtWDataFileFormat::~avtWDataFileFormat()
{
    for (int i = 0; i < wdmd.nvar; i++)
        delete variable[i];
}

// ****************************************************************************
//  Method: avtWDataFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: gabrielw -- generated by xml2avt
//  Creation:   Wed Jul 29 19:18:00 PST 2020
//
// ****************************************************************************

void avtWDataFileFormat::GetCycles(std::vector<int> &cycles)
{
    for (int i = 0; i < wdmd.cycles; ++i)
        cycles.push_back(i);
}

void avtWDataFileFormat::GetTimes(std::vector<double> &times)
{
    for (int i = 0; i < wdmd.cycles; ++i)
    {
        double _t;
        wdata_get_time(&wdmd, i, &_t);
        times.push_back(_t);
    }
}

int avtWDataFileFormat::GetNTimesteps(void)
{
    return wdmd.cycles;
}

// ****************************************************************************
//  Method: avtWDataFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: gabrielw -- generated by xml2avt
//  Creation:   Wed Jul 29 19:18:00 PST 2020
//
// ****************************************************************************

void avtWDataFileFormat::FreeUpResources(void)
{
}

// ****************************************************************************
//  Method: avtWDataFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: gabrielw -- generated by xml2avt
//  Creation:   Wed Jul 29 19:18:00 PST 2020
//
// ****************************************************************************

void avtWDataFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    std::string meshname = "mesh";
    //
    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH,
    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH
    avtMeshType mt = AVT_RECTILINEAR_MESH;

    int nblocks = 1; //<-- this must be 1 for MTSD
    int block_origin = 0;
    int spatial_dimension = 3;
    int topological_dimension = 3;
    double *extents = NULL;
    //
    // Here's the call that tells the meta-data object that we have a mesh:
    //
    debug4 << "[WDATA] avtWDataFileFormat::PopulateDatabaseMetaData->AddMeshToMetaData" << endl;

    AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                      spatial_dimension, topological_dimension);

    // CODE TO ADD A SCALAR VARIABLE
    for (int ii = 0; ii < variable.size(); ii++)
    {
        WDataVariable *_var = variable[ii];
        if (_var->isScalar())
            for (int ivar = 0; ivar < _var->numberOfVariables(); ivar++)
            {
                avtScalarMetaData *smd = new avtScalarMetaData;
                smd->name = _var->getIthVariableName(ivar);
                smd->meshName = meshname;
                smd->centering = AVT_NODECENT;
                if (strcmp(_var->getIthVariableUnit(ivar).c_str(), "none") == 0)
                    smd->hasUnits = false;
                else if (strcmp(_var->getIthVariableUnit(ivar).c_str(), "null") == 0)
                    smd->hasUnits = false;
                else
                    smd->hasUnits = true;
                smd->units = _var->getIthVariableUnit(ivar);
                md->Add(smd);

                debug4 << "[WDATA] avtWDataFileFormat::PopulateDatabaseMetaData->Added scalar variable: " << smd->name << endl;
            }

        // CODE TO ADD A VECTOR VARIABLE
        if (_var->isVector())
            for (int ivar = 0; ivar < _var->numberOfVariables(); ivar++)
            {
                avtVectorMetaData *smv = new avtVectorMetaData;
                smv->name = _var->getIthVariableName(ivar);
                smv->meshName = meshname;
                smv->centering = AVT_NODECENT;
                if (strcmp(_var->getIthVariableUnit(ivar).c_str(), "none") == 0)
                    smv->hasUnits = false;
                else if (strcmp(_var->getIthVariableUnit(ivar).c_str(), "null") == 0)
                    smv->hasUnits = false;
                else
                    smv->hasUnits = true;
                smv->units = _var->getIthVariableUnit(ivar);
                smv->varDim = 3;
                md->Add(smv);

                debug4 << "[WDATA] avtWDataFileFormat::PopulateDatabaseMetaData->Added vector variable: " << smv->name << endl;
            }
    }

    // CONSTS
    for (int ii = 0; ii < wdmd.nconsts; ii++)
    {
        // Here's the way to add expressions:
        char cdef[256];
        Expression const_expr;
        sprintf(cdef, "const_%s", wdmd.consts[ii].name);
        const_expr.SetName(cdef);
        sprintf(cdef, "coord(mesh)[0]*0 + (%f)", wdmd.consts[ii].value);
        const_expr.SetDefinition(cdef);
        // const_expr.SetType(Expression::VectorMeshVar);
        const_expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&const_expr);
    }

    // db comment
    md->SetDatabaseComment(dbcomment);
}

// ****************************************************************************
//  Method: avtWDataFileFormat::GetMesh
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
//  Programmer: gabrielw -- generated by xml2avt
//  Creation:   Wed Jul 29 19:18:00 PST 2020
//
// ****************************************************************************

vtkDataSet *
avtWDataFileFormat::GetMesh(int timestate, const char *meshname)
{
    debug4 << "[WDATA] avtWDataFileFormat::GetMesh" << endl;

    int ndims = 3;
    int dims[3];
    int i;

    dims[0] = wdmd.nx;
    dims[1] = wdmd.ny;
    dims[2] = wdmd.nz;

    vtkFloatArray *coords[3] = {0, 0, 0};

    // Read the X coordinates from the file.
    coords[0] = vtkFloatArray::New();
    coords[0]->SetNumberOfTuples(dims[0]);
    float *xarray = (float *)coords[0]->GetVoidPointer(0);
    for (i = 0; i < wdmd.nx; i++)
        xarray[i] = (float)(wdmd.dx * i);

    // Read the Y coordinates from the file.
    coords[1] = vtkFloatArray::New();
    coords[1]->SetNumberOfTuples(dims[1]);
    float *yarray = (float *)coords[1]->GetVoidPointer(0);
    for (i = 0; i < wdmd.ny; i++)
        yarray[i] = (float)(wdmd.dy * i);

    // Read the Z coordinates from the file.
    coords[2] = vtkFloatArray::New();
    coords[2]->SetNumberOfTuples(dims[2]);
    float *zarray = (float *)coords[2]->GetVoidPointer(0);
    for (i = 0; i < wdmd.nz; i++)
        zarray[i] = (float)(wdmd.dz * i);

    //
    // Create the vtkRectilinearGrid object and set its dimensions
    // and coordinates.
    //
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rgrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rgrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();
    return rgrid;
}

// ****************************************************************************
//  Method: avtWDataFileFormat::GetVar
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
//  Programmer: gabrielw -- generated by xml2avt
//  Creation:   Wed Jul 29 19:18:00 PST 2020
//
// ****************************************************************************

vtkDataArray *
avtWDataFileFormat::GetVar(int timestate, const char *varname)
{

    int ntuples = wdata_get_blocklength(&wdmd); // this is the number of entries in the variable.
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(ntuples);

    //     float * _data = new float[ntuples];
    float *_data = (float *)rv->GetVoidPointer(0);

    bool hasdata;
    for (int ii = 0; ii < variable.size(); ii++)
    {
        hasdata = variable[ii]->getVariable(varname, timestate, _data);
        if (hasdata)
            return rv;
    }

    return NULL;
}

// ****************************************************************************
//  Method: avtWDataFileFormat::GetVectorVar
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
//  Programmer: gabrielw -- generated by xml2avt
//  Creation:   Wed Jul 29 19:18:00 PST 2020
//
// ****************************************************************************

vtkDataArray *
avtWDataFileFormat::GetVectorVar(int timestate, const char *varname)
{
    int ntuples = wdata_get_blocklength(&wdmd); // this is the number of entries in the variable.
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(ntuples);
    float *_data = (float *)rv->GetVoidPointer(0);

    bool hasdata;
    for (int ii = 0; ii < variable.size(); ii++)
    {
        hasdata = variable[ii]->getVariable(varname, timestate, _data);
        if (hasdata)
            return rv;
    }
    return NULL;
}
