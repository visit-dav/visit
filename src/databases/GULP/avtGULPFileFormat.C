/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                            avtGULPFileFormat.C                           //
// ************************************************************************* //

#include <avtGULPFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <sstream>

using     std::string;


// ****************************************************************************
//  Method: avtGULPFileFormat constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 22, 2010
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 30 11:03:21 EDT 2012
//    Added support for binary files, and for forces and potential energies.
// ****************************************************************************

avtGULPFileFormat::avtGULPFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    filename = fn;
    OpenFileAtBeginning();

    md_read = false;
    has_force_and_pe = false;

    natoms = 0;
    ntimesteps = 0;
    current_timestep = -1;
}


// ****************************************************************************
//  Method: avtGULPFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 22, 2010
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 30 11:03:21 EDT 2012
//    Added support for forces and potential energies.
//
// ****************************************************************************

void
avtGULPFileFormat::FreeUpResources(void)
{
    x.clear();
    y.clear();
    z.clear();
    vx.clear();
    vy.clear();
    vz.clear();
    fx.clear();
    fy.clear();
    fz.clear();
    pe.clear();
    current_timestep = -1;
}


// ****************************************************************************
//  Method: avtGULPFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 22, 2010
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 30 11:03:21 EDT 2012
//    Added support for forces and potential energies.
//
// ****************************************************************************

void
avtGULPFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadAllMetaData();

    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 0,
                                               AVT_POINT_MESH);
    mmd->nodesAreCritical = true;
    md->Add(mmd);

    AddScalarVarToMetaData(md, "vx", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "vy", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "vz", "mesh", AVT_NODECENT);
    Expression velocity_expr;
    velocity_expr.SetName("velocity");
    velocity_expr.SetDefinition("{vx, vy, vz}");
    velocity_expr.SetType(Expression::VectorMeshVar);
    md->AddExpression(&velocity_expr);

    if (has_force_and_pe)
    {
        AddScalarVarToMetaData(md, "fx", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "fy", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "fz", "mesh", AVT_NODECENT);
        Expression force_expr;
        force_expr.SetName("force");
        force_expr.SetDefinition("{fx, fy, fz}");
        force_expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&force_expr);


        AddScalarVarToMetaData(md, "PE", "mesh", AVT_NODECENT);
    }
}


// ****************************************************************************
//  Method: avtGULPFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 22, 2010
//
// ****************************************************************************

vtkDataSet *
avtGULPFileFormat::GetMesh(int ts, const char *name)
{
    ReadTimestep(ts);

    string meshname(name);
    if (meshname == "mesh")
    {
        vtkPolyData *pd  = vtkPolyData::New();
        vtkPoints   *pts = vtkPoints::New();

        pts->SetNumberOfPoints(natoms);
        pd->SetPoints(pts);
        pts->Delete();
        for (int j = 0 ; j < natoms ; j++)
        {
            pts->SetPoint(j, x[j], y[j], z[j]);
        }
 
        vtkCellArray *verts = vtkCellArray::New();
        pd->SetVerts(verts);
        verts->Delete();
        for (int k = 0 ; k < natoms ; k++)
        {
            verts->InsertNextCell(1);
            verts->InsertCellPoint(k);
        }

        return pd;
    }
    
    return NULL;
}


// ****************************************************************************
//  Method: avtGULPFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 22, 2010
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 30 11:03:21 EDT 2012
//    Added support for forces and potential energies.
//
// ****************************************************************************

vtkDataArray *
avtGULPFileFormat::GetVar(int ts, const char *varname)
{
    ReadTimestep(ts);

    if (string(varname) == "vx" ||
        string(varname) == "vy" ||
        string(varname) == "vz" ||
        string(varname) == "fx" ||
        string(varname) == "fy" ||
        string(varname) == "fz" ||
        string(varname) == "PE")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(natoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        if (string(varname) == "vx")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = vx[i];
        }
        else if (string(varname) == "vy")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = vy[i];
        }
        else if (string(varname) == "vz")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = vz[i];
        }
        else if (string(varname) == "fx")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = fx[i];
        }
        else if (string(varname) == "fy")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = fy[i];
        }
        else if (string(varname) == "fz")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = fz[i];
        }
        else // (string(varname) == "PE")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = pe[i];
        }
        return scalars;
    }

    return NULL;
}


