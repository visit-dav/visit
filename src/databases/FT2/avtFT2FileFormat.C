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
//                            avtFT2FileFormat.C                           //
// ************************************************************************* //

#include <avtFT2FileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>
#include <vtkPointData.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <sstream>

using std::istringstream;
using std::string;
using std::vector;

static const char *varnames[FT2_NVARS] =
{
    "mz",
    "intensity",
    "resolution",
    "baseline",
    "noise"
};


// ****************************************************************************
//  Method: avtFT2FileFormat constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   February  5, 2013
//
// ****************************************************************************

avtFT2FileFormat::avtFT2FileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    filename = fn;
    OpenFileAtBeginning();

    metaDataRead = false;

    currentTimestep = -1;
    // reset limits
    for (int i=0; i<FT2_NVARS; i++)
    {
        varmin[i] = +FLT_MAX;
        varmax[i] = -FLT_MAX;
    }

    // Note: setting the curve limits in metadata doesn't appear to actually
    // do anything, so we have no reason to enable this option here nor
    // expose it to the users.
    getLimitsAcrossWholeFile = false;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  5, 2013
//
// ****************************************************************************

int
avtFT2FileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return (int)filePositions.size();
}


// ****************************************************************************
//  Method: avtFT2FileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  5, 2013
//
// ****************************************************************************

void
avtFT2FileFormat::FreeUpResources(void)
{
    for (int i=0; i < FT2_NVARS; ++i)
        vars[i].clear();
    currentTimestep = -1;
}

// ****************************************************************************
// Method:  avtFT2FileFormat::OpenFileAtBeginning
//
// Purpose:
//   Either opens the file, or resets it to the beginning position.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February  5, 2013
//
// ****************************************************************************
void
avtFT2FileFormat::OpenFileAtBeginning()
{
    if (!in.is_open())
    {
        in.open(filename.c_str());
        if (!in)
        {
            EXCEPTION1(InvalidFilesException, filename.c_str());
        }
    }
    else
    {
        in.clear();
        in.seekg(0, ios::beg);
    }
}



// ****************************************************************************
//  Method: avtFT2FileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  5, 2013
//
// ****************************************************************************

void
avtFT2FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    ReadAllMetaData();

    for (int i = 0 ; i < FT2_NVARS ; ++i)
    {
        avtCurveMetaData *curve = new avtCurveMetaData;
        curve->name = varnames[i];
        if (getLimitsAcrossWholeFile)
        {
            curve->hasSpatialExtents = true;
            curve->minSpatialExtents = 0;
            curve->maxSpatialExtents = vars[i].size() - 1;
            curve->hasDataExtents = true;
            curve->minDataExtents = varmin[i];
            curve->maxDataExtents = varmax[i];
        }
        md->Add(curve);
    }

    // We're mostly interested in intensity vs m/z.  Do that one special.
    avtCurveMetaData *curve = new avtCurveMetaData;
    curve->name = "intensity_vs_mz";
    if (getLimitsAcrossWholeFile)
    {
        curve->hasSpatialExtents = true;
        curve->minSpatialExtents = varmin[0];
        curve->maxSpatialExtents = varmax[0];
        curve->hasDataExtents = true;
        curve->minDataExtents = 0; //varmin[1]; // force minimum intensity to 0
        curve->maxDataExtents = varmax[1];
    }
    md->Add(curve);
}


// ****************************************************************************
//  Method: avtFT2FileFormat::GetMesh
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
//  Programmer: Jeremy Meredith
//  Creation:   February  5, 2013
//
// ****************************************************************************

vtkDataSet *
avtFT2FileFormat::GetMesh(int timestate, const char *meshname)
{
    ReadAllMetaData();
    ReadTimeStep(timestate);

    int nPts = vars[0].size();

    vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(nPts,VTK_FLOAT);
 
    vtkFloatArray    *vals = vtkFloatArray::New();
    vals->SetNumberOfComponents(1);
    vals->SetNumberOfTuples(nPts);
    vals->SetName(meshname);

    rg->GetPointData()->SetScalars(vals);
    vals->Delete();
    vtkFloatArray *xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());

    int varindex = -1;
    for (int i=0; i<FT2_NVARS; ++i)
    {
        if (string(meshname) == string(varnames[i]))
        {
            varindex = i;
            break;
        }
    }

    if (varindex == -1)
    {
        // this is intensity_vs_mz; it's special
        for (int i=0; i<nPts; ++i)
        {
            xc->SetValue(i, vars[0][i]);
            vals->SetValue(i, vars[1][i]);
        }
    }
    else
    {
        // normal var from a data section column
        for (int i=0; i<nPts; ++i)
        {
            xc->SetValue(i, i);
            vals->SetValue(i, vars[varindex][i]);
        }
    }

    return rg;
}


// ****************************************************************************
//  Method: avtFT2FileFormat::GetVar
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
//  Programmer: Jeremy Meredith
//  Creation:   February  5, 2013
//
// ****************************************************************************

vtkDataArray *
avtFT2FileFormat::GetVar(int timestate, const char *varname)
{
    // curve variables are in getmesh and don't use variables
    EXCEPTION1(InvalidVariableException, varname);

}

