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
//                            avtTSTTFileFormat.C                           //
// ************************************************************************* //

#include "TSTTB.hh"
#include "TSTTM.hh"

#include <snprintf.h>

#include <avtiTapsFileFormat.h>

#include <map>
#include <string>
#include <vector>

#include <vtkCellType.h>
#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <BJHash.h>
#include <DebugStream.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

using     std::map;
using     std::string;
using     std::vector;

static char* entTypes[] = {
    "vertex", "edge", "face", "region", "all types"};

static char* entTopologies[] = {
    "point", "line segment", "polygon", "triangle",
    "quadrilateral", "polyhedron", "tetrahedron",
    "pyramid", "prism", "hexahedron", "septahedron",
    "all topologies"};

static char *tsttDataTypeNames[] = {
    "integer", "double", "ehandle", "bytes" };

static int TSTTEntityTopologyToVTKZoneType(int ttype)
{
    switch (ttype)
    {
    case TSTTM::EntityTopology_POINT:         return VTK_VERTEX;
    case TSTTM::EntityTopology_LINE_SEGMENT:  return VTK_LINE;
    case TSTTM::EntityTopology_POLYGON:       return VTK_POLYGON;
    case TSTTM::EntityTopology_TRIANGLE:      return VTK_TRIANGLE;
    case TSTTM::EntityTopology_QUADRILATERAL: return VTK_QUAD;
    case TSTTM::EntityTopology_TETRAHEDRON:   return VTK_TETRA;
    case TSTTM::EntityTopology_HEXAHEDRON:    return VTK_HEXAHEDRON;
    case TSTTM::EntityTopology_PRISM:         return VTK_WEDGE;
    case TSTTM::EntityTopology_PYRAMID:       return VTK_PYRAMID;
    }
    return -1;
}

#if 0
static int NextDomainId(bool init = false)
{
    static int n;
    if (init) n = 0;
    n++;
    return n-1;
}

static void
ClassifyEntitySet(TSTTM::Mesh mesh, EntitySetHandle esh, int level, int memId,
    map<EntitySetHandle, VisItEntitySetInfo_t> &esMap)
{
    TSTTB::EntSet mesh_eset = aMesh;
    TSTTB::EntTag mesh_ent = aMesh;
    TSTTB::SetTag mesh_stag = aMesh;

    // compute an indentation for debugging
    std::string ident;
    for (int i = 0; i < level; i++)
        ident += "    ";

    VisItEntitySetInfo_t esInfo;

    if (level == 1)
    {
        if (mesh_eset.getNumEntSets(esh, 0) > 0)
            esInfo.domainId = NextDomainId();
    }

    sidl::array<TagHandle> tags;
    int tags_size;
    mesh_stag.getAllEntSetTags(esh, tags, tags_size);
    if (tags_size)
    {
        debug5 << ident << "  tags = " << tags_size << endl;
        debug5 << ident << "  {" << endl;
        debug5 << ident << "                name           type     size     value(s)..." << endl;
        for (int t = 0; t < tags_size; t++)
        {
            TSTTB::TagValueType tagType = mesh_stag.getTagType(tags[t]);
            int tagSize = mesh_stag.getTagSizeValues(tags[t]);
            string tagName = mesh_stag.getTagName(tags[t]);

            char lineBuf[256]; // for debugging output
            if (tagType == TSTTB::TagValueType_INTEGER)
            {
                int theVal = mesh_stag.getEntSetIntData(esh, tags[t]);
                if (debug5_real)
                {
                    sprintf(lineBuf, "% 16s     % 8s     %03d     %d", 
                        tagName.c_str(), tsttDataTypeNames[tagType], tagSize, theVal);
                }
            }
            else if (tagType == TSTTB::TagValueType_DOUBLE)
            {
                double theVal = mesh_stag.getEntSetDblData(esh, tags[t]);
                if (debug5_real)
                {
                    sprintf(lineBuf, "% 16s     % 8s     %03d     %f", 
                        tagName.c_str(), tsttDataTypeNames[tagType], tagSize, theVal);
                }
            }
            else if (tagType == TSTTB::TagValueType_ENTITY_HANDLE)
            {
                EntitySetHandle theVal = mesh_stag.getEntSetEHData(esh, tags[t]);
                if (debug5_real)
                {
                    sprintf(lineBuf, "% 16s     % 8s     %03d     %X", 
                        tagName.c_str(), tsttDataTypeNames[tagType], tagSize, theVal);
                }
            }
            else if (tagType == TSTTB::TagValueType_BYTES)
            {
                sidl::array<char> theVal;
                int theValSize;
                mesh_stag.getEntSetData(esh, tags[t], theVal, theValSize);
                std::string valBuf;
                for (int k = 0; k < theValSize; k++)
                {
                    char tmpChars[32];
                    if (theValSize > 4)
                        sprintf(tmpChars, "%c", theVal[k]);
                    else
                        sprintf(tmpChars, "%hhX", theVal[k]);
                    valBuf += std::string(tmpChars);
                }

                // classify group by hash on category value
                if (tagName == "CATEGORY")
                {
                    esInfo.groupId = BJHash::Hash(valBuf.c_str(), valBuf.size(), 0);
                }

                if (debug5_real)
                {
                    sprintf(lineBuf, "% 16s     % 8s     %03d     %s", 
                        tagName.c_str(), tsttDataTypeNames[tagType], tagSize, valBuf.c_str());
                }
            }
            else
            {
                if (debug5_real)
                {
                    sprintf(lineBuf, "% 16s     % 8s     %03d     %s", 
                        tagName.c_str(), "UNKNOWN", tagSize, "UNKNOWN");
                }
            }
            debug5 << ident << "       " << lineBuf << endl;
        }
        debug5 << ident << "  }" << endl;
    }
    else
    {
        debug5 << ident << "  tags = NONE" << endl;
    }
}

