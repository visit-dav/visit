// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vector>
#include <string>
#include <cgnslib.h>
#include <cgns_io.h>


// ****************************************************************************
// Function: PrintDataType
//
// Purpose:
//   Prints the name of the data type to debug4.
//
// Arguments:
//   dt : The data type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 09:33:43 PDT 2005
//
// Modifications:
//
// ****************************************************************************

static void
PrintDataType(DataType_t dt)
{
    switch(dt)
    {
    case DataTypeNull:
        debug4 << "DataTypeNull";
        break;
    case DataTypeUserDefined:
        debug4 << "DataTypeUserDefined";
        break;
    case Integer:
        debug4 << "Integer";
        break;
    case RealSingle:
        debug4 << "RealSingle";
        break;
    case RealDouble:
        debug4 << "RealDouble";
        break;
    case Character:
        debug4 << "Character";
        break;
    case LongInteger:
        debug4 << "LongInteger";
        break;
    }
}

// ****************************************************************************
// Function: PrintElementType
//
// Purpose:
//   Prints the name of the element type to debug4.
//
// Arguments:
//   et : The element type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 09:33:43 PDT 2005
//
// Modifications:
//
// ****************************************************************************

static void
PrintElementType(ElementType_t et)
{
    if(et == ElementTypeNull) { debug4 << "ElementTypeNull"; }
    else if(et == ElementTypeUserDefined) debug4 << "ElementTypeUserDefined";
    else if(et == NODE) { debug4 << "NODE"; }
    else if(et == BAR_2) { debug4 << "BAR_2"; }
    else if(et == BAR_3) { debug4 << "BAR_3"; }
    else if(et == TRI_3) { debug4 << "TRI_3"; }
    else if(et == TRI_6) { debug4 << "TRI_6"; }
    else if(et == QUAD_4) { debug4 << "QUAD_4"; }
    else if(et == QUAD_8) { debug4 << "QUAD_8"; }
    else if(et == QUAD_9) { debug4 << "QUAD_9"; }
    else if(et == TETRA_4) { debug4 << "TETRA_4"; }
    else if(et == TETRA_10) { debug4 << "TETRA_10"; }
    else if(et == PYRA_5) { debug4 << "PYRA_5"; }
    else if(et == PYRA_14) { debug4 << "PYRA_14"; }
    else if(et == PENTA_6) { debug4 << "PENTA_6"; }
    else if(et == PENTA_15) { debug4 << "PENTA_15"; }
    else if(et == PENTA_18) { debug4 << "PENTA_18"; }
    else if(et == HEXA_8) { debug4 << "HEXA_8"; }
    else if(et == HEXA_20) { debug4 << "HEXA_20"; }
    else if(et == HEXA_27) { debug4 << "HEXA_27"; }
    else if(et == MIXED) { debug4 << "MIXED"; }
    else if(et == NGON_n) { debug4 << "NGON_n"; }
}


// ****************************************************************************
// Function: getChildrenIds
//
// Purpose:
//   Retrieve the children ids from a CGNS node.
//
//   NOTE: while this can be used for general purposes, it is currently
//   here for debugging.
//
// Arguments:
//     cgioNum       The cgio number.
//     parentId      The node id to retrieve children from.
//     childrenIds   A vector to store the children ids in.
//
// Programmer: Alister Maguire
// Creation:   Tue Mar  2 08:01:12 PST 2021
//
// Modifications:
//
// ****************************************************************************

int getChildrenIds(int cgioNum, double parentId,
    std::vector<double> &childrenIds)
{
    int numChildren;
    cgio_number_children(cgioNum, parentId, &numChildren);

    childrenIds.resize(numChildren);
    double* tmpChildrenIds = new double[numChildren];

    int numChildrenIds;
    cgio_children_ids(cgioNum, parentId, 1, numChildren,
        &numChildrenIds, tmpChildrenIds);

    if (numChildrenIds != numChildren)
    {
        delete [] tmpChildrenIds;
        debug1 << "Mismatch in number of children and child Ids read" << endl;
        return 1;
    }

    for (int cIdx = 0; cIdx < numChildren; cIdx++)
    {
        childrenIds[cIdx] = tmpChildrenIds[cIdx];
    }

    delete [] tmpChildrenIds;
    return 0;
}


