// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtGEOSFileReader.C
// ****************************************************************************

#include <avtGEOSFileReader.h>


#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <DBOptionsAttributes.h>
#include <Expression.h>
#include <FileFunctions.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

using     std::map;
using     std::set;
using     std::string;
using     std::vector;


// ****************************************************************************
//  GEOSInternal
// ****************************************************************************
namespace GEOSInternal
{
    string fileName;
    string baseDir;
    string blockName;
    string          blockExtension;
    vector<string>  groupNames;
    vector<string>  blockFiles;
    vector<int>     blockFileGroupAssociation;
    string          errorMessage;
    size_t          blockCount;
    vector<vector <string>> varsPerGroup;

    void ClearVarsPerGroup()
    {
        for (size_t i = 0; i < varsPerGroup.size(); ++i)
            varsPerGroup[i].clear();
        varsPerGroup.clear();
    }

    void CleanUp()
    {
        blockName.clear();
        groupNames.clear();
        blockFiles.clear();
        blockFileGroupAssociation.clear();
        errorMessage.clear();
        ClearVarsPerGroup();
    }


    void SetFileName(const std::string &fn)
    {
        CleanUp();
        fileName = fn;
        baseDir = FileFunctions::Dirname(fileName);
    }


    void ParseDSParent(vtkXMLDataElement *parent, string &groupNameBase)
    {
        blockCount += parent->GetNumberOfNestedElements(); 
        for (int j = 0; j < parent->GetNumberOfNestedElements(); ++j)
        {
            vtkXMLDataElement *dsNode = parent->GetNestedElement(j);
            if (string(dsNode->GetName()) == "DataSet")
            {
                string bfn(dsNode->GetAttribute("file"));
                blockFiles.push_back(baseDir + VISIT_SLASH_STRING + bfn);
                blockFileGroupAssociation.push_back(int(groupNames.size()));
                if(blockExtension.empty())
                {
                    size_t pos = bfn.find_last_of('.');
                    if (pos == string::npos)
                        blockExtension = "none";
                    else 
                        blockExtension = bfn.substr(pos+1);
                }
            }
        }
        groupNames.push_back(groupNameBase);
    }

    void ParseRegion(vtkXMLDataElement *el, string &groupNameBase)
    {
        string blockName(el->GetAttribute("name")); 
        string region = groupNameBase + "/" + blockName;
        for (int j = 0; j < el->GetNumberOfNestedElements(); ++j)
        {
            if(string(el->GetNestedElement(j)->GetName()) == "DataSet")
            {
                ParseDSParent(el, region);
                break;
            }
            else
            {
                ParseRegion(el->GetNestedElement(j), region);
            }
        }
    }

    void ParseElement(vtkXMLDataElement *el, string &groupNameBase)
    {
        if (string(el->GetName()) == "Block")
        {
            string blockName(el->GetAttribute("name")); 
            if(blockName.find("Region") != string::npos)
            {
                ParseRegion(el, groupNameBase);
            }
            else
            {
                if(!groupNameBase.empty())
                    groupNameBase += string("/");
                groupNameBase += blockName;
                for (int j = 0; j < el->GetNumberOfNestedElements(); ++j)
                    ParseElement(el->GetNestedElement(j), groupNameBase);
            }
        }
    }

    bool Parse()
    {
        vtkNew<vtkXMLDataParser> parser;
        parser->SetFileName(fileName.c_str());
        if (!parser->Parse())
        {
            errorMessage = "Error parsing file: " + fileName;
            return false;
        }

        vtkXMLDataElement *root = parser->GetRootElement();
        if (!root)
        {
            errorMessage = "Error retrieving RootElement after parsing file.";
            return false;
        }
        string rootName(root->GetName());
        if (rootName != "VTKFile")
        {
            errorMessage = fileName + "does not contain 'VTKFile' tag.";
            return false;
        }

        string dsType = root->GetAttribute("type");
        if (dsType != "vtkMultiBlockDataSet")
        {
            errorMessage = string("VTKFile type: ") +
                          dsType + string(" currently not supported.");
            return false;
        }

        vtkXMLDataElement *mbdsNode = root->GetNestedElement(0);
        if (mbdsNode->GetName() != dsType)
        {
            errorMessage = "First nested element doesn't match " + dsType;
            return false;
        }
        blockCount = 0;
        bool goodParse  = true;
        for (int i = 0; i < mbdsNode->GetNumberOfNestedElements(); ++i)
        {
            string groupNameBase("");
            ParseElement(mbdsNode->GetNestedElement(i), groupNameBase);
        }
        if(blockCount != blockFiles.size())
        {
            goodParse = false;
            errorMessage=string("Error processing for File names, expected: ")
                + std::to_string(blockCount) + string(" but found: ")
                + std::to_string(blockFiles.size());
        }
        return goodParse;
    }

