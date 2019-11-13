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
//                            avtMEUMMAPSFileFormat.C                           //
// ************************************************************************* //

#include <avtMTMDFileFormatInterface.h>
#include <avtMEUMMAPSFileFormat.h>
#include <ADIOS2HelperFuncs.h>

#include <string>
#include <map>

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <VisItStreamUtil.h>

using namespace std;

bool
avtMEUMMAPSFileFormat::Identify(const char *fname)
{
    bool retval = false;
    string engineName = ADIOS2Helper_GetEngineName(fname);
    string fileName   = ADIOS2Helper_GetFileName(fname);
    bool stagingMode  = ADIOS2Helper_IsStagingEngine(engineName);

    adios2::ADIOS adios;
    adios2::IO io(adios.DeclareIO("ReadBP"));
    io.SetEngine(engineName);
    adios2::Engine reader = io.Open(fileName, adios2::Mode::Read);
    adios2::StepStatus status =
        reader.BeginStep(adios2::StepMode::Read, -1.0f);
    if (status == adios2::StepStatus::OK)
    {
        //std::cout<<" Identifier for MEUMAPPS received streaming step = "<<reader.CurrentStep()<<endl;

        std::map<std::string, adios2::Params> variables, attributes;
        variables = io.AvailableVariables();
        attributes = io.AvailableAttributes();

        int afind = 0;
        for (auto it = attributes.begin(); it != attributes.end(); it++)
        {
            if (it->first == "app" && it->second["Value"] == "\"meumapps\"")
                afind++;
        }

        int vfind = 0;
        vector<string> reqVars = {"Nx", "Ny", "dx", "dy", "dz"};
        for (auto vi = variables.begin(); vi != variables.end(); vi++)
            if (std::find(reqVars.begin(), reqVars.end(), vi->first) != reqVars.end())
                vfind++;

        retval = (afind == 1 && vfind==reqVars.size());
        reader.EndStep();
    }
    reader.Close();
    return retval;
}

bool avtMEUMMAPSFileFormat::IdentifyADIOS2(
                    std::map<std::string, adios2::Params> &variables,
                    std::map<std::string, adios2::Params> &attributes)
{
    int afind = 0;
    for (auto it = attributes.begin(); it != attributes.end(); it++)
    {
        if (it->first == "app" && it->second["Value"] == "\"meumapps\"")
            afind++;
    }

    int vfind = 0;
    vector<string> reqVars = {"Nx", "Ny", "dx", "dy", "dz"};
    for (auto vi = variables.begin(); vi != variables.end(); vi++)
        if (std::find(reqVars.begin(), reqVars.end(), vi->first) != reqVars.end())
            vfind++;

    return (afind == 1 && vfind==reqVars.size());
}

avtFileFormatInterface *
avtMEUMMAPSFileFormat::CreateInterface(const char *const *list,
                                         int nList,
                                         int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
        ffl[i] = new avtMEUMMAPSFileFormat(list[i*nBlock]);

    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}

avtFileFormatInterface *
avtMEUMMAPSFileFormat::CreateInterfaceADIOS2(
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
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0; i < nTimestepGroups; i++)
    {
        if (!i)
        {
            ffl[i] =  new avtMEUMMAPSFileFormat(adios, reader, io, variables, attributes, list[i*nBlock]);
        }
        else
        {
            ffl[i] =  new avtMEUMMAPSFileFormat(list[i*nBlock]);
        }
    }
    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}


// ****************************************************************************
//  Method: avtMEUMMAPSFileFormat constructor
//
//  Programmer: dpn -- generated by xml2avt
//  Creation:   Tue Sep 11 13:23:54 PST 2018
//
// ****************************************************************************

avtMEUMMAPSFileFormat::avtMEUMMAPSFileFormat(const char *filename)
    :  adios(std::make_shared<adios2::ADIOS>(adios2::DebugON)),
       io(adios->DeclareIO("ReadBP")),
       numTimeSteps(1),
       avtMTMDFileFormat(filename)
{
    reader = io.Open(filename, adios2::Mode::Read);
    variables = io.AvailableVariables();
    auto attributes = io.AvailableAttributes();
//    for (auto &a : attributes)
//        cout<<"Attr: "<<a.first<<" "<<a.second<<endl;

    if (variables.size() > 0)
    {
        auto var0 = variables.begin()->second;
        string nsteps = var0["AvailableStepsCount"];
        numTimeSteps = std::stoi(nsteps);
    }

//    for (auto &v : variables)
//        cout<<"Var: "<<v.first<<endl;

    origin = {0,0,0};
    spacing.push_back(std::stof(variables["dx"]["Value"]));
    spacing.push_back(std::stof(variables["dy"]["Value"]));
    spacing.push_back(std::stof(variables["dz"]["Value"]));
    meshSz.push_back(std::stoi(variables["Nx"]["Value"]) + 1);
    meshSz.push_back(std::stoi(variables["Ny"]["Value"]) + 1);
    meshSz.push_back(std::stoi(variables["Nz"]["Value"]) + 1);

    dT = std::stof(variables["dt"]["Value"]);
//    cout<<"NT= "<<numTimeSteps<<endl;
}

