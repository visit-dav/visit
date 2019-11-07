/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                            avtLAMMPSFileFormat.C                           //
// ************************************************************************* //

#include <avtMTSDFileFormatInterface.h>
#include <avtLAMMPSFileFormat.h>

#include <string>
#include <map>

#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <VisItStreamUtil.h>

using namespace std;

bool
avtLAMMPSFileFormat::Identify(const char *fname)
{
    shared_ptr<adios2::ADIOS> adios = std::make_shared<adios2::ADIOS>(adios2::DebugON);
    adios2::IO io = adios2::IO(adios->DeclareIO("ReadBPLAMMPS"));
    io.SetEngine("BP");
    adios2::Engine reader = io.Open(fname, adios2::Mode::Read);
    auto attributes = io.AvailableAttributes();
    auto variables = io.AvailableVariables();

    bool isLAMMPS = true;
    if (variables.find("atoms") == variables.end() ||
        variables.find("natoms") == variables.end() ||
        variables.find("ntimestep") == variables.end() ||
        attributes.find("LAMMPS/dump_style") == attributes.end() ||
        attributes.find("LAMMPS/num_ver") == attributes.end() ||
        attributes.find("LAMMPS/version") == attributes.end())
    {
        isLAMMPS = false;
    }
    reader.Close();
    return isLAMMPS;
}

bool avtLAMMPSFileFormat::IdentifyADIOS2(
                    std::map<std::string, adios2::Params> &variables,
                    std::map<std::string, adios2::Params> &attributes)
{
    int vfind = 0;
    vector<string> reqVars = {"atoms", "natoms", "ntimestep"};

    for (auto vi = variables.begin(); vi != variables.end(); vi++)
        if (std::find(reqVars.begin(), reqVars.end(), vi->first) != reqVars.end())
            vfind++;

    int afind = 0;
    vector<string> reqAttrs = {"LAMMPS/dump_style", "LAMMPS/num_ver", "LAMMPS/version"};

    for (auto ai = attributes.begin(); ai != attributes.end(); ai++)
        if (std::find(reqAttrs.begin(), reqAttrs.end(), ai->first) != reqAttrs.end())
            afind++;

    return (vfind == reqVars.size() && afind == reqAttrs.size());
}

avtFileFormatInterface *
avtLAMMPSFileFormat::CreateInterface(const char *const *list,
                                         int nList,
                                         int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTSDFileFormat ***ffl = new avtMTSDFileFormat**[nTimestepGroups];
    for (int i = 0; i < nTimestepGroups; i++)
    {
        ffl[i] =  new avtMTSDFileFormat*[nBlock];
        for (int j = 0; j < nBlock; j++)
            ffl[i][j] =  new avtLAMMPSFileFormat(list[i*nBlock +j]);
    }
    return new avtMTSDFileFormatInterface(ffl, nTimestepGroups, nBlock);
}

avtFileFormatInterface *
avtLAMMPSFileFormat::CreateInterfaceADIOS2(
        const char *const *list,
        int nList,
        int nBlock,
        std::shared_ptr<adios2::ADIOS> adios,
        adios2::Engine &reader,
        adios2::IO &io,
        std::map<std::string, adios2::Params> &variables,
        std::map<std::string, adios2::Params> &attributes
        )
{
    int nTimestepGroups = nList / nBlock;
    avtMTSDFileFormat ***ffl = new avtMTSDFileFormat**[nTimestepGroups];
    for (int i = 0; i < nTimestepGroups; i++)
    {
        ffl[i] =  new avtMTSDFileFormat*[nBlock];
        for (int j = 0; j < nBlock; j++)
            if (!i && !j)
            {
                ffl[i][j] =  new avtLAMMPSFileFormat(adios, reader, io, variables, attributes, list[i*nBlock +j]);
            }
            else
            {
                ffl[i][j] =  new avtLAMMPSFileFormat(list[i*nBlock +j]);
            }
    }
    return new avtMTSDFileFormatInterface(ffl, nTimestepGroups, nBlock);
}

// ****************************************************************************
//  Method: avtLAMMPSFileFormat constructor
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

avtLAMMPSFileFormat::avtLAMMPSFileFormat(const char *filename)
    : adios(std::make_shared<adios2::ADIOS>(adios2::DebugON)),
      numTimeSteps(1),
      currentTimestep(-1),
      numAtoms(-1),
      numColumns(-1),
      avtMTSDFileFormat(&filename, 1)
{
    io = adios2::IO(adios->DeclareIO("ReadBP"));
    io.SetEngine("BP");
    reader = io.Open(filename, adios2::Mode::Read);
    variables = io.AvailableVariables();
    attributes = io.AvailableAttributes();

    if (variables.find("atoms") == variables.end() ||
        variables.find("natoms") == variables.end() ||
        variables.find("ntimestep") == variables.end() ||
        attributes.find("columns") == attributes.end())
    {
        EXCEPTION1(InvalidFilesException, filename);
    }

    string columnsStr = attributes["columns"]["Value"];
    numColumns = std::stoi(attributes["columns"]["Elements"]);
    GenerateTableOffsets(columnsStr);

    numTimeSteps = std::stoi(variables["atoms"]["AvailableStepsCount"]);

    times.resize(numTimeSteps);
    vector<uint64_t> tbuff(numTimeSteps);
    adios2::Variable<uint64_t> t = io.InquireVariable<uint64_t>("ntimestep");
    t.SetStepSelection({0, numTimeSteps});
    reader.Get(t, tbuff.data(), adios2::Mode::Sync);
    for (int i = 0; i < numTimeSteps; i++)
        times[i] = (double)tbuff[i];
}