// ****************************************************************************
// Method:  avtGULPFileFormat::OpenFileAtBeginning
//
// Purpose:
//   Either opens the file, or resets it to the beginning position.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    June 22, 2010
//
// Modifications:
//   Jeremy Meredith, Mon Jul 30 10:25:22 EDT 2012
//   Added binary vs ascii check.
//
// ****************************************************************************
void
avtGULPFileFormat::OpenFileAtBeginning()
{
    if (!in.is_open())
    {
        in.open(filename.c_str(),ios::binary);
        if (!in)
        {
            EXCEPTION1(InvalidFilesException, filename.c_str());
        }

        // check for binary
        char tmp[100];
        in.read(tmp, 100);
        int nread = in.gcount();
        binary = false;
        for (int i=0; i<nread; i++)
        {
            const unsigned char c = tmp[i];
            if (c==0 || (c>1 && c<7) || (c>13 && c<32) || c>127)
            {
                binary = true;
                break;
            }
        }

        if (!binary)
        {
            // If it's ASCII, let's open it without the binary flag.
            in.close();
            in.clear();
            in.open(filename.c_str());
        }
        else
        {
            in.clear();
            in.seekg(0, ios::beg);
        }
    }
    else
    {
        in.clear();
        in.seekg(0, ios::beg);
    }
}

