// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//   GEOSXParser.C
// ************************************************************************* //

#include <GEOSXParser.h>

#include <FileFunctions.h>

#include <vtkNew.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>


#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

// Parses vtm files of this format:
//
// <?xml version="1.0"?>
// <VTKFile type="vtkMultiBlockDataSet" version="1.0">
//    <vtkMultiBlockDataSet>
//        <Block name="backgroundGrid">
//            <Block name="CellElementRegion">
//                <Block name="backgroundGrid_CellRegion1">
//                    <DataSet name="rank_0" file="000000/0_CellRegion1.vtu" />
//                    <DataSet name="rank_1" file="000000/1_CellRegion1.vtu" />
//                </Block>
//            </Block>
//        </Block>
//        <Block name="particles">
//            <Block name="ParticleRegion">
//                <Block name="particles_ParticleRegion1`">
//                    <DataSet name="rank_0" file="000000/0_ParticleRegion1.vtu" />
//                    <DataSet name="rank_1" file="000000/1_ParticleRegion1.vtu" />
//                </Block>
//                <Block name="particles_ParticleRegion2`">
//                    <DataSet name="rank_0" file="000000/0_ParticleRegion2.vtu" />
//                    <DataSet name="rank_1" file="000000/1_ParticleRegion2.vtu" />
//                </Block>
//            </Block>
//        </Block>
//    </vtkMultiBlockDataSet>
// </VTKFile>
//
//  VisIt will present
//     mesh: backgroundGrid
//         group: CellRegion1
//             blocks: CellRegion1_rank_0, CellRegion1_rank_1
//     mesh: particles
//         group: ParticleRegion1
//             blocks: ParticleRegion1_rank_0, ParticleRegion1_rank_1
//         group: ParticleRegion2
//             blocks: ParticleRegion2_rank_0, ParticleRegion2_rank_1
//
//


namespace GEOSXParserInternal
{
    set<string> meshNames;
    // files associated with each mesh
    map<string, vector<string>> meshFiles;
    // groups names associated with each mesh
    map<string, vector<string>> meshGroups;
    // groupIds corresponding to each block for each mesh
    map<string, vector<int>> meshGroupIds;
    // block names associated with each mesh
    map<string, vector<string>> meshBlocks;
    // nodal var names associated with each mesh
    map<string, set<string>> meshVarsNodal;
    // zonal var names associated with each mesh
    map<string, set<string>> meshVarsZonal;
    // vars that occur in both nodal and zonal lists
    map<string, vector<string>> meshVarsDuplicate;

    map<string, vector<int>> firstBlockPerGroup;

    map<pair<string, int>, int> dsIndex;

    string errorMessage;

    size_t numDatasets;

    void
    CleanUp()
    {
        meshNames.clear();
        meshFiles.clear();
        meshGroups.clear();
        meshGroupIds.clear();
        meshBlocks.clear();
        meshVarsNodal.clear();
        meshVarsZonal.clear();
        meshVarsDuplicate.clear();
        firstBlockPerGroup.clear();
        dsIndex.clear();
    }

    bool
    FinalizeInfo()
    {
        if(meshNames.empty() || meshFiles.empty() || meshBlocks.empty())
            return false;

        numDatasets = 0;
        map<string, vector<string>>::iterator it;
        int count = 0;
        for(it = meshFiles.begin(); it != meshFiles.end(); ++it)
        {
            numDatasets += it->second.size();
            for (size_t j = 0; j < it->second.size(); ++j)
                dsIndex[pair<string,int>(it->first, int(j))] = count++;
        }
        return true;
    }

    // Walks through the nested elements,
    // retrieving necessary information.
    bool
    GetInfo(vtkXMLDataElement*mbdsNode, const string &baseDir)
    {
        // the top-level nested elements will be the meshes
        for (int i = 0; i < mbdsNode->GetNumberOfNestedElements(); ++i)
        {
            vtkXMLDataElement *meshEl = mbdsNode->GetNestedElement(i);
            string elType(meshEl->GetName());
            if(elType != "Block")
            {
#ifdef DEBUG_PARSER
                cerr << "error processing mesh block " << i << endl;
#endif
                continue;
            }
            string meshName(meshEl->GetAttribute("name"));
            meshNames.insert(meshName);
            int blockIndex=0;
            // 2nd and 3rd level nested elements will be combined to form groups
            for(int j = 0; j < meshEl->GetNumberOfNestedElements(); ++j)
            {
                vtkXMLDataElement *groupEl1 = meshEl->GetNestedElement(j);
                string groupElType(groupEl1->GetName());
                if(groupElType != "Block")
                {
#ifdef DEBUG_PARSER
                    cerr << "error processing first group block  " << j << endl;
#endif
                    continue;
                }
                for (int k = 0; k < groupEl1->GetNumberOfNestedElements(); ++k)
                {
                    vtkXMLDataElement *groupEl2 = groupEl1->GetNestedElement(k);
                    groupElType = groupEl2->GetName();
                    if(groupElType != "Block")
                    {
#ifdef DEBUG_PARSER
                        cerr << "error procssing second group block " << k << endl;
#endif
                        continue;
                    }
                    string gn2(groupEl2->GetAttribute("name"));
                    string groupName(gn2.substr(meshName.size()+1, string::npos));
                    meshGroups[meshName].push_back(groupName);
                    firstBlockPerGroup[meshName].push_back(blockIndex);
                    int nBlocks = groupEl2->GetNumberOfNestedElements();

                    for (int m = 0; m < nBlocks; ++m)
                    {
                        string blockBase = groupName + "_";
                        meshGroupIds[meshName].push_back(meshGroups[meshName].size()-1);
                        //now we are down to the blocks, finally!
                        vtkXMLDataElement *blockEl = groupEl2->GetNestedElement(m);
                        string blockElType(blockEl->GetName());
                        if(blockElType != "DataSet")
                        {
#ifdef DEBUG_PARSER
                            cerr << "error procssing block level, expecting DataSet, got " << blockElType << "(" << blockEl->GetAttribute("name") << ")" << endl;
#endif
                            continue;
                        }
                        string blockName(blockBase);
                        if(blockEl->GetAttribute("name"))
                        {
                            blockName += blockEl->GetAttribute("name");
                        }
                        else // for completeness, shouldn't happen
                        {
                            blockName += std::to_string(blockIndex);
                        }
                        blockIndex++;
                        string fileName(blockEl->GetAttribute("file"));
                        meshBlocks[meshName].push_back(blockName);
                        meshFiles[meshName].push_back(fileName);
                    } // looping over 4th level nested elements: blocks, files
                } // looping over 3rd level nested elements (groups)
            } // looping over 2nd level nested elements (groups)
        } // looping over top level nested elements (meshes)

        return true; // which cerr conditions above should lead to failure?
    } // GetInfo
}