    string & GetBlockFileName(const int blockNum )
    {
        blockName = baseDir + VISIT_SLASH_STRING +
                    blockFiles[blockNum];
        return blockName;
    }

    int GetGroupForBlock(const int blockNum )
    {
        return blockFileGroupAssociation[blockNum];
    }

    int GetNumberOfBlocks()
    {
        return int(blockCount);
    }

    int GetNumberOfGroups()
    {
        return int(groupNames.size());
    }

    bool VarBelongsToBlock(const int blockNum, const string & var)
    {
        int groupForThisBlock = GEOSInternal::GetGroupForBlock(blockNum);
        vector<string> vpg = varsPerGroup[groupForThisBlock];
        auto it = std::find(vpg.begin(), vpg.end(), var);
        return (it != vpg.end());
    }
}

// ****************************************************************************
//  
// ****************************************************************************

// ****************************************************************************
//  Method: avtGEOSFileReader constructor
//
//  Programmer: biagas2 -- generated by xml2avt
//  Creation:   Wed Dec 4 14:55:21 PST 2024
//
// ****************************************************************************

avtGEOSFileReader::avtGEOSFileReader(const char *filename, const DBOptionsAttributes *opt)
    : avtVTKFileReader(filename, opt),
      timeSteps(),
      timeStepFileNames(),
      rootFile(filename)
{
    currentTS = -1;    
    haveReadRoot = false;
}

// ****************************************************************************
//  Method: avtGEOSFileReader destructor
//
//  Programmer: biagas2 -- generated by xml2avt
//  Creation:   Wed Dec 4 14:55:21 PST 2024
//
// ****************************************************************************

avtGEOSFileReader::~avtGEOSFileReader()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtGEOSFileReader::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: biagas2 -- generated by xml2avt
//  Creation:   Wed Dec 4 14:55:21 PST 2024
//
// ****************************************************************************

int
avtGEOSFileReader::GetNTimesteps(void)
{
    if(!haveReadRoot)
        ReadRootFile();

    return int(timeSteps.size());
}

// ****************************************************************************
//  Method: avtGEOSFileReader::GetTimes
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: biagas2 -- generated by xml2avt
//  Creation:   Wed Dec 4 14:55:21 PST 2024
//
// ****************************************************************************

void
avtGEOSFileReader::GetTimes(std::vector<double> &times)
{
    if(!haveReadRoot)
        ReadRootFile();

    times = timeSteps;
}

// ****************************************************************************
//  Method: avtGEOSFileReader::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: biagas2 -- generated by xml2avt
//  Creation:   Wed Dec 4 14:55:21 PST 2024
//
// ****************************************************************************

void
avtGEOSFileReader::FreeUpResources(void)
{
    //GEOSInternal::CleanUp();
    //avtVTKMultiGroupFileReader::FreeUpResources();
}

void
avtGEOSFileReader::RegisterVariableList(const char* primary,
                       const std::vector<CharStrRef> &)
{
    primaryVar=primary;
}

// ****************************************************************************
//  Method: avtGEOSFileReader::ReadRootFile
//
//  Purpose:
//    Read the root file (.pvd), parsing for timestates and filenames.
//
//  Programmer: Kathleen Biagas 
//  Creation:   Thu Dec 5, 2024
//
//  Modifications:
//
// ****************************************************************************