// ****************************************************************************
// Method:  avtGULPFileFormat::ReadAllMetaData
//
// Purpose:
//   Scans the file.  Reads the main meta-data and figures out how
//   many time steps there are, as well as their file positions for
//   random access later.
//
// Arguments:
//   
//
// Programmer:  Jeremy Meredith
// Creation:    June 22, 2010
//
// Modifications:
//   Jeremy Meredith, Mon Jul 30 10:25:22 EDT 2012
//   Added binary vs ascii file support and for newer file versions
//   with forces and potential energies.
//
// ****************************************************************************
void
avtGULPFileFormat::ReadAllMetaData()
{
    if (md_read)
        return;

    OpenFileAtBeginning();
    md_read = true;

    if (binary)
    {
        int rec;
        in.read((char*)&rec, sizeof(int));
        double version;
        in.read((char*)&version, sizeof(double));
        in.read((char*)&rec, sizeof(int));

        has_force_and_pe = true;
        if (version < 4)
        {
            cerr  << "WARNING: Expected version 4.0, got "<<version
                  << ".  Things may not work as expected....\n";
            cerr << "Assuming it's an earlier file version without forces, PEs\n";
            has_force_and_pe = false;
        }
        

        in.read((char*)&rec, sizeof(int));
        int ndim;
        in.read((char*)&natoms, sizeof(int));
        in.read((char*)&ndim, sizeof(int));
        in.read((char*)&rec, sizeof(int));

        istream::pos_type curpos;
        while (in.good() && !in.eof())
        {
            double ts_time, ts_ke, ts_e, ts_t;
            in.read((char*)&rec, sizeof(int));
            in.read((char*)&ts_time, sizeof(double));
            in.read((char*)&ts_ke, sizeof(double));
            in.read((char*)&ts_e, sizeof(double));
            in.read((char*)&ts_t, sizeof(double));
            in.read((char*)&rec, sizeof(int));

            // mark at the start of the atom coords
            curpos = in.tellg();

            // skip atom positions
            for (int axis=0; axis<3; axis++)
            {
                in.read((char*)&rec, sizeof(int));
                in.seekg(rec, ios::cur); 
                in.read((char*)&rec, sizeof(int));
            }

            // skip atom velocities
            for (int axis=0; axis<3; axis++)
            {
                in.read((char*)&rec, sizeof(int));
                in.seekg(rec, ios::cur); 
                in.read((char*)&rec, sizeof(int));
            }

            // skip atom forces
            if (has_force_and_pe)
            {
                for (int axis=0; axis<3; axis++)
                {
                    in.read((char*)&rec, sizeof(int));
                    in.seekg(rec, ios::cur); 
                    in.read((char*)&rec, sizeof(int));
                }

                in.read((char*)&rec, sizeof(int));
                in.seekg(rec, ios::cur); 
                in.read((char*)&rec, sizeof(int));
            }

            if (!in.good())
            {
                // skip this timestep; it's been truncated or the
                // file ended (correctly) after the last timestep
            }
            else
            {
                file_positions.push_back(curpos);
                times.push_back(ts_time);
                ntimesteps++;
            }
        }
    }
    else
    {
        char line[4096];
        string buff, buff2;

        string version;
        in >> version;

        has_force_and_pe = false;
        if (version != "3.40")
        {
            cerr << "WARNING: Expected version 3.40, got "<<version
                 <<".  Things may not work as expected....\n";
            cerr <<"Assuming it's a later file version with forces, PEs\n";
            has_force_and_pe = true;
        }

        in >> natoms;
        if (natoms == 0)
            InvalidFilesException(filename,
                                  "Didn't get valid atom count.");
        in >> dimension;
        //if (dimension != 0)
        //    InvalidFilesException(filename,
        //                          "We only support 0-dimension files for now.");

        in.getline(line, 4096); // rest of natoms/dimension line

        istream::pos_type curpos;
        while (in.good() && !in.eof())
        {
            curpos = in.tellg();
            bool good = true;


            in >> buff >> buff2;
            if (buff != "#" || buff2 != "Time/KE/E/T")
            {
                if (ntimesteps == 0)
                    InvalidFilesException(filename,
                                          "Parse error, expected '#  Time/KE/E/T'");
                else
                    good = false;
            }

            double t;
            in >> t;
            in >> buff; // ignore KE
            in >> buff; // ignore E
            in >> buff; // ignore T

            in >> buff >> buff2;
            if (buff != "#" || buff2 != "Coordinates")
            {
                if (ntimesteps == 0)
                    InvalidFilesException(filename,
                                          "Parse error, expected '#  Coordinates'");
                else
                    good = false;
            }

            in.getline(line, 4096); // rest of "Coordinates" line

            for (int i=0; i<natoms; ++i)
                in.getline(line, 4096);

            in >> buff >> buff2;
            if (buff != "#" || buff2 != "Velocities")
            {
                if (ntimesteps == 0)
                    InvalidFilesException(filename,
                                          "Parse error, expected '#  Velocities'");
                else
                    good = false;
            }

            in.getline(line, 4096); // rest of "Velocities" line

            for (int i=0; i<natoms; ++i)
                in.getline(line, 4096);

            // check for early EOF or other parsing problem
            if (!good || !in.good())
            {
                // skip this timestep; it's been truncated
            }
            else
            {
                file_positions.push_back(curpos);
                times.push_back(t);
                ntimesteps++;
            }
        }
    }

}