avtMEUMMAPSFileFormat::avtMEUMMAPSFileFormat(std::shared_ptr<adios2::ADIOS> adios,
        adios2::Engine &reader,
        adios2::IO &io,
        std::map<std::string, adios2::Params> &variables,
        std::map<std::string, adios2::Params> &attributes,
        const char *filename)
    : adios(adios),
      reader(reader),
      io(io),
      numTimeSteps(1),
      avtMTMDFileFormat(filename),
      variables(variables)
{
    if (variables.size() > 0)
    {
        auto var0 = variables.begin()->second;
        string nsteps = var0["AvailableStepsCount"];
        numTimeSteps = std::stoi(nsteps);
    }

//    for (auto &v : variables)
//        cout<<"Var: "<<v.first<<endl;

    origin = {0,0,0};
    spacing.push_back(std::stof(variables["dx"]["Value"]));
    spacing.push_back(std::stof(variables["dy"]["Value"]));
    spacing.push_back(std::stof(variables["dz"]["Value"]));
    meshSz.push_back(std::stoi(variables["Nx"]["Value"]) + 1);
    meshSz.push_back(std::stoi(variables["Ny"]["Value"]) + 1);
    meshSz.push_back(std::stoi(variables["Nz"]["Value"]) + 1);

    dT = std::stof(variables["dt"]["Value"]);
//    cout<<"NT= "<<numTimeSteps<<endl;
}

// ****************************************************************************
//  Method: avtMEUMMAPSFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: dpn -- generated by xml2avt
//  Creation:   Tue Sep 11 13:23:54 PST 2018
//
// ****************************************************************************

void
avtMEUMMAPSFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtMEUMMAPSFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: dpn -- generated by xml2avt
//  Creation:   Tue Sep 11 13:23:54 PST 2018
//
// ****************************************************************************

void
avtMEUMMAPSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    int numBlocks = -1;

    for (auto &v : variables)
    {
        if (v.second["SingleValue"] == "false" &&
            v.first != "istart" && v.first != "iend") //skip parallel writer var
        {
            AddScalarVarToMetaData(md, v.first, "mesh",  AVT_NODECENT);
            if (numBlocks == -1)
            {
                vector<adios2::Variable<double>::Info> blockInfo;
                blockInfo = reader.BlocksInfo(io.InquireVariable<double>(v.first), timeState);
                numBlocks = blockInfo.size();
            }
        }
    }

    AddMeshToMetaData(md, "mesh", AVT_RECTILINEAR_MESH, NULL, numBlocks, 0, 3, 3);
//    cout<<"numblocks= "<<numBlocks<<endl;
}


// ****************************************************************************
//  Method: avtMEUMMAPSFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: dpn -- generated by xml2avt
//  Creation:   Tue Sep 11 13:23:54 PST 2018
//
// ****************************************************************************

