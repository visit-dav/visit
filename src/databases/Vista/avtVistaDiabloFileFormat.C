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
//                         avtVistaDiabloFileFormat.C                        //
// ************************************************************************* //

#include <stdarg.h>
#include <stdio.h>

#include <snprintf.h>

#include <map>
#include <string>
#include <vector>

#include <StringHelpers.h>
#include <avtVistaDiabloFileFormat.h>
#include <avtSTMDFileFormatInterface.h>
#include <avtFileFormatInterface.h>

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>

#include <BadIndexException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <UnexpectedValueException.h>
#include <Utility.h>

#include <visit-config.h>

using std::map;
using std::string;
using std::vector;

static char tempStr[1024];

// ****************************************************************************
//  Function:  HasAllComponents 
//
//  Purpose:   Check that all components names passed in have count equal
//             to pieceCount in fieldMap
//
//  Programmer:  Mark C. Miller 
//  Creation:    February 17, 2004 
//
// ****************************************************************************
static bool
HasAllComponents(vector<string>& fieldList, bool eraseAllIfFound,
   const char *compName, ...)
{
    int i;
    bool retval = true;
    const char *name;
    va_list ap;

    va_start(ap, compName);
    name = compName;
    vector<int> entriesToErase;
    while (name != 0 && retval)
    {
        bool foundIt = false;
        for (i = 0; i < fieldList.size(); i++)
        {
            const char *baseName = StringHelpers::Basename(fieldList[i].c_str());
            if (strcmp(baseName, name) == 0)
            {
                foundIt = true;
                entriesToErase.push_back(i);
                break;
            }
        }
        if (!foundIt)
            retval = false;
        name = va_arg(ap, const char *);
    }
    va_end(ap);

    // erase entries if it was requested
    if (retval && eraseAllIfFound)
    {
        vector<string> fieldListTmp;
        for (i = 0; i < fieldList.size(); i++)
        {
            int j;
            bool dontCopy = false;
            for (j = 0; j < entriesToErase.size(); j++)
            {
                if (entriesToErase[j] == i)
                {
                    dontCopy = true;
                    break;
                }
            }
            if (!dontCopy)
                fieldListTmp.push_back(fieldList[i]);
        }
        fieldList = fieldListTmp;
    }

    return retval;
}

// ****************************************************************************
// Method: avtVistaDiabloFileFormat::CreateInterface
//
// Purpose:
//   Creates a file format interface that works for avtVistaDiabloFileFormat files.
//
// Arguments:
//   vista     : The Vista file object
//   filename  : the name of the file
//
// Returns:    A file format interface object that lets us access the files
//             in the filename list.
//
// Programmer: Mark C. Miller
// Creation:   July 14, 2004 
//
// ****************************************************************************

avtFileFormatInterface *
avtVistaDiabloFileFormat::CreateInterface(avtVistaFileFormat *vff,
    const char *const *list, int nList)
{
    avtFileFormatInterface *inter = 0;

    if (nList && list)
    {
        avtSTMDFileFormat **ffl = new avtSTMDFileFormat*[nList];
        ffl[0] = new avtVistaDiabloFileFormat(list[0], vff);
        for (int i = 1; i < nList; i++)
        {
            ffl[i] = new avtVistaDiabloFileFormat(list[i]);
        }
        inter = new avtSTMDFileFormatInterface(ffl, nList);
    }

    return inter;
}

// ****************************************************************************
//  Method: avtVistaDiabloFileFormat constructor
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
//  Modifications:
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Replaced use of basename/dirname with StringHelper functions
//
// ****************************************************************************

avtVistaDiabloFileFormat::avtVistaDiabloFileFormat(const char *filename)
    : avtVistaFileFormat(filename, FTYPE_DIABLO)
{
    spatialDim = 0;
    numPieces = 0;
    pieceNodes = 0;
}

// ****************************************************************************
//  Method: avtVistaDiabloFileFormat constructor
//
//  Programmer: Mark C. Miller
//  Creation:   July 15, 2004 
//
// ****************************************************************************

