/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtPLOT2DFileFormat.C                          //
// ************************************************************************* //

#include <avtPLOT2DFileFormat.h>

#include <ctype.h>
#include <visitstream.h>

#include <vtkFloatArray.h>
#include <vtkStructuredGrid.h>

#include <Expression.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>


//
// Define the static const's
//

const char   *avtPLOT2DFileFormat::MESHNAME = "mesh";


// ****************************************************************************
//  Method: avtPLOT2DFileFormat constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Hank Childs
//  Creation:    August 11, 2003
//
// ****************************************************************************

avtPLOT2DFileFormat::avtPLOT2DFileFormat(const char *fname) 
    : avtSTSDFileFormat(fname)
{
    //
    // The .p2d is a faux extension.  From that, infer what the grid and
    // solutions files are.
    //
    if (strstr(fname, ".p2d") == NULL)
    {
        EXCEPTION1(InvalidFilesException, fname);
    }
    char *start_of_ext = strstr(fname, ".p2d");
    char soln_file[1024];
    strncpy(soln_file, fname, start_of_ext - fname);
    strcpy(soln_file + (start_of_ext - fname), ".q");
    char grid_file[1024];
    strncpy(grid_file, fname, start_of_ext - fname);
    strcpy(grid_file + (start_of_ext - fname), ".g");

    grid_filename = grid_file;
    soln_filename = soln_file;

    haveReadVars = false;
    rho = NULL;
    u = NULL;
    v = NULL;
    E = NULL;
}


// ****************************************************************************
//  Method: avtPLOT2DFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2003
//
// ****************************************************************************

avtPLOT2DFileFormat::~avtPLOT2DFileFormat()
{
    if (rho != NULL)
    {
        rho->Delete();
        rho = NULL;
    }
    if (u != NULL)
    {
        u->Delete();
        u = NULL;
    }
    if (v != NULL)
    {
        v->Delete();
        v = NULL;
    }
    if (E != NULL)
    {
        E->Delete();
        E = NULL;
    }
    haveReadVars = false;
}


// ****************************************************************************
//  Method: avtPLOT2DFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh.  The mesh is actually just the dataset, so return that.
//
//  Arguments:
//      mesh     The desired meshname, this should be MESHNAME.
//
//  Returns:     The mesh as a PLOT2D dataset.
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2003
//
// ****************************************************************************

vtkDataSet *
avtPLOT2DFileFormat::GetMesh(const char *mesh)
{
    if (strcmp(mesh, MESHNAME) != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    ifstream ifile(grid_filename.c_str());
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, grid_filename.c_str());
    }

    char line[1024];
    ifile.getline(line, 1024);
    int nx, ny;
    sscanf(line, "%d %d", &nx, &ny);
    int dims[3] = { nx, ny, 1 };
    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
    sgrid->SetDimensions(dims);
    vtkPoints *pts = vtkPoints::New();
    int npts = nx*ny;
    pts->SetNumberOfPoints(npts);
    float *ptr = (float *) pts->GetVoidPointer(0);

    bool haveLine = false;
    char *tmp = NULL;
    for (int j = 0 ; j < 2 ; j++)
    {
        for (int i = 0 ; i < npts ; i++)
        {
            if (!haveLine)
            {
                ifile.getline(line, 1024);
                tmp = line;
                haveLine = true;
            }
            // Pass over preceding whitespace.  This is mostly for when
            // we read in a new line for the first time, since the "trailing"
            // whitespace check below will also do this.
            while (isspace(*tmp))
                tmp++;
            ptr[3*i + j] = atof(tmp);
            // Pass over the number.
            while (*tmp != '\0' && !isspace(*tmp))
                tmp++;
            // Pass over trailing whitespace.
            while (*tmp != '\0' && isspace(*tmp))
                tmp++;
            if (*tmp == '\0')
            {
                haveLine = false;
            }
        }
    }
    for (int k = 0 ; k < npts ; k++)
        ptr[3*k + 2] = 0.;

    sgrid->SetPoints(pts);
    pts->Delete();

    return sgrid;
}


// ****************************************************************************
//  Method: avtPLOT2DFileFormat::GetVar
//
//  Purpose:
//      Gets a var.  
//
//  Arguments:
//      var     The desired varname
//
//  Returns:    A VTK float array of the variable.
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2003
//
// ****************************************************************************