void
avtGEOSFileReader::ReadRootFile()
{
    vtkNew<vtkXMLDataParser> pvdParser;
    baseDir = FileFunctions::Dirname(rootFile) + VISIT_SLASH_STRING;
    string errorMessage;

    pvdParser->SetFileName(rootFile.c_str());

    if (!pvdParser->Parse())
    {
        errorMessage = "Error parsing file: " + rootFile;
        EXCEPTION2(InvalidFilesException, rootFile, errorMessage);
    }

    vtkXMLDataElement *root = pvdParser->GetRootElement();
    if (!root)
    {
        errorMessage = "Error retrieving RootElement after parsing file.";
        EXCEPTION2(InvalidFilesException, rootFile, errorMessage);
    }
    string rootName(root->GetName());
    if (rootName != "VTKFile")
    {
        errorMessage = rootFile + "does not contain 'VTKFile' tag.";
        EXCEPTION2(InvalidFilesException, rootFile, errorMessage);
    }

    string fType = root->GetAttribute("type");
    if (fType != "Collection")
    {
        errorMessage = string("VTKFile type: ") +
                      fType + string(" currently not supported.");
        EXCEPTION2(InvalidFilesException, rootFile, errorMessage);
    }

    vtkXMLDataElement *collectionNode = root->GetNestedElement(0);
    if (collectionNode->GetName() != fType)
    {
        errorMessage = "First nested element doesn't match " + fType;
        EXCEPTION2(InvalidFilesException, rootFile, errorMessage);
    }

    for (int i = 0; i < collectionNode->GetNumberOfNestedElements(); ++i)
    {
        vtkXMLDataElement *el = collectionNode->GetNestedElement(i);
        if (string(el->GetName()) == "DataSet")
        {
            if (el->GetAttribute("file") == 0)
            {
                debug3 << "DataSet element " << i << " does not have "
                       << "`file` element, ignoring." << endl;
                continue;
            }
            if (el->GetAttribute("timestep") == 0)
            {
                debug3 << "DataSet element " << i << " does not have "
                       << "`timestep` element, ignoring." << endl;
                continue;
            }
            timeStepFileNames.push_back(el->GetAttribute("file"));
            double t;
            el->GetScalarAttribute("timestep", t);
            timeSteps.push_back(t);
        }
    }
    if(timeSteps.empty())
    {
        errorMessage = string("Error retrieving file and/or timestep")
                   + string(" attributes from ") + string(rootFile);
        EXCEPTION2(InvalidFilesException, rootFile, errorMessage);
    }
    haveReadRoot = true;
}



// ****************************************************************************
//  Method: avtGEOSFileReader::ReadTimeStepFile
//
//  Purpose:
//      Read the meta file for given time step. 
//
//  Arguments:
//      ts      The time step
//
//  Output:
//
//  Programmer: Kathleen Biagas 
//  Creation:   Thu Dec 5, 2024
//
//  Modifications:
//
// ****************************************************************************

