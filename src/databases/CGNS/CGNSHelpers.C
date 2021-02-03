// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vector>
#include <string>
#include <cgnslib.h>
#include <cgns_io.h>
#include<unordered_map>


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
        //FIXME
        delete[] tmpChildrenIds;
        std::cerr << "Mismatch in number of children and child IDs read" << std::endl;
        return 1;
    }
  
    for (int cIdx = 0; cIdx < numChildren; cIdx++)
    {
        childrenIds[cIdx] = tmpChildrenIds[cIdx];
    }
  
    delete[] tmpChildrenIds;
    return 0;
}


int getBaseIds(int cgioNum, double rootId, std::vector<double>& baseIds)
{
    char nodeLabel[33];
    std::size_t nbases = 0;
    std::size_t nc;

    baseIds.clear();
    if (baseIds.size() < 1)
    {
        std::cerr << "Error: Not enough nodes under the root description file." << std::endl;
        return 1;
    }

    for (nbases = 0, nc = 0; nc < baseIds.size(); nc++)
    {
        if (cgio_get_label(cgioNum, baseIds[nc], nodeLabel) != CG_OK)
        {
            return 1;
        }
        if (strcmp(nodeLabel, "CGNSBase_t") == 0)
        {
            if (nbases < nc)
            {
                baseIds[nbases] = baseIds[nc];
            }
            nbases++;
        }
        else
        {
            cgio_release_id(cgioNum, baseIds[nc]);
        }
    }
    baseIds.resize(nbases);

    if (nbases < 1)
    {
        std::cerr << "Error: Not enough bases in the file." << std::endl;
        return 1;
    }

    return 0;
}


void populateSectionNameToIdMap(int cgioNum, vector<double> &idArray,
    std::unordered_map<std::string, double> &nameToIdMap)
{
    int status = 0;
    for (std::vector<double>::iterator idItr = idArray.begin();
         idItr != idArray.end(); ++idItr)
    {
        //
        // CGNS has a maximum name size of 33. FIXME: double check this.
        //
        char name[33];
        status = cgio_get_name(cgioNum, *idItr, name);
        //FIXME: check status 
        
        char *label = new char[CGIO_MAX_NAME_LENGTH + 1];
        status = cgio_get_label(cgioNum, *idItr, label);

        if (strcmp(label, "Elements_t") == 0)
        {
            nameToIdMap[std::string(name)] = *idItr;
        }

        delete [] label;

        std::vector<double> childrenIds;
        getChildrenIds(cgioNum, *idItr, childrenIds);

        populateSectionNameToIdMap(cgioNum, childrenIds,
            nameToIdMap);
    }
}


std::unordered_map<std::string, double> generateSectionNameToIdMap(
    int cgioNum, double rootId)
{

    std::unordered_map<std::string, double> nameToIdMap;
    vector<double> baseIds;
    getBaseIds(cgioNum, rootId, baseIds);

    return nameToIdMap;    
}


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
        //FIXME
        delete[] tmpChildrenIds;
        std::cerr << "Mismatch in number of children and child IDs read" << std::endl;
        return 1;
    }
  
    for (int cIdx = 0; cIdx < numChildren; cIdx++)
    {
        childrenIds[cIdx] = tmpChildrenIds[cIdx];

        int nameLength;
        char names[33];
        cgio_children_names(cgioNum, parentId, cIdx+1, 1,
          33, &nameLength, names);

        //FIXME: this can't be used for windows?
        char *childLabel = new char[CGIO_MAX_NAME_LENGTH + 1];
        cgio_get_label(cgioNum, tmpChildrenIds[cIdx], childLabel);

        cerr << prepend << cIdx << " NAME: " << string(names) << " (" << string(childLabel) << ")" << endl;
        //fprintf(stderr, "\nID: %f", tmpChildrenIds[cIdx]);

        std::vector<double> bar;
        showChildren(cgioNum, tmpChildrenIds[cIdx], bar, prepend + "    ");
    }
  
    delete[] tmpChildrenIds;
    return 0;
}


//FIXME