static void
ProcessEntitySetHierarchy(TSTTM::Mesh aMesh, int level, int esId, EntitySetHandle esh,
    map<EntitySetHandle,VisItEntitySetInfo_t> &esMap)
{
    TSTTB::EntSet mesh_eset = aMesh;
    TSTTB::EntTag mesh_ent = aMesh; 
    TSTTB::SetTag mesh_stag = aMesh;

    // compute an indentation for debugging
    std::string ident;
    for (int i = 0; i < level; i++)
        ident += "    ";

    debug5 << ident << "For Entity Set<" << level << "," << esId << ">:" << endl;
    debug5 << ident << "{" << endl;

    sidl::array<EntitySetHandle> sets;
    int sets_size;
    mesh_eset.getEntSets(esh, 1, sets, sets_size);
    if (sets_size > 0)
    {
        debug5 << ident << "  sub-entity sets = " << sets_size << endl;
        debug5 << ident << "  {" << endl;

        for (int i = 0; i < sets_size; i++)
        {
            // recurse first, then classify
            ProcessEntitySetHierarchy(aMesh, level+1, i, sets[i], esMap);
            ClassifyEntitySet(aMesh, level, i, sets[i], esMap);
        }

        debug5 << ident << "  }" << endl;
    }
    else
    {
        debug5 << ident << "    entity sets = NONE" << endl;
    }
}
#endif

// ****************************************************************************
//  Method: avtTSTT constructor
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
// ****************************************************************************

avtTSTTFileFormat::avtTSTTFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    vmeshFileName = filename;
    geomDim = topoDim = 0;
    vertEnts = edgeEnts = faceEnts = regnEnts = 0;
    numVerts = numEdges = numFaces = numRegns = 0;
    haveMixedElementMesh = false;
}


// ****************************************************************************
//  Method: avtTSTTFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
// ****************************************************************************

void
avtTSTTFileFormat::FreeUpResources(void)
{
    if (vertEnts != 0)
        delete vertEnts;
    vertEnts = 0;
    if (edgeEnts != 0)
        delete edgeEnts;
    edgeEnts = 0;
    if (faceEnts != 0)
        delete faceEnts;
    faceEnts = 0;
    if (regnEnts != 0)
        delete regnEnts;
    regnEnts = 0;
}


// ****************************************************************************
//  Method: avtTSTTFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
//  Modifications:
//
//    Mark C. Miller, Thu Mar 22 09:37:55 PDT 2007
//    Added code to detect variable centering and populate md with variable
//    info
//
// ****************************************************************************