vtkDataArray *
avtPLOT2DFileFormat::GetVar(const char *var)
{
    if (!haveReadVars)
    {
        ifstream ifile(soln_filename.c_str());
        if (ifile.fail())
        {
            EXCEPTION1(InvalidFilesException, soln_filename.c_str());
        }
 
        rho = vtkFloatArray::New();
        u = vtkFloatArray::New();
        v = vtkFloatArray::New();
        E = vtkFloatArray::New();
        vtkFloatArray *arrs[4] = { rho, u, v, E };

        char line[1024];

        // The first line contains NX and NY.
        ifile.getline(line, 1024);
        int nx, ny;
        sscanf(line, "%d %d", &nx, &ny);
        int npts = nx*ny;
 
        // The next line contains the free-stream mach number, angle-of-atack,
        // Reynold's number and integration time.
        ifile.getline(line, 1024);
        float mach, angle, Reynolds, time;
        sscanf(line, "%f %f %f %f", &mach, &angle, &Reynolds, &time);
        metadata->SetTime(timestep, time);

        for (int var = 0 ; var < 4 ; var++)
        {
            vtkDataArray *arr = arrs[var];
            arr->SetNumberOfTuples(npts);
            bool haveLine = false;
            char *tmp = NULL;
            for (int pt = 0 ; pt < npts ; pt++)
            {
                if (!haveLine)
                {
                    ifile.getline(line, 1024);
                    tmp = line;
                    haveLine = true;
                }

                // Pass over preceding whitespace.  This is mostly for when
                // we read in a new line for the first time, since the 
                //"trailing" whitespace check below will also do this.
                while (isspace(*tmp))
                    tmp++;
                arr->SetTuple1(pt, atof(tmp));
                // Pass over the number.
                while (*tmp != '\0' && !isspace(*tmp))
                    tmp++;
                // Pass over trailing whitespace.
                while (*tmp != '\0' && isspace(*tmp))
                    tmp++;
                if (*tmp == '\0')
                {
                    haveLine = false;
                }
            }
        }
        haveReadVars = true;
    }

    vtkFloatArray *rv = NULL;
    if (strcmp(var, "rho") == 0)
        rv = rho;
    else if (strcmp(var, "u") == 0)
        rv = u;
    else if (strcmp(var, "v") == 0)
        rv = v;
    else if (strcmp(var, "E") == 0)
        rv = E;

    if (rv == NULL)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    //
    // The calling function will assume that it owns this memory after it
    // calls it, so increment the reference count.
    //
    rv->Register(NULL);

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT2DFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources allocated by this reader.
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2003
//
// ****************************************************************************

void
avtPLOT2DFileFormat::FreeUpResources(void)
{
    if (rho != NULL)
    {
        rho->Delete();
        rho = NULL;
    }
    if (u != NULL)
    {
        u->Delete();
        u = NULL;
    }
    if (v != NULL)
    {
        v->Delete();
        v = NULL;
    }
    if (E != NULL)
    {
        E->Delete();
        E = NULL;
    }
    haveReadVars = false;
}


// ****************************************************************************
//  Method: avtPLOT2DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Aug 14 22:56:35 PDT 2003
//    Added some common expressions.
//
// ****************************************************************************

void
avtPLOT2DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int spat = 2;
    int topo = 2;

    avtMeshType type = AVT_CURVILINEAR_MESH;

    AddMeshToMetaData(md, MESHNAME, type, NULL, 1, 0, spat, topo);

    AddScalarVarToMetaData(md, "rho", MESHNAME, AVT_NODECENT);
    AddScalarVarToMetaData(md, "u", MESHNAME, AVT_NODECENT);
    AddScalarVarToMetaData(md, "v", MESHNAME, AVT_NODECENT);
    AddScalarVarToMetaData(md, "E", MESHNAME, AVT_NODECENT);

    Expression momentum_expr;
    momentum_expr.SetName("momentum");
    momentum_expr.SetDefinition("{u, v}");
    momentum_expr.SetType(Expression::VectorMeshVar);
    md->AddExpression(&momentum_expr);

    Expression velocity_expr;
    velocity_expr.SetName("velocity");
    velocity_expr.SetDefinition("momentum/rho");
    velocity_expr.SetType(Expression::VectorMeshVar);
    md->AddExpression(&velocity_expr);

    Expression KineticEnergy_expr;
    KineticEnergy_expr.SetName("KineticEnergy");
    KineticEnergy_expr.SetDefinition("0.5*(momentum*momentum)/(rho*rho)");
    KineticEnergy_expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&KineticEnergy_expr);

    Expression pressure_expr;
    pressure_expr.SetName("pressure");
    pressure_expr.SetDefinition("0.4*(E - KineticEnergy*rho)");
    pressure_expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&pressure_expr);

    Expression enthalpy_expr;
    enthalpy_expr.SetName("enthalpy");
    enthalpy_expr.SetDefinition("1.4*(E/rho - KineticEnergy)");
    enthalpy_expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&enthalpy_expr);

    Expression entropy_expr;
    entropy_expr.SetName("entropy");
    entropy_expr.SetDefinition("2.5*log(((0.4*(E - rho*KineticEnergy))/1.4) / (rho^1.4))");
    entropy_expr.SetType(Expression::ScalarMeshVar);
    md->AddExpression(&entropy_expr);
}