avtVistaDiabloFileFormat::avtVistaDiabloFileFormat(const char *filename,
   avtVistaFileFormat *vff) : avtVistaFileFormat(filename, vff)
{
    spatialDim = 0;
    numPieces = 0;
    pieceNodes = 0;
}

// ****************************************************************************
//  Destructor:  avtVistaDiabloFileFormat::~avtVistaDiabloFileFormat
//
//  Programmer:  Mark C. Miller 
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
// ****************************************************************************
avtVistaDiabloFileFormat::~avtVistaDiabloFileFormat()
{
    FreeUpResources();

    if (pieceNodes)
        delete [] pieceNodes;

    avtVistaFileFormat::FreeUpResources();
}

// ****************************************************************************
//  Method: avtVistaDiabloFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
// ****************************************************************************

void
avtVistaDiabloFileFormat::FreeUpResources(void)
{
}

// ****************************************************************************
//  Method:  GetFileNameForRead
//
//  Purpose:   Determines file name for a read based on domain number
//             variables
//
//  Programmer:  Mark C. Miller
//  Creation:    October 27, 2004 
//
// ****************************************************************************
void
avtVistaDiabloFileFormat::GetFileNameForRead(int dom, char *fileName, int size)
{
    int filePart = chunkToFileMap[dom];
    if (filePart == MASTER_FILE_INDEX)
        strncpy(fileName, masterFileName.c_str(), size);
    else
    {
        const char *tmp1 = masterFileName.c_str();
        int i = strlen(tmp1) - 1;
        while (i && tmp1[i] != '_')
            i--;
        string tmp2 = string(masterFileName,0,i);
        SNPRINTF(fileName, size, "%s_%05d.SILO", tmp2.c_str(), filePart);
    }
}


// ****************************************************************************
//  Method: avtVistaDiabloFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Notes on Vista File Format:
//     pre-defined named entities that can be in a view
//        + Fields (group)
//        + Indexset (integer dataset)
//        + Relations (group)
//            - elemToNode (zonelist)
//        + Attr (group)
//        + Params (group)
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
//  Modifications:
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Added support for 2D meshes
//
//    Mark C. Miller, Tue Oct 26 10:28:36 PDT 2004
//    Filtered out BCs from relations. Added node-centered fields
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed memory leaks
//
//    Kathleen Bonnell, Mon May 23 16:55:35 PDT 2005 
//    Fixed memory leaks.
//
// ****************************************************************************