void
avtTSTTFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i, j;

    tsttMesh = TSTTM::Factory::newMesh("");
    rootSet = tsttMesh.getRootSet();

    // remove extension from filename
    string tmpFileName = vmeshFileName;
    int q = vmeshFileName.size();
    if (vmeshFileName[q-6] == '.' && vmeshFileName[q-5] == 'v' &&
        vmeshFileName[q-4] == 'm' && vmeshFileName[q-3] == 'e' &&
        vmeshFileName[q-2] == 's' && vmeshFileName[q-1] == 'h')
        tmpFileName = string(vmeshFileName, 0, q-6);
    else if (vmeshFileName[q-6] == '.' && vmeshFileName[q-5] == 'c' &&
             vmeshFileName[q-4] == 'u' && vmeshFileName[q-3] == 'b')
        tmpFileName = string(vmeshFileName, 0, q-4);

    // ok, try loading the mesh.
    try
    {
        tsttMesh.load(rootSet, tmpFileName);

        // determine spatial and topological dimensions of mesh
        geomDim = tsttMesh.getGeometricDim();
        topoDim = -1;
        numVerts = tsttMesh.getNumOfType(rootSet, TSTTM::EntityType_VERTEX);
        if (numVerts > 0)
            topoDim = 0;
        numEdges = tsttMesh.getNumOfType(rootSet, TSTTM::EntityType_EDGE);
        if (numEdges > 0)
            topoDim = 1;
        numFaces = tsttMesh.getNumOfType(rootSet, TSTTM::EntityType_FACE);
        if (numFaces > 0)
            topoDim = 2;
        numRegns = tsttMesh.getNumOfType(rootSet, TSTTM::EntityType_REGION);
        if (numRegns > 0)
            topoDim = 3;

        //
        // Decide if we've got a 'mixed element' mesh
        //
        if ((numEdges > 0 && numFaces > 0) ||
            (numEdges > 0 && numRegns > 0) ||
            (numFaces > 0 && numRegns > 0))
            haveMixedElementMesh = true;

        //
        // If we have a mixed element mesh, serve up the mesh to VisIt
        // such that each element type gets its own domain. Otherwise,
        // just serve up a single block mesh.
        //
        if (haveMixedElementMesh)
        {
            vector<string> blockPieceNames;
            if (numEdges)
            {
                domToEntType[blockPieceNames.size()] = TSTTM::EntityType_EDGE;
                blockPieceNames.push_back("EDGE");
            }
            if (numFaces)
            {
                domToEntType[blockPieceNames.size()] = TSTTM::EntityType_FACE;
                blockPieceNames.push_back("FACE");
            }
            if (numRegns)
            {
                domToEntType[blockPieceNames.size()] = TSTTM::EntityType_REGION;
                blockPieceNames.push_back("REGION");
            }
            avtMeshMetaData *mmd = new avtMeshMetaData("mesh", blockPieceNames.size(),
                0, 0, 0, geomDim, topoDim, AVT_UNSTRUCTURED_MESH);
            mmd->blockTitle = string("Entity Types");
            mmd->blockPieceName = string("etype");
            mmd->blockNames = blockPieceNames;
            md->Add(mmd);
        }
        else
        {
            if (numEdges > 0)
                domToEntType[0] = TSTTM::EntityType_EDGE;
            else if (numFaces > 0)
                domToEntType[0] = TSTTM::EntityType_FACE;
            else if (numRegns > 0)
                domToEntType[0] = TSTTM::EntityType_REGION;
            AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, 0, 1, 0,
                geomDim, topoDim);
        }

        //
        // Because its so convenient, add the zonetype expression as a
        // variable on the mesh
        //
        Expression expr;
        expr.SetName("zonetype");
        expr.SetDefinition("zonetype(mesh)");
        expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&expr);

        // create variants of the tsttMesh object handle ??
        TSTTB::EntSet tsttMeshes = tsttMesh;
        TSTTB::SetTag tsttMesht  = tsttMesh;
        TSTTM::Entity tsttMeshe  = tsttMesh;
        TSTTB::EntTag tsttMeshet = tsttMesh;
        TSTTB::ArrTag tsttMeshat = tsttMesh;

        //
        // How to determine centering WITHOUT invoking problem sized
        // data work; e.g. an array of entity handles? We pick the
        // first entity in each type class (0d, 1d, 2d, and 3d) and
        // find all the tags defined on those entities and then
        // register them as node or zone centered variables.
        //
        int entTypeClass;
        for (entTypeClass = 0; entTypeClass < 4; entTypeClass++)
        {
            // initialize an entity iterator and get the first entity
            void *entIt;
            tsttMeshe.initEntIter(rootSet, (TSTTM::EntityType) entTypeClass,
                TSTTM::EntityTopology_ALL_TOPOLOGIES, entIt);
            void *oneEntity;
            tsttMeshe.getNextEntIter(entIt, oneEntity);
            tsttMeshe.endEntIter(entIt);

            // get all the tags defined on this one entity
            sidl::array<void*> tagsOnOneEntity;
            int tagsOnOneEntity_size;
            tsttMeshet.getAllTags(oneEntity, tagsOnOneEntity, tagsOnOneEntity_size);

            // make a vector of the found handles and copy it
            // to the saved list of primitive tag handles
            vector<void*> tmpTagHandles;
            for (int kk = 0; kk < tagsOnOneEntity_size; kk++)
                tmpTagHandles.push_back(tagsOnOneEntity[kk]);
            primitiveTagHandles[entTypeClass] = tmpTagHandles;
        }

        for (entTypeClass = 0; entTypeClass < 4; entTypeClass++)
        {
            avtCentering centering = entTypeClass == 0 ? AVT_NODECENT : AVT_ZONECENT;

            vector<void*> tagHandles = primitiveTagHandles[entTypeClass];

            for (int tagIdx = 0; tagIdx < tagHandles.size(); tagIdx++)
            {
                void *theTag = tagHandles[tagIdx]; 
                string tagName = tsttMesht.getTagName(theTag);
                int valSize = tsttMesht.getTagSizeValues(theTag);
                avtVarType var_type = GuessVarTypeFromNumDimsAndComps(geomDim, valSize);

                // TSTT can sometimes define same tag on multiple entities
                bool shouldSkip = false;
                if (entTypeClass > 0)
                {
                    vector<void*> tagHandlesOnVerts = primitiveTagHandles[0];
                    for (int t = 0; t < tagHandlesOnVerts.size(); t++)
                    {
                        string vertTagName = tsttMesht.getTagName(tagHandlesOnVerts[t]);
                        if (vertTagName == tagName)
                        {
                            shouldSkip = true;
                            break;
                        }
                    }
                }
                if (shouldSkip)
                    continue;

                if (var_type == AVT_SCALAR_VAR)
                    AddScalarVarToMetaData(md, tagName, "mesh", centering);
                else if (var_type == AVT_VECTOR_VAR)
                    AddVectorVarToMetaData(md, tagName, "mesh", centering, valSize);
                else if (var_type == AVT_SYMMETRIC_TENSOR_VAR)
                    AddSymmetricTensorVarToMetaData(md, tagName, "mesh", centering, valSize);
                else if (var_type == AVT_TENSOR_VAR)
                    AddTensorVarToMetaData(md, tagName, "mesh", centering, valSize);
                else
                {
                    vector<string> memberNames;
                    for (int c = 0; c < valSize; c++)
                    {
                        char tmpName[64];
                        SNPRINTF(tmpName, sizeof(tmpName), "%s_%03d", tagName.c_str(), c);
                        memberNames.push_back(tmpName);
                    }
                    AddArrayVarToMetaData(md, tagName, memberNames, "mesh", centering);
                }
            }
        }
    }
    catch (TSTTB::Error TErr)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Encountered TSTT error (%d) \"%s\""
            "\nUnable to open file!", TErr.getDescription().c_str(),
            TErr.getErrorType());
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
        return;
    }
}


