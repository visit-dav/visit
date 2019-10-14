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
//                            avtXdmfFileFormat.C                            //
// ************************************************************************* //

#include <avtXdmfFileFormat.h>

#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedShortArray.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkFieldData.h>
#include <vtkInformation.h>
#include <vtkRectilinearGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtksys/SystemTools.hxx>

#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidDBTypeException.h>
#include <InvalidSourceException.h>
#include <InvalidVariableException.h>

#include <XdmfArray.h>
#include <XdmfAttribute.h>
#include <XdmfDataItem.h>
#include <XdmfDOM.h>
#include <XdmfGeometry.h>
#include <XdmfGrid.h>
#include <XdmfHex64Generator.h>
#include <XdmfHex125Generator.h>
#include <XdmfInformation.h>
#include <XdmfTime.h>
#include <XdmfTopology.h>

#include <iterator>
#include <queue>
#include <string>

using std::string;

// ****************************************************************************
//  Method: avtXdmfFileFormat constructor
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

avtXdmfFileFormat::avtXdmfFileFormat(const char *fname) :
    avtMTMDFileFormat(fname), filename(fname), currentGrid(NULL), dom(NULL)
{
    dom = new XdmfDOM();

    std::string directory = vtksys::SystemTools::GetFilenamePath(fname) + "/";
    if (directory == "/") {
        directory = vtksys::SystemTools::GetCurrentWorkingDirectory() + "/";
    }
    dom->SetWorkingDirectory(directory.c_str());
    if (dom->Parse(fname) == XDMF_FAIL) {
        EXCEPTION1(InvalidDBTypeException, "The file could not be opened");
    }

    this->Stride[0] = this->Stride[1] = this->Stride[2] = 1;

    firstGrid = "/Xdmf/Domain/Grid";
    numGrids = 0;

    //  Modifications:
    //      Eric Brugger, Wed Jan 12 14:52:54 PST 2011
    //      I modified the routine to return a single grid in the case of a
    //      collection where the collection type was unset (since the default
    //      type is spatial) and in the case of a tree. I also had it set the
    //      time in the case of a single time state.

    XdmfXmlNode gridElement = dom->FindElementByPath("/Xdmf/Domain/Grid");
    XdmfGrid grid;
    grid.SetDOM(dom);
    grid.SetElement(gridElement);
    grid.UpdateInformation();

    if (grid.GetGridType() == XDMF_GRID_COLLECTION) {
        if (grid.GetCollectionType() == XDMF_GRID_COLLECTION_TEMPORAL) {
            firstGrid = "/Xdmf/Domain/Grid/Grid";
            XdmfGrid childGrid;
            for (int i = 0; i < grid.GetNumberOfChildren(); ++i) {
                std::stringstream gridLocation;
                gridLocation << firstGrid << "[" << i + 1 << "]";
                childGrid.SetDOM(dom);
                childGrid.SetElement(dom->FindElementByPath(gridLocation.str().c_str()));
                childGrid.UpdateInformation();
                timesteps.push_back(childGrid.GetTime()->GetValue());
            }
            numGrids = 1;
        }
        else if(grid.GetCollectionType() == XDMF_GRID_COLLECTION_SPATIAL ||
                grid.GetCollectionType() == XDMF_GRID_COLLECTION_UNSET) {
            numGrids = 1;
            timesteps.push_back(grid.GetTime()->GetValue());
        }
    }
    else if (grid.GetGridType() == XDMF_GRID_TREE)
    {
        numGrids = 1;
        timesteps.push_back(grid.GetTime()->GetValue());
    }
    else
    {
        // Just throw an empty time in here because we only have one timestep
        numGrids = dom->FindNumberOfElements("Grid", dom->FindElementByPath("/Xdmf/Domain"));
        timesteps.push_back(grid.GetTime()->GetValue());
    }

}

// ****************************************************************************
//  Method: avtXdmfFileFormat destructor
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

avtXdmfFileFormat::~avtXdmfFileFormat()
{
    delete dom;
}

// ****************************************************************************
//  Method: HasInvariantMetaData
//
//  Mark C. Miller, Wed Aug 28 14:44:27 EDT 2019
// ****************************************************************************

bool
avtXdmfFileFormat::HasInvariantMetaData() const
{
   XdmfXmlNode infoElem = dom->FindElementByPath("/Xdmf/Information");
   if (infoElem && !strcmp(dom->Get(infoElem, "Name"), "TimeVaryingMetaData"))
   {
       if (!strcmp(dom->Get(infoElem, "Value"), "True"))
           return false;
   }
   return true;
}

// ****************************************************************************
//  Method: HasInvariantSIL
//
//  Mark C. Miller, Wed Aug 28 14:44:27 EDT 2019
// ****************************************************************************

bool
avtXdmfFileFormat::HasInvariantSIL() const
{
    return HasInvariantMetaData();
}


// ****************************************************************************
//  Method: avtXdmfFileFormat::AddArrayExpressions
//
//  Purpose:
//      Creates expressions for each component in an XdmfAttribute.  This should be called whenever
//      an XdmfAttribute is read into VisIt as an array (numComponents is not appropriate for Scalar, Vector,
//      or Tensor)
//
//  Arguments:
//      vtkMetaDatabase * md --- the metadata database to insert expressions into.
//      std::string attributeName --- the name of the XdmfAttribute associated with the expressions.
//      std::vector<std::string> names --- a vector containing the names of each component of the XdmfAttribute.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

void avtXdmfFileFormat::AddArrayExpressions(avtDatabaseMetaData * md, std::string attributeName, std::vector<
        std::string> & names)
{
    for (size_t i = 0; i < names.size(); ++i) {
        Expression expression;
        expression.SetName(names[i]);

        std::stringstream definition;
        definition << "array_decompose(" << this->GetFormattedExpressionName(attributeName) << "," << i << ")";
        expression.SetDefinition(definition.str());
        expression.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&expression);
    }
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::CopyXdmfArray
//
//  Purpose:
//      Return a new vtkDataArray containing values copied from an XdmfArray.
//      If numVTKComponents > numXdmfComponents, pad vtkDataArray with zeroes.
//      numVTKComponents must be >= numXdmfComponents, else return NULL.
//
//  Arguments:
//      XdmfArray * array --- the array to copy values from
//      int numVTKComponents --- the number of components to create in the vtkDataArray
//      int numXdmfComponents --- the number of components stored in the XdmfArray
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

