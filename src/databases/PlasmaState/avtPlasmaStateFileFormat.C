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
//                            avtPlasmaStateFileFormat.C                           //
// ************************************************************************* //

#include <avtPlasmaStateFileFormat.h>
#include <vtkPointData.h>
#include <vtkVisItUtility.h>
#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

using namespace std;


// ****************************************************************************
//  Method: avtPlasmaStateFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

avtPlasmaStateFileFormat::avtPlasmaStateFileFormat(const char *filename)
    : avtMTMDFileFormat(filename),
      file(filename)
{
    debug5<<"avtPlasmaStateFileFormat::avtPlasmaStateFileFormat"<<endl;
    
    if (!file.is_valid())
        EXCEPTION1(InvalidFilesException, filename);


    //Read in metadata.
    GetDimensions();
    GetVariables();
    GetMeshSet();

    //Determine file type.
    int natts = file.num_atts();
    fileType = PLASMA_STATE;
    for (int i = 0; i < natts; i++)
    {
        NcAtt *a = file.get_att(i);
        if (!strcmp(a->name(), "monitor_comp_version"))
            fileType = MONITOR;
    }

    //See how many time steps we have.
    nTimeSteps = 1;
    if (fileType == PLASMA_STATE)
        nTimeSteps = 1;
    else
        nTimeSteps = dims["timeDim"];
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

int
avtPlasmaStateFileFormat::GetNTimesteps(void)
{
    return nTimeSteps;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetTimes()
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

void
avtPlasmaStateFileFormat::GetTimes(vector<double> &times)
{
    times.resize(0);
    if (nTimeSteps == 1)
        times.push_back(0.0);
    else
    {
        NcVar *t = file.get_var("time");
        if (!t)
            EXCEPTION1(InvalidVariableException, "time");
        NcValues *vals = t->values();
        if (!vals)
            EXCEPTION1(InvalidVariableException, "time");
        if (nTimeSteps != vals->num())
            EXCEPTION1(InvalidVariableException, "time");
        
        times.resize(nTimeSteps);
        for (int i = 0; i < nTimeSteps; i++)
            times[i] = vals->as_double(i);
    }
}


// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

void
avtPlasmaStateFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

void
avtPlasmaStateFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    debug5<<"avtPlasmaStateFileFormat::PopulateDatabaseMetaData"<<endl;

    if (fileType == MONITOR)
        MonitorPopulateMD(md, timeState);
    else
        PlasmaStatePopulateMD(md, timeState);
}

// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::GetDimensions
//
//  Purpose:
//      Read dimensions.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

void
avtPlasmaStateFileFormat::GetDimensions()
{
    int ndim = file.num_dims();
    for (int i=0; i < ndim; i++)
    {
        NcDim *d = file.get_dim(i);
        if (!d || !d->is_valid())
            continue;

        dims[d->name()] = d->size();
    }
}

// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::GetVariables
//
//  Purpose:
//      Read variables.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

void
avtPlasmaStateFileFormat::GetVariables()
{
    vars.resize(0);

    int nvars = file.num_vars();
    for (int i=0; i<nvars; i++)
    {
        NcVar *v = file.get_var(i);
        if (!v || !v->is_valid())
            continue;
        
        varInfo info;
        info.filenm = v->name();
        info.nm = FixName(v->name());
        info.timeVarying = false;

        for (int j = 0; j < v->num_dims(); j++)
        {
            NcDim *d = v->get_dim(j);
            if (!d || !d->is_valid())
                continue;
            
            if (string(d->name()) == string("timeDim"))
                info.timeVarying = true;
            else
                info.dims.push_back(d->name());
        }

        if (info.dims.size() == 0)
            continue;

        //Form the mesh name.
        info.mesh = info.dims[0];
        for (int i = 1; i < info.dims.size(); i++)
            info.mesh = info.mesh + "_X_" + info.dims[i];

        //Units, if any.
        info.units = "";
        NcAtt *units = v->get_att("units");
        if (units && units->is_valid())
        {
            string s = units->as_string(0);
            if (s != "" && s != "-")
                info.units = s;
        }

        //Centering.
        info.centering = AVT_NODECENT;
        if (info.nm.find("zone") != -1)
            info.centering = AVT_ZONECENT;

        vars.push_back(info);
    }
}


// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::GetMeshSet
//
//  Purpose:
//      Based on the variables, determine what meshes we have.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

void
avtPlasmaStateFileFormat::GetMeshSet()
{
    for (int i = 0; i < vars.size(); i++)
    {

        bool meshFound = false;
        for (int m = 0; m < meshes.size(); m++)
        {
            if (meshes[m].nm == vars[i].mesh &&
                meshes[m].timeVarying == vars[i].timeVarying)
            {
                meshFound = true;
                break;
            }
        }

        if (!meshFound)
        {
            meshInfo info;

            info.nm = vars[i].mesh;
            info.timeVarying = vars[i].timeVarying;
            info.dims = vars[i].dims;

            meshes.push_back(info);
        }
    }
}

// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::MonitorPopulateMD
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

void
avtPlasmaStateFileFormat::MonitorPopulateMD(avtDatabaseMetaData *md, int timeState)
{
    debug5<<"avtPlasmaStateFileFormat::MonitorPopulateMD"<<endl;

    //Meshes.
    for (int i = 0; i < meshes.size(); i++)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = meshes[i].nm;
        mesh->meshType = AVT_RECTILINEAR_MESH;
        mesh->numBlocks = 1;
        mesh->blockOrigin = 1;
        mesh->cellOrigin = 0;
        mesh->spatialDimension = meshes[i].dims.size();
        mesh->topologicalDimension = meshes[i].dims.size();
        mesh->blockTitle = "blocks";
        mesh->blockPieceName = "block";
        mesh->hasSpatialExtents = false;
        mesh->xUnits = "m"; //Get these!
        mesh->yUnits = "m";
        mesh->zUnits = "m";
        md->Add(mesh);

        debug5<<"Mesh: "<<mesh->name<<" "<<mesh->spatialDimension<<" "<<mesh->topologicalDimension<<endl;
    }

    //Variables.
    for (int i = 0; i < vars.size(); i++)
    {
        if (vars[i].dims.size() == 1)
            continue;
        
        avtScalarMetaData *var = new avtScalarMetaData();
        var->name = vars[i].nm;
        var->meshName = vars[i].mesh;
        var->centering = vars[i].centering;
        var->hasDataExtents = false;
        var->treatAsASCII = false;
        
        var->hasUnits = false;
        if (vars[i].units != "")
        {
            var->hasUnits = true;
            var->units = vars[i].units;
        }

        md->Add(var);

        debug5<<"Var: "<<var->name<<" "<<var->meshName<<endl;
    }

    //Curves.
    for (int i = 0; i < vars.size(); i++)
    {
        if (vars[i].dims.size() != 1)
            continue;

        avtCurveMetaData *crv = new avtCurveMetaData();
        crv->name = vars[i].nm;
        crv->centering = vars[i].centering;
        crv->xLabel = "";
        crv->yLabel = "";
        crv->xUnits = "";
        //See if we can parse the x units from the var name.
        size_t left = vars[i].nm.find("["), right = vars[i].nm.find("]");
        if (left !=-1 && right != -1 && (right-left-1 > 0))
            crv->xUnits = vars[i].nm.substr(left+1,right-left-1);

        crv->yUnits = vars[i].units;
        md->Add(crv);

        debug5<<"Crv: "<<crv->name<<endl;
    }

}


// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::PlasmaStatePopulateMD
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

void
avtPlasmaStateFileFormat::PlasmaStatePopulateMD(avtDatabaseMetaData *md, int timeState)
{
}


// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

vtkDataSet *
avtPlasmaStateFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    debug5<<"avtPlasmaStateFileFormat::GetMesh "<<meshname<<endl;

    for ( int i = 0; i < meshes.size(); i++)
    {
        if (meshes[i].nm == meshname)
        {
            int d[3] = {0,0,0};
            vtkFloatArray *coords[3];
            int numDims =  meshes[i].dims.size();
            
            for (int j = 0; j < 3; j++)
            {
                coords[j] = vtkFloatArray::New();
                if (j < numDims)
                {
                    d[j] = dims[meshes[i].dims[j]];
                    coords[j]->SetNumberOfTuples(d[j]);
                    for (int k = 0; k < d[j]; k++)
                        coords[j]->SetComponent(k, 0, k);
                }
                else
                {
                    d[j] = 1;
                    coords[j]->SetNumberOfTuples(1);
                    coords[j]->SetComponent(0, 0, 0.);
                }
            }   
            
            vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
            grid->SetDimensions(d);
            grid->SetXCoordinates(coords[0]);
            grid->SetYCoordinates(coords[1]);
            grid->SetZCoordinates(coords[2]);

            coords[0]->Delete();
            coords[1]->Delete();
            coords[2]->Delete();
            
            return grid;
        }
    }

    //See if it's a 'curve'.
    for (int i = 0; i < vars.size(); i++)
    {
        if (vars[i].dims.size() == 1 &&
            vars[i].nm == meshname)
        {
            NcVar *v = file.get_var(vars[i].filenm.c_str());
            if (!v || !v->is_valid())
                EXCEPTION1(InvalidVariableException, meshname);
            
            NcValues *input = v->values();
            if (!input)
                EXCEPTION1(InvalidVariableException, meshname);
            
            int npts = dims[vars[i].dims[0]];

            vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(npts, VTK_FLOAT);
            vtkFloatArray *xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());
            vtkFloatArray *yv= vtkFloatArray::New();
            yv->SetNumberOfComponents(1);
            yv->SetNumberOfTuples(npts);
            yv->SetName(meshname);
            
            int offset = 0;
            if (vars[i].timeVarying)
                offset = timestate*npts;
            
            for (int j = 0; j < npts; j++)
            {
                xc->SetValue(j, (float)j);
                yv->SetValue(j, v->as_float(j+offset));
            }
            rg->GetPointData()->SetScalars(yv);
            yv->Delete();
            return rg;
        }
    }
    
    EXCEPTION1(InvalidVariableException, meshname);
    return NULL;
}


// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

vtkDataArray *
avtPlasmaStateFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    debug5<<"avtPlasmaStateFileFormat::GetVar(): "<<timestate<<" "<<varname<<endl;
    int idx = -1;
    for (int i = 0; i < vars.size(); i++)
        if (vars[i].nm == varname)
        {
            idx = i;
            break;
        }

    if (idx == -1)
        EXCEPTION1(InvalidVariableException, varname);

    varInfo &vInfo = vars[idx];

    NcVar *v = file.get_var(vInfo.filenm.c_str());
    if (!v || !v->is_valid())
        EXCEPTION1(InvalidVariableException, varname);


    //We only have 2D variables for now.
    if (vInfo.dims.size() != 2)
        EXCEPTION1(InvalidVariableException, varname);

    int xDim = dims[vInfo.dims[0]], yDim = dims[vInfo.dims[1]];
    int nTuples = xDim*yDim;
    vtkFloatArray *rv = vtkFloatArray::New();
    
    rv->SetNumberOfTuples(nTuples);
    float *p = (float*)rv->GetVoidPointer(0);

    //Get the data.
    NcValues *values = v->values();
    if (!values)
        EXCEPTION1(InvalidVariableException, varname);
    
    //Get the data array and offset to the nth time step.
    double *data = &(((double *)values->base())[timestate*nTuples]);
    
    //Need to transpose it.
    int cnt = 0;
    for (int x = 0; x < xDim; x++)
        for (int y = 0; y < yDim; y++)
        {
            p[y*xDim + x] = data[cnt];
            cnt++;
        }
    
    return rv;
}


// ****************************************************************************
//  Method: avtPlasmaStateFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

vtkDataArray *
avtPlasmaStateFileFormat::GetVectorVar(int timestate, int domain,const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::FixName
//
//  Purpose:
//      Convert file names to something VisIt can handle.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu May 28 08:54:35 PDT 2009
//
// ****************************************************************************

string
avtPlasmaStateFileFormat::FixName(const string &nm) const
{
    string str = nm;
    
    size_t left = nm.find("(");
    if (left != -1)
    {
        str.replace(left, 1, "[");
        size_t right = nm.find(")");
        if (right != -1)
            str.replace(right, 1, "]");
    }

    return str;
}