// Modifications:
//   Jeremy Meredith, Mon Jul 30 10:25:22 EDT 2012
//   Added binary vs ascii file support and for newer file versions
//   with forces and potential energies.
//
void
avtGULPFileFormat::ReadTimestep(int ts)
{
    if (current_timestep == ts)
        return;

    ReadAllMetaData();
    OpenFileAtBeginning();
    current_timestep = ts;

    in.seekg(file_positions[ts]);

    if (binary)
    {
        int rec;
        double val;

        // fill the atoms
        x.resize(natoms);
        y.resize(natoms);
        z.resize(natoms);

        in.read((char*)&rec, sizeof(int));
        for (int i=0; i<natoms; ++i)
        {
            in.read((char*)&val, sizeof(double));
            x[i] = val;
        }
        in.read((char*)&rec, sizeof(int));

        in.read((char*)&rec, sizeof(int));
        for (int i=0; i<natoms; ++i)
        {
            in.read((char*)&val, sizeof(double));
            y[i] = val;
        }
        in.read((char*)&rec, sizeof(int));

        in.read((char*)&rec, sizeof(int));
        for (int i=0; i<natoms; ++i)
        {
            in.read((char*)&val, sizeof(double));
            z[i] = val;
        }
        in.read((char*)&rec, sizeof(int));

        // velocities
        vx.resize(natoms);
        vy.resize(natoms);
        vz.resize(natoms);

        in.read((char*)&rec, sizeof(int));
        for (int i=0; i<natoms; ++i)
        {
            in.read((char*)&val, sizeof(double));
            vx[i] = val;
        }
        in.read((char*)&rec, sizeof(int));

        in.read((char*)&rec, sizeof(int));
        for (int i=0; i<natoms; ++i)
        {
            in.read((char*)&val, sizeof(double));
            vy[i] = val;
        }
        in.read((char*)&rec, sizeof(int));

        in.read((char*)&rec, sizeof(int));
        for (int i=0; i<natoms; ++i)
        {
            in.read((char*)&val, sizeof(double));
            vz[i] = val;
        }
        in.read((char*)&rec, sizeof(int));

        if (has_force_and_pe)
        {
            // forces
            fx.resize(natoms);
            fy.resize(natoms);
            fz.resize(natoms);

            in.read((char*)&rec, sizeof(int));
            for (int i=0; i<natoms; ++i)
            {
                in.read((char*)&val, sizeof(double));
                fx[i] = val;
            }
            in.read((char*)&rec, sizeof(int));

            in.read((char*)&rec, sizeof(int));
            for (int i=0; i<natoms; ++i)
            {
                in.read((char*)&val, sizeof(double));
                fy[i] = val;
            }
            in.read((char*)&rec, sizeof(int));

            in.read((char*)&rec, sizeof(int));
            for (int i=0; i<natoms; ++i)
            {
                in.read((char*)&val, sizeof(double));
                fz[i] = val;
            }
            in.read((char*)&rec, sizeof(int));

            // pe
            pe.resize(natoms);

            in.read((char*)&rec, sizeof(int));
            for (int i=0; i<natoms; ++i)
            {
                in.read((char*)&val, sizeof(double));
                pe[i] = val;
            }
            in.read((char*)&rec, sizeof(int));
        }
    }
    else
    {
        char line[4096];
        in.getline(line, 4096); // skip the time comment header
        in.getline(line, 4096); // skip the time, energy, etc
        in.getline(line, 4096); // skip the coordinates comment header

        // fill the atoms
        x.resize(natoms);
        z.resize(natoms);
        y.resize(natoms);
        for (int i=0; i<natoms; ++i)
        {
            in >> x[i] >> y[i] >> z[i];
        }

        in.getline(line, 4096); // finish the last line of atoms
        in.getline(line, 4096); // skip the velocities comment header

        // fill the velocities
        vx.resize(natoms);
        vz.resize(natoms);
        vy.resize(natoms);
        for (int i=0; i<natoms; ++i)
        {
            in >> vx[i] >> vy[i] >> vz[i];
        }

        if (has_force_and_pe)
        {
            // NOTE: We have no example ASCII files v4.0+ yet to test this.

            // fill the forces
            in.getline(line, 4096); // finish the last line of velocities
            in.getline(line, 4096); // skip the forces comment header

            fx.resize(natoms);
            fz.resize(natoms);
            fy.resize(natoms);
            for (int i=0; i<natoms; ++i)
            {
                in >> fx[i] >> fy[i] >> fz[i];
            }

            // fill the PE values
            in.getline(line, 4096); // finish the last line of forces
            in.getline(line, 4096); // skip the PE comment header

            pe.resize(natoms);
            for (int i=0; i<natoms; ++i)
            {
                in >> pe[i];
            }
        }
    }
}

// ****************************************************************************
// Method:  avtGULPFileFormat::GetTimes
//
// Purpose:
//   Gets the time values.
//
// Arguments:
//   t          the vector to fill
//
// Programmer:  Jeremy Meredith
// Creation:    June 22, 2010
//
// ****************************************************************************
void
avtGULPFileFormat::GetTimes(std::vector<double> &t)
{
    for (size_t i=0; i<times.size(); i++)
        t.push_back(times[i]);
}

// ****************************************************************************
// Method:  avtGULPFileFormat::GetNTimesteps
//
// Purpose:
//   Gets the number of time steps.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    June 22, 2010
//
// ****************************************************************************
int
avtGULPFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return ntimesteps;
}