vtkDataArray* avtXdmfFileFormat::CopyXdmfArray(XdmfArray * array, int numVTKComponents, int numXdmfComponents)
{

    if(numVTKComponents < numXdmfComponents)
    {
        // Not clear how to copy data.
        return NULL;
    }

    switch (array->GetNumberType()) {
        case XDMF_INT8_TYPE: {
            vtkCharArray * vtkArray = vtkCharArray::New();
            this->CopyXdmfArray<XdmfInt8> (array, vtkArray, numVTKComponents, numXdmfComponents);
            return vtkArray;
        }
        case XDMF_UINT8_TYPE: {
            vtkUnsignedCharArray * vtkArray = vtkUnsignedCharArray::New();
            this->CopyXdmfArray<XdmfUInt8> (array, vtkArray, numVTKComponents, numXdmfComponents);
            return vtkArray;
        }
        case XDMF_INT16_TYPE: {
            vtkShortArray * vtkArray = vtkShortArray::New();
            this->CopyXdmfArray<XdmfInt16> (array, vtkArray, numVTKComponents, numXdmfComponents);
            return vtkArray;
        }
        case XDMF_UINT16_TYPE: {
            vtkUnsignedShortArray * vtkArray = vtkUnsignedShortArray::New();
            this->CopyXdmfArray<XdmfUInt16> (array, vtkArray, numVTKComponents, numXdmfComponents);
            return vtkArray;
        }
        case XDMF_INT32_TYPE: {
            vtkIntArray * vtkArray = vtkIntArray::New();
            this->CopyXdmfArray<XdmfInt32> (array, vtkArray, numVTKComponents, numXdmfComponents);
            return vtkArray;
        }
        case XDMF_UINT32_TYPE: {
            vtkUnsignedIntArray * vtkArray = vtkUnsignedIntArray::New();
            this->CopyXdmfArray<XdmfUInt32> (array, vtkArray, numVTKComponents, numXdmfComponents);
            return vtkArray;
        }
        case XDMF_INT64_TYPE: {
            vtkLongArray * vtkArray = vtkLongArray::New();
            this->CopyXdmfArray<XdmfInt64> (array, vtkArray, numVTKComponents, numXdmfComponents);
            return vtkArray;
        }
        case XDMF_FLOAT32_TYPE: {
            vtkFloatArray * vtkArray = vtkFloatArray::New();
            this->CopyXdmfArray<XdmfFloat32> (array, vtkArray, numVTKComponents, numXdmfComponents);
            return vtkArray;
        }
        case XDMF_FLOAT64_TYPE: {
            default:
            vtkDoubleArray * vtkArray = vtkDoubleArray::New();
            this->CopyXdmfArray<XdmfFloat64> (array, vtkArray, numVTKComponents, numXdmfComponents);
            return vtkArray;
        }
    }
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::CopyXdmfArray
//
//  Purpose:
//      Fill vtkDataArray with values copied from an XdmfArray.
//      If numVTKComponents > numXdmfComponents, pad vtkDataArray with zeroes.
//
//  Arguments:
//      XdmfArray * array --- the array to copy values from
//      vtkDataArray * vtkDataArray --- the array to store values in
//      int numVTKComponents --- the number of components to create in the vtkDataArray
//      int numXdmfComponents --- the number of components stored in the XdmfArray
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

template<typename T>
void avtXdmfFileFormat::CopyXdmfArray(XdmfArray * array, vtkDataArray * vtkDataArray, int numVTKComponents,
        int numXdmfComponents)
{
    vtkDataArray->SetNumberOfComponents(numVTKComponents);
    vtkDataArray->SetNumberOfTuples(array->GetNumberOfElements() / numXdmfComponents);
    for (int i = numXdmfComponents; i < numVTKComponents; ++i) {
        vtkDataArray->FillComponent(i, 0);
    }
    for (vtkIdType i = 0; i < vtkDataArray->GetNumberOfTuples(); ++i) {
        for (vtkIdType j = 0; j < numXdmfComponents; ++j) {
            T val;
            array->GetValues((i * numXdmfComponents) + j, &val, 1);
            vtkDataArray->SetComponent(i, j, val);
        }
    }
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::CopyXdmfArrayByPointer
//
//  Purpose:
//      Returns a new vtkDataArray containing the values stored in the XdmfArray.
//      This does not make a copy of the values --- the vtkDataArray points to the values
//      stored in the XdmfArray.
//
//  Arguments:
//      XdmfArray * array --- the array to copy values from
//      int numComponents --- the number of components to create in the vtkDataArray
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

vtkDataArray * avtXdmfFileFormat::CopyXdmfArrayByPointer(XdmfArray * array, int numComponents)
{
    switch (array->GetNumberType()) {
        case XDMF_INT8_TYPE: {
            vtkCharArray * vtkArray = vtkCharArray::New();
            vtkArray->SetNumberOfComponents(numComponents);
            vtkArray->SetNumberOfTuples(array->GetNumberOfElements() / numComponents);
            vtkArray->SetArray((char*) array->GetDataPointer(), array->GetNumberOfElements(), 0);
            array->Reset(0);
            return vtkArray;
        }
        case XDMF_UINT8_TYPE: {
            vtkUnsignedCharArray * vtkArray = vtkUnsignedCharArray::New();
            vtkArray->SetNumberOfComponents(numComponents);
            vtkArray->SetNumberOfTuples(array->GetNumberOfElements() / numComponents);
            vtkArray->SetArray((unsigned char*) array->GetDataPointer(), array->GetNumberOfElements(), 0);
            array->Reset(0);
            return vtkArray;
        }
        case XDMF_INT16_TYPE: {
            vtkShortArray * vtkArray = vtkShortArray::New();
            vtkArray->SetNumberOfComponents(numComponents);
            vtkArray->SetNumberOfTuples(array->GetNumberOfElements() / numComponents);
            vtkArray->SetArray((short*) array->GetDataPointer(), array->GetNumberOfElements(), 0);
            array->Reset(0);
            return vtkArray;
        }
        case XDMF_UINT16_TYPE: {
            vtkUnsignedShortArray * vtkArray = vtkUnsignedShortArray::New();
            vtkArray->SetNumberOfComponents(numComponents);
            vtkArray->SetNumberOfTuples(array->GetNumberOfElements() / numComponents);
            vtkArray->SetArray((unsigned short*) array->GetDataPointer(), array->GetNumberOfElements(), 0);
            array->Reset(0);
            return vtkArray;
        }
        case XDMF_UINT32_TYPE: {
            vtkUnsignedIntArray * vtkArray = vtkUnsignedIntArray::New();
            vtkArray->SetNumberOfComponents(numComponents);
            vtkArray->SetNumberOfTuples(array->GetNumberOfElements() / numComponents);
            vtkArray->SetArray((unsigned int*) array->GetDataPointer(), array->GetNumberOfElements(), 0);
            array->Reset(0);
            return vtkArray;
        }
        case XDMF_INT32_TYPE: {
            vtkIntArray * vtkArray = vtkIntArray::New();
            vtkArray->SetNumberOfComponents(numComponents);
            vtkArray->SetNumberOfTuples(array->GetNumberOfElements() / numComponents);
            vtkArray->SetArray((int*) array->GetDataPointer(), array->GetNumberOfElements(), 0);
            array->Reset(0);
            return vtkArray;
        }
        case XDMF_INT64_TYPE: {
            vtkLongArray * vtkArray = vtkLongArray::New();
            vtkArray->SetNumberOfComponents(numComponents);
            vtkArray->SetNumberOfTuples(array->GetNumberOfElements() / numComponents);
            vtkArray->SetArray((long*) array->GetDataPointer(), array->GetNumberOfElements(), 0);
            array->Reset(0);
            return vtkArray;
        }
        case XDMF_FLOAT32_TYPE: {
            vtkFloatArray * vtkArray = vtkFloatArray::New();
            vtkArray->SetNumberOfComponents(numComponents);
            vtkArray->SetNumberOfTuples(array->GetNumberOfElements() / numComponents);
            vtkArray->SetArray((float*) array->GetDataPointer(), array->GetNumberOfElements(), 0);
            array->Reset(0);
            return vtkArray;
        }
        case XDMF_FLOAT64_TYPE:
        default: {
            vtkDoubleArray * vtkArray = vtkDoubleArray::New();
            vtkArray->SetNumberOfComponents(numComponents);
            vtkArray->SetNumberOfTuples(array->GetNumberOfElements() / numComponents);
            vtkArray->SetArray((double*) array->GetDataPointer(), array->GetNumberOfElements(), 0);
            array->Reset(0);
            return vtkArray;
        }
    }
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

void avtXdmfFileFormat::FreeUpResources(void)
{
    delete currentGrid;
    currentGrid = NULL;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetAttributeFromName
//
//  Purpose:
//      Convenience function to find an attribute by name.
//
//  Arguments:
//      XdmfGrid * grid --- the grid used to find the attribute
//      const char * --- the name of the attribute to find.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

XdmfAttribute * avtXdmfFileFormat::GetAttributeFromName(XdmfGrid * grid, const char * attributeName)
{
    std::string name = attributeName;
    if (numGrids > 1) {
        name = name.substr(name.rfind("/") + 1, name.length());
    }

    for (int i = 0; i < grid->GetNumberOfAttributes(); ++i) {
        if (strcmp(name.c_str(), grid->GetAttribute(i)->GetName()) == 0) {
            return grid->GetAttribute(i);
        }
    }
    EXCEPTION1(InvalidVariableException, attributeName);
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetComponentNames
//
//  Purpose:
//      Constructs names for each component of an XdmfAttribute.
//        A three component vector named "Vector" will have component names
//          "Vector-1", "Vector-2", "Vector-3"
//        A four component tensor named "Tensor" will have component names
//          "Tensor-1-1", "Tensor-1-2", "Tensor-2-1", "Tensor-2-2"
//
//  Arguments:
//      std::string & attributeName --- the name of the XdmfAttribute used to construct component names.
//      XdmfInt32 attributeType --- the type of the XdmfAttribute used to construct component names.
//      int numComponents --- the number of components stored in the XdmfAttribute.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

std::vector<std::string> avtXdmfFileFormat::GetComponentNames(std::string attributeName, XdmfInt32 attributeType,
        int numComponents)
{
    std::vector<std::string> componentNames;

    if(attributeType == XDMF_ATTRIBUTE_TYPE_TENSOR6)
    {
        numComponents = this->GetNumberOfSymmetricalTensorComponents(numComponents);
    }

    int tensorLength = 0;
    if (attributeType == XDMF_ATTRIBUTE_TYPE_TENSOR || attributeType == XDMF_ATTRIBUTE_TYPE_TENSOR6) {
        tensorLength = (int) sqrt((float)numComponents);
    }

    for (int i = 0; i < numComponents; ++i) {
        std::stringstream name;
        if (attributeType == XDMF_ATTRIBUTE_TYPE_TENSOR || attributeType == XDMF_ATTRIBUTE_TYPE_TENSOR6) {
            int padding = (int) log10((float)tensorLength) + 1;
            name << attributeName << "-" << setw(padding) << (int) (i / tensorLength) + 1 << "-" << setw(padding) << (i
                    % tensorLength) + 1;
        }
        else {
            name << attributeName << "-" << setw((int) log10((float)numComponents) + 1) << i + 1;
        }
        componentNames.push_back(name.str());
    }
    return componentNames;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetDims
//
//  Purpose:
//      Determines the dimensions of a structured grid from an Xdmf grid.
//
//  Arguments:
//      grid    --- the grid to determine the dimensions of.
//      dims[3] --- the dimensions to fill in.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2012
//
// ****************************************************************************

void avtXdmfFileFormat::GetDims(XdmfGrid *grid, int dims[3])
{
    int whole_extents[6];
    int update_extents[6];
    this->GetWholeExtent(grid, whole_extents);

    memcpy(update_extents, whole_extents, sizeof(int) * 6);

    // convert to stridden update extents.
    int scaled_extents[6];
    this->ScaleExtents(update_extents, scaled_extents, this->Stride);

    this->GetDims(scaled_extents, dims);
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetDims
//
//  Purpose:
//      Determines the dimensions of a structured grid from the extents.
//
//  Arguments:
//      exts[6] --- the extents of the grid.
//      dims[3] --- the dimensions to fill in.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

void avtXdmfFileFormat::GetDims(int exts[6], int dims[3])
{
    dims[0] = exts[1] - exts[0] + 1;
    dims[1] = exts[3] - exts[2] + 1;
    dims[2] = exts[5] - exts[4] + 1;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetFormattedExpressionName
//
//  Purpose:
//      Returns a formatted attributeName appropriate for insertion into expressions.
//      XdmfAttribute names can contain spaces which cause problems when creating expressions.
//      Deal with whitespace by adding escape characters between spaces.
//
//  Arguments:
//      std::string & attributeName --- the attribute name to format.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

std::string avtXdmfFileFormat::GetFormattedExpressionName(std::string & attributeName)
{
    std::stringstream formatted;

    // Deal with whitespace
    std::stringstream stream(attributeName);
    std::istream_iterator<std::string> it(stream);
    std::istream_iterator<std::string> end;
    std::vector<std::string> tokens(it, end);

    std::vector<std::string>::const_iterator iter = tokens.begin();
    if (iter != tokens.end()) {
        formatted << *iter;
        iter++;
    }

    for (; iter != tokens.end(); ++iter) {
        formatted << "\\ " << *iter;
    }

    return formatted.str();
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetGrid
//
//  WARNING --- If you call this method you must delete the created grid or memory
//              will leak!
//
//  Purpose:
//      Get the XdmfGrid at a certain timestate.
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************
XdmfGrid * avtXdmfFileFormat::GetGrid(int timestate)
{
    std::stringstream gridLocation;
    gridLocation << firstGrid << "[" << timestate+1 << "]";

    XdmfXmlNode gridElement = dom->FindElementByPath(gridLocation.str().c_str());
    XdmfGrid * grid = new XdmfGrid();
    grid->SetDOM(dom);
    grid->SetElement(gridElement);
    return grid;
}

// ****************************************************************************
// Method: avtXdmfFileFormat::FirstRealGrid
//
// Purpose:
//   Looks for the first real grid along a series of collection grids.
//
// Arguments:
//   start : The starting grid.
//
// Returns:    A pointer to the first real grid along the way.
//
// Note:       We don't currently do a full tree traversal.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 14 16:44:03 PDT 2015
//
// Modifications:
//
// ****************************************************************************

XdmfGrid *
avtXdmfFileFormat::FirstRealGrid(XdmfGrid *start)
{
    XdmfGrid *retval = start;

    // If the grid we want to read is itself a collection, go another level down.
    if(retval->GetGridType() == XDMF_GRID_COLLECTION && 
      (retval->GetCollectionType() == XDMF_GRID_COLLECTION_SPATIAL || 
       retval->GetCollectionType() == XDMF_GRID_COLLECTION_UNSET))
    {
        retval = FirstRealGrid(retval->GetChild(0));
    }

    return retval;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
//  Modifications:
//    Eric Brugger, Wed Dec 12 09:35:48 PST 2012
//    I added support for processing the BaseIndex and GhostOffsets properties
//    for strucutured grids.
//
//    Brad Whitlock, Tue Apr 14 16:36:26 PDT 2015
//    Handle grids that are collections of collections of real grids.
//
// ****************************************************************************

vtkDataSet * avtXdmfFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    this->SetCurrentGrid(timestate, meshname);

    XdmfGrid * gridToRead = currentGrid;
    if(currentGrid->GetGridType() == XDMF_GRID_COLLECTION && 
      (currentGrid->GetCollectionType() == XDMF_GRID_COLLECTION_SPATIAL || 
       currentGrid->GetCollectionType() == XDMF_GRID_COLLECTION_UNSET))
    {
        gridToRead = this->FirstRealGrid(currentGrid->GetChild(domain));
    }

    gridToRead->Update(); // Read in heavy data
    int vtk_data_type = this->GetMeshDataType(gridToRead);

    vtkDataSet* dataSet = 0;

    switch (vtk_data_type) {
        case VTK_STRUCTURED_GRID:
            dataSet = this->ReadStructuredGrid(gridToRead);
            break;
        case VTK_RECTILINEAR_GRID:
            dataSet = this->ReadRectilinearGrid(gridToRead);
            break;
        case VTK_UNSTRUCTURED_GRID:
            dataSet = this->ReadUnstructuredGrid(gridToRead);
            break;
        default:
            EXCEPTION1(InvalidVariableException, meshname);
    }

    //
    // If we have a structured mesh, process the base index and ghost
    // zone information.
    //
    if (vtk_data_type == VTK_RECTILINEAR_GRID ||
        vtk_data_type == VTK_STRUCTURED_GRID)
    {
        int ndims = this->GetTopologicalDimensions(gridToRead->GetTopology()->GetTopologyType());

        for (int i = 0; i < gridToRead->GetNumberOfInformations(); i++)
        {
            XdmfInformation *info = gridToRead->GetInformation(i);
            if (XDMF_WORD_CMP(info->GetName(), "BaseIndex"))
            {
                int baseIndex[3] = {0, 0, 0};
                if (ndims == 2)
                    sscanf(info->GetValue(), "%d %d",
                           &baseIndex[1], &baseIndex[0]);
                else
                    sscanf(info->GetValue(), "%d %d %d",
                           &baseIndex[2], &baseIndex[1], &baseIndex[0]);

                vtkIntArray *arr = vtkIntArray::New();
                arr->SetNumberOfTuples(3);
                arr->SetValue(0, baseIndex[0]);
                arr->SetValue(1, baseIndex[1]);
                arr->SetValue(2, baseIndex[2]);
                arr->SetName("base_index");
                dataSet->GetFieldData()->AddArray(arr);
                arr->Delete();
            }
            else if (XDMF_WORD_CMP(info->GetName(), "GhostOffsets"))
            {
                int ghostOffsets[6] = {0, 0, 0, 0, 0, 0};
                if (ndims == 2)
                    sscanf(info->GetValue(), "%d %d %d %d",
                           &ghostOffsets[2], &ghostOffsets[3],
                           &ghostOffsets[0], &ghostOffsets[1]);
                else
                    sscanf(info->GetValue(), "%d %d %d %d %d %d",
                           &ghostOffsets[4], &ghostOffsets[5],
                           &ghostOffsets[2], &ghostOffsets[3],
                           &ghostOffsets[0], &ghostOffsets[1]);

                int dims[3];
                this->GetDims(gridToRead, dims);

                this->GetStructuredGhostZones(dims, ghostOffsets, dataSet);
            }
        }
    }

    gridToRead->Release();
    return dataSet;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetMeshDataType
//
//  Purpose:
//      Translate between Xdmf and VTK Grid Types
//
//  Arguments:
//      XdmfGrid * grid --- the grid used to determine the VTK grid type.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2009
//
// ****************************************************************************

int avtXdmfFileFormat::GetMeshDataType(XdmfGrid* grid)
{
    if (grid->GetTopology()->GetClass() == XDMF_UNSTRUCTURED) {
        return VTK_UNSTRUCTURED_GRID;
    }
    XdmfInt32 topologyType = grid->GetTopology()->GetTopologyType();
    if (topologyType == XDMF_2DSMESH || topologyType == XDMF_3DSMESH) {
        return VTK_STRUCTURED_GRID;
    }
    else if (topologyType == XDMF_2DCORECTMESH || topologyType == XDMF_3DCORECTMESH ||
             topologyType == XDMF_2DRECTMESH || topologyType == XDMF_3DRECTMESH) {
        return VTK_RECTILINEAR_GRID;
    }
    return -1;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

int avtXdmfFileFormat::GetNTimesteps(void)
{
    return (int)timesteps.size();
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetNumberOfCellComponents
//
//  Purpose:
//      Determines the number of components contained in a cell centered
//      XdmfAttribute. For example, a grid that contains 8 cells and an
//      attribute that contains 16 values would have two components.  If the
//      number of values is not a multiple of the number of cells, return 0.
//
//  Arguments:
//      XdmfGrid * grid --- the grid containing the attribute and the topology.
//      XdmfAttribute * attribute --- the cell centered attribute used to find
//      the number of components
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
//  Modifications:
//    Kathleen Biagas, Tue Jun 13 10:28:40 PDT 2017
//    Parse 'numCellStrings' consistently.  It may still contain 'Dimensions'
//    type values even if designated 'NumberOfElements'.
//
// ****************************************************************************

long
avtXdmfFileFormat::GetNumberOfCellComponents(XdmfGrid* grid,
                                             XdmfAttribute* attribute)
{
    long numValues = this->GetNumberOfValues(attribute);

    long numCells = 0;

    const char * numCellsString = grid->GetTopology()->GetDOM()->GetAttribute(
        grid->GetTopology()->GetElement(), "NumberOfElements");
    if (numCellsString == NULL)
    {
        numCellsString = grid->GetTopology()->GetDOM()->GetAttribute(
            grid->GetTopology()->GetElement(), "Dimensions");

        if (numCellsString == NULL)
        {
            return 0;
        }
    }
    numCells = 1;

    std::stringstream stream(numCellsString);
    std::istream_iterator<std::string> it(stream);
    std::istream_iterator<std::string> end;
    std::vector<std::string> tokens(it, end);
    std::vector<std::string>::const_iterator iter = tokens.begin();
    for (; iter != tokens.end(); ++iter)
    {
        long val = atoi((*iter).c_str());
        if (grid->GetTopology()->GetClass() == XDMF_STRUCTURED)
        {
            if (val - 1 > 0)
            {
                numCells *= (val - 1);
            }
        }
        else
        {
            if (val > 0)
            {
                numCells *= val;
            }
        }
    }

    if (numCells == 0 || numValues % numCells != 0)
    {
        return 0;
    }
    return numValues / numCells;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetNumberOfComponents
//
//  Purpose:
//      Determines the number of components contained in a XdmfAttribute.  If the number
//      of values is not divisible by the number of cells (for cell centered) or the number
//      of points (for node centered) return 0.
//
//  Arguments:
//      XdmfGrid * grid --- the grid containing the attribute
//      XdmfAttribute * attribute --- the attribute used to find the number of components
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

int avtXdmfFileFormat::GetNumberOfComponents(XdmfGrid* grid, XdmfAttribute* attribute)
{
    if (attribute->GetAttributeCenter() == XDMF_ATTRIBUTE_CENTER_NODE) {
        return this->GetNumberOfNodeComponents(grid, attribute);
    }
    else if (attribute->GetAttributeCenter() == XDMF_ATTRIBUTE_CENTER_CELL) {
        return this->GetNumberOfCellComponents(grid, attribute);
    }
    return 0;
}


// ****************************************************************************
//  Method: avtXdmfFileFormat::GetNumberOfNodeComponents
//
//  Purpose:
//      Determines the number of components contained in a node centered XdmfAttribute.
//      For example, a grid that contains 8 points and an attribute that contains 16 values would
//      have two components.  If the number of values is not a multiple of the number of points,
//      return 0.
//
//  Arguments:
//      XdmfGrid * grid --- the grid containing the attribute and the geometry.
//      XdmfAttribute * attribute --- the node centered attribute used to find the number of components
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

long avtXdmfFileFormat::GetNumberOfNodeComponents(XdmfGrid * grid, XdmfAttribute * attribute)
{
    long numValues = this->GetNumberOfValues(attribute);
    long numPoints = this->GetNumberOfPoints(grid);

    if(numPoints == 0 || numValues % numPoints != 0)
    {
        return 0;
    }
    return numValues / numPoints;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetNumberOfPoints
//
//  Purpose:
//      Finds the number of points contained in an XdmfGrid.
//
//  Arguments:
//      XdmfGrid * grid --- the grid used to find the number of points.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

long avtXdmfFileFormat::GetNumberOfPoints(XdmfGrid * grid)
{
    XdmfDOM * dom = grid->GetGeometry()->GetDOM();
    XdmfXmlNode element = grid->GetGeometry()->GetElement();
    int numGeometryDataItems = dom->FindNumberOfElements("DataItem",
                                                         element);
    numGeometryDataItems += dom->FindNumberOfElements("DataTransform",
                                                      element);
    numGeometryDataItems += dom->FindNumberOfElements("DataStructure",
                                                      element);
 
    if (numGeometryDataItems == 0) {
        return 0;
    }

    long numPoints = 1;

    if (grid->GetGeometry()->GetGeometryType() == XDMF_GEOMETRY_VXVY ||
        grid->GetGeometry()->GetGeometryType() == XDMF_GEOMETRY_VXVYVZ) {
        // numPoints = Product of Geometry Shape
        for (int i = 0; i < numGeometryDataItems; ++i) {
            XdmfDataItem xmfDataItem;
            xmfDataItem.SetDOM(grid->GetGeometry()->GetDOM());
            xmfDataItem.SetElement(dom->FindDataElement(i, element));
            xmfDataItem.UpdateInformation();
            int rank = xmfDataItem.GetRank();
            if (rank < 1) {
                continue;
            }
            XdmfInt64 * shape = new XdmfInt64[rank];
            xmfDataItem.GetShape(shape);
            for (int j = 0; j < rank; ++j) {
                numPoints *= shape[j];
            }
            delete[] shape;
        }
    }
    else if (grid->GetGeometry()->GetGeometryType() == XDMF_GEOMETRY_ORIGIN_DXDY
            || grid->GetGeometry()->GetGeometryType() == XDMF_GEOMETRY_ORIGIN_DXDYDZ) {
        // numPoints = Product of Topology Shape
        const char * numCellsString = grid->GetTopology()->GetDOM()->GetAttribute(grid->GetTopology()->GetElement(),
                "Dimensions");
        if (numCellsString == NULL) {
            return 0;
        }
        std::stringstream stream(numCellsString);
        std::istream_iterator<std::string> it(stream);
        std::istream_iterator<std::string> end;
        std::vector<std::string> tokens(it, end);
        std::vector<std::string>::const_iterator iter = tokens.begin();
        for (; iter != tokens.end(); ++iter) {
            long val = atoi((*iter).c_str());
            if (val > 0) {
                numPoints *= val;
            }
        }
    }
    else {
        numPoints = this->GetNumberOfValues(grid->GetGeometry());
        if (grid->GetGeometry()->GetGeometryType() == XDMF_GEOMETRY_XY ||
            grid->GetGeometry()->GetGeometryType() == XDMF_GEOMETRY_X_Y) {
            numPoints = numPoints / 2;
        }
        else {
            numPoints = numPoints / 3;
        }
    }
    return numPoints;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetNumberOfSymmetricalTensorComponents
//
//  Purpose:
//      Determines the number of components needed for a symmetrical tensor...  I.E. if there are
//      6 components in a symmetrical tensor, we need 9 components to store the tensor:
//
//      0 1 2
//      1 3 4
//      2 4 5
//
//      If a valid number of components cannot be determined, return 0.
//
//  Arguments:
//      int numComponents --- the number of symmetrical components
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

int avtXdmfFileFormat::GetNumberOfSymmetricalTensorComponents(int numComponents)
{
    int tensorSize = 1;
    int sum = 0;
    while (true) {
        sum += tensorSize;
        if (sum == numComponents) {
            return tensorSize*tensorSize;
        }
        if (sum > numComponents) {
            return 0;
        }
        tensorSize++;
    }
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetNumberOfValues
//
//  Purpose:
//      Finds the number of values contained in all child DataItems of an XdmfElement.
//      This reads the number of values without actually having to read all the values from disk.
//
//  Arguments:
//      XdmfElement * element --- finds the number of values contained in this element.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

long avtXdmfFileFormat::GetNumberOfValues(XdmfElement * element)
{
    long numVals = 0;

    XdmfDOM * dom = element->GetDOM();
    XdmfXmlNode xmlElement = element->GetElement();
    int numDataItems = dom->FindNumberOfElements("DataItem",
                                                 xmlElement);
    numDataItems += dom->FindNumberOfElements("DataTransform",
                                              xmlElement);
    numDataItems += dom->FindNumberOfElements("DataStructure",
                                              xmlElement);

    for (int i = 0; i < numDataItems; ++i) {
        long currNumVals = 1;
        XdmfDataItem xmfDataItem;
        xmfDataItem.SetDOM(element->GetDOM());
        xmfDataItem.SetElement(dom->FindDataElement(i, xmlElement));
        xmfDataItem.UpdateInformation();
        int rank = xmfDataItem.GetRank();
        if (rank < 1) {
            return 0;
        }
        XdmfInt64 * shape = new XdmfInt64[rank];
        xmfDataItem.GetShape(shape);
        for (int j = 0; j < rank; ++j) {
            currNumVals *= shape[j];
        }
        delete[] shape;
        numVals += currNumVals;
    }
    return numVals;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetTopologicalDimensions
//
//  Purpose:
//      Gets the topological dimensions of an XdmfTopology
//
//  Arguments:
//      XdmfInt32 topologyType --- finds the topological dimensions of this Xdmf topology type.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

int avtXdmfFileFormat::GetTopologicalDimensions(XdmfInt32 topologyType)
{
    switch (topologyType) {
        case (XDMF_POLYVERTEX):
            return 0;
        case (XDMF_POLYLINE):
            return 1;
        case (XDMF_POLYGON):
        case (XDMF_TRI):
        case (XDMF_TRI_6):
        case (XDMF_TRI_7):
        case (XDMF_QUAD):
        case (XDMF_QUAD_6):
        case (XDMF_QUAD_8):
        case (XDMF_QUAD_9):
        case (XDMF_2DSMESH):
        case (XDMF_2DRECTMESH):
        case (XDMF_2DCORECTMESH):
            return 2;
        default:
            return 3;
    }
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetSpatialDimensions
//
//  Purpose:
//      Gets the spatial dimension of an XdmfGeometry
//
//  Arguments:
//      XdmfInt32 geometryType --- finds the spatial dimensions of this Xdmf geometry type.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

int avtXdmfFileFormat::GetSpatialDimensions(XdmfInt32 geometryType)
{
    switch(geometryType)
    {
        case(XDMF_GEOMETRY_XY):
        case(XDMF_GEOMETRY_X_Y):
        case(XDMF_GEOMETRY_VXVY):
        case(XDMF_GEOMETRY_ORIGIN_DXDY):
            return 2;
        default:
            return 3;
    }
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetTimes
//
//  Purpose:
//      Get time values associated with this file
//
//  Arguments:
//      std::vector<double> & times --- vector to add time values to.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
//  Modifications:
//    Hank Childs, Thu Aug 18 17:22:19 PDT 2011
//    Indicate times are accurate.
//
// ****************************************************************************
void avtXdmfFileFormat::GetTimes(std::vector<double> & times)
{
    times.insert(times.begin(),
                 timesteps.begin(),
                 timesteps.end());
    metadata->SetTimesAreAccurate(true);

    return;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: kleiter -- generated by xml2avt
//  Creation:   Mon Mar 29 15:43:05 PST 2010
//
// Modifications:
//    Brad Whitlock, Tue Apr 14 16:36:26 PDT 2015
//    Handle grids that are collections of collections of real grids.
//
// ****************************************************************************

vtkDataArray *
avtXdmfFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    if (numGrids > 1) {
        std::string gridName = varname;
        gridName = gridName.substr(0, gridName.rfind("/"));
        this->SetCurrentGrid(timestate, gridName.c_str());
    }

    XdmfGrid * gridToExamine = currentGrid;
    if(currentGrid->GetGridType() == XDMF_GRID_COLLECTION && 
       (currentGrid->GetCollectionType() == XDMF_GRID_COLLECTION_SPATIAL ||
        currentGrid->GetCollectionType() == XDMF_GRID_COLLECTION_UNSET))
    {
        gridToExamine = this->FirstRealGrid(currentGrid->GetChild(domain));
    }

    XdmfAttribute * attribute = this->GetAttributeFromName(gridToExamine, varname);
    attribute->Update();
    return this->CopyXdmfArrayByPointer(attribute->GetValues(), 1);
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  This is called
//      for all Xdmf data of VTK component size > 1 --- i.e. Vectors, Tensors,
//      Matrices.  If Xdmf component size < VTK component size, for things like
//      two component vectors, then we fill in zeroes to fill out the array.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 11 17:53:55 PST 2012
//    I corrected an error where the reader would return an error indicating
//    that the variable was invalid when the variable was defined on a multi
//    block mesh.
//
//    Brad Whitlock, Tue Apr 14 16:36:26 PDT 2015
//    Handle grids that are collections of collections of real grids.
//
// ****************************************************************************

vtkDataArray *
avtXdmfFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{

    if (numGrids > 1) {
        std::string gridName = varname;
        gridName = gridName.substr(0, gridName.rfind("/"));
        this->SetCurrentGrid(timestate, gridName.c_str());
    }

    XdmfGrid * gridToExamine = currentGrid;
    if(currentGrid->GetGridType() == XDMF_GRID_COLLECTION && 
      (currentGrid->GetCollectionType() == XDMF_GRID_COLLECTION_SPATIAL ||
       currentGrid->GetCollectionType() == XDMF_GRID_COLLECTION_UNSET))
    {
        gridToExamine = this->FirstRealGrid(currentGrid->GetChild(domain));
    }
    XdmfAttribute * attribute = this->GetAttributeFromName(gridToExamine, varname);
    attribute->Update();

    int numXdmfComponents = this->GetNumberOfComponents(gridToExamine, attribute);

    int numVTKComponents = 1;
    if (attribute->GetAttributeType() == XDMF_ATTRIBUTE_TYPE_VECTOR) {
        numVTKComponents = 3;
    }
    else if (attribute->GetAttributeType() == XDMF_ATTRIBUTE_TYPE_TENSOR6) {
        numVTKComponents = 9;
    }
    else if (attribute->GetAttributeType() == XDMF_ATTRIBUTE_TYPE_TENSOR) {
        numVTKComponents = 9;
    }

    if(numXdmfComponents == 0)
    {
        if(gridToExamine->GetTopology()->GetClass() != XDMF_UNSTRUCTURED)
        {
            numXdmfComponents = numVTKComponents;
            if(attribute->GetAttributeType() == XDMF_ATTRIBUTE_TYPE_TENSOR6)
            {
              numXdmfComponents = 6;
            }
        }
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Read in the array
    if (attribute->GetAttributeType() == XDMF_ATTRIBUTE_TYPE_TENSOR6) {
        // Rearrange Components to Make Square Matrix
        int tensorSize = (int)sqrt((float)this->GetNumberOfSymmetricalTensorComponents(numXdmfComponents));
        if(tensorSize == 0)
        {
            EXCEPTION1(InvalidVariableException, varname);
        }
        vtkDataArray * toCopy = this->CopyXdmfArray(attribute->GetValues(), numVTKComponents, numXdmfComponents);
        vtkDataArray * toReturn = vtkDataArray::CreateDataArray(toCopy->GetDataType());
        if(numXdmfComponents <= 6)
        {
            toReturn->SetNumberOfComponents(9);
            for(int i=numXdmfComponents; i<9; ++i)
            {
                toReturn->FillComponent(i, 0);
            }
        }
        else
        {
            toReturn->SetNumberOfComponents(tensorSize * tensorSize);
        }
        toReturn->SetNumberOfTuples(toCopy->GetNumberOfTuples());
        std::queue<double> symmVals;
        for (vtkIdType i = 0; i < toCopy->GetNumberOfTuples(); ++i) {
            int id = 0;
            for (int j = 0; j < tensorSize; ++j) {
                for (int k = 0; k < tensorSize; ++k) {
                    if (j > k) {
                        toReturn->SetComponent(i, tensorSize * j + k, symmVals.front());
                        symmVals.pop();
                    }
                    else if (j == k) {
                        toReturn->SetComponent(i, tensorSize * j + k, toCopy->GetComponent(i, id));
                        id++;
                    }
                    else {
                        toReturn->SetComponent(i, tensorSize * j + k, toCopy->GetComponent(i, id));
                        symmVals.push(toCopy->GetComponent(i, id));
                        id++;
                    }
                }
            }
        }
        toCopy->Delete();
        return toReturn;
    }
    else if (numXdmfComponents >= numVTKComponents) {
        // Read in the entire xdmf dataset.
        return this->CopyXdmfArrayByPointer(attribute->GetValues(), numXdmfComponents);
    }
    else
    {
        // This is called when zeroes need to be filled in.
        return this->CopyXdmfArray(attribute->GetValues(), numVTKComponents, numXdmfComponents);
    }
    return 0;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetVTKCellType
//
//  Purpose:
//      Translate between Xdmf and VTK Cell Types.  If no suitable VTK element
//      type can be found, return VTK_EMPTY_CELL
//
//  Arguments
//      XdmfInt32 cellType --- the Xdmf cell type to translate
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

int avtXdmfFileFormat::GetVTKCellType(XdmfInt32 cellType)
{
    switch (cellType) {
        case XDMF_POLYVERTEX:
            return VTK_POLY_VERTEX;
        case XDMF_POLYLINE:
            return VTK_POLY_LINE;
        case XDMF_POLYGON:
            return VTK_POLYGON;
        case XDMF_TRI:
            return VTK_TRIANGLE;
        case XDMF_QUAD:
            return VTK_QUAD;
        case XDMF_TET:
            return VTK_TETRA;
        case XDMF_PYRAMID:
            return VTK_PYRAMID;
        case XDMF_WEDGE:
            return VTK_WEDGE;
        case XDMF_HEX:
            return VTK_HEXAHEDRON;
        case XDMF_EDGE_3:
            return VTK_QUADRATIC_EDGE;
        case XDMF_TRI_6:
            return VTK_QUADRATIC_TRIANGLE;
        case XDMF_TRI_7:
            return VTK_BIQUADRATIC_TRIANGLE;
        case XDMF_QUAD_6:
            return VTK_QUADRATIC_LINEAR_QUAD;
        case XDMF_QUAD_8:
            return VTK_QUADRATIC_QUAD;
        case XDMF_QUAD_9:
            return VTK_BIQUADRATIC_QUAD;
        case XDMF_TET_10:
            return VTK_QUADRATIC_TETRA;
        case XDMF_PYRAMID_13:
            return VTK_QUADRATIC_PYRAMID;
        case XDMF_WEDGE_12:
            return VTK_QUADRATIC_LINEAR_WEDGE;
        case XDMF_WEDGE_15:
            return VTK_QUADRATIC_WEDGE;
        case  XDMF_WEDGE_18 :
            return VTK_BIQUADRATIC_QUADRATIC_WEDGE;
        case XDMF_HEX_20:
            return VTK_QUADRATIC_HEXAHEDRON;
        case  XDMF_HEX_24 :
            return VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON;
        case  XDMF_HEX_27 :
            return VTK_TRIQUADRATIC_HEXAHEDRON;
    }
    return VTK_EMPTY_CELL;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetWholeExtent
//
//  Purpose:
//      Determine whole extents from xdmf structured grids.
//
//  Arguments
//      XdmfGrid * grid --- the grid to get extents from.
//      int extents[6] --- the extents array to fill.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

bool avtXdmfFileFormat::GetWholeExtent(XdmfGrid* grid, int extents[6])
{
    extents[0] = extents[2] = extents[4] = 0;
    extents[1] = extents[3] = extents[5] = -1;

    XdmfInt64 dimensions[XDMF_MAX_DIMENSION];
    XdmfDataDesc* xmfDataDesc = grid->GetTopology()->GetShapeDesc();
    XdmfInt32 num_of_dims = xmfDataDesc->GetShape(dimensions);
    // clear out un-filled dimensions.
    for (int cc = num_of_dims; cc < 3; cc++) // only need to until the 3rd dimension
    // since we don't care about any higher
    // dimensions yet.
    {
        dimensions[cc] = 1;
    }

    // vtk Dims are i,j,k XDMF are k,j,i
    if(static_cast<XdmfInt64> (0) > dimensions[0] - 1)
    {
        extents[5] = static_cast<XdmfInt64> (0);
    }
    else
    {
        extents[5] = dimensions[0] - 1;
    }
    if(static_cast<XdmfInt64> (0) > dimensions[1] - 1)
    {
        extents[3] = static_cast<XdmfInt64> (0);
    }
    else
    {
        extents[3] = dimensions[1] - 1;
    }
    if(static_cast<XdmfInt64> (0) > dimensions[2] - 1)
    {
        extents[1] = static_cast<XdmfInt64> (0);
    }
    else
    {
        extents[1] = dimensions[2] - 1;
    }
    return true;
}


// ****************************************************************************
//  Method: avtXdmfFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Populate database metadata for a timeState.
//
//  Arguments:
//      avtkMetaDatabase * md --- the metadata database to populate.
//      int timeState --- the timeState to populate the database with.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
//  Modifications:
//    Hank Childs, Thu Aug 18 17:22:19 PDT 2011
//    Make sure the times are set.
//
//    Jean Favre/Hank Childs, Wed Oct 12 05:31:51 PDT 2011
//    Fix problem with time setting.  (Fix by Jean, commit by Hank)
//
//    Brad Whitlock, Tue Apr 14 16:36:26 PDT 2015
//    Handle grids that are collections of collections of real grids.
//
// ****************************************************************************

void avtXdmfFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    for(int i=0; i<numGrids; ++i)
    {
        XdmfGrid * grid = this->GetGrid(timeState + i);
        grid->UpdateInformation();

        int nblocks = 1;
        XdmfGrid * gridToExamine = grid;
        if(grid->GetGridType() == XDMF_GRID_COLLECTION && (grid->GetCollectionType() == XDMF_GRID_COLLECTION_SPATIAL || grid->GetCollectionType() == XDMF_GRID_COLLECTION_UNSET))
        {
            nblocks = grid->GetNumberOfChildren();
            // Take the first spatial element to generate metadata
            gridToExamine = this->FirstRealGrid(grid->GetChild(0));
        }

        int block_origin = 0;
        int spatial_dimension = this->GetSpatialDimensions(grid->GetGeometry()->GetGeometryType());
        int topological_dimension = this->GetTopologicalDimensions(grid->GetTopology()->GetTopologyType());
        double *extents = NULL;

        avtMeshType mt = AVT_UNSTRUCTURED_MESH;
        if (gridToExamine->GetTopology()->GetTopologyType() == XDMF_POLYVERTEX) {
            mt = AVT_POINT_MESH;
        }
        else if (gridToExamine->GetTopology()->GetTopologyType() == XDMF_2DSMESH ||
                 gridToExamine->GetTopology()->GetTopologyType() == XDMF_3DSMESH) {
            mt = AVT_CURVILINEAR_MESH;
        }
        else if (gridToExamine->GetTopology()->GetTopologyType() == XDMF_2DRECTMESH ||
                 gridToExamine->GetTopology()->GetTopologyType() == XDMF_2DCORECTMESH ||
                 gridToExamine->GetTopology()->GetTopologyType() == XDMF_3DRECTMESH ||
                 gridToExamine->GetTopology()->GetTopologyType() == XDMF_3DCORECTMESH) {
            mt = AVT_RECTILINEAR_MESH;
        }

        AddMeshToMetaData(md, grid->GetName(), mt, extents, nblocks, block_origin, spatial_dimension, topological_dimension);
 
        md->SetTimes(timesteps);

        for (int i = 0; i < gridToExamine->GetNumberOfAttributes(); ++i) {
            XdmfAttribute * attribute = gridToExamine->GetAttribute(i);
            avtCentering center = AVT_UNKNOWN_CENT;
            switch (attribute->GetAttributeCenter()) {
                case (XDMF_ATTRIBUTE_CENTER_NODE): {
                    center = AVT_NODECENT;
                    break;
                }
                case (XDMF_ATTRIBUTE_CENTER_CELL): {
                    center = AVT_ZONECENT;
                    break;
                }
            }

            std::stringstream attributeName;
            if (numGrids > 1) {
                attributeName << grid->GetName() << "/";
            }
            attributeName << attribute->GetName();

            int numComponents = this->GetNumberOfComponents(gridToExamine, attribute);
            if (numComponents > 0 || mt != AVT_UNSTRUCTURED_MESH) {
                switch (attribute->GetAttributeType()) {
                    case (XDMF_ATTRIBUTE_TYPE_SCALAR): {
                        if (numComponents <= 1) {
                            AddScalarVarToMetaData(md, attributeName.str(), grid->GetName(), center);
                        }
                        else {
                            std::vector<std::string> names = this->GetComponentNames(attributeName.str(),
                                    attribute->GetAttributeType(), numComponents);
                            AddArrayVarToMetaData(md, attributeName.str(), names, grid->GetName(), center);
                            AddArrayExpressions(md, attributeName.str(), names);
                        }
                        break;
                    }
                    case (XDMF_ATTRIBUTE_TYPE_VECTOR): {
                        std::vector<std::string> names = this->GetComponentNames(attributeName.str(),
                                attribute->GetAttributeType(), numComponents);
                        if (numComponents <= 3) {
                            AddVectorVarToMetaData(md, attributeName.str(), grid->GetName(), center, numComponents,
                                    NULL);
                        }
                        else {
                            AddArrayVarToMetaData(md, attributeName.str(), names, grid->GetName(), center);
                            AddArrayExpressions(md, attributeName.str(), names);
                        }
                        break;
                    }
                    case (XDMF_ATTRIBUTE_TYPE_TENSOR6): {
                        std::vector<std::string> names = this->GetComponentNames(attributeName.str(),
                                attribute->GetAttributeType(), numComponents);
                        if (numComponents <= 6) {
                            AddSymmetricTensorVarToMetaData(md, attributeName.str(), grid->GetName(), center,
                                    numComponents);
                        }
                        else {
                            AddArrayVarToMetaData(md, attributeName.str(), names, grid->GetName(), center);
                            AddArrayExpressions(md, attributeName.str(), names);
                        }
                        break;
                    }
                    case (XDMF_ATTRIBUTE_TYPE_TENSOR): {
                        std::vector<std::string> names = this->GetComponentNames(attributeName.str(),
                                attribute->GetAttributeType(), numComponents);
                        if (numComponents <= 9) {
                            AddTensorVarToMetaData(md, attributeName.str(), grid->GetName(), center, 3);
                        }
                        else {
                            AddArrayVarToMetaData(md, attributeName.str(), names, grid->GetName(), center);
                            AddArrayExpressions(md, attributeName.str(), names);
                        }
                        break;
                    }
                }
            }
        }
        delete grid;
    }
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::ReadRectilinearGrid
//
//  Purpose:
//      Constructs a vtkRectilinear from an XdmfGrid by reading geometry
//      and connectivity
//
//  Arguments:
//      XdmfGrid * grid --- the grid to read into a vtkRectilinearGrid
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
//  Modifications:
//    Brad Whitlock, Thu Apr 29 13:48:33 PST 2010
//    I special-cased XDMF_GEOMETRY_ORIGIN_DXDY so it does what we want.
//
// ****************************************************************************

vtkRectilinearGrid* avtXdmfFileFormat::ReadRectilinearGrid(XdmfGrid* grid)
{
    vtkRectilinearGrid * rg = vtkRectilinearGrid::New();
    int whole_extents[6];
    int update_extents[6];
    this->GetWholeExtent(grid, whole_extents);

    memcpy(update_extents, whole_extents, sizeof(int) * 6);

    // convert to stridden update extents.
    int scaled_extents[6];
    this->ScaleExtents(update_extents, scaled_extents, this->Stride);

    int scaled_dims[3];
    this->GetDims(scaled_extents, scaled_dims);

    // Now read rectilinear geometry.
    XdmfGeometry* xmfGeometry = grid->GetGeometry();

    vtkDoubleArray * xarray = vtkDoubleArray::New();
    vtkDoubleArray * yarray = vtkDoubleArray::New();
    vtkDoubleArray * zarray = vtkDoubleArray::New();

    int rgdims[3]={0,0,0};
    if(xmfGeometry->GetGeometryType() ==  XDMF_GEOMETRY_VXVY)
    {
        rgdims[0] = scaled_dims[1];
        rgdims[1] = scaled_dims[2];
        rgdims[2] = 1;
        rg->SetDimensions(rgdims);

        xarray->SetNumberOfTuples(scaled_dims[1]);
        yarray->SetNumberOfTuples(scaled_dims[2]);
        zarray->SetNumberOfTuples(1);
    }
    else if(xmfGeometry->GetGeometryType() ==  XDMF_GEOMETRY_ORIGIN_DXDY)
    {   // scaled_dims seems to contain {1, NX, NY}
        rgdims[0] = scaled_dims[1];
        rgdims[1] = scaled_dims[2];
        rgdims[2] = 1;
        rg->SetDimensions(rgdims);

        xarray->SetNumberOfTuples(scaled_dims[1]);
        yarray->SetNumberOfTuples(scaled_dims[2]);
        zarray->SetNumberOfTuples(1);
    }
    else
    {
        rg->SetDimensions(scaled_dims);
        xarray->SetNumberOfTuples(scaled_dims[0]);
        yarray->SetNumberOfTuples(scaled_dims[1]);
        zarray->SetNumberOfTuples(scaled_dims[2]);
    }

    rg->SetXCoordinates(xarray);
    rg->SetYCoordinates(yarray);
    rg->SetZCoordinates(zarray);

    switch (xmfGeometry->GetGeometryType())
    {
        case XDMF_GEOMETRY_ORIGIN_DXDY:
        {   // scaled_extents seems to contain zmin,zmax,xmin,xmax,ymin,ymax
            XdmfFloat64* origin = xmfGeometry->GetOrigin();
            XdmfFloat64* dxdydz = xmfGeometry->GetDxDyDz();
            for (int cc = scaled_extents[2]; cc <= scaled_extents[3]; cc++) {
                xarray->GetPointer(0)[cc - scaled_extents[2]] = origin[0] + (dxdydz[0] * cc * this->Stride[0]);
            }
            for (int cc = scaled_extents[4]; cc <= scaled_extents[5]; cc++) {
                yarray->GetPointer(0)[cc - scaled_extents[4]] = origin[1] + (dxdydz[1] * cc * this->Stride[1]);
            }
            for (int cc = scaled_extents[0]; cc <= scaled_extents[1]; cc++) {
                zarray->GetPointer(0)[cc - scaled_extents[0]] = origin[2] + (dxdydz[2] * cc * this->Stride[2]);
            }
            break;
        }
        case XDMF_GEOMETRY_ORIGIN_DXDYDZ:
        {
            XdmfFloat64* origin = xmfGeometry->GetOrigin();
            XdmfFloat64* dxdydz = xmfGeometry->GetDxDyDz();
            for (int cc = scaled_extents[0]; cc <= scaled_extents[1]; cc++) {
                xarray->GetPointer(0)[cc - scaled_extents[0]] = origin[0] + (dxdydz[0] * cc * this->Stride[0]);
            }
            for (int cc = scaled_extents[2]; cc <= scaled_extents[3]; cc++) {
                yarray->GetPointer(0)[cc - scaled_extents[2]] = origin[1] + (dxdydz[1] * cc * this->Stride[1]);
            }
            for (int cc = scaled_extents[4]; cc <= scaled_extents[5]; cc++) {
                zarray->GetPointer(0)[cc - scaled_extents[4]] = origin[2] + (dxdydz[2] * cc * this->Stride[2]);
            }
            break;
        }
        case XDMF_GEOMETRY_VXVY:
        {
            zarray->FillComponent(0, 0);
            xmfGeometry->GetVectorX()->GetValues(update_extents[2], xarray->GetPointer(0), scaled_dims[1],
                    this->Stride[1]);
            xmfGeometry->GetVectorY()->GetValues(update_extents[4], yarray->GetPointer(0), scaled_dims[2],
                    this->Stride[2]);
            break;
        }
        case XDMF_GEOMETRY_VXVYVZ:
        {
            xmfGeometry->GetVectorX()->GetValues(update_extents[0], xarray->GetPointer(0), scaled_dims[0],
                    this->Stride[0]);
            xmfGeometry->GetVectorY()->GetValues(update_extents[2], yarray->GetPointer(0), scaled_dims[1],
                    this->Stride[1]);
            xmfGeometry->GetVectorZ()->GetValues(update_extents[4], zarray->GetPointer(0), scaled_dims[2],
                    this->Stride[2]);
            break;
        }
        default:
            EXCEPTION0(InvalidSourceException);
            return NULL;
    }

    xarray->Delete();
    yarray->Delete();
    zarray->Delete();

    return rg;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::ReadStructuredGrid
//
//  Purpose:
//      Constructs a vtkStructuredGrid from an XdmfGrid by reading geometry
//      and connectivity
//
//  Arguments:
//      XdmfGrid * grid --- the grid to read into a vtkStructuredGrid
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
//  Modifications:
//    Brad Whitlock, Thu Apr 29 13:50:05 PST 2010
//    I removed code to add a reference to the points.
//
// ****************************************************************************

vtkStructuredGrid* avtXdmfFileFormat::ReadStructuredGrid(XdmfGrid* grid)
{
    vtkStructuredGrid* sg = vtkStructuredGrid::New();

    int whole_extents[6];
    int update_extents[6];
    this->GetWholeExtent(grid, whole_extents);

    memcpy(update_extents, whole_extents, sizeof(int) * 6);

    int scaled_extents[6];
    this->ScaleExtents(update_extents, scaled_extents, this->Stride);
    sg->SetExtent(scaled_extents);

    vtkPoints * points = vtkPoints::New();
    vtkDoubleArray * array = vtkDoubleArray::New();
    array->SetNumberOfComponents(3);
    points->SetData(array);
    array->Delete();

    int numPoints = grid->GetGeometry()->GetNumberOfPoints();
    //if (update_extents && whole_extents) //TODO: update extents and whole extents are addresses and will always evaluate to true (FIX)!, not sure how to fix so I am leaving the condition blank which is equivalent to always true but without the warning.
    {
        // we are reading a sub-extent.
        int scaled_extents[6];
        int scaled_dims[3];
        this->ScaleExtents(update_extents, scaled_extents, this->Stride);
        this->GetDims(scaled_extents, scaled_dims);
        numPoints = (scaled_dims[0] * scaled_dims[1] * scaled_dims[2]);
    }
    points->SetNumberOfPoints(numPoints);

    XdmfFloat64* tempPoints = new XdmfFloat64[grid->GetGeometry()->GetNumberOfPoints() * 3];
    grid->GetGeometry()->GetPoints()->GetValues(0, tempPoints, grid->GetGeometry()->GetNumberOfPoints() * 3);
    vtkIdType pointId = 0;
    int xdmf_dims[3];
    this->GetDims(whole_extents, xdmf_dims);

    for (int z = update_extents[4]; z <= update_extents[5]; z++) {
        if ((z - update_extents[4]) % this->Stride[2]) {
            continue;
        }

        for (int y = update_extents[2]; y <= update_extents[3]; y++) {
            if ((y - update_extents[2]) % this->Stride[1]) {
                continue;
            }

            for (int x = update_extents[0]; x <= update_extents[1]; x++) {
                if ((x - update_extents[0]) % this->Stride[0]) {
                    continue;
                }

                int xdmf_index[3] = { x, y, z };
                XdmfInt64 offset = vtkStructuredData::ComputePointId(xdmf_dims, xdmf_index);
                points->SetPoint(pointId, tempPoints[3 * offset], tempPoints[3 * offset + 1],
                        tempPoints[3 * offset + 2]);
                pointId++;
            }
        }
    }

    delete[] tempPoints;

    sg->SetPoints(points);
    points->Delete();

    return sg;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::ReadUnstructuredGrid
//
//  Purpose:
//      Constructs a vtkUnstructuredGrid from an XdmfGrid by reading geometry
//      and connectivity
//
//  Arguments:
//      XdmfGrid * grid --- the grid to read into a vtkUnstructuredGrid
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

vtkUnstructuredGrid* avtXdmfFileFormat::ReadUnstructuredGrid(XdmfGrid* grid)
{
    vtkUnstructuredGrid * data = vtkUnstructuredGrid::New();

    if (grid->GetTopology()->GetTopologyType() == XDMF_MIXED) {
        // We have cells with mixed types.
        XdmfInt64 conn_length = grid->GetTopology()->GetConnectivity()->GetNumberOfElements();
        XdmfInt64* xmfConnections = new XdmfInt64[conn_length];
        grid->GetTopology()->GetConnectivity()->GetValues(0, xmfConnections, conn_length);

        vtkIdType numCells = grid->GetTopology()->GetShapeDesc()->GetNumberOfElements();
        int *cell_types = new int[numCells];

        /* Create Cell Array */
        vtkCellArray* cells = vtkCellArray::New();

        /* Get the pointer. Make it Big enough ... too big for now */
        vtkIdType* cells_ptr = cells->WritePointer(numCells, conn_length);

        /* xmfConnections : N p1 p2 ... pN */
        /* i.e. Triangles : 3 0 1 2    3 3 4 5   3 6 7 8 */
        vtkIdType index = 0;
        int sub = 0;
        XdmfTopology top;
        for (vtkIdType cc = 0; cc < numCells; cc++) {
            top.SetTopologyType(xmfConnections[index]);
            int vtk_cell_typeI = this->GetVTKCellType(xmfConnections[index++]);
            XdmfInt32 numPointsPerCell = top.GetNodesPerElement();
            if (numPointsPerCell == -1) {
                // encountered an unknown cell.
                cells->Delete();
                delete[] cell_types;
                delete[] xmfConnections;
                return NULL;
            }

            if (numPointsPerCell == 1) {
                // cell type does not have a fixed number of points in which case the
                // next entry in xmfConnections tells us the number of points.
                numPointsPerCell = xmfConnections[index++];
                sub++; // used to shrink the cells array at the end.
            }

            cell_types[cc] = vtk_cell_typeI;
            *cells_ptr++ = numPointsPerCell;
            for (vtkIdType i = 0; i < numPointsPerCell; i++) {
                *cells_ptr++ = xmfConnections[index++];
            }
        }
        // Resize the Array to the Proper Size
        cells->GetData()->Resize(index - sub);
        data->SetCells(cell_types, cells);
        cells->Delete();
        delete[] cell_types;
        delete[] xmfConnections;
    }
    else {
        XdmfTopology * topology = grid->GetTopology();

        XdmfInt32 topologyType = topology->GetTopologyType();
        int vtkCellType = this->GetVTKCellType(topologyType);

        XdmfGrid * newGrid = NULL;
        if (vtkCellType == VTK_EMPTY_CELL) {
          if(topologyType == XDMF_HEX_64) {
            vtkCellType = VTK_HEXAHEDRON;
            XdmfHex64Generator generator;
            XdmfGrid * newGrid = generator.Split(grid,
                                   NULL,
                                   false);
            topology = newGrid->GetTopology();
          }
          else if(topologyType == XDMF_HEX_125) {
            vtkCellType = VTK_HEXAHEDRON;
            XdmfHex125Generator generator;
            newGrid = generator.Split(grid,
                                   NULL,
                                   false);
            topology = newGrid->GetTopology();
          }
          else {
            EXCEPTION0( InvalidSourceException);
          }
        }

        XdmfInt32 nodesPerElement = topology->GetNodesPerElement();

        vtkIdType numCells = topology->GetNumberOfElements();

        /* Create Cell Array */
        vtkCellArray * cells = vtkCellArray::New();

        /* Get the pointer */
        vtkIdType * cells_ptr = cells->WritePointer(numCells, numCells * (1 + nodesPerElement));

        XdmfInt32 arrayOffset = 0;
        for(vtkIdType i=0; i<numCells; ++i) {
          *cells_ptr++ = nodesPerElement;
          topology->GetConnectivity()->GetValues(arrayOffset,
                                                 cells_ptr,
                                                 nodesPerElement);
          cells_ptr += nodesPerElement;
          arrayOffset += nodesPerElement;
        }
        data->SetCells(vtkCellType, cells);
        cells->Delete();
        delete newGrid;
    }

    // Read the geometry.
    vtkPoints * points = vtkPoints::New();
    points->SetNumberOfPoints(grid->GetGeometry()->GetNumberOfPoints());
    vtkDataArray * array = this->CopyXdmfArrayByPointer(grid->GetGeometry()->GetPoints(), 3);
    points->SetData(array);
    array->Delete();
    data->SetPoints(points);
    points->Delete();
    grid->GetGeometry()->Release();

    return data;
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::GetStructuredGhostZones
//
//  Purpose:
//    Retrieves ghost zone information from the structured mesh and adds it
//    to the dataset.
//
//  Arguments:
//      dims         The dimensions of the mesh.
//      ghostOffsets The ghost offsets of the mesh.
//      ds           The vtkDataSet in which to store the ghost level
//                   information.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2012
//
// ****************************************************************************

void
avtXdmfFileFormat::GetStructuredGhostZones(int dims[3], int ghostOffsets[6],
    vtkDataSet *ds)
{
    unsigned char realVal = 0;
    unsigned char ghostVal = 0;
    avtGhostData::AddGhostZoneType(ghostVal,
        DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    int ncells = ds->GetNumberOfCells();
    vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
    ghostCells->SetName("avtGhostZones");
    ghostCells->SetNumberOfComponents(1);
    ghostCells->SetNumberOfTuples(ncells);

    unsigned char *buf = ghostCells->GetPointer(0);
    for (int i = 0; i < ncells; i++)
        buf[i] = ghostVal;

    int iMin[3], iMax[3];
    for (int i = 0; i < 3; i++)
    {
        iMin[i] = ghostOffsets[i*2];
        iMax[i] = dims[i] - 1 - ghostOffsets[i*2+1];
    }
    for (int k = iMin[2]; k < iMax[2]; k++)
    {
        for (int j = iMin[1]; j < iMax[1]; j++)
        {
            for (int i = iMin[0]; i < iMax[0]; i++)
            {
                int ndx = k * (dims[1]-1) *
                              (dims[0]-1) +
                          j * (dims[0]-1) + i;
                buf[ndx] = realVal;
            }
        }
    }
    ds->GetCellData()->AddArray(ghostCells);
    ghostCells->Delete();

    vtkIntArray *realDims = vtkIntArray::New();
    realDims->SetName("avtRealDims");
    realDims->SetNumberOfValues(6);
    realDims->SetValue(0, iMin[0]);
    realDims->SetValue(1, iMax[0]);
    realDims->SetValue(2, iMin[1]);
    realDims->SetValue(3, iMax[1]);
    realDims->SetValue(4, iMin[2]);
    realDims->SetValue(5, iMax[2]);
    ds->GetFieldData()->AddArray(realDims);
    ds->GetFieldData()->CopyFieldOn("avtRealDims");
    realDims->Delete();

    ds->GetInformation()->Set(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
}

// ****************************************************************************
//  Method: avtXdmfFileFormat::ScaleExtents
//
//  Purpose:
//      Scales extents by stride when reading in structured grids.
//
//  Arguments:
//      int in_exts[6] --- input extents
//      int out_exts[6] --- output extents
//      int stride[3] --- stride
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

void avtXdmfFileFormat::ScaleExtents(int in_exts[6], int out_exts[6], int stride[3])
{
    out_exts[0] = in_exts[0] / stride[0];
    out_exts[1] = in_exts[1] / stride[0];
    out_exts[2] = in_exts[2] / stride[1];
    out_exts[3] = in_exts[3] / stride[1];
    out_exts[4] = in_exts[4] / stride[2];
    out_exts[5] = in_exts[5] / stride[2];
}


// ****************************************************************************
//  Method: avtXdmfFileFormat::SetCurrentGrid
//
//  Purpose:
//      Sets the current grid to the proper XdmfGrid needed to read from.
//
//  Arguments:
//      int timestate --- current time being read.
//      const char * meshname --- XdmfGrid name.
//
//  Programmer: Kenneth Leiter
//  Creation:   March 29, 2010
//
// ****************************************************************************

void avtXdmfFileFormat::SetCurrentGrid(int timestate, const char * meshname)
{
    if (!currentGrid || strcmp(currentGrid->GetName(), meshname) != 0) {
        if (currentGrid != NULL) {
            delete currentGrid;
        }
        currentGrid = new XdmfGrid();
        if (timestate == 0) {
            int id = 1;
            while (true) {
                std::stringstream gridLocation;
                gridLocation << firstGrid << "[" << id << "]";
                XdmfXmlNode gridElement = dom->FindElementByPath(gridLocation.str().c_str());
                if (gridElement == NULL) {
                  // If we can't find a grid, just take the first.
                  currentGrid->SetDOM(dom);
                  currentGrid->SetElement(dom->FindElementByPath(firstGrid.c_str()));
                  currentGrid->UpdateInformation();
                  break;
                }
                currentGrid->SetDOM(dom);
                currentGrid->SetElement(gridElement);
                currentGrid->UpdateInformation();
                if (strcmp(currentGrid->GetName(), meshname) == 0) {
                    break;
                }
                id++;
            }
        }
        else {
            currentGrid = this->GetGrid(timestate);
            currentGrid->UpdateInformation();
        }
    }
}