//FIXME: read_data_type isn't implemented until version 4.x
//int getSectionConnectivity(const int cgioNum, const double cgioSectionId,
//    const int dim, const cgsize_t* srcStart, const cgsize_t* srcEnd,
//    const cgsize_t* srcStride, const cgsize_t* memStart,
//    const cgsize_t* memEnd, const cgsize_t* memStride,
//    const cgsize_t* memDim, vtkIdType* localElements)
//{
//    const char* connectivityPath = "ElementConnectivity";
//    double cgioElemConnectId;
//    char dataType[3];
//    std::size_t sizeOfCnt = 0;
//
//    cgio_get_node_id(cgioNum, cgioSectionId, connectivityPath, &cgioElemConnectId);
//    cgio_get_data_type(cgioNum, cgioElemConnectId, dataType);
//
//    if (strcmp(dataType, "I4") == 0)
//    { 
//        sizeOfCnt = sizeof(int);
//    }
//    else if (strcmp(dataType, "I8") == 0)
//    { 
//        sizeOfCnt = sizeof(cglong_t);
//    }
//    else
//    { 
//        std::cerr << "ElementConnectivity data_type unknown\n";
//    }
//
//    if (sizeOfCnt == sizeof(vtkIdType))
//    { 
//        if (cgio_read_data_type(cgioNum, cgioElemConnectId, srcStart, srcEnd, srcStride, dataType, dim,
//              memDim, memStart, memEnd, memStride, (void*)localElements) != CG_OK)
//        { 
//            char message[81];
//            cgio_error_message(message);
//            std::cerr << "cgio_read_data_type :" << message;
//            return 1;
//        }
//    }
//    else
//    {
//        // Need to read into temp array to convert data
//        cgsize_t nn = 1;
//        for (int ii = 0; ii < dim; ii++)
//        {
//            nn *= memDim[ii];
//        }
//        if (sizeOfCnt == sizeof(int))
//        {
//            int* data = new int[nn];
//            if (data == 0)
//            {
//                std::cerr << "Allocation failed for temporary connectivity array\n";
//            }
//
//            if (cgio_read_data_type(cgioNum, cgioElemConnectId, srcStart, srcEnd, srcStride, "I4", dim,
//                  memDim, memStart, memEnd, memStride, (void*)data) != CG_OK)
//            {
//                delete[] data;
//                char message[81];
//                cgio_error_message(message);
//                std::cerr << "cgio_read_data_type :" << message;
//                return 1;
//            }
//            for (cgsize_t n = 0; n < nn; n++)
//            {
//                localElements[n] = static_cast<vtkIdType>(data[n]);
//            }
//            delete[] data;
//        }
//        else if (sizeOfCnt == sizeof(cglong_t))
//        {
//            cglong_t* data = new cglong_t[nn];
//            if (data == 0)
//            {
//                std::cerr << "Allocation failed for temporary connectivity array\n";
//                return 1;
//            }
//            if (cgio_read_data_type(cgioNum, cgioElemConnectId, srcStart, srcEnd, srcStride, "I8", dim,
//                  memDim, memStart, memEnd, memStride, (void*)data) != CG_OK)
//            {
//                delete[] data;
//                char message[81];
//                cgio_error_message(message);
//                std::cerr << "cgio_read_data_type :" << message;
//                return 1;
//            }  
//            for (cgsize_t n = 0; n < nn; n++)
//            {
//                localElements[n] = static_cast<vtkIdType>(data[n]);
//            }
//            delete[] data;
//        }
//    }
//    cgio_release_id(cgioNum, cgioElemConnectId);
//    return 0;
//}


#if CGNS_VERSION < 4000
int getSectionConnectivity(const int cgioNum, const double cgioSectionId,
    const int dim, const cgsize_t* srcStart, const cgsize_t* srcEnd,
    const cgsize_t* srcStride, const cgsize_t* memStart,
    const cgsize_t* memEnd, const cgsize_t* memStride,
    const cgsize_t* memDim, cglong_t *connectivity,
    int connectivitySize)
{
    const char* connectivityPath = "ElementConnectivity";
    double cgioElemConnectId;
    char dataType[3];
    std::size_t sizeOfCnt = 0;

    cgio_get_node_id(cgioNum, cgioSectionId, connectivityPath, &cgioElemConnectId);
    cgio_get_data_type(cgioNum, cgioElemConnectId, dataType);

    //cgsize_t connSize = 1;

    //for (int i = 0; i < dim; i++)
    //{
    //    connSize *= memDim[i];
    //}

    bool usingAllocTemp = false;
    void *tempData      = NULL;

    if (strcmp(dataType, "I4") == 0)
    { 
        tempData = new int[connectivitySize];
        usingAllocTemp = true;
    }
    else if (strcmp(dataType, "I8") == 0)
    { 
        tempData = connectivity;
    }
    else
    { 
        //FIXME: visit error
        std::cerr << "ElementConnectivity data_type unknown\n";
    }

    if (cgio_read_data(cgioNum, cgioElemConnectId, srcStart, srcEnd, srcStride, dim,
          memDim, memStart, memEnd, memStride, (void*)tempData) != CG_OK)
    {
        if (usingAllocTemp)
        {
            delete [] (int *)tempData;
        }
        //FIXME: visit error?
        //char message[81];
        //cgio_error_message(message);
        //std::cerr << "cgio_read_data :" << message;
        return 1;
    }

    if (usingAllocTemp)
    {
        for (int i = 0; i < connectivitySize; i++)
        {
            connectivity[i] = static_cast<cglong_t>(((int *)tempData)[i]);
        }
        delete [] (int *)tempData;
    }

    cgio_release_id(cgioNum, cgioElemConnectId);
    return 0;
}
#endif
