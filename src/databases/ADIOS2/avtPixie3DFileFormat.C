// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtPixie3DFileFormat.C                           //
// ************************************************************************* //

#include <avtMTMDFileFormatInterface.h>
#include <avtPixie3DFileFormat.h>
#include <ADIOS2HelperFuncs.h>

#include <string>
#include <map>
#include <numeric>

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkStructuredGrid.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <VisItStreamUtil.h>
#include <DebugStream.h>

using namespace std;

// ****************************************************************************
//  Method: RemoveLeadingSlash
//
//  Purpose: Remove slash from name.
//
//  Programmer: Dave Pugmire
//  Creation:
//
//  Modifications:
//
// ****************************************************************************

static std::string
RemoveLeadingSlash(const std::string &str)
{
    std::string s;
    if (str[0] == '/')
        s = str.substr(1, str.size());
    else
        s = str;
    return s;
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::Identify
//
//  Purpose: Determine if file is for Pixie3D
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:48:07 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

bool
avtPixie3DFileFormat::Identify(const char *fname)
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
        std::map<std::string, adios2::Params> variables, attributes;
        variables = io.AvailableVariables();
        attributes = io.AvailableAttributes();

        if (attributes.find("/schema/name") != attributes.end())
        {
            auto m = attributes[string("/schema/name")];
            if (m.find("Value") != m.end() && m["Value"] == "\"Pixie\"")
                retval = true;
        }
        reader.EndStep();
    }
    reader.Close();
    return retval;
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::IdentifyADIOS2
//
//  Purpose: Determine if file is for Pixie3D
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:48:07 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

bool avtPixie3DFileFormat::IdentifyADIOS2(
                    std::map<std::string, adios2::Params> &variables,
                    std::map<std::string, adios2::Params> &attributes)
{
    bool retval = false;
    if (attributes.find("/schema/name") != attributes.end())
    {
        auto m = attributes[string("/schema/name")];
        if (m.find("Value") != m.end() && m["Value"] == "\"Pixie\"")
            retval = true;
    }

    return retval;
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::CreateInterface
//
//  Purpose: Create interface for file
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

avtFileFormatInterface *
avtPixie3DFileFormat::CreateInterface(const char *const *list,
                                         int nList,
                                         int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
        ffl[i] = new avtPixie3DFileFormat(list[i*nBlock]);

    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::CreateInterface
//
//  Purpose: Create interface for file
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

avtFileFormatInterface *
avtPixie3DFileFormat::CreateInterfaceADIOS2(
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
            ffl[i] =  new avtPixie3DFileFormat(adios, reader, io, variables, attributes, list[i*nBlock]);
        }
        else
        {
            ffl[i] =  new avtPixie3DFileFormat(list[i*nBlock]);
        }
    }
    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}


// ****************************************************************************
//  Method: avtPixie3DFileFormat::avtPixie3DFileFormat
//
//  Purpose: Constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