void
avtVistaDiabloFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i, j, n;

    const Node *top = vTree->GetTop();

    // Find top-level pieces in the vista tree by finding all relation nodes
    numPieces = 0;
    pieceNodes = 0;
    vTree->FindNodes(top, "/%R.*", &pieceNodes, &numPieces, BottomUp);
    if (numPieces == 0)
    {
        cerr << "WARNING!!! Didn't find any domains, nothing to visualize" << endl;
        return;
    }

    //
    // Strip out anything that looks like a boundary condition relation.
    // They should really be fields, but thats not how Diablo is writing them
    //
    vector<Node*> tmpPieceNodes;
    for (i = 0; i < numPieces; i++)
    {
        if (StringHelpers::FindRE(pieceNodes[i]->parent->text,"BC") < 0)
            tmpPieceNodes.push_back(pieceNodes[i]);
    }
    if (tmpPieceNodes.size() < numPieces)
    {
        numPieces = tmpPieceNodes.size();
        for (i = 0; i < tmpPieceNodes.size(); i++)
            pieceNodes[i] = tmpPieceNodes[i];
    }

    // Adjust pieces upwards by two levels
    string otherViewName = pieceNodes[0]->otherView;
    for (i = 0; i < numPieces; i++)
        pieceNodes[i] = pieceNodes[i]->parent->parent;

    // use the string grouping utility to see how many "groups" we got
    vector<string> pieceNames;
    vector<vector<string> > pieceGroups;
    vector<string> groupNames;
    for (i = 0; i < numPieces; i++)
        pieceNames.push_back(pieceNodes[i]->text);
    StringHelpers::GroupStrings(pieceNames, pieceGroups, groupNames);

    // if we get more than one candidate group that look like domains, pick one
    if (pieceGroups.size() > 1)
    {
        cerr << "WARNING!!! Found more than one candidate group of domains" << endl;
        cerr << "           Using group named \"" << groupNames[0] << "\"" << endl;

        // rebuild the list of pieceNodes using only the group we've chosen
        delete [] pieceNodes;
        numPieces = 0;
        pieceNodes = 0;
        sprintf(tempStr,"%s/%%R.*", groupNames[0].c_str());
        vTree->FindNodes(top, tempStr, &pieceNodes, &numPieces, BottomUp);
        if (numPieces != pieceGroups[0].size())
        {
            cerr << "WARNING!!! Unable to find domains" << endl;
            return;
        }
        // Adjust pieces upwards by one level
        for (i = 0; i < numPieces; i++)
            pieceNodes[i] = pieceNodes[i]->parent;
    }
    pieceNames.clear();
    pieceGroups.clear();
    groupNames.clear();
    
    // determinie block name stuff 
    string blockPieceName;
    string blockTitle;
    vector<string> blockNames;
    string aPieceName = pieceNodes[0]->text;
    string::size_type firstDigit = aPieceName.find_first_of("0123456789");
    if (firstDigit != string::npos)
    {
        string sepChars = "_-.:;=#+@";
        while ((sepChars.find(aPieceName[firstDigit-1]) != string::npos) &&
               (firstDigit-1 != 0))
            firstDigit--;
        blockTitle = string(aPieceName, 0, firstDigit) + "s";
        blockPieceName = string(aPieceName, 0, firstDigit);
        for (i = 0; i < numPieces; i++)
            blockNames.push_back(pieceNodes[i]->text);
    }
    else
    {
       blockTitle = "blocks";
       blockPieceName = "block";
    }

    //
    // For now, just assume 3 spatial dimensions
    //
    spatialDim = 3;

    //
    // Add the GLOBAL mesh object
    //
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = top->child[0]->text;
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->topologicalDimension = spatialDim;
    mesh->spatialDimension = spatialDim;
    cerr << "WARNING!!! Assuming both topological and spatial dimension is " << spatialDim << endl;
    mesh->numBlocks = numPieces;
    mesh->blockTitle = blockTitle;
    mesh->blockPieceName = blockPieceName;
    mesh->blockNames = blockNames;
    md->Add(mesh);

    //
    // Iterate over the pieces, finding fields on them
    //
    for (i = 0; i < numPieces; i++)
    {
        vector<FieldInfo_t> fieldsOnThisPiece;

        // First pass for zone-centered fields, 2nd pass for node-centered
        for (n = 0; n < 2; n++)
        {
            avtCentering centering = n == 0 ? AVT_ZONECENT : AVT_NODECENT;
            const char *viewName = n == 0 ? "material1" : "node_set";

            int numFieldNodes = 0;
            Node **fieldNodes = 0;
        
            const Node *elemView = vTree->GetNodeFromPath(pieceNodes[i], viewName);
            vTree->FindNodes(elemView, "/%F.*", &fieldNodes, &numFieldNodes, BottomUp);

            vector<string> fieldNames;
            vector<vector<string> > fieldGroups;
            groupNames.clear();
            for (j = 0; j < numFieldNodes; j++)
            {
                char *tmpFieldName = vTree->GetPathFromNode(elemView,fieldNodes[j]);
                fieldNames.push_back(tmpFieldName);
                delete [] tmpFieldName;
            }
            delete [] fieldNodes;
            StringHelpers::GroupStringsAsPaths(fieldNames, fieldGroups, groupNames);

            // iterate over the groups adding fields to the list of fields on
            // this piece
            for (j = 0; j < fieldGroups.size(); j++)
            {

                bool eraseEntriesIfFound = true;
                if (HasAllComponents(fieldGroups[j], eraseEntriesIfFound,
                    "CauchyStress_sig11", "CauchyStress_sig12", "CauchyStress_sig22",
                    "CauchyStress_sig23", "CauchyStress_sig31", "CauchyStress_sig33", 0))
                {
                    FieldInfo_t fieldInfo;
                    fieldInfo.visitVarName = string(groupNames[j],1,string::npos);
                    fieldInfo.varType = AVT_SYMMETRIC_TENSOR_VAR;
                    fieldInfo.centering = centering;
                    fieldInfo.compNames.push_back("CauchyStress_sig11");
                    fieldInfo.compNames.push_back("CauchyStress_sig12");
                    fieldInfo.compNames.push_back("CauchyStress_sig22");
                    fieldInfo.compNames.push_back("CauchyStress_sig23");
                    fieldInfo.compNames.push_back("CauchyStress_sig31");
                    fieldInfo.compNames.push_back("CauchyStress_sig33");
                    fieldsOnThisPiece.push_back(fieldInfo);
                }
                if (HasAllComponents(fieldGroups[j], eraseEntriesIfFound,
                    "disp_incx", "disp_incy", "disp_incz", 0))
                {
                    FieldInfo_t fieldInfo;
                    fieldInfo.visitVarName = "disp_inc"; 
                    fieldInfo.varType = AVT_VECTOR_VAR;
                    fieldInfo.centering = centering;
                    fieldInfo.compNames.push_back("disp_incx");
                    fieldInfo.compNames.push_back("disp_incy");
                    fieldInfo.compNames.push_back("disp_incz");
                    fieldsOnThisPiece.push_back(fieldInfo);
                }
                if (HasAllComponents(fieldGroups[j], eraseEntriesIfFound,
                    "disp_np1x", "disp_np1y", "disp_np1z", 0))
                {
                    FieldInfo_t fieldInfo;
                    fieldInfo.visitVarName = "disp_np1"; 
                    fieldInfo.varType = AVT_VECTOR_VAR;
                    fieldInfo.centering = centering;
                    fieldInfo.compNames.push_back("disp_np1x");
                    fieldInfo.compNames.push_back("disp_np1y");
                    fieldInfo.compNames.push_back("disp_np1z");
                    fieldsOnThisPiece.push_back(fieldInfo);
                }
                if (HasAllComponents(fieldGroups[j], eraseEntriesIfFound,
                    "coord_nx", "coord_ny", "coord_nz", 0))
                {
                    FieldInfo_t fieldInfo;
                    fieldInfo.visitVarName = "coord_n"; 
                    fieldInfo.varType = AVT_VECTOR_VAR;
                    fieldInfo.centering = centering;
                    fieldInfo.compNames.push_back("coord_nx");
                    fieldInfo.compNames.push_back("coord_ny");
                    fieldInfo.compNames.push_back("coord_nz");
                    fieldsOnThisPiece.push_back(fieldInfo);
                }
                if (HasAllComponents(fieldGroups[j], eraseEntriesIfFound,
                    "coord_np1x", "coord_np1y", "coord_np1z", 0))
                {
                    FieldInfo_t fieldInfo;
                    fieldInfo.visitVarName = "coord_np1"; 
                    fieldInfo.varType = AVT_VECTOR_VAR;
                    fieldInfo.centering = centering;
                    fieldInfo.compNames.push_back("coord_np1x");
                    fieldInfo.compNames.push_back("coord_np1y");
                    fieldInfo.compNames.push_back("coord_np1z");
                    fieldsOnThisPiece.push_back(fieldInfo);
                }

                for (int k = 0; k < fieldGroups[j].size(); k++)
                {
                    FieldInfo_t fieldInfo;
                    fieldInfo.visitVarName  = string(fieldGroups[j][k],1,string::npos);
                    fieldInfo.varType = AVT_SCALAR_VAR;
                    fieldInfo.centering = centering;
                    fieldInfo.compNames.push_back(string(fieldGroups[j][k],1,string::npos));
                    fieldsOnThisPiece.push_back(fieldInfo);
                }

            }

        }

        fieldInfos.push_back(fieldsOnThisPiece);
    }

    //
    // Add variables defined on every piece
    //
    vector<FieldInfo_t> pieceZeroFields = fieldInfos[0];
    for (i = 0; i  < pieceZeroFields.size(); i++)
    {
        FieldInfo_t zeroField = pieceZeroFields[i];
        bool isDefinedOnAllPieces = true;

        // search for this field in all other pieces
        for (j = 1; j < numPieces; j++)
        {
            vector<FieldInfo_t> thisPieceFields = fieldInfos[j];

            bool isDefinedOnThisPiece = false;
            for (int k = 0; k < thisPieceFields.size(); k++)
            {
                FieldInfo_t pieceField = thisPieceFields[k];

                if (pieceField.visitVarName == zeroField.visitVarName)
                {
                    isDefinedOnThisPiece = true;
                    break;
                }
            }

            if (!isDefinedOnThisPiece)
            {
                isDefinedOnAllPieces = false;
                break;
            }
        }

        if (isDefinedOnAllPieces)
        {
            if (zeroField.varType == AVT_SCALAR_VAR)
                AddScalarVarToMetaData(md, zeroField.visitVarName, mesh->name, zeroField.centering);
            else if (zeroField.varType == AVT_VECTOR_VAR)
                AddVectorVarToMetaData(md, zeroField.visitVarName, mesh->name, zeroField.centering, spatialDim);
            else if (zeroField.varType == AVT_SYMMETRIC_TENSOR_VAR)
                AddSymmetricTensorVarToMetaData(md, zeroField.visitVarName, mesh->name, zeroField.centering, 6);
        }
    }
}