// ****************************************************************************
//  Method: avtTSTTFileFormat::GetMesh
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
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
// ****************************************************************************

vtkDataSet *
avtTSTTFileFormat::GetMesh(int domain, const char *meshname)
{
    int i, j;

    try
    {
        //
        // The domain number indicates the entity type we're looking for
        //
        TSTTM::EntityType entType = domToEntType[domain]; 
        int numEnts = tsttMesh.getNumOfType(rootSet, entType);
        if (numEnts == 0)
            return 0;

        int coords2Size, mapSize;
        sidl::array<double> coords2;
        sidl::array<int> inEntSetMap;
        TSTTM::StorageOrder storageOrder2 = TSTTM::StorageOrder_UNDETERMINED;
        tsttMesh.getAllVtxCoords(rootSet, coords2, coords2Size,
            inEntSetMap, mapSize, storageOrder2);

        int vertCount = mapSize;
        if (vertCount == 0)
            return 0;

        //
        // If its a 1D or 2D mesh, the coords could be 3-tuples where
        // the 'extra' values are always zero or they could be reduced
        // dimensioned tuples. The TSTT interface doesn't specify.
        //
        int tupleSize = coords2Size / vertCount;

        //
        // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
        //
        vtkPoints *points  = vtkPoints::New();
        points->SetNumberOfPoints(vertCount);
        float *pts = (float *) points->GetVoidPointer(0);
        for (i = 0; i < vertCount; i++)
        {
            if (storageOrder2 == TSTTM::StorageOrder_INTERLEAVED)
            {
                for (j = 0; j < tupleSize; j++)
                    pts[i*3+j] = (float) coords2.get(i*tupleSize+j);
                for (j = tupleSize; j < 3; j++)
                    pts[i*3+j] = 0.0;
            }
            else if (storageOrder2 == TSTTM::StorageOrder_BLOCKED)
            {
                for (j = 0; j < tupleSize; j++)
                    pts[i*3+j] = (float) coords2.get(j*vertCount+i);
                for (j = tupleSize; j < 3; j++)
                    pts[i*3+j] = 0.0;
            }
        }

        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New(); 
        ugrid->SetPoints(points);
        points->Delete();

        sidl::array<int> offset; int offsetSize;
        sidl::array<int> index; int indexSize;
        sidl::array<TSTTM::EntityTopology> topos; int32_t toposSize;
        tsttMesh.getVtxCoordIndex(rootSet, entType,
            TSTTM::EntityTopology_ALL_TOPOLOGIES, TSTTM::EntityType_VERTEX, 
            offset, offsetSize, index, indexSize, topos, toposSize);

        for (int off = 0; off < offsetSize; off++)
        {
            int vtkZoneType = TSTTEntityTopologyToVTKZoneType(topos[off]);
            vtkIdType vertIds[256];
            int jj = 0;
            for (int idx = offset[off];
                 idx < ((off+1) < offsetSize ? offset[off+1] : indexSize); idx++)
                vertIds[jj++] = index[idx];
            ugrid->InsertNextCell(vtkZoneType, jj, vertIds);
        }

        return ugrid;

    }
    catch (TSTTB::Error TErr)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Encountered TSTT error (%d) \"%s\""
            "\nUnable to open file!", TErr.getDescription().c_str(),
            TErr.getErrorType());
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
        return 0;
    }
}