avtPixie3DFileFormat::avtPixie3DFileFormat(const char *filename)
    :  adios(std::make_shared<adios2::ADIOS>()),
       io(adios->DeclareIO("ReadBP")),
       numTimeSteps(1),
       avtMTMDFileFormat(filename),
       initialized(false)
{
    reader = io.Open(filename, adios2::Mode::Read);
    variables = io.AvailableVariables();
    attributes = io.AvailableAttributes();
    Initialize();
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::avtPixie3DFileFormat
//
//  Purpose: Constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

avtPixie3DFileFormat::avtPixie3DFileFormat(std::shared_ptr<adios2::ADIOS> adios,
        adios2::Engine &reader,
        adios2::IO &io,
        std::map<std::string, adios2::Params> &_variables,
        std::map<std::string, adios2::Params> &_attributes,
        const char *filename)
    : adios(adios),
      reader(reader),
      io(io),
      numTimeSteps(1),
      avtMTMDFileFormat(filename),
      variables(_variables),
      attributes(_attributes),
      initialized(false)
{
    Initialize();
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::Initialize
//
//  Purpose: Extract metadata from file.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtPixie3DFileFormat::Initialize()
{
    if (initialized)
        return;

    //Set up time and cycles.
    numTimeSteps = 1;
    if (variables.find("time") != variables.end());
    {
        auto timeV = variables["time"];
        string nsteps = timeV["AvailableStepsCount"];
        if (!nsteps.empty())
        {
            numTimeSteps = std::stoi(nsteps);
            auto vt = io.InquireVariable<double>("time");
            vt.SetStepSelection({0, numTimeSteps});
            reader.Get(vt, times, adios2::Mode::Sync);

            auto vc = io.InquireVariable<int>("itime");
            vc.SetStepSelection({0, numTimeSteps});
            reader.Get(vc, cycles, adios2::Mode::Sync);
        }
    }
    if (numTimeSteps == 1 && times.empty() && cycles.empty())
    {
        times = {0.0};
        cycles = {0};
    }

    for (auto &a : attributes)
    {
        size_t i = a.first.find("/coords/");
        size_t j = a.first.find("/ncoord");
        string meshName;
        if (i != std::string::npos && j == std::string::npos)
        {
            meshName = a.first.substr(1,i-1) + "_mesh";
            string varName = a.second["Value"];
            //remove the quotes on the variable name.
            varName = varName.substr(1, varName.size()-2);
            if (meshes.find(meshName) == meshes.end())
            {
                meshInfo mi;
                mi.meshType = AVT_CURVILINEAR_MESH;
                meshes[meshName] = mi;
            }
            meshes[meshName].coordVars.push_back(varName);
        }
    }

    //Add defaults, if present.
    vector<string> def = {"/cells/X", "/cells/Y", "/cells/Z"};
    int cnt = 0;
    for (auto &d : def)
        if (variables.find(d) != variables.end())
            cnt++;

    if (cnt == def.size())
    {
        meshInfo mi;
        mi.meshType = AVT_RECTILINEAR_MESH;
        mi.coordVars = def;
        meshes["default_mesh"] = mi;
    }

    //add var to mesh mapping.
    vector<string> skipVars = {"/cells/X", "/cells/Y", "/cells/Z", "/nodes/X", "/nodes/Y", "/nodes/Z"};
    for (auto &v : variables)
    {
        auto params = v.second;
        if (params["SingleValue"] == "true" ||
            std::find(skipVars.begin(), skipVars.end(), v.first) != skipVars.end())
            continue;

        string varname = RemoveLeadingSlash(v.first);
        varMesh[varname] = GetMeshName(varname);
    }

    initialized = true;
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::GetMeshName
//
//  Purpose: Given variable name, return the meshName.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

std::string
avtPixie3DFileFormat::GetMeshName(const std::string &varName) const
{
    string vnm = varName;
    std::size_t i = vnm.find("/");
    if (i != std::string::npos)
        vnm = vnm.substr(0,i);

    vnm = vnm + "_mesh";
    std::string meshName = "default_mesh";
    auto it = meshes.find(vnm);
    if (it != meshes.end())
        meshName = it->first;

    return meshName;
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::FreeUpResources
//
//  Purpose: Given variable name, return the meshName.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtPixie3DFileFormat::FreeUpResources(void)
{
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose: Report metadata for this file.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtPixie3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int ts)
{
    int numBlocks = 1;
    md->SetFormatCanDoDomainDecomposition(false);

    //Reset prev timestate.
    for (auto &m : meshes)
    {
        m.second.currCoords.clear();
        m.second.currDims.clear();
        auto c0 = m.second.coordVars[0];
        auto v = io.InquireVariable<double>(c0);
        v.SetStepSelection({ts, 1});
        auto shape = v.Shape();
        if (m.second.meshType == AVT_RECTILINEAR_MESH)
        {
            if (shape[0] == 1) //2D mesh
            {
                m.second.currCoords = {m.second.coordVars[0], m.second.coordVars[1]};
                m.second.currDims = {(int)shape[2], (int)shape[1]};
            }
            else //3D mesh
            {
                m.second.currCoords = {m.second.coordVars[2], m.second.coordVars[1], m.second.coordVars[0]};
                m.second.currDims = {(int)shape[2], (int)shape[1], (int)shape[0]};
            }
        }
        else if (m.second.meshType == AVT_CURVILINEAR_MESH)
        {
            if (shape[0] == 2) //2D mesh
            {
                m.second.currCoords = {m.second.coordVars[0], m.second.coordVars[1]};
                m.second.currDims = {(int)shape[2], (int)shape[1]};
            }
            else //3D mesh
            {
                m.second.currCoords = {m.second.coordVars[2], m.second.coordVars[1], m.second.coordVars[0]};
                m.second.currDims = {(int)shape[2], (int)shape[1], (int)shape[0]};
            }
        }

        int ndim = m.second.currDims.size();
        AddMeshToMetaData(md, m.first, m.second.meshType, NULL, numBlocks, 0, ndim, ndim);
    }

    //Add variables
    for (auto v : varMesh)
    {
        avtScalarMetaData *smd = new avtScalarMetaData();
        smd->name = v.first;
        smd->originalName = "/" + v.first;
        smd->meshName = v.second;
        smd->centering = (v.second == "default_mesh" ? AVT_ZONECENT : AVT_NODECENT);
        smd->hasDataExtents = false;
        md->Add(smd);
    }

    //Add expressions.
    if (variables.find("visit_expressions") != variables.end())
    {
        auto ve = variables["visit_expressions"];
        adios2::Variable<string> v = io.InquireVariable<string>("visit_expressions");

        string rawExpression;
        reader.Get(v, rawExpression, adios2::Mode::Sync);

        std::string::size_type s = 0;
        while (s != std::string::npos)
        {
            std::string::size_type nexts = rawExpression.find_first_of(";", s);
            std::string exprStr;
            if (nexts != std::string::npos)
            {
                exprStr = std::string(rawExpression, s, nexts-s);
                nexts += 1;
            }
            else
                exprStr = std::string(rawExpression,s, std::string::npos);

            // remove offending chars from exprStr (spaces)
            std::string newExprStr;
            for (int i = 0; i < exprStr.size(); i++)
            {
                if (exprStr[i] != ' ')
                    newExprStr += exprStr[i];
            }

            std::string::size_type t = newExprStr.find_first_of(':');

            Expression vec;
            vec.SetName(std::string(newExprStr,0,t));
            vec.SetDefinition(std::string(newExprStr,t+1,std::string::npos));
            vec.SetType(Expression::VectorMeshVar);
            md->AddExpression(&vec);

            s = nexts;
        }
    }
}


// ****************************************************************************
//  Method: avtPixie3DFileFormat::GetMesh
//
//  Purpose: Create the given mesh
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtPixie3DFileFormat::GetMesh(int ts, int domain, const char *meshname)
{
    if (meshes.find(meshname) == meshes.end())
        return NULL;

    auto mi = meshes[meshname];

    if (mi.meshType == AVT_CURVILINEAR_MESH)
    {
        vtkStructuredGrid *sgrid  = vtkStructuredGrid::New();
        vtkPoints *points = vtkPoints::New();
        sgrid->SetPoints(points);
        points->Delete();

        int nPts = std::accumulate(mi.currDims.begin(), mi.currDims.end(), 1, std::multiplies<int>());
        int ndims = mi.currDims.size();

        vector<vector<double>> vals(ndims);
        int dims[3] = {1,1,1};
        for (int i = 0; i < ndims; i++)
        {
            adios2::Variable<double> v = io.InquireVariable<double>(mi.currCoords[i]);
            v.SetStepSelection({ts, 1});
            reader.Get(v, vals[i], adios2::Mode::Sync);
            dims[i] = mi.currDims[i];
        }

        sgrid->SetDimensions(dims[0], dims[1], dims[2]);
        points->SetNumberOfPoints(nPts);

        float *pts = (float *)points->GetVoidPointer(0);
        double *coord0 = &vals[0][0];
        double *coord1 = &vals[1][0];
        double *coord2 = (ndims == 3 ? &vals[2][0] : NULL);

        float *tmp = pts;
        for (int k = 0; k < dims[2]; k++)
            for (int j = 0; j < dims[1]; j++)
                for (int i = 0; i < dims[0]; i++)
                {
                    *tmp++ = (float) *coord0++;
                    *tmp++ = (float) *coord1++;
                    *tmp++ = (coord2 ? *coord2++ : 0.0f);
                }

        return sgrid;
    }
    else if (mi.meshType == AVT_RECTILINEAR_MESH)
    {
        vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
        vtkFloatArray *coords[3];

        int ndims = mi.currDims.size();
        int dims[3] = {1,1,1};

        for (int i = 0; i < 3; i++)
        {
            if (i < ndims)
                dims[i] = mi.currDims[i]+1; //zone centered.

            coords[i] = vtkFloatArray::New();
            int d = dims[i];
            coords[i]->SetNumberOfTuples(d);

            //add one because it's zone centered.
            float m = 0.0, M = (float)dims[i];
            float c = m, dc = (M-m)/(float)dims[i];

            float *data = (float *)coords[i]->GetVoidPointer(0);
            for (int j = 0; j < dims[i]; j++, c+=dc)
                *data++ = c;
        }

        grid->SetDimensions(dims[0], dims[1], dims[2]);
        grid->SetXCoordinates(coords[0]);
        grid->SetYCoordinates(coords[1]);
        grid->SetZCoordinates(coords[2]);

        coords[0]->Delete();
        coords[1]->Delete();
        coords[2]->Delete();
        return grid;
    }

    debug5<<"Unknown mesh type: "<<mi.meshType<<endl;
    return NULL;
}

// ****************************************************************************
//  Method: avtPixie3DFileFormat::GetVar
//
//  Purpose: Create the given scalar variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtPixie3DFileFormat::GetVar(int ts, int domain, const char *varname)
{
    string varname2 = RemoveLeadingSlash(varname);
    if (variables.find(varname) == variables.end() || varMesh.find(varname2) == varMesh.end())
        return NULL;

    auto var = variables[varname];
    string varType = var["Type"];

    string meshName = varMesh[varname2];
    meshInfo &mi = meshes[meshName];

    int ndims = mi.currDims.size();
    vector<size_t> s = {0,0,0}, c = {1,1,1};
    if (ndims == 2)
    {
        c[0] = 1;
        c[1] = mi.currDims[0];
        c[2] = mi.currDims[1];
    }
    else
    {
        c[0] = mi.currDims[0];
        c[1] = mi.currDims[1];
        c[2] = mi.currDims[2];
    }


    if (varType == "double")
    {
        adios2::Variable<double> v = io.InquireVariable<double>(varname);

        v.SetStepSelection({ts, 1});
        v.SetSelection(adios2::Box<adios2::Dims>(s, c));

        vector<double> buff;
        reader.Get(v, buff, adios2::Mode::Sync);

        int nVals = buff.size();

        vtkDoubleArray *arr = vtkDoubleArray::New();
        arr->SetNumberOfComponents(1);
        arr->SetNumberOfTuples(nVals);

        for (int i = 0; i < nVals; i++)
            arr->SetTuple1(i, (float)buff[i]);
        return arr;
    }
    else
        debug5<<"Unknown variable type."<<endl;

    return NULL;
}


// ****************************************************************************
//  Method: avtPixie3DFileFormat::GetVectorVar
//
//  Purpose: Create the given vector variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:49:29 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtPixie3DFileFormat::GetVectorVar(int ts, int domain, const char *varname)
{
    return NULL;
}