// ****************************************************************************
//  Method: avtVistaDiabloFileFormat::GetMesh
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
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
//  Modifications:
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Added support for 2D meshes
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed memory leaks
//
//    Kathleen Bonnell, Mon May 23 16:55:35 PDT 2005 
//    Fixed memory leaks.
//
// ****************************************************************************

vtkDataSet *
avtVistaDiabloFileFormat::GetMesh(int domain, const char *meshname)
{
    int i;
    const Node *top = vTree->GetTop();

    //
    // Find 'elem' and 'node' Vista nodes beneath this domain's Vista node
    //
    int numElemViews = 0;
    Node **elemViews = 0;
    vTree->FindNodes(pieceNodes[domain], "/%Vmaterial1", &elemViews, &numElemViews, TopDown);
    if ((numElemViews == 0) || (numElemViews > 1))
    {
        EXCEPTION2(UnexpectedValueException, 1, numElemViews);
    }
    int numNodeViews = 0;
    Node **nodeViews = 0;
    vTree->FindNodes(pieceNodes[domain], "/%Vnode_set", &nodeViews, &numNodeViews, TopDown);
    if ((numNodeViews == 0) || (numNodeViews > 1))
    {
        EXCEPTION2(UnexpectedValueException, 1, numNodeViews);
    }

    int numElems = elemViews[0]->len;
    int numNodes = nodeViews[0]->len;

    delete [] elemViews;
    delete [] nodeViews;

    //
    // Figure out which file to read from
    //
    char fileName[1024];
    GetFileNameForRead(domain, fileName, sizeof(fileName));

    //
    // Read coordinate arrays
    //
    double *coords[3] = {0, 0, 0};
    for (i = 0; i < spatialDim; i++)
    {
        const char *piecePath = vTree->GetPathFromNode(top, pieceNodes[domain]);
        char fieldName[32];
        sprintf(fieldName, "coord_np1%c", (char) ('x'+i));
        sprintf(tempStr, "%s/node_set/Fields/%s", piecePath, fieldName);
        delete [] piecePath;

        size_t dSize = 0;
        ReadDataset(fileName, tempStr, 0, &dSize, (void**) &coords[i]);

        if (dSize != numNodes)
        {
            EXCEPTION2(UnexpectedValueException, numNodes, dSize);
        }
    }
    for (i = spatialDim; i < 3; i++)
    {
        int j;
        coords[i] = new double[numNodes];
        for (j = 0; j < numNodes; j++)
            coords[i][j] = 0.0;
    }

    //
    // Read elem to node relation (connectivity)
    //
    int numNodesPerElem = 0;
    int *elemToNode = 0;
    {
        const char *piecePath = vTree->GetPathFromNode(top, pieceNodes[domain]);
        sprintf(tempStr, "%s/material1/Relations/node", piecePath);
        delete [] piecePath;

        size_t dSize = 0;
        ReadDataset(fileName, tempStr, 0, &dSize, (void**) &elemToNode);

        numNodesPerElem = dSize / numElems;

        if ((numNodesPerElem != 4) && (numNodesPerElem != 8))
        {
            EXCEPTION2(UnexpectedValueException, numNodesPerElem, "4 or 8");
        }

        // fix off-by-one error
        for (i = 0; i < dSize; i++)
            elemToNode[i] -= 1;
    }

    //
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    vtkPoints            *points  = vtkPoints::New();
    points->SetNumberOfPoints(numNodes);
    float *pts = (float *) points->GetVoidPointer(0);
    float *tmp = pts;
    const double *coords0 = coords[0];
    const double *coords1 = coords[1];
    const double *coords2 = coords[2];
    for (i = 0 ; i < numNodes; i++)
    {
        *tmp++ = *coords0++;
        *tmp++ = *coords1++;
        *tmp++ = *coords2++;
    }
    delete [] coords[0];
    delete [] coords[1];
    delete [] coords[2];

    //
    // Ok, build the VTK unstructured grid object
    //
    vtkUnstructuredGrid    *ugrid   = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    ugrid->Allocate(numElems * numNodesPerElem);
    for (i = 0; i < numElems; i++)
    {
        int vtkCellType = VTK_HEXAHEDRON;
        if (numNodesPerElem == 4)
           vtkCellType = VTK_QUAD;
        ugrid->InsertNextCell(vtkCellType, numNodesPerElem, &elemToNode[i*numNodesPerElem]);
    }
    points->Delete();
    delete [] elemToNode;
    return ugrid;
}