// ****************************************************************************
// Method:  avtFT2FileFormat::ReadAllMetaData
//
// Purpose:
//   Skims the entire file to read the meta data (i.e. get time steps,
//   optionally to find variable limits.)
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February  5, 2013
//
// Modifications:
// ****************************************************************************

void
avtFT2FileFormat::ReadAllMetaData()
{
    if (metaDataRead)
        return;

    OpenFileAtBeginning();

    char line[4096];

    bool inHeader = false;
    bool gotTime = false;
    bool gotCycle = false;
    while (in.good() && !in.eof())
    {
        // mark position at start of line
        istream::pos_type position = in.tellg();
        // read line
        in.getline(line,4096);

        // handle header items
        if (line[0] == 'H')
        {
            inHeader = true;
        }
        else if (line[0] == 'S')
        {
            istringstream sin(line);
            string tmp;
            sin >> tmp;
            int cycle;
            sin >> cycle;
            cycles.push_back(cycle);
            gotCycle = true;
            inHeader = true;
        }
        else if (line[0] == 'Z')
        {
            inHeader = true;
        }
        else if (line[0] == 'I')
        {
            istringstream sin(line);
            string tmp, chk;
            sin >> tmp >> chk;
            if (chk == "RetentionTime")
            {
                double time;
                sin >> time;
                times.push_back(time);
                gotTime = true;
            }
            inHeader = true;
        }
        else if (line[0] == 'D')
        {
            inHeader = true;
        }
        else  // data row
        {
            if (inHeader)
            {
                if (!gotTime)
                    EXCEPTION2(InvalidFilesException, filename.c_str(),
                               "Header for a time step did not contain RetentionTime");
                if (!gotCycle)
                    EXCEPTION2(InvalidFilesException, filename.c_str(),
                               "Header for a time step did not contain S row for cycle");

                filePositions.push_back(position);
                inHeader = false;
                gotCycle = false;
                gotTime = false;
            }

            if (getLimitsAcrossWholeFile)
            {
                // We don't want to read the data here unless we need
                // to find variable limits across the entire file.
                istringstream sin(line);
                float t;
                for (int i=0; i<FT2_NVARS; i++)
                {
                    sin >> t;
                    if (t < varmin[i])
                        varmin[i] = t;
                    if (t > varmax[i])
                        varmax[i] = t;
                }
            }
        }
    }

    if (filePositions.size() != times.size() ||
        filePositions.size() != cycles.size())
    {
        EXCEPTION2(InvalidFilesException, filename.c_str(),
                   "Header and data blocks did not cleanly alternate");
    }

    metaDataRead = true;


    //for (int i=0; i<FT2_NVARS; i++)
    //{
    //    cout << "varmin["<<i<<"] = " << varmin[i] << endl;
    //    cout << "varmax["<<i<<"] = " << varmax[i] << endl;
    //}
}

// ****************************************************************************
// Method:  avtFT2FileFormat::ReadTimeStep
//
// Purpose:
//   Reads all data for a given time step.
//
// Arguments:
//   ts         the time step
//
// Programmer:  Jeremy Meredith
// Creation:    February  5, 2013
//
// Modifications:
// ****************************************************************************

void
avtFT2FileFormat::ReadTimeStep(int ts)
{
    ReadAllMetaData();

    // don't read this time step if it's already in memory
    if (currentTimestep == ts)
        return;

    // empty the current timestep storage
    FreeUpResources();
    
    // go to the right spot in the file where the data actually starts
    OpenFileAtBeginning();
    in.seekg(filePositions[ts]);

    // read the data
    char line[4096];
    in.getline(line,4096);
    while (in.good() && !in.eof())
    {
        if (line[0]=='H' ||
            line[0]=='S' ||
            line[0]=='Z' ||
            line[0]=='I' ||
            line[0]=='D')
        {
            // hit the next header; break
            break;
        }

        istringstream sin(line);
        for (int i=0; i<FT2_NVARS; i++)
        {
            float t;
            sin >> t;
            vars[i].push_back(t);
        }

        // read the next line
        in.getline(line,4096);
    }


    // mark that the currently-read timestep is this one
    currentTimestep = ts;
}

// ****************************************************************************
// Method:  avtFT2FileFormat::GetTimes
//
// Purpose:
//   Returns time values for each timestep.
//
// Programmer:  Jeremy Meredith
// Creation:    February  5, 2013
//
// ****************************************************************************
void
avtFT2FileFormat::GetTimes(std::vector<double> &timeOut)
{
    int nTimeSteps = filePositions.size();
    for (int i=0; i<nTimeSteps; i++)
        timeOut.push_back(times[i]);
}

// ****************************************************************************
// Method:  avtFT2FileFormat::GetCycles
//
// Purpose:
//   Return cycle values for each timestep.
//
// Programmer:  Jeremy Meredith
// Creation:    August 16, 2010
//
// ****************************************************************************
void
avtFT2FileFormat::GetCycles(std::vector<int> &cycleOut)
{
    int nTimeSteps = filePositions.size();
    for (int i=0; i<nTimeSteps; i++)
        cycleOut.push_back(cycles[i]);
}