void
avtGEOSFileReader::ReadTimeStepFile(int ts)
{
    if(currentTS == ts)
        return;
    
    string dataFile = baseDir + timeStepFileNames[ts];
    GEOSInternal::SetFileName(dataFile);

    if (!GEOSInternal::Parse())
    {
        string em = GEOSInternal::errorMessage;
        EXCEPTION2(InvalidFilesException, dataFile, em);
        return;
    }

    // vars defined in avtVTKMultiGroupFileReader
    groupNames = GEOSInternal::groupNames;
    ngroups = int(groupNames.size());
    groupIds = GEOSInternal::blockFileGroupAssociation;
    pieceFileNames = GEOSInternal::blockFiles;
    pieceExtension = GEOSInternal::blockExtension;

    // vars defined in avtVTKMultiBlockFileReader
    nblocks = int(groupIds.size());
    if (!pieceDatasets.empty())
    {
        for (size_t i = 0; i < pieceDatasets.size(); i++)
        {
            if (pieceDatasets[i] != NULL)
                pieceDatasets[i]->Delete();
        }
        pieceDatasets.clear();
    }
    pieceDatasets.resize(nblocks);
    for(int i = 0; i < nblocks; ++i)
    {
        pieceDatasets[i] = nullptr;
    }

    if(ngroups > 1)
    {
        // need to find the first block for each group and read that,
        // in case vars are defined in one but not the others
        // Also need to check for duplicate var names in node/zone
        // and append 'nodal' or 'zonal' to distinguish.
        // Need to populate varsPerGroup.
        vector <vector <string> > vpg;
        vpg.resize(ngroups);
        int gid = groupIds[0];
        vector<int> dsToRead;
        dsToRead.push_back(0);
        for(size_t b = 1; b < groupIds.size(); ++b)
        {
            if(groupIds[b] != gid)
            {
                gid = groupIds[b];
                dsToRead.push_back(b);
            }
        }

        set<string> varsZonal;
        set<string> varsNodal;
        for (size_t i = 0; i < dsToRead.size(); ++i)
        {
            int piece = dsToRead[i];
            if (pieceDatasets[piece] == nullptr)
                ReadInDataset(piece);
              
            vtkPointData *pd = pieceDatasets[piece]->GetPointData();
            for (int j = 0; j < pd->GetNumberOfArrays(); ++j)
            {
                vtkDataArray *arr = pd->GetArray(j);
                string varName = arr->GetName();
                vpg[i].push_back(varName);
            }
            vtkCellData *cd = pieceDatasets[piece]->GetCellData();
            for (int j = 0; j < cd->GetNumberOfArrays(); ++j)
            {
                vtkDataArray *arr = cd->GetArray(j);
                string varName = arr->GetName();
                vpg[i].push_back(varName);
            }
        }
        GEOSInternal::varsPerGroup = vpg;
    }

    currentTS = ts;
}

void
avtGEOSFileReader::ReadInFile(const int d)
{
    groupNames = GEOSInternal::groupNames;
    ngroups = int(groupNames.size());
    groupIds = GEOSInternal::blockFileGroupAssociation;
    nblocks = int(groupIds.size());
    pieceFileNames = GEOSInternal::blockFiles;
    pieceExtension = GEOSInternal::blockExtension;
    if (!pieceDatasets.empty())
    {
        for (size_t i = 0; i < pieceDatasets.size(); i++)
        {
            if (pieceDatasets[i] != NULL)
                pieceDatasets[i]->Delete();
        }
        pieceDatasets.clear();
    }
    pieceDatasets.resize(nblocks);
    for(int i = 0; i < nblocks; ++i)
    {
        pieceDatasets[i] = nullptr;
    }
    readInDataset = true;
}

// ****************************************************************************
//  Method: avtGEOSFileReader::ActivateTimestep
//
//  Purpose:
//      Select the current time step
//
//  Arguments:
//      ts      The new current time step
//
//  Output:
//
//  Programmer: Kathleen Biagas 
//  Creation:   Thu Dec 5, 2024
//
//  Modifications:
//
// ****************************************************************************

void
avtGEOSFileReader::ActivateTimestep(int ts)
{
    if(!haveReadRoot)
    {
        ReadRootFile();
    }
    if(currentTS != ts)
    {
        ReadTimeStepFile(ts);
    }

    currentTS = ts;
}


// ****************************************************************************
//  Method: avtGEOSFileReader::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: biagas2 -- generated by xml2avt
//  Creation:   Wed Dec 4 14:55:21 PST 2024
//
// ****************************************************************************