// ****************************************************************************
//  Method: GEOSXParser constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
// ****************************************************************************

GEOSXParser::GEOSXParser() : fileName(), baseDir(), blockExtension(),
  errorMessage()
{
}


// ****************************************************************************
//  Method: GEOSXParser destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
// ****************************************************************************

GEOSXParser::~GEOSXParser()
{
    GEOSXParserInternal::CleanUp();
}


// ****************************************************************************
//  Method: GEOSXParser::SetFileName
//
//  Purpose:
//    Set's the file that will be parsed.
//
//  Arguments:
//    fn        The file name.
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
// ****************************************************************************

void
GEOSXParser::SetFileName(const string &fn)
{
    fileName = fn;
    baseDir = FileFunctions::Dirname(fileName);
}


// ****************************************************************************
//  Method: GEOSXParser::Parse
//
//  Purpose:
//    Parses the 'vtm' file for DataSet file names, and other information
//    that VisIt might want like mesh and block names.
//
//  Note:
//     A lot of error checking here, perhaps more than is truly needed.
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
// ****************************************************************************

bool
GEOSXParser::Parse()
{
    GEOSXParserInternal::CleanUp();
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

    bool soFarSoGood = GEOSXParserInternal::GetInfo(mbdsNode, baseDir);
    if(soFarSoGood)
        soFarSoGood = GEOSXParserInternal::FinalizeInfo();

    if(!soFarSoGood)
        errorMessage = GEOSXParserInternal::errorMessage;
    return soFarSoGood;
}

// ****************************************************************************
//  Method: GEOSXParser::GetNumberOfDataSets
//
//  Return:
//      The number of datasets as read from the meta file.
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
// ****************************************************************************

int
GEOSXParser::GetNumberOfDataSets()
{
    return int(GEOSXParserInternal::numDatasets);
}


// ****************************************************************************
//  Method: GEOSXParser::GetMeshNames
//
//  Return:
//      The mesh names as read from the meta file.
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
// ****************************************************************************

set<string> &
GEOSXParser::GetMeshNames()
{
    return GEOSXParserInternal::meshNames;
}


// ****************************************************************************
//  Method: GEOSXParser::GetBlockFileName
//
//  Return:
//      The file name for the block with given mesh name at given index.
//
//  Arguments:
//      meshName   The mesh name.
//      blockNum   The block index.
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
// ****************************************************************************

string &
GEOSXParser::GetBlockFileName(const string &meshName, const int blockNum )
{
    blockName = baseDir + VISIT_SLASH_STRING +
                GEOSXParserInternal::meshFiles[meshName][blockNum];
    return blockName;
}


// ****************************************************************************
//  Method: GEOSXParser::GetNumberOfBlocks
//
//  Return:  The number of blocks.
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
// ****************************************************************************

int
GEOSXParser::GetNumberOfBlocks(const string & meshName)
{
    return int(GEOSXParserInternal::meshFiles[meshName].size());
}


// ****************************************************************************
//  Method: GEOSXParser::GetDataSetIndex
//
//  Return:  The index for the dataset matching meshName and blockNum
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
// ****************************************************************************

int
GEOSXParser::GetDataSetIndex(const string &meshName, const int blockNum)
{
    return GEOSXParserInternal::dsIndex[pair<string,int>(meshName, blockNum)];
}


vector<string> &
GEOSXParser::GetMeshGroupNames(const string &meshName)
{
    return GEOSXParserInternal::meshGroups[meshName];
}

vector<int> &
GEOSXParser::GetMeshGroupIds(const string &meshName)
{
    return GEOSXParserInternal::meshGroupIds[meshName];
}

vector<string> &
GEOSXParser::GetMeshBlockNames(const string &meshName)
{
    return GEOSXParserInternal::meshBlocks[meshName];
}

vector<int> &
GEOSXParser::GetMeshBlockPerGroup(const string &meshName)
{
    return GEOSXParserInternal::firstBlockPerGroup[meshName];
}