avtLAMMPSFileFormat::avtLAMMPSFileFormat(std::shared_ptr<adios2::ADIOS> adios,
        adios2::Engine &reader,
        adios2::IO &io,
        std::map<std::string, adios2::Params> &variables,
        std::map<std::string, adios2::Params> &attributes,
        const char *filename)
    : adios(adios),
      reader(reader),
      io(io),
      numTimeSteps(1),
      currentTimestep(-1),
      numAtoms(-1),
      numColumns(-1),
      avtMTSDFileFormat(&filename, 1),
      variables(variables),
      attributes(attributes)
{
    string columnsStr = attributes["columns"]["Value"];
    numColumns = std::stoi(attributes["columns"]["Elements"]);
    GenerateTableOffsets(columnsStr);

    numTimeSteps = std::stoi(variables["atoms"]["AvailableStepsCount"]);

    times.resize(numTimeSteps);
    vector<uint64_t> tbuff(numTimeSteps);
    adios2::Variable<uint64_t> t = io.InquireVariable<uint64_t>("ntimestep");
    t.SetStepSelection({0, numTimeSteps});
    reader.Get(t, tbuff.data(), adios2::Mode::Sync);
    for (int i = 0; i < numTimeSteps; i++)
        times[i] = (double)tbuff[i];
}
// ****************************************************************************
//  Method: avtADIOS2FileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

void
avtLAMMPSFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtLAMMPSFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

void
avtLAMMPSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 1,
                                               AVT_POINT_MESH);
    mmd->nodesAreCritical = true;
    md->Add(mmd);

    for (auto &s : scalarOffsets)
        AddScalarVarToMetaData(md, s.first, "mesh", AVT_NODECENT);
}

// ****************************************************************************
//  Method: avtLAMMPSFileFormat::GetMesh
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
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

vtkDataSet *
avtLAMMPSFileFormat::GetMesh(int timestate, const char *meshname)
{
    ReadTimestep(timestate);

    vtkPolyData *pd = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    vtkCellArray *verts = vtkCellArray::New();

    pts->SetNumberOfPoints(numAtoms);
    pd->SetPoints(pts);
    pd->SetVerts(verts);
    pts->Delete();
    verts->Delete();

    //2D case.
    if (coordOffset[2] == -1)
        for (int i = 0; i < numAtoms; i++)
            pts->SetPoint(i,
                          data[i*numColumns + coordOffset[0]],
                          data[i*numColumns + coordOffset[1]],
                          0);
    else
        for (int i = 0; i < numAtoms; i++)
            pts->SetPoint(i,
                          data[i*numColumns + coordOffset[0]],
                          data[i*numColumns + coordOffset[1]],
                          data[i*numColumns + coordOffset[2]]);

    //Add the cells.
    for (int i = 0; i < numAtoms; i++)
    {
        verts->InsertNextCell(1);
        verts->InsertCellPoint(i);
    }

    return pd;
}


// ****************************************************************************
//  Method: avtLAMMPSFileFormat::GetVar
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
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

vtkDataArray *
avtLAMMPSFileFormat::GetVar(int timestate, const char *varname)
{
    ReadTimestep(timestate);

    if (scalarOffsets.find(varname) == scalarOffsets.end())
        EXCEPTION1(InvalidVariableException, varname);

    int offset = scalarOffsets[varname];

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfComponents(1);
    arr->SetNumberOfTuples(numAtoms);
    for (int i = 0; i < numAtoms; i++)
        arr->SetTuple1(i, data[i*numColumns + offset]);

    return arr;
}

// ****************************************************************************
//  Method: avtLAMMPSFileFormat::GetVectorVar
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
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Thu Apr 12 08:39:47 PDT 2018
//
// ****************************************************************************

vtkDataArray *
avtLAMMPSFileFormat::GetVectorVar(int timestate, const char *varname)
{
    return NULL;
}


void
avtLAMMPSFileFormat::ReadTimestep(int timestate)
{
    if (currentTimestep == timestate)
        return;

    adios2::Variable<double> atomsV = io.InquireVariable<double>("atoms");
    adios2::Variable<unsigned long int> natomsV = io.InquireVariable<unsigned long int>("natoms");
    atomsV.SetStepSelection({timestate, 1});
    natomsV.SetStepSelection({timestate, 1});

    vector<unsigned long int> buff(1);
    reader.Get(natomsV, buff.data(), adios2::Mode::Sync);
    numAtoms = buff[0];

    data.resize(numAtoms*5);
    reader.Get(atomsV, data.data(), adios2::Mode::Sync);
    currentTimestep = timestate;
}

void
avtLAMMPSFileFormat::GenerateTableOffsets(std::string &columnsStr)
{
    //Remove the braces..
    string str = columnsStr.substr(1, columnsStr.size()-2);

    //Clean it up a bit. remove spaces and quotes.
    str.erase(std::remove(str.begin(), str.end(), '"'), str.end());
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());

    //parse out the tokens which are delimted by the common comma.
    stringstream ss(str);
    vector<string> tokens;
    string item;
    while (getline(ss, item, ','))
        tokens.push_back(item);

    int cnt = 0;
    coordOffset = {-1,-1,-1};
    for (auto &t : tokens)
    {
        if (t == "x" || t == "xs" || t == "xu" || t == "xsu")
            coordOffset[0] = cnt;
        else if (t == "y" || t == "ys" || t == "yu" || t == "ysu")
            coordOffset[1] = cnt;
        else if (t == "z" || t == "zs" || t == "zu" || t == "zsu")
            coordOffset[2] = cnt;
        else
            scalarOffsets[t] = cnt;

        cnt++;
    }
}