// ****************************************************************************
//  Method: avtTSTTFileFormat::GetVar
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
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtTSTTFileFormat::GetVar(int domain, const char *varname)
{
    TSTTB::SetTag tsttMesht  = tsttMesh;
    TSTTB::ArrTag tsttMeshat = tsttMesh;
    vtkDataArray *result = 0;

    //
    // Scan through known primitive tags looking for one with the given name
    //
    int entType;
    void *tagToGet = 0;
    for (entType = 0; entType < 4; entType++)
    {
        vector<void*> tagHandles = primitiveTagHandles[entType];
        for (int tagIdx = 0; tagIdx < tagHandles.size(); tagIdx++)
        {
            void *theTag = tagHandles[tagIdx]; 
            string tagName = tsttMesht.getTagName(theTag);
            int nmsz = tagName.size();
            if (string(tagName, 0, nmsz) == string(varname, 0, nmsz))
            {
                tagToGet = theTag;
                goto tagFound;
            }
        }
    }
    EXCEPTION1(InvalidVariableException, varname);

tagFound:
    try
    {
        //
        // Get the array of entities if we haven't already
        //
        sidl::array<void*> *entHandles_p = 0;
        int ents_size;
        switch (entType)
        {
            case TSTTM::EntityType_VERTEX: entHandles_p = vertEnts; ents_size = numVerts; break;
            case TSTTM::EntityType_EDGE:   entHandles_p = edgeEnts; ents_size = numFaces; break;
            case TSTTM::EntityType_FACE:   entHandles_p = faceEnts; ents_size = numEdges; break;
            case TSTTM::EntityType_REGION: entHandles_p = regnEnts; ents_size = numRegns; break;
        }
        if (entHandles_p == 0)
        {
            entHandles_p = new sidl::array<void*>;
            tsttMesh.getEntities(rootSet, (TSTTM::EntityType) entType,
                TSTTM::EntityTopology_ALL_TOPOLOGIES, *entHandles_p, ents_size);
            int expectedSize;
            switch (entType)
            {
                case TSTTM::EntityType_VERTEX:
                    vertEnts = entHandles_p; expectedSize = numVerts; break;
                case TSTTM::EntityType_EDGE:
                    edgeEnts = entHandles_p; expectedSize = numEdges; break;
                case TSTTM::EntityType_FACE:
                    faceEnts = entHandles_p; expectedSize = numFaces; break;
                case TSTTM::EntityType_REGION:
                    regnEnts = entHandles_p; expectedSize = numRegns; break;
            }
            if (expectedSize != ents_size)
            {
                char tmpMsg[256];
                SNPRINTF(tmpMsg, sizeof(tmpMsg), "for variable \"%s\" "
                    "getEntities() returned %d entities but VisIt expected %d",
                    varname, ents_size, expectedSize);
                EXCEPTION1(InvalidVariableException, tmpMsg);
            }
        }

        //
        // Now, get the tag data and put it in an appropriate vtk data array 
        //
        int tagSizeValues = tsttMesht.getTagSizeValues(tagToGet);
        int tagTypeId = tsttMesht.getTagType(tagToGet);
        int arraySize;
        switch (tagTypeId)
        {
            case TSTTB::TagValueType_INTEGER:
            {
                sidl::array<int> intArray;
                tsttMeshat.getIntArrData(*entHandles_p, ents_size, tagToGet, intArray, arraySize); 
                if (arraySize != ents_size * tagSizeValues)
                {
                    char tmpMsg[256];
                    SNPRINTF(tmpMsg, sizeof(tmpMsg), "getIntArrData() returned %d values "
                        " but VisIt expected %d * %d", arraySize, ents_size, tagSizeValues);
                    EXCEPTION1(InvalidVariableException, tmpMsg);
                }
                vtkIntArray *ia = vtkIntArray::New();
                ia->SetNumberOfComponents(tagSizeValues);
                ia->SetNumberOfTuples(ents_size);
                for (int i = 0; i < arraySize; i++)
                    ia->SetValue(i, intArray[i]);
                result = ia;
                break;
            }
            case TSTTB::TagValueType_DOUBLE:
            {
                sidl::array<double> dblArray;
                tsttMeshat.getDblArrData(*entHandles_p, ents_size, tagToGet, dblArray, arraySize); 
                if (arraySize != ents_size * tagSizeValues)
                {
                    char tmpMsg[256];
                    SNPRINTF(tmpMsg, sizeof(tmpMsg), "getDblArrData() returned %d values "
                        " but VisIt expected %d * %d", arraySize, ents_size, tagSizeValues);
                    EXCEPTION1(InvalidVariableException, tmpMsg);
                }
                vtkDoubleArray *da = vtkDoubleArray::New();
                da->SetNumberOfComponents(tagSizeValues);
                da->SetNumberOfTuples(ents_size);
                for (int i = 0; i < arraySize; i++)
                    da->SetValue(i, dblArray[i]);
                result = da;
                break;
            }
            case TSTTB::TagValueType_ENTITY_HANDLE:
            case TSTTB::TagValueType_BYTES:
            {
                char tmpMsg[256];
                SNPRINTF(tmpMsg, sizeof(tmpMsg), "Unable to handle data type of \"%s\"",
                    tsttDataTypeNames[tagTypeId]);
                EXCEPTION1(InvalidVariableException, tmpMsg);
            }
        }
    }
    catch (TSTTB::Error TErr)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Encountered TSTT error (%d) \"%s\""
            "\nUnable to open file!", TErr.getDescription().c_str(),
            TErr.getErrorType());
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
        return 0;
    }

    return result;
}


// ****************************************************************************
//  Method: avtTSTTFileFormat::GetVectorVar
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
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtTSTTFileFormat::GetVectorVar(int domain, const char *varname)
{
    return GetVar(domain, varname);
}