vtkDataSet *
avtMEUMMAPSFileFormat::GetMesh(int ts, int domain, const char *meshname)
{
    vector<adios2::Variable<double>::Info> blockInfo;
    blockInfo = reader.BlocksInfo(io.InquireVariable<double>("T"), ts);

    auto info = blockInfo[domain];
    int nx = 200, ny = 200, nz = 200;
    int i0 = info.Start[0];
    int i1 = i0 + info.Count[0];
    int ix0 = i0 % nz;
    int iy0 = (i0/nz) % ny;
    int iz0 = i0/(ny*nz);

    int ix1 = i1 % nz;
    int iy1 = (i1/nz) % ny;
    int iz1 = i1/(ny*nz);

//    cout<<"GetMesh: "<<domain<<" "<<info.Start<<" "<<info.Count<<" ("<<ix0<<" "<<iy0<<" "<<iz0<<") ("<<ix1<<" "<<iy1<<" "<<iz1<<")"<<endl;

    int dims[3] = {ix1-ix0, iy1-iy0, iz1-iz0};

    vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
    grid->SetDimensions(dims);
    vtkFloatArray *coords[3] = {vtkFloatArray::New(),vtkFloatArray::New(),vtkFloatArray::New()};

    for (int c = 0; c < 3; c++)
        coords[c]->SetNumberOfTuples(dims[c]);

    for (int i = 0; i < dims[0]; i++) coords[0]->SetTuple1(i, ix0+i);
    for (int i = 0; i < dims[1]; i++) coords[1]->SetTuple1(i, iy0+i);
    for (int i = 0; i < dims[2]; i++) coords[2]->SetTuple1(i, iz0+i);

    grid->SetXCoordinates(coords[0]);
    grid->SetYCoordinates(coords[1]);
    grid->SetZCoordinates(coords[2]);
    coords[0]->Delete();
    coords[1]->Delete();
    coords[2]->Delete();

    return grid;


    /*
    int i0, i1;
    GetDomainIdx(domain, i0, i1);

    cout<<"CreateMesh sz= "<<meshSz<<endl;
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
    int dims[3] = {meshSz[0], meshSz[1], meshSz[2]};

    grid->SetDimensions(dims);
    vtkFloatArray *coords[3] = {vtkFloatArray::New(),vtkFloatArray::New(),vtkFloatArray::New()};

    for (int c = 0; c < 3; c++)
    {
        coords[c]->SetNumberOfTuples(dims[c]);
        for (int i = 0; i < dims[c]; i++)
            coords[c]->SetTuple1(i, origin[c] + spacing[c]*(double)i);
    }

    grid->SetXCoordinates(coords[0]);
    grid->SetYCoordinates(coords[1]);
    grid->SetZCoordinates(coords[2]);
    coords[0]->Delete();
    coords[1]->Delete();
    coords[2]->Delete();

    return grid;
    */
}


// ****************************************************************************
//  Method: avtMEUMMAPSFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: dpn -- generated by xml2avt
//  Creation:   Tue Sep 11 13:23:54 PST 2018
//
// ****************************************************************************

vtkDataArray *
avtMEUMMAPSFileFormat::GetVar(int ts, int domain, const char *varname)
{
//    cout<<"GetVar: "<<varname<<endl;

    if (variables.find(varname) == variables.end())
        return NULL;

    int i0, i1;
    GetDomainIdx(domain, i0, i1);

    auto var = variables[varname];
    string varType = var["Type"];
    if (varType == "double")
    {
        adios2::Variable<double> v = io.InquireVariable<double>(varname);
//        cout<<"DIMS= "<<v.Shape()<<endl;
//        cout<<variables[varname]<<endl;

        v.SetSelection(adios2::Box<adios2::Dims>({0}, v.Shape()));

        size_t numVals = 1;
        for (int i = 0; i < v.Shape().size(); i++)
            numVals *= v.Shape()[i];

        vector<double> buff(numVals);
        reader.Get(v, buff.data(), adios2::Mode::Sync);

        vtkDoubleArray *arr = vtkDoubleArray::New();
        arr->SetNumberOfComponents(1);
        arr->SetNumberOfTuples(numVals);
        for (int i = 0; i < numVals; i++)
            arr->SetTuple1(i, buff[i]);
        return arr;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtMEUMMAPSFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: dpn -- generated by xml2avt
//  Creation:   Tue Sep 11 13:23:54 PST 2018
//
// ****************************************************************************

vtkDataArray *
avtMEUMMAPSFileFormat::GetVectorVar(int ts, int domain, const char *varname)
{
    return NULL;
}


void
avtMEUMMAPSFileFormat::GetDomainIdx(int domain, int &i0, int &i1)
{
    if (startIdx.empty())
    {
        int numBlocks = std::stoi(variables["numprocs"]["Value"]);
        startIdx.resize(numBlocks);
        adios2::Variable<int> varIStart = io.InquireVariable<int>("istart");
        reader.Get(varIStart, startIdx.data(), adios2::Mode::Sync);
    }

    if (endIdx.empty())
    {
        int numBlocks = std::stoi(variables["numprocs"]["Value"]);
        endIdx.resize(numBlocks);
        adios2::Variable<int> varIEnd = io.InquireVariable<int>("iend");
        reader.Get(varIEnd, endIdx.data(), adios2::Mode::Sync);
    }

    i0 = startIdx[domain];
    i1 = endIdx[domain];
}