void
avtGEOSFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    ReadTimeStepFile(timeState); 

    if(!timeSteps.empty())
    {
        md->SetTimes(timeSteps);
        md->SetTimesAreAccurate(true);
        md->SetNumStates(int(timeSteps.size()));
    }
    md->SetMustRepopulateOnStateChange(true);

    if (pieceDatasets[0] == nullptr)
        ReadInDataset(0);

    FillMeshMetaData(md, pieceDatasets[0], "mesh");

    if(ngroups == 1)
    {
        FillVarsMetaData(md, pieceDatasets[0]->GetPointData(), "mesh",
             AVT_NODECENT);
        FillVarsMetaData(md, pieceDatasets[0]->GetCellData(), "mesh",
             AVT_ZONECENT, pieceDatasets[0]->GetFieldData());
    }
    else
    {
        // need to find the first block for each group and read that,
        // in case vars are defined in one but not the others
        // Also need to check for duplicate var names in node/zone
        // and append 'nodal' or 'zonal' to distinguish.
        int gid = groupIds[0];
        vector<int> dsToRead;
        dsToRead.push_back(0);
        for(size_t b = 1; b < groupIds.size(); ++b)
        {
            if(groupIds[b] != gid)
            {
                gid = groupIds[b];
                dsToRead.push_back(b);
            }
        }

        set<string> varsZonal;
        set<string> varsNodal;
        map<string, int> varComponents;
        map<string, int> varTypes;
        for (size_t i = 0; i < dsToRead.size(); ++i)
        {
            int piece = dsToRead[i];
            if (pieceDatasets[piece] == nullptr)
                ReadInDataset(piece);
              
            vtkPointData *pd = pieceDatasets[piece]->GetPointData();
            for (int j = 0; j < pd->GetNumberOfArrays(); ++j)
            {
                vtkDataArray *arr = pd->GetArray(j);
                string varName = arr->GetName();
                varsNodal.insert(varName);
                varComponents[varName] = arr->GetNumberOfComponents();
                varTypes[varName] = arr->GetDataType();
            }
            vtkCellData *cd = pieceDatasets[piece]->GetCellData();
            for (int j = 0; j < cd->GetNumberOfArrays(); ++j)
            {
                vtkDataArray *arr = cd->GetArray(j);
                string varName = arr->GetName();
                varsZonal.insert(varName);
                varComponents[varName] = arr->GetNumberOfComponents();
                varTypes[varName] = arr->GetDataType();
            }
        }
        // check for duplicates (so that they can be differentiated
        // by appending '_nodal' or '_zonal'.
        vector<string> dups;
        std::set_intersection(varsNodal.begin(), varsNodal.end(),
                              varsZonal.begin(), varsZonal.end(),
                              std::back_inserter(dups));

        for(auto iter = varsNodal.begin(); iter != varsNodal.end(); ++iter)
        {
            string vName = string(*iter);
            int nComp = varComponents[vName];
            int dType = varTypes[vName];
            if (std::find(dups.begin(), dups.end(), vName) != dups.end())
            {
                FillSingleVarMetaData(md, "mesh", vName, nComp,
                                      dType, AVT_UNKNOWN_CENT);
            }
            else
            {
                FillSingleVarMetaData(md, "mesh", vName, nComp,
                                      dType, AVT_NODECENT);
            }
        }
        for(auto iter = varsZonal.begin(); iter != varsZonal.end(); ++iter)
        {
            string vName = string(*iter);
            int nComp = varComponents[vName];
            int dType = varTypes[vName];
            // duplicates were already handled above with AVT_UNKWON_CENT.
            if (std::find(dups.begin(), dups.end(), vName) == dups.end())
            {
                FillSingleVarMetaData(md, "mesh", vName, nComp,
                                      dType, AVT_ZONECENT);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtGEOSFileReader::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Kathleen Biagas
//  Creation:   December 12, 2024
//
// ****************************************************************************

vtkDataSet *
avtGEOSFileReader::GetMesh(int domain, const char *meshname)
{
    // if primaryVar (from RegisterVariables)
    // is registered with this domain, then retrieve it
    // otherwise return NULL:

    if(ngroups > 1 && !primaryVar.empty() && 
       primaryVar != meshname && 
       primaryVar != "domains")
    { 
        if(!GEOSInternal::VarBelongsToBlock(domain, primaryVar))
            return nullptr;
    }
    return avtVTKFileReader::GetMesh(domain, meshname);
}


// ****************************************************************************
//  Method: avtGEOSFileReader::GetVar
//
//  Purpose:
//      Gets a variable associated.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Kathleen Biagas
//  Creation:   December 12, 2024
//
// ****************************************************************************

vtkDataArray *
avtGEOSFileReader::GetVar(int domain, const char *varname)
{
    // is this var available for this domain?
    // if not return nullptr;

    if(!primaryVar.empty() && string(varname) == primaryVar &&
       !GEOSInternal::VarBelongsToBlock(domain, primaryVar))
    {
       return nullptr;
    }

    return avtVTKFileReader::GetVar(domain, varname);
}