// ****************************************************************************
// Function: getBaseIds
//
// Purpose:
//   Retrieve the base Ids from a root Id of a dataset.
//
//   NOTE: while this can be used for general purposes, it is currently
//   here for debugging.
//
// Arguments:
//     cgioNum       The cgio number.
//     rootId        The root Id of a dataset.
//     baseIds       A vector to store the base Ids in.
//
// Programmer: Alister Maguire
// Creation:   Tue Mar  2 08:01:12 PST 2021
//
// Modifications:
//
// ****************************************************************************

int getBaseIds(int cgioNum, double rootId, std::vector<double>& baseIds)
{
    char nodeLabel[33];
    std::size_t numBases = 0;

    baseIds.clear();
    getChildrenIds(cgioNum, rootId, baseIds);

    if (baseIds.size() < 1)
    {
        debug1 << "Error: Not enough nodes under the root "
            << "description file." << endl;
        return 1;
    }

    for (int c = 0; c < baseIds.size(); c++)
    {
        if (cgio_get_label(cgioNum, baseIds[c], nodeLabel) != CG_OK)
        {
            return 1;
        }
        if (strcmp(nodeLabel, "CGNSBase_t") == 0)
        {
            if (numBases < c)
            {
                baseIds[numBases] = baseIds[c];
            }
            numBases++;
        }
        else
        {
            cgio_release_id(cgioNum, baseIds[c]);
        }
    }

    baseIds.resize(numBases);

    if (numBases < 1)
    {
        debug1 << "Error: Not enough bases in the file." << endl;
        return 1;
    }

    return 0;
}


// ****************************************************************************
// Function: showChildren
//
// Purpose:
//   Show all children of a given CGNS node.
//
//   NOTE: this is for aid in debugging.
//
// Arguments:
//     cgioNum       The cgio number.
//     parentId      The Id of the node to show children from.
//     childrenIds   A vector to store the children Ids in.
//     prepend       An optional string to prepend to the beginning of
//                   the output information.
//
// Programmer: Alister Maguire
// Creation:   Tue Mar  2 08:01:12 PST 2021
//
// Modifications:
//
// ****************************************************************************

int showChildren(int cgioNum, double parentId,
    std::vector<double> &childrenIds, std::string prepend="")
{
    int numChildren;
    cgio_number_children(cgioNum, parentId, &numChildren);

    childrenIds.resize(numChildren);
    double* tmpChildrenIds = new double[numChildren];

    int numChildrenIds;
    cgio_children_ids(cgioNum, parentId, 1, numChildren,
        &numChildrenIds, tmpChildrenIds);

    if (numChildrenIds != numChildren)
    {
        delete [] tmpChildrenIds;
        debug1 << "Mismatch in number of children and child Ids read." << endl;
        return 1;
    }

    //
    // Recursively iterate through all children, displaying their
    // information in the debug stream as we go.
    //
    for (int cIdx = 0; cIdx < numChildren; cIdx++)
    {
        childrenIds[cIdx] = tmpChildrenIds[cIdx];

        int nameLength;
        char names[33];
        cgio_children_names(cgioNum, parentId, cIdx+1, 1,
          33, &nameLength, names);

        char childLabel[33];
        cgio_get_label(cgioNum, tmpChildrenIds[cIdx], childLabel);

        debug1 << prepend << cIdx << " NAME: " <<
            string(names) << " (" << string(childLabel) << ")" << endl;

        std::vector<double> bar;
        showChildren(cgioNum, tmpChildrenIds[cIdx], bar, prepend + "    ");
    }

    delete [] tmpChildrenIds;
    return 0;
}