// ****************************************************************************
//  Method: avtVistaDiabloFileFormat::GetVar
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
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
// ****************************************************************************

vtkDataArray *
avtVistaDiabloFileFormat::GetVar(int domain, const char *varname)
{
    return ReadVar(domain, varname);
}

// ****************************************************************************
//  Method: avtVistaDiabloFileFormat::GetVectorVar
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
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
// ****************************************************************************

vtkDataArray *
avtVistaDiabloFileFormat::GetVectorVar(int domain, const char *varname)
{
    return ReadVar(domain, varname);
}

// ****************************************************************************
//  Method: avtVistaDiabloFileFormat::ReadVar
//
//  Purpose:
//      Reads scalar, vector or tensor data into a vtkFloatArray. 
//
//  Programmer: Mark C. Miller
//  Creation:   March 15, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Added support for 2D meshes
//
//    Mark C. Miller, Tue Oct 26 10:28:36 PDT 2004
//    Added support for node-centered fields. Used ReadDataset function that
//    always returns float data
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed memory leaks
//
// ****************************************************************************

vtkFloatArray *
avtVistaDiabloFileFormat::ReadVar(int domain, const char *visitName)
{
    int i, j;
    const Node *top = vTree->GetTop();

    vector<string> vistaNames;
    bool foundIt = false;
    FieldInfo_t fieldInfo;
    for (i = 0; i < fieldInfos[domain].size(); i++)
    {
        if (fieldInfos[domain][i].visitVarName == visitName)
        {
            fieldInfo = fieldInfos[domain][i];
            foundIt = true;
            break;
        }
    }

    if (!foundIt)
    {
        EXCEPTION1(InvalidVariableException, visitName);
    }

    vistaNames = fieldInfo.compNames;
    int numComponents = vistaNames.size();
    avtCentering centering = fieldInfo.centering;

    //
    // Find 'elem' or 'node' Vista nodes beneath this domain's Vista node
    //
    int numViews = 0;
    Node **views = 0;
    if (centering == AVT_ZONECENT)
        vTree->FindNodes(pieceNodes[domain], "/%Vmaterial1", &views, &numViews, TopDown);
    else
        vTree->FindNodes(pieceNodes[domain], "/%Vnode_set", &views, &numViews, TopDown);
    if ((numViews == 0) || (numViews > 1))
    {
        EXCEPTION2(UnexpectedValueException, 1, numViews);
    }

    int numVals = views[0]->len;

    delete [] views;

    //
    // Figure out which file to read from
    //
    char fileName[1024];
    GetFileNameForRead(domain, fileName, sizeof(fileName));

    //
    // Read all the component's data
    //
    float **compData = new float*[numComponents];
    for (i = 0; i < numComponents; i++)
    {
        size_t dSize;

        const char *piecePath = vTree->GetPathFromNode(top, pieceNodes[domain]);
        if (centering == AVT_ZONECENT)
        {
            if (string(visitName).find_first_of('/') == string::npos)
                sprintf(tempStr, "%s/material1/Fields/%s", piecePath, visitName);
            else
                sprintf(tempStr, "%s/material1/%s/Fields/%s", piecePath, visitName, vistaNames[i].c_str());
        }
        else
        {
            sprintf(tempStr, "%s/node_set/Fields/%s", piecePath, vistaNames[i].c_str());
        }
        delete [] piecePath;

        compData[i] = 0;
        if (ReadDataset(fileName, tempStr, &dSize, &compData[i]))
        {
            if (dSize != numVals)
            {
                EXCEPTION2(UnexpectedValueException, numVals, dSize);
            }
            continue;
        }
        else
        {
            // if we get here, we must not have been able to read
            // the variable as either zone- or node-centered
            EXCEPTION1(InvalidVariableException, visitName);
        }
    }

    //
    // Regardless of what kind of variable we have in Vista,
    // VTK supports only scalar (1 component), vector (3 component)
    // of tensor (9 compoenent) variables. So, do the mapping here.
    //
    avtVarType varType = fieldInfo.varType; 
    if (varType == AVT_UNKNOWN_TYPE)
    {
        EXCEPTION1(InvalidVariableException, visitName);
    }

    int numAllocComponents = 0;
    switch (varType)
    {
        case AVT_SCALAR_VAR: numAllocComponents = 1; break;
        case AVT_VECTOR_VAR: numAllocComponents = 3; break;
        case AVT_SYMMETRIC_TENSOR_VAR:
        case AVT_TENSOR_VAR: numAllocComponents = 9; break;
        default: break;
    }
    if (numAllocComponents == 0)
    {
        EXCEPTION2(UnexpectedValueException, numAllocComponents, ">0");
    }

    vtkFloatArray *var_data = vtkFloatArray::New();
    var_data->SetNumberOfComponents(numAllocComponents);
    var_data->SetNumberOfTuples(numVals);
    float *fbuf = (float*) var_data->GetVoidPointer(0);
    for (i = 0; i < numVals; i++)
    {
        for (j = 0; j < numComponents; j++)
        {
            *fbuf++ = compData[j][i];
        }
        for (j = numComponents; j < numAllocComponents; j++)
            *fbuf++ = 0.0;
    }

    // clean-up
    for (i = 0; i < numComponents; i++)
        delete [] compData[i];
    delete [] compData;

    return var_data;
}
